
#include "uci_engine.h"
#include <vector>
#include <algorithm>

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
    if (!CreateProcessW(nullptr, cmd.data(), nullptr, nullptr, TRUE, CREATE_NEW_PROCESS_GROUP, nullptr, nullptr, &si, &pi)){
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
