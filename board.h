
#pragma once
#include <windows.h>
#include <gdiplus.h>
#include <string>
using namespace Gdiplus;

struct BoardState{
    // 8x8 board, indices 0..63 (a1=0, h1=7, a8=56)
    char squares[64]{};
    bool white_to_move = true;
    void set_startpos();
    void apply_uci(const std::string& uci); // very basic
};

// Windowed board control
void board_register(HINSTANCE hInst);
HWND board_create(HWND parent, int ctrl_id);
void board_set_assets_dir(const std::wstring& dir);
void board_set_position(const BoardState& st);

// Helpers
void board_get_square_from_point(POINT pt, int& file, int& rank);

// SAN->UCI minimal helpers
std::string board_san_to_uci(const std::string& san);
// Reset
void board_reset_start();
