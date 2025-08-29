#pragma once
#include <string>
#include <thread>
#include <atomic>
#include <functional>
#ifdef _WIN32
#include <windows.h>
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
#endif
    std::thread reader_;
    std::atomic<bool> running_{false};
    Callback callback_ = nullptr;

    void reader_loop();
};
