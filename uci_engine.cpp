
#include "uci_engine.h"
#include <vector>
#include <algorithm>
#ifndef _WIN32
#include <filesystem>
#ifdef QT_CORE_LIB
#include <QString>
#include <QObject>
#include <QByteArray>
#else
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#endif
#endif

#ifdef _WIN32

UCIEngine::UCIEngine(){}
UCIEngine::~UCIEngine(){ stop(); }

void UCIEngine::set_callback(Callback cb){ callback_ = std::move(cb); }

bool UCIEngine::start(const std::wstring& path){
    stop();
    SECURITY_ATTRIBUTES sa{}; sa.nLength=sizeof(sa); sa.bInheritHandle=TRUE;
    HANDLE outRd, outWr, inRd, inWr;
    if (!CreatePipe(&outRd, &outWr, &sa, 0)) return false;
    if (!SetHandleInformation(outRd, HANDLE_FLAG_INHERIT, 0)) return false;
    if (!CreatePipe(&inRd, &inWr, &sa, 0)) return false;
    if (!SetHandleInformation(inWr, HANDLE_FLAG_INHERIT, 0)) return false;

    STARTUPINFOW si{}; si.cb = sizeof(si);
    si.dwFlags |= STARTF_USESTDHANDLES;
    si.hStdInput  = inRd;
    si.hStdOutput = outWr;
    si.hStdError  = outWr;
    PROCESS_INFORMATION pi{};

    std::wstring cmd = L"\"" + path + L"\"";
    DWORD flags = CREATE_NEW_PROCESS_GROUP;
#ifdef _WIN32
    flags |= CREATE_NO_WINDOW; // avoid spawning a console window
#endif
    if (!CreateProcessW(nullptr, cmd.data(), nullptr, nullptr, TRUE, flags, nullptr, nullptr, &si, &pi)){
        CloseHandle(outRd); CloseHandle(outWr); CloseHandle(inRd); CloseHandle(inWr);
        return false;
    }
    CloseHandle(outWr); CloseHandle(inRd);
    hStdOutRd_ = outRd; hStdInWr_ = inWr;
    hProcess_ = pi.hProcess; hThread_ = pi.hThread;
    running_.store(true);
    reader_ = std::thread(&UCIEngine::reader_loop, this);
    return true;
}

void UCIEngine::stop(){
    if (!running_.load()) return;
    running_.store(false);
    if (hStdInWr_) { CloseHandle(hStdInWr_); hStdInWr_=nullptr; }
    if (hStdOutRd_){ CloseHandle(hStdOutRd_); hStdOutRd_=nullptr; }
    if (hProcess_){
        TerminateProcess(hProcess_, 0);
        CloseHandle(hProcess_); hProcess_=nullptr;
    }
    if (hThread_){ CloseHandle(hThread_); hThread_=nullptr; }
    if (reader_.joinable()) reader_.join();
}

bool UCIEngine::send_line(const std::string& s){
    if (!running_.load() || !hStdInWr_) return false;
    std::string line = s; line.push_back('\n');
    DWORD wrote=0; return WriteFile(hStdInWr_, line.data(), (DWORD)line.size(), &wrote, nullptr);
}

void UCIEngine::reader_loop(){
    char buf[4096];
    while (running_.load()){
        DWORD avail=0;
        if (!PeekNamedPipe(hStdOutRd_, nullptr, 0, nullptr, &avail, nullptr)){
            Sleep(10); continue;
        }
        if (avail==0){ Sleep(5); continue; }
        DWORD rd=0;
        if (!ReadFile(hStdOutRd_, buf, std::min<DWORD>(avail, sizeof(buf)-1), &rd, nullptr)) break;
        if (rd>0){
            buf[rd]=0;
            if (callback_) callback_(std::string(buf, rd));
        }
    }
}

#else

UCIEngine::UCIEngine(){}
UCIEngine::~UCIEngine(){ stop(); }

void UCIEngine::set_callback(Callback cb){ callback_ = std::move(cb); }

#ifdef QT_CORE_LIB

bool UCIEngine::start(const std::wstring& path){
    stop();
    process_ = new QProcess();
    QObject::connect(process_, &QProcess::readyReadStandardOutput, [this](){
        if (callback_) {
            callback_(process_->readAllStandardOutput().toStdString());
        }
    });
    QObject::connect(process_, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [this](int, QProcess::ExitStatus){
        running_.store(false);
    });
    process_->start(QString::fromStdWString(path));
    running_.store(process_->waitForStarted());
    return running_.load();
}

void UCIEngine::stop(){
    if (!running_.load()) return;
    running_.store(false);
    if (process_){
        process_->kill();
        process_->waitForFinished();
        delete process_;
        process_ = nullptr;
    }
    if (reader_.joinable()) reader_.join();
}

bool UCIEngine::send_line(const std::string& s){
    if (!running_.load() || !process_) return false;
    QByteArray line = QByteArray::fromStdString(s + "\n");
    return process_->write(line) == line.size();
}

void UCIEngine::reader_loop(){ }

#else

bool UCIEngine::start(const std::wstring& path){
    stop();
    int inPipe[2];
    int outPipe[2];
    if (pipe(inPipe) < 0) return false;
    if (pipe(outPipe) < 0){ close(inPipe[0]); close(inPipe[1]); return false; }

    pid_t pid = fork();
    if (pid == 0){
        // child
        dup2(inPipe[0], STDIN_FILENO);
        dup2(outPipe[1], STDOUT_FILENO);
        dup2(outPipe[1], STDERR_FILENO);
        close(inPipe[0]); close(inPipe[1]);
        close(outPipe[0]); close(outPipe[1]);
        std::string exe = std::filesystem::path(path).string();
        execl(exe.c_str(), exe.c_str(), (char*)nullptr);
        _exit(1);
    }
    if (pid < 0){
        close(inPipe[0]); close(inPipe[1]);
        close(outPipe[0]); close(outPipe[1]);
        return false;
    }

    // parent
    close(inPipe[0]);
    close(outPipe[1]);
    fdInWr_ = inPipe[1];
    fdOutRd_ = outPipe[0];
    pid_ = pid;
    running_.store(true);
    reader_ = std::thread(&UCIEngine::reader_loop, this);
    return true;
}

void UCIEngine::stop(){
    if (!running_.load()) return;
    running_.store(false);
    if (fdInWr_ != -1){ close(fdInWr_); fdInWr_ = -1; }
    if (fdOutRd_ != -1){ close(fdOutRd_); fdOutRd_ = -1; }
    if (pid_ > 0){
        kill(pid_, SIGTERM);
        waitpid(pid_, nullptr, 0);
        pid_ = -1;
    }
    if (reader_.joinable()) reader_.join();
}

bool UCIEngine::send_line(const std::string& s){
    if (!running_.load() || fdInWr_ == -1) return false;
    std::string line = s; line.push_back('\n');
    return write(fdInWr_, line.data(), line.size()) == (ssize_t)line.size();
}

void UCIEngine::reader_loop(){
    char buf[4096];
    while (running_.load()){
        ssize_t rd = read(fdOutRd_, buf, sizeof(buf));
        if (rd > 0){
            if (callback_) callback_(std::string(buf, rd));
        } else if (rd == 0){
            break; // EOF
        } else {
            if (errno == EINTR) continue;
            break;
        }
    }
}

#endif

#endif
