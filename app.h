
#pragma once
#include <string>
#include <vector>
#include "board.h"
#include "uci_engine.h"
#include "exp.h"
#include "pgn.h"

struct MoveRow {
    std::string token; // SAN or UCI
    int exp_cp = 0;
    int depth = 0;
    int quality = 0;
    int engine_cp = 0;
};

class ChessBoard {
public:
    BoardState st;
    void set_startpos(){ st.set_startpos(); }
    void apply_uci(const std::string& u){ st.apply_uci(u); }
};

struct AppState {
    // PGN/EXP
    PgnDatabase pgn_db;
    ExpDatabase exp_db;
    std::wstring current_pgn_path, current_exp_path;

    // Engine
    UCIEngine engine;
    std::wstring engine_path;
    std::string engine_log;

    // Board + moves
    ChessBoard board;
    std::vector<MoveRow> current_moves;
    int current_ply = 0;

    // Analysis preferences
    int analysis_depth = 12;
    int analysis_movetime = 0;

    void clear_engine_log(){ engine_log.clear(); }
    void append_engine_log(const std::string& s){ engine_log += s; }
};
