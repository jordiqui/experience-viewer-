
#include "board.h"
#include <algorithm>
#include <string>
#include <windows.h>

// ---------- Global state for board control ----------
static ULONG_PTR g_gdiplusToken = 0;
static std::wstring g_assets_dir;
static BoardState g_cur;
static HWND g_board_hwnd = nullptr;
static bool g_warned_img = false;

// ---------- BoardState ----------

void BoardState::set_startpos(){
    const char* start =
        "rnbqkbnr"
        "pppppppp"
        "        "
        "        "
        "        "
        "        "
        "PPPPPPPP"
        "RNBQKBNR";
    // a1 = index 0 (bottom-left), we map so that r=0 is rank 1
    for(int r=0;r<8;++r){
        for(int f=0; f<8; ++f){
            squares[r*8+f] = start[(7-r)*8 + f];
        }
    }
    white_to_move = true;
}

static inline int sq_of(int f,int r){ return r*8+f; }

void BoardState::apply_uci(const std::string& uci){
    if (uci.size() < 4) return;
    int f1 = uci[0] - 'a';
    int r1 = uci[1] - '1';
    int f2 = uci[2] - 'a';
    int r2 = uci[3] - '1';
    if (f1<0||f1>7||r1<0||r1>7||f2<0||f2>7||r2<0||r2>7) return;

    int from = sq_of(f1,r1);
    int to   = sq_of(f2,r2);
    char piece = squares[from];

    // handle castling rook move (very basic)
    if ((piece=='K' && f1==4 && r1==0 && (f2==6 || f2==2)) ||
        (piece=='k' && f1==4 && r1==7 && (f2==6 || f2==2)))
    {
        if (f2==6){ // king side
            squares[sq_of(5,r1)] = squares[sq_of(7,r1)];
            squares[sq_of(7,r1)] = ' ';
        } else if (f2==2){ // queen side
            squares[sq_of(3,r1)] = squares[sq_of(0,r1)];
            squares[sq_of(0,r1)] = ' ';
        }
    }

    squares[to] = piece;
    squares[from] = ' ';

    // promotion
    if (uci.size() >= 5){
        char pr = uci[4];
        if (piece=='P' && r2==7) squares[to] = (char)toupper((unsigned char)pr);
        if (piece=='p' && r2==0) squares[to] = (char)tolower((unsigned char)pr);
    }

    white_to_move = !white_to_move;
}

// ---------- GDI+ helpers ----------

static const wchar_t* BOARD_CLASS = L"EV_BoardWnd";

static const wchar_t* piece_name(wchar_t c){
    switch(c){
    case L'P': return L"w_p.png"; case L'N': return L"w_n.png"; case L'B': return L"w_b.png";
    case L'R': return L"w_r.png"; case L'Q': return L"w_q.png"; case L'K': return L"w_k.png";
    case L'p': return L"b_p.png"; case L'n': return L"b_n.png"; case L'b': return L"b_b.png";
    case L'r': return L"b_r.png"; case L'q': return L"b_q.png"; case L'k': return L"b_k.png";
    default:   return L"";
    }
}

// ---------- Public API ----------

void board_register(HINSTANCE){
    if (!g_gdiplusToken){
        GdiplusStartupInput gi;
        GdiplusStartup(&g_gdiplusToken, &gi, nullptr);
    }
    WNDCLASSEXW wc{}; wc.cbSize = sizeof(wc);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszClassName = BOARD_CLASS;
    wc.lpfnWndProc = [](HWND h, UINT m, WPARAM w, LPARAM l)->LRESULT{
        switch(m){
        case WM_PAINT:{
            PAINTSTRUCT ps; HDC hdc = BeginPaint(h, &ps);
            Graphics g(hdc);
            RECT rc; GetClientRect(h,&rc);
            int W = rc.right-rc.left, H = rc.bottom-rc.top;
            int size = std::min(W,H);
            int offx = (W-size)/2, offy = (H-size)/2;
            int cell = size/8;

            // Draw board
            SolidBrush light(Color(240,217,181));
            SolidBrush dark(Color(181,136,99));
            for(int rr=0; rr<8; ++rr){
                for(int ff=0; ff<8; ++ff){
                    bool darksq = ((rr+ff)&1);
                    Rect rect(offx+ff*cell, offy+(7-rr)*cell, cell, cell);
                    g.FillRectangle(darksq? &dark : &light, rect);
                }
            }

            // Draw pieces
            for (int r = 0; r < 8; ++r) {
                for (int f = 0; f < 8; ++f) {
                    char P = g_cur.squares[r * 8 + f];
                    if (P == ' ') continue;
                    const wchar_t* fn = piece_name(P);
                    std::wstring full = g_assets_dir + L"\\" + fn;
                    Image img(full.c_str());
                    Rect rect(offx + f * cell + 2, offy + (7 - r) * cell + 2, cell - 4, cell - 4);
                    if (img.GetLastStatus() != Ok) {
                        Pen pen(Color(200, 0, 0), 3.0f);
                        g.DrawLine(&pen, rect.X, rect.Y, rect.GetRight(), rect.GetBottom());
                        g.DrawLine(&pen, rect.GetRight(), rect.Y, rect.X, rect.GetBottom());
                        if (!g_warned_img) {
                            std::wstring msg = L"No se pudo cargar sprite: " + full +
                                               L"\nAsegúrate de que assets/ contenga w_p.png...b_k.png";
                            MessageBoxW(nullptr, msg.c_str(), L"Sprite faltante", MB_OK | MB_ICONWARNING);
                            g_warned_img = true;
                        }
                    } else {
                        g.DrawImage(&img, rect);
                    }
                }
            }

            EndPaint(h, &ps);
            return 0;
        }
        default:
            return DefWindowProcW(h,m,w,l);
        }
    };
    RegisterClassExW(&wc);
}

HWND board_create(HWND parent, int ctrl_id){
    HWND h = CreateWindowExW(0, BOARD_CLASS, L"", WS_CHILD|WS_VISIBLE,
                             0,0,100,100, parent, (HMENU)(INT_PTR)ctrl_id,
                             GetModuleHandleW(nullptr), nullptr);
    g_board_hwnd = h;
    return h;
}

void board_set_assets_dir(const std::wstring& dir){
    g_assets_dir = dir;
    // If not found, try parent (useful if running from build/)
    std::wstring probe = g_assets_dir + L"\\w_p.png";
    if (GetFileAttributesW(probe.c_str()) == INVALID_FILE_ATTRIBUTES){
        size_t pos = g_assets_dir.find_last_of(L"\\/");
        if (pos != std::wstring::npos){
            std::wstring parent = g_assets_dir.substr(0, pos);
            std::wstring alt = parent + L"\\assets";
            std::wstring probe2 = alt + L"\\w_p.png";
            if (GetFileAttributesW(probe2.c_str()) != INVALID_FILE_ATTRIBUTES){
                g_assets_dir = alt;
            }
        }
    }
    // warn user if the basic pawn sprite still cannot be found after
    // probing alternative locations.  The GUI is unusable without the
    // piece images, so emit a message box once to alert the user.
    std::wstring finalProbe = g_assets_dir + L"\\w_p.png";
    if (GetFileAttributesW(finalProbe.c_str()) == INVALID_FILE_ATTRIBUTES && !g_warned_img) {
        MessageBoxW(nullptr,
                    L"No se encuentran los sprites en 'assets'. Asegúrate de que existan w_p.png … b_k.png.",
                    L"Sprites no encontrados", MB_OK | MB_ICONWARNING);
        g_warned_img = true;
    }
    if (g_board_hwnd) InvalidateRect(g_board_hwnd, nullptr, FALSE);
}

void board_set_position(const BoardState& st){
    g_cur = st;
    if (g_board_hwnd) InvalidateRect(g_board_hwnd, nullptr, FALSE);
}

void board_get_square_from_point(POINT, int& file, int& rank){
    file = rank = -1; // not used yet
}

std::string board_san_to_uci(const std::string&){
    return std::string(); // minimal stub; GUI has a simple converter
}

void board_reset_start(){
    BoardState st; st.set_startpos(); board_set_position(st);
}
