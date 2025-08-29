
#include "uci_options.h"
#include "utils.h"
#include <sstream>
#ifdef _WIN32
#include <commctrl.h>

std::vector<UciOption> uci_parse_options(const std::string& log){
    std::vector<UciOption> v;
    std::istringstream iss(log);
    std::string line;
    while (std::getline(iss, line)){
        if (line.rfind("option name ", 0)==0){
            UciOption o;
            std::istringstream ls(line);
            std::string tok; ls >> tok; ls >> tok; // option name
            std::string name;
            while (ls >> tok){
                if (tok=="type") break;
                if (!name.empty()) name.push_back(' ');
                name += tok;
            }
            o.name = name;
            std::string type; ls >> type;
            o.type = type;
            while (ls >> tok){
                if (tok=="min") ls >> o.minv;
                else if (tok=="max") ls >> o.maxv;
                else if (tok=="default") { ls >> o.def; }
                else if (tok=="var"){ std::string vv; ls >> vv; o.vars.push_back(vv); }
            }
            v.push_back(o);
        }
    }
    return v;
}

int show_uci_options_dialog(HWND parent,
                            const std::vector<UciOption>& opts,
                            const std::function<void(const std::string& line)>& send,
                            int& out_depth, int& out_movetime_ms){
    int row=24, x=12, lbl=200;
    int rows = 2 + (int)opts.size();
    int W=420; int H=80 + row*rows;
    HWND dlg = CreateWindowExW(WS_EX_DLGMODALFRAME, L"STATIC", L"UCI Options",
                               WS_POPUP|WS_CAPTION|WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT, W, H,
                               parent, nullptr, GetModuleHandleW(nullptr), nullptr);
    if (!dlg) return 0;
    int y=12;

    CreateWindowW(L"STATIC", L"Analysis depth (0=ignore):", WS_CHILD|WS_VISIBLE, x, y+4, lbl, row, dlg, nullptr, GetModuleHandleW(nullptr), nullptr);
    wchar_t bufD[32]; _snwprintf(bufD, 32, L"%d", out_depth);
    HWND edD = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", bufD, WS_CHILD|WS_VISIBLE|ES_NUMBER, x+lbl+6, y, 80, row, dlg, nullptr, GetModuleHandleW(nullptr), nullptr);
    y += row + 8;
    CreateWindowW(L"STATIC", L"Analysis movetime (ms, 0=ignore):", WS_CHILD|WS_VISIBLE, x, y+4, lbl, row, dlg, nullptr, GetModuleHandleW(nullptr), nullptr);
    wchar_t bufM[32]; _snwprintf(bufM, 32, L"%d", out_movetime_ms);
    HWND edM = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", bufM, WS_CHILD|WS_VISIBLE|ES_NUMBER, x+lbl+6, y, 100, row, dlg, nullptr, GetModuleHandleW(nullptr), nullptr);
    y += row + 12;

    struct Ctrl { const UciOption* opt; HWND hwnd; };
    std::vector<Ctrl> ctrls;
    for (const auto& o : opts){
        if (o.type=="check"){
            HWND cb = CreateWindowW(L"BUTTON", utf8_to_wide(o.name).c_str(),
                                    WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX,
                                    x, y, W-24, row, dlg, nullptr, GetModuleHandleW(nullptr), nullptr);
            if (o.def=="true" || o.def=="1") SendMessageW(cb, BM_SETCHECK, BST_CHECKED, 0);
            ctrls.push_back({&o, cb});
            y += row + 4;
        } else if (o.type=="spin" || o.type=="string"){
            CreateWindowW(L"STATIC", utf8_to_wide(o.name).c_str(), WS_CHILD|WS_VISIBLE,
                          x, y+4, lbl, row, dlg, nullptr, GetModuleHandleW(nullptr), nullptr);
            std::wstring def = utf8_to_wide(o.def);
            DWORD style = WS_CHILD|WS_VISIBLE;
            if (o.type=="spin") style |= ES_NUMBER;
            HWND ed = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", def.c_str(), style,
                                     x+lbl+6, y, 120, row, dlg, nullptr, GetModuleHandleW(nullptr), nullptr);
            ctrls.push_back({&o, ed});
            y += row + 4;
        } else if (o.type=="combo"){
            CreateWindowW(L"STATIC", utf8_to_wide(o.name).c_str(), WS_CHILD|WS_VISIBLE,
                          x, y+4, lbl, row, dlg, nullptr, GetModuleHandleW(nullptr), nullptr);
            HWND cb = CreateWindowW(WC_COMBOBOXW, L"", WS_CHILD|WS_VISIBLE|CBS_DROPDOWNLIST,
                                    x+lbl+6, y, 150, row*5, dlg, nullptr, GetModuleHandleW(nullptr), nullptr);
            int defIdx = 0; int idx = 0;
            for (const auto& v : o.vars){
                std::wstring wv = utf8_to_wide(v);
                SendMessageW(cb, CB_ADDSTRING, 0, (LPARAM)wv.c_str());
                if (v==o.def) defIdx = idx;
                idx++;
            }
            SendMessageW(cb, CB_SETCURSEL, defIdx, 0);
            ctrls.push_back({&o, cb});
            y += row + 4;
        }
    }

    HWND ok = CreateWindowW(L"BUTTON", L"OK", WS_CHILD|WS_VISIBLE|BS_DEFPUSHBUTTON,
                            W-200, H-50, 80, 28, dlg, (HMENU)1, GetModuleHandleW(nullptr), nullptr);
    HWND ca = CreateWindowW(L"BUTTON", L"Cancel", WS_CHILD|WS_VISIBLE,
                            W-110, H-50, 80, 28, dlg, (HMENU)2, GetModuleHandleW(nullptr), nullptr);

    ShowWindow(dlg, SW_SHOW);
    UpdateWindow(dlg);
    int ret = 0; bool loop=true;
    while (loop){
        MSG msg; if (!GetMessageW(&msg, nullptr, 0, 0)) break;
        if (msg.message==WM_KEYDOWN && msg.wParam==VK_ESCAPE){ ret=0; loop=false; }
        if (msg.message==WM_COMMAND){
            if ((HWND)msg.lParam == ok){ ret=1; loop=false; }
            if ((HWND)msg.lParam == ca){ ret=0; loop=false; }
        }
        if (!IsDialogMessageW(dlg, &msg)){
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }
    if (ret){
        wchar_t b1[64], b2[64];
        GetWindowTextW(edD, b1, 64); GetWindowTextW(edM, b2, 64);
        out_depth = _wtoi(b1); out_movetime_ms = _wtoi(b2);
        for (auto& c : ctrls){
            const UciOption& o = *c.opt;
            if (o.type=="check"){
                bool on = SendMessageW(c.hwnd, BM_GETCHECK,0,0)==BST_CHECKED;
                send("setoption name " + o.name + " value " + (on?"true":"false"));
            } else if (o.type=="spin" || o.type=="string"){
                wchar_t buf[256]; GetWindowTextW(c.hwnd, buf, 256);
                send("setoption name " + o.name + " value " + wide_to_utf8(buf));
            } else if (o.type=="combo"){
                int sel = (int)SendMessageW(c.hwnd, CB_GETCURSEL,0,0);
                std::string val = (sel>=0 && sel<(int)o.vars.size())? o.vars[sel] : "";
                send("setoption name " + o.name + " value " + val);
            }
        }
    }
    DestroyWindow(dlg);
    return ret;
}

#else

std::vector<UciOption> uci_parse_options(const std::string&){ return {}; }
int show_uci_options_dialog(HWND, const std::vector<UciOption>&,
                            const std::function<void(const std::string& line)>&,
                            int& out_depth, int& out_movetime_ms){
    (void)out_depth; (void)out_movetime_ms; return 0;
}

#endif
