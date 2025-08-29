
#include "uci_options.h"
#include "utils.h"
#include <sstream>
#ifdef _WIN32

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
    const int W=420, H=180;
    HWND dlg = CreateWindowExW(WS_EX_DLGMODALFRAME, L"STATIC", L"UCI Options",
                               WS_POPUP|WS_CAPTION|WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT, W, H,
                               parent, nullptr, GetModuleHandleW(nullptr), nullptr);
    if (!dlg) return 0;
    int x=12, y=12, lbl=200, row=24;

    CreateWindowW(L"STATIC", L"Analysis depth (0=ignore):", WS_CHILD|WS_VISIBLE, x, y+4, lbl, row, dlg, nullptr, GetModuleHandleW(nullptr), nullptr);
    wchar_t bufD[32]; _snwprintf(bufD, 32, L"%d", out_depth);
    HWND edD = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", bufD, WS_CHILD|WS_VISIBLE|ES_NUMBER, x+lbl+6, y, 80, row, dlg, nullptr, GetModuleHandleW(nullptr), nullptr);
    y += row + 8;
    CreateWindowW(L"STATIC", L"Analysis movetime (ms, 0=ignore):", WS_CHILD|WS_VISIBLE, x, y+4, lbl, row, dlg, nullptr, GetModuleHandleW(nullptr), nullptr);
    wchar_t bufM[32]; _snwprintf(bufM, 32, L"%d", out_movetime_ms);
    HWND edM = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", bufM, WS_CHILD|WS_VISIBLE|ES_NUMBER, x+lbl+6, y, 100, row, dlg, nullptr, GetModuleHandleW(nullptr), nullptr);
    y += row + 12;

    HWND ok = CreateWindowW(L"BUTTON", L"OK", WS_CHILD|WS_VISIBLE|BS_DEFPUSHBUTTON, W-200, H-50, 80, 28, dlg, (HMENU)1, GetModuleHandleW(nullptr), nullptr);
    HWND ca = CreateWindowW(L"BUTTON", L"Cancel", WS_CHILD|WS_VISIBLE, W-110, H-50, 80, 28, dlg, (HMENU)2, GetModuleHandleW(nullptr), nullptr);

    ShowWindow(dlg, SW_SHOW);
    UpdateWindow(dlg);
    int ret = 0;
    bool loop=true;
    while (loop){
        MSG msg;
        if (!GetMessageW(&msg, nullptr, 0, 0)) break;
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
        GetWindowTextW(edD, b1, 64);
        GetWindowTextW(edM, b2, 64);
        out_depth = _wtoi(b1);
        out_movetime_ms = _wtoi(b2);
        for (auto& o: opts){
            if (o.type=="check"){
                std::string v = o.def.empty()? "false" : o.def;
                send("setoption name " + o.name + " value " + v);
            } else if (o.type=="spin"){
                std::string v = o.def.empty()? "0" : o.def;
                send("setoption name " + o.name + " value " + v);
            } else if (o.type=="string"){
                std::string v = o.def;
                send("setoption name " + o.name + " value " + v);
            } else if (o.type=="combo"){
                std::string v = o.def.empty()? (o.vars.empty()? "" : o.vars.front()) : o.def;
                send("setoption name " + o.name + " value " + v);
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
