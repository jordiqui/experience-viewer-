#pragma once
#include <string>
#include <thread>
#include <atomic>
#include <functional>
#include <windows.h>

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
    HANDLE hProcess_ = nullptr;
    HANDLE hThread_ = nullptr;
    HANDLE hStdInWr_ = nullptr;
    HANDLE hStdOutRd_ = nullptr;
    std::thread reader_;
    std::atomic<bool> running_{false};
    Callback callback_ = nullptr;

    void reader_loop();
};
