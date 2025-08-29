
#include "gui.h"
#ifdef _WIN32
#include "utils.h"
#include "board.h"
#include "uci_options.h"
#include "app.h"
#include <commctrl.h>
#include <windowsx.h>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <map>
#include <cctype>

#pragma comment(lib, "comctl32.lib")

static const wchar_t* APP_CLASS = L"ExpViewerWndClass";
static const int ID_LIST = 1001;
static const int ID_MOVES = 1004;
static const int ID_BOARD = 1005;
static const int ID_STATUS = 1003;

enum {
    IDM_FILE_OPEN_EXP = 2001,
    IDM_FILE_OPEN_PGN,
    IDM_FILE_RELOAD_EXP,
    IDM_FILE_SAVE_EXP,
    IDM_FILE_EXIT,
    IDM_UCI_LOAD,
    IDM_UCI_CONFIGURE,
    IDM_NAV_PREV,
    IDM_NAV_NEXT,
    IDM_ANALYSE_CURRENT,
    IDM_ANALYSE_GAME
};
static AppState g_app;
static HWND g_hList = nullptr;   // PGN list
static HWND g_hMoves = nullptr;  // Moves list (right bottom)
static HWND g_hBoard = nullptr;  // Board (right top)
static HWND g_hStatus = nullptr;

static void set_status(const std::wstring& s) { SendMessageW(g_hStatus, SB_SETTEXT, 0, (LPARAM)s.c_str()); }

static HMENU build_menu(){
    HMENU hMenu = CreateMenu();
    HMENU hFile = CreatePopupMenu();
    AppendMenuW(hFile, MF_STRING, IDM_FILE_OPEN_EXP, L"&Open EXP...");
    AppendMenuW(hFile, MF_STRING, IDM_FILE_OPEN_PGN, L"Open &PGN...");
    AppendMenuW(hFile, MF_STRING, IDM_FILE_RELOAD_EXP, L"&Reload EXP");
    AppendMenuW(hFile, MF_STRING, IDM_FILE_SAVE_EXP, L"&Save EXP");
    AppendMenuW(hFile, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(hFile, MF_STRING, IDM_FILE_EXIT, L"E&xit");

    HMENU hUci = CreatePopupMenu();
    AppendMenuW(hUci, MF_STRING, IDM_UCI_LOAD, L"&Load Engine...");
    AppendMenuW(hUci, MF_STRING, IDM_UCI_CONFIGURE, L"&Configure Options…");

    HMENU hNav = CreatePopupMenu();
    AppendMenuW(hNav, MF_STRING, IDM_NAV_PREV, L"&Prev Move\tLeft");
    AppendMenuW(hNav, MF_STRING, IDM_NAV_NEXT, L"&Next Move\tRight");

    HMENU hAn = CreatePopupMenu();
    AppendMenuW(hAn, MF_STRING, IDM_ANALYSE_CURRENT, L"Analyse &Current Move");
    AppendMenuW(hAn, MF_STRING, IDM_ANALYSE_GAME, L"Analyse &Whole Game");

    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hFile, L"&File");
    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hUci,  L"&UCI");
    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hNav,  L"&Navigate");
    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hAn,   L"&Analysis");
    return hMenu;
}

static void init_list_columns(){
    ListView_SetExtendedListViewStyle(g_hList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
    LVCOLUMNW col{};
    col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
    const wchar_t* headers[] = { L"Match", L"Event", L"Date" };
    int widths[] = { 260, 140, 100 };
    for (int i=0;i<3;i++) {
        col.pszText = const_cast<LPWSTR>(headers[i]);
        col.cx = widths[i];
        col.iSubItem = i;
        ListView_InsertColumn(g_hList, i, &col);
    }
}
static void init_moves_columns(){
    ListView_SetExtendedListViewStyle(g_hMoves, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
    LVCOLUMNW col{};
    col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
    const wchar_t* headers[] = { L"#", L"Move", L"EXP cp", L"Depth", L"Quality", L"Engine eval" };
    int widths[] = { 40, 120, 80, 60, 70, 100 };
    for (int i=0;i<6;i++) {
        col.pszText = const_cast<LPWSTR>(headers[i]);
        col.cx = widths[i];
        col.iSubItem = i;
        ListView_InsertColumn(g_hMoves, i, &col);
    }
}

static void layout_children(HWND hWnd){
    RECT rc; GetClientRect(hWnd,&rc);
    int statusH = 22;
    int leftW = (rc.right - rc.left)/2 - 20;
    MoveWindow(g_hStatus, rc.left, rc.bottom - statusH, rc.right-rc.left, statusH, TRUE);

    // Left: PGN list
    MoveWindow(g_hList, rc.left, rc.top, leftW, rc.bottom - rc.top - statusH, TRUE);

    // Right: Board (top), Moves (bottom)
    int rightX = rc.left + leftW + 8;
    int rightW = rc.right - rightX;
    int boardH = (rc.bottom - rc.top - statusH) * 2 / 3;
    MoveWindow(g_hBoard, rightX, rc.top, rightW, boardH, TRUE);
    MoveWindow(g_hMoves, rightX, rc.top + boardH + 8, rightW, rc.bottom - rc.top - statusH - boardH - 8, TRUE);
}

static bool is_uci_token(const std::string& t){
    if (t.size()<4) return false;
    auto f=[&](char c){ return c>='a'&&c<='h'; };
    auto r=[&](char c){ return c>='1'&&c<='8'; };
    if (!f(t[0])||!r(t[1])||!f(t[2])||!r(t[3])) return false;
    if (t.size()==5){
        char p=t[4]; std::string v="qrbn";
        if (v.find(tolower(p))==std::string::npos) return false;
    }
    return true;
}

static void rebuild_board_to_ply(){
    g_app.board.set_startpos();
    for (int i=0;i<g_app.current_ply && i<(int)g_app.current_moves.size(); ++i){
        auto& r = g_app.current_moves[i];
        std::string uci = r.token;
        if (!is_uci_token(uci)){
            uci = board_san_to_uci(r.token);
        }
        if (!uci.empty()) g_app.board.apply_uci(uci);
    }
    board_set_position(g_app.board.st);
}

static void on_next(){
    if (g_app.current_ply < (int)g_app.current_moves.size()){
        ++g_app.current_ply;
        rebuild_board_to_ply();
    }
}
static void on_prev(){
    if (g_app.current_ply > 0){
        --g_app.current_ply;
        rebuild_board_to_ply();
    }
}

static void fill_moves_for_game(int gameIndex){
    ListView_DeleteAllItems(g_hMoves);
    g_app.current_moves.clear();
    g_app.current_ply = 0;
    g_app.board.set_startpos();
    board_set_position(g_app.board.st);

    if (gameIndex < 0 || gameIndex >= (int)g_app.pgn_db.games.size()) return;
    auto& G = g_app.pgn_db.games[gameIndex];

    std::istringstream ss(G.moves_san);
    std::string tok;
    int ply=1;
    while (ss >> tok){
        if (!tok.empty() && (isdigit((unsigned char)tok[0]) || tok=="1-0"||tok=="0-1"||tok=="1/2-1/2"||tok=="*")) continue;

        MoveRow row{};
        row.token = tok;
        // EXP cross if UCI (best-effort)
        if (is_uci_token(tok)){
            for (auto& e : g_app.exp_db.items){
                if (e.key == tok){
                    row.exp_cp = (int)(e.score*100.0);
                    break;
                }
            }
        }

        LVITEMW item{}; item.mask = LVIF_TEXT; item.iItem = ply-1;
        std::wstring wply = std::to_wstring(ply);
        item.pszText = const_cast<LPWSTR>(wply.c_str());
        int rowidx = ListView_InsertItem(g_hMoves, &item);

        {
            std::wstring wtok = utf8_to_wide(tok);
            ListView_SetItemText(g_hMoves, rowidx, 1, const_cast<LPWSTR>(wtok.c_str()));
        }
        {
            // Show exp cp as +0.42 format
            double v = row.exp_cp/100.0;
            std::wstringstream ws; ws.setf(std::ios::fixed); ws.precision(2);
            if (v>=0) ws<<L'+'; ws<<v;
            auto s = ws.str();
            ListView_SetItemText(g_hMoves, rowidx, 2, const_cast<LPWSTR>(s.data()));
        }
        ListView_SetItemText(g_hMoves, rowidx, 3, L"");
        ListView_SetItemText(g_hMoves, rowidx, 4, L"");
        ListView_SetItemText(g_hMoves, rowidx, 5, L"");

        g_app.current_moves.push_back(row);
        ++ply;
    }
    set_status(L"Moves loaded: " + std::to_wstring(g_app.current_moves.size()));
}

static void on_open_pgn(){
    auto path = open_file_dialog(L"PGN Files\0*.pgn\0All Files\0*.*\0\0", L"Open PGN File");
    if (path.empty()) return;
    if (!g_app.pgn_db.load(path)) { show_message(L"Failed to load PGN file.", L"Error", MB_OK|MB_ICONERROR); return; }
    g_app.current_pgn_path = path;

    // fill PGN list
    ListView_DeleteAllItems(g_hList);
    int idx = 0;
    for (const auto& g : g_app.pgn_db.games) {
        LVITEMW item{};
        item.mask = LVIF_TEXT;
        item.iItem = idx;
        std::ostringstream oss;
        auto itW = g.tags.find("White");
        auto itB = g.tags.find("Black");
        auto itR = g.tags.find("Result");
        oss << (itW!=g.tags.end()? itW->second : "White")
            << " - "
            << (itB!=g.tags.end()? itB->second : "Black")
            << "  ["
            << (itR!=g.tags.end()? itR->second : "*")
            << "]";
        auto ws = utf8_to_wide(oss.str());
        item.pszText = const_cast<wchar_t*>(ws.c_str());
        int row = ListView_InsertItem(g_hList, &item);
        auto itE = g.tags.find("Event");
        auto itD = g.tags.find("Date");
        ListView_SetItemText(g_hList, row, 1, const_cast<wchar_t*>(utf8_to_wide(itE!=g.tags.end()? itE->second : "").c_str()));
        ListView_SetItemText(g_hList, row, 2, const_cast<wchar_t*>(utf8_to_wide(itD!=g.tags.end()? itD->second : "").c_str()));
        ++idx;
    }
    set_status(L"PGN loaded: " + std::to_wstring(g_app.pgn_db.games.size()) + L" games.");
}

static std::vector<std::string> split_moves(const std::string& s){
    std::vector<std::string> out;
    std::string cur;
    for(char c : s){
        if (isspace((unsigned char)c)){
            if(!cur.empty()){ out.push_back(cur); cur.clear(); }
        } else cur.push_back(c);
    }
    if(!cur.empty()) out.push_back(cur);
    return out;
}

static void populate_exp_tree(HWND hTree){
    TreeView_DeleteAllItems(hTree);
    TVINSERTSTRUCTW rootIns{};
    rootIns.hParent = TVI_ROOT;
    rootIns.hInsertAfter = TVI_LAST;
    rootIns.item.mask = TVIF_TEXT;
    rootIns.item.pszText = const_cast<LPWSTR>(L"Start");
    HTREEITEM root = TreeView_InsertItem(hTree, &rootIns);

    std::map<std::wstring, HTREEITEM> nodes;
    nodes[L""] = root;

    for(const auto& e : g_app.exp_db.items){
        auto moves = split_moves(e.key);
        if(moves.empty()) continue;
        std::wstring prefix;
        for(size_t i=0;i<moves.size();++i){
            if(i>0) prefix += L' ';
            prefix += utf8_to_wide(moves[i]);
            if(!nodes.count(prefix)){
                std::wstring parentPrefix = prefix.substr(0, prefix.find_last_of(L' '));
                HTREEITEM parent = nodes[parentPrefix];
                std::wstring text = utf8_to_wide(moves[i]);
                TVINSERTSTRUCTW ins{};
                ins.hParent = parent;
                ins.hInsertAfter = TVI_LAST;
                ins.item.mask = TVIF_TEXT;
                ins.item.pszText = const_cast<LPWSTR>(text.c_str());
                HTREEITEM hItem = TreeView_InsertItem(hTree, &ins);
                nodes[prefix] = hItem;
            }
        }
        HTREEITEM hItem = nodes[prefix];
        std::wstringstream ws; ws.setf(std::ios::fixed); ws.precision(2);
        ws << utf8_to_wide(moves.back()) << L" (" << e.count << L", " << e.score << L")";
        std::wstring text = ws.str();
        TVITEMW tvi{}; tvi.mask = TVIF_TEXT; tvi.hItem = hItem; tvi.pszText = const_cast<LPWSTR>(text.c_str());
        TreeView_SetItem(hTree, &tvi);
    }
    TreeView_Expand(hTree, root, TVE_EXPAND);
}

static LRESULT CALLBACK ExpTreeWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam){
    static HWND hTree = nullptr;
    switch(msg){
        case WM_CREATE:
            hTree = CreateWindowExW(0, WC_TREEVIEWW, L"", WS_CHILD|WS_VISIBLE|TVS_HASLINES|TVS_LINESATROOT|TVS_HASBUTTONS,
                                   0,0,0,0, hWnd, (HMENU)1, GetModuleHandleW(nullptr), nullptr);
            populate_exp_tree(hTree);
            break;
        case WM_SIZE:
            MoveWindow(hTree, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
            break;
        default:
            return DefWindowProcW(hWnd, msg, wParam, lParam);
    }
    return 0;
}

static void show_exp_tree_window(){
    WNDCLASSW wc{};
    wc.lpszClassName = L"ExpTreeWndClass";
    wc.lpfnWndProc = ExpTreeWndProc;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hInstance = GetModuleHandleW(nullptr);
    static bool registered = false;
    if(!registered){
        RegisterClassW(&wc);
        registered = true;
    }
    CreateWindowExW(0, wc.lpszClassName, L"EXP Opening Tree", WS_OVERLAPPEDWINDOW|WS_VISIBLE,
                    CW_USEDEFAULT, CW_USEDEFAULT, 400, 600,
                    nullptr, nullptr, wc.hInstance, nullptr);
}

static void on_open_exp(){
    auto path = open_file_dialog(L"EXP Files\0*.exp\0All Files\0*.*\0\0", L"Open Experience File");
    if (path.empty()) return;
    if (!g_app.exp_db.load(path)) { show_message(L"Failed to load EXP file.", L"Error", MB_OK|MB_ICONERROR); return; }
    g_app.current_exp_path = path;
    auto st = g_app.exp_db.compute_stats();
    std::wstringstream ws; ws.setf(std::ios::fixed); ws.precision(2);
    ws << L"EXP loaded: " << st.entries << L" entries, avg score " << st.avg_score;
    set_status(ws.str());
    show_exp_tree_window();
}

static void on_load_engine(){
    auto path = open_file_dialog(L"Engines\0*.exe\0All Files\0*.*\0\0", L"Load UCI Engine");
    if (path.empty()) return;
    if (!g_app.engine.start(path)) { show_message(L"Could not start engine process.", L"UCI", MB_OK|MB_ICONERROR); return; }
    g_app.engine_path = path;
    g_app.clear_engine_log();
    g_app.engine.set_callback([](const std::string& s){ g_app.append_engine_log(s); });
    set_status(L"Engine loaded.");
}

static void on_configure_engine(HWND hWnd){
    if (!g_app.engine.running()){ show_message(L"No engine running.", L"UCI", MB_OK|MB_ICONWARNING); return; }
    g_app.clear_engine_log();
    g_app.engine.send_line("uci");
    Sleep(250);
    auto opts = uci_parse_options(g_app.engine_log);
    int depth=g_app.analysis_depth, mt=g_app.analysis_movetime;
    int ok = show_uci_options_dialog(hWnd, opts, [](const std::string& line){ g_app.engine.send_line(line); }, depth, mt);
    if (ok){
        g_app.analysis_depth = depth;
        g_app.analysis_movetime = mt;
    }
}

static std::string build_position_cmd(int upto_ply){
    std::ostringstream oss;
    oss << "position startpos";
    if (upto_ply>0){
        oss << " moves";
        int N = std::min(upto_ply, (int)g_app.current_moves.size());
        // rebuild moves as UCI best-effort
        BoardState tmp; tmp.set_startpos();
        for (int i=0; i<N; ++i){
            std::string m = g_app.current_moves[i].token;
            if (!is_uci_token(m)) m = board_san_to_uci(m);
            if (!m.empty()){
                oss << " " << m;
                tmp.apply_uci(m);
            }
        }
    }
    return oss.str();
}

static std::wstring cp_to_human_w(int cp){
    std::wstringstream ws; ws.setf(std::ios::fixed); ws.precision(2);
    double v = cp/100.0;
    if (v>=0) ws<<L'+'; ws<<v;
    return ws.str();
}

static void analyse_at_ply(int ply){
    if (!g_app.engine.running()){ show_message(L"No engine running.", L"UCI", MB_OK|MB_ICONWARNING); return; }
    if (ply<1 || ply>(int)g_app.current_moves.size()) return;
    int row = ply-1;
    // Build position and go
    auto pos = build_position_cmd(row);
    g_app.engine.send_line(pos);
    if (g_app.analysis_movetime>0){
        g_app.engine.send_line("go movetime " + std::to_string(g_app.analysis_movetime));
    } else {
        int d = g_app.analysis_depth>0? g_app.analysis_depth : 12;
        g_app.engine.send_line("go depth " + std::to_string(d));
    }
    // Capture output for a short time window and pick last "score cp" or "mate"
    Sleep(100 + (g_app.analysis_movetime>0? g_app.analysis_movetime : 1000));
    std::string log = g_app.engine_log;
    int best_cp = 0; bool have=false; bool mate=false; int mateN=0;
    std::istringstream iss(log);
    std::string line;
    while (std::getline(iss,line)){
        if (line.find(" score ")!=std::string::npos){
            // parse cp or mate
            auto p = line.find(" score ");
            auto rest = line.substr(p+7);
            if (rest.rfind("cp ",0)==0){
                int cp=0;
                try{ cp = std::stoi(rest.substr(3)); }catch(...){}
                best_cp = cp; have=true; mate=false;
            } else if (rest.rfind("mate ",0)==0){
                int m=0; try{ m=std::stoi(rest.substr(5)); }catch(...){}
                mate=true; mateN=m; have=true;
            }
        }
    }
    if (have){
        std::wstring w;
        if (mate){
            w = (mateN>0? L"#"+std::to_wstring(mateN) : L"#-"+std::to_wstring(-mateN));
        } else {
            w = cp_to_human_w(best_cp);
        }
        ListView_SetItemText(g_hMoves, row, 5, const_cast<LPWSTR>(w.c_str()));
    }
}

static void analyse_whole_game(){
    for (int ply=1; ply <= (int)g_app.current_moves.size(); ++ply){
        analyse_at_ply(ply);
    }
}

static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam){
    switch(msg){
        case WM_CREATE:{
            InitCommonControls();
            g_hList = CreateWindowW(WC_LISTVIEWW, L"", WS_CHILD|WS_VISIBLE|LVS_REPORT, 0,0,0,0, hWnd, (HMENU)ID_LIST, GetModuleHandleW(nullptr), nullptr);
            g_hMoves= CreateWindowW(WC_LISTVIEWW, L"", WS_CHILD|WS_VISIBLE|LVS_REPORT, 0,0,0,0, hWnd, (HMENU)ID_MOVES, GetModuleHandleW(nullptr), nullptr);
            board_register(GetModuleHandleW(nullptr));
            g_hBoard= board_create(hWnd, ID_BOARD);
            g_hStatus= CreateWindowW(STATUSCLASSNAMEW, L"", WS_CHILD|WS_VISIBLE, 0,0,0,0, hWnd, (HMENU)ID_STATUS, GetModuleHandleW(nullptr), nullptr);
            init_list_columns();
            init_moves_columns();
            // assets dir
            wchar_t mod[MAX_PATH]; GetModuleFileNameW(nullptr, mod, MAX_PATH);
            std::wstring dir(mod); auto pos = dir.find_last_of(L"\\/"); if (pos!=std::wstring::npos) dir.erase(pos);
            board_set_assets_dir(dir + L"\\assets");
            // Verify assets exist
            std::wstring probe = dir + L"\\assets\\w_p.png"; // Verify assets exist
            if (GetFileAttributesW(probe.c_str()) == INVALID_FILE_ATTRIBUTES) {
                show_message(L"No se encuentran los sprites en 'assets'. Asegúrate de que existan w_p.png ... b_k.png.", L"Sprites no encontrados", MB_OK|MB_ICONWARNING);
            }
            // initial board
            g_app.board.set_startpos();
            board_set_position(g_app.board.st);
            set_status(L"Ready.");
            break;
        }
        case WM_SIZE: layout_children(hWnd); break;
        case WM_COMMAND:{
            switch(LOWORD(wParam)){
                case IDM_FILE_OPEN_PGN: on_open_pgn(); break;
                case IDM_FILE_OPEN_EXP: on_open_exp(); break;
                case IDM_FILE_RELOAD_EXP:
                    if (!g_app.current_exp_path.empty() && g_app.exp_db.load(g_app.current_exp_path)) set_status(L"EXP reloaded.");
                    break;
                case IDM_FILE_SAVE_EXP:
                    if (g_app.exp_db.save()) set_status(L"EXP saved."); else set_status(L"EXP save failed or binary format.");
                    break;
                case IDM_FILE_EXIT: PostQuitMessage(0); break;
                case IDM_UCI_LOAD: on_load_engine(); break;
                case IDM_UCI_CONFIGURE: on_configure_engine(hWnd); break;
                case IDM_NAV_PREV: on_prev(); break;
                case IDM_NAV_NEXT: on_next(); break;
                case IDM_ANALYSE_CURRENT:
                    analyse_at_ply(std::max(1,g_app.current_ply));
                    break;
                case IDM_ANALYSE_GAME:
                    analyse_whole_game();
                    break;
            } break;
        }
        case WM_NOTIFY:{
            LPNMHDR hdr = (LPNMHDR)lParam;
            if (hdr->hwndFrom == g_hList && hdr->code == LVN_ITEMCHANGED){
                NMLISTVIEW* nmlv = (NMLISTVIEW*)lParam;
                if ((nmlv->uChanged & LVIF_STATE) && (nmlv->uNewState & LVIS_SELECTED) && !(nmlv->uOldState & LVIS_SELECTED)) {
                    fill_moves_for_game(nmlv->iItem);
                }
            }
            return 0;
        }
        case WM_KEYDOWN:{
            if (wParam==VK_RIGHT) { on_next(); return 0; }
            if (wParam==VK_LEFT)  { on_prev(); return 0; }
            break;
        }
        case WM_DESTROY: g_app.engine.stop(); PostQuitMessage(0); break;
        default: return DefWindowProcW(hWnd,msg,wParam,lParam);
    }
    return 0;
}

int run_gui(HINSTANCE hInstance) {
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hIcon   = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hInstance = hInstance;
    wc.lpszClassName = APP_CLASS;
    wc.lpfnWndProc = WndProc;
    if (!RegisterClassExW(&wc)) return 1;

    HWND hWnd = CreateWindowExW(0, APP_CLASS, L"Experience Viewer Pro",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 1200, 720,
        nullptr, build_menu(), hInstance, nullptr);
    if (!hWnd) return 2;

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return (int)msg.wParam;
}
#endif
