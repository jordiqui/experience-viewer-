#pragma once
#include <string>
#include <thread>
#include <atomic>
#include <functional>
#ifdef _WIN32
#include <windows.h>
#else
#ifdef QT_CORE_LIB
#include <QProcess>
#else
#include <sys/types.h>
#endif
#endif

class UCIEngine {
public:
    using Callback = std::function<void(const std::string&)>;

    UCIEngine();
    ~UCIEngine();

    bool start(const std::wstring& path);
    void stop();

    bool send_line(const std::string& s);
    void set_callback(Callback cb);

    bool running() const { return running_.load(); }

private:
#ifdef _WIN32
    HANDLE hProcess_ = nullptr;
    HANDLE hThread_ = nullptr;
    HANDLE hStdInWr_ = nullptr;
    HANDLE hStdOutRd_ = nullptr;
#elif defined(QT_CORE_LIB)
    QProcess* process_ = nullptr;
#else
    pid_t pid_ = -1;
    int fdInWr_ = -1;
    int fdOutRd_ = -1;
#endif
    std::thread reader_;
    std::atomic<bool> running_{false};
    Callback callback_ = nullptr;

    void reader_loop();
};
