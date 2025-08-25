
#pragma once
#include <string>
#include <vector>
#include <functional>
#include <windows.h>

struct UciOption{
    std::string name;
    std::string type;   // check, spin, combo, string, button
    int minv=0, maxv=0; // for spin
    std::string def;    // default
    std::vector<std::string> vars; // for combo
};

std::vector<UciOption> uci_parse_options(const std::string& engine_log);

int show_uci_options_dialog(HWND parent,
                            const std::vector<UciOption>& opts,
                            const std::function<void(const std::string& line)>& send,
                            int& out_depth, int& out_movetime_ms);
