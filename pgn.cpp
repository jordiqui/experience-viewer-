
#include "pgn.h"
#include "utils.h"
#include <fstream>
#include <sstream>

bool PgnDatabase::load(const std::wstring& path){
    games.clear();
    std::string text; if (!read_file_text(path, text)) return false; std::istringstream f(text); std::string line;
    PgnGame cur;
    // (unused)
    while (std::getline(f,line)){
        if (!line.empty() && line[0]=='['){size_t sp = line.find(' ');
            size_t qt1 = line.find('"');
            size_t qt2 = line.rfind('"');
            if (sp!=std::string::npos && qt1!=std::string::npos && qt2!=std::string::npos && qt2>qt1){
                std::string key = line.substr(1, sp-1);
                std::string val = line.substr(qt1+1, qt2-qt1-1);
                cur.tags[key]=val;
            }
        } else if (line.empty()){
            if (!cur.moves_san.empty()){
                games.push_back(cur);
                cur = PgnGame{};
            }} else {
            // moves
            if (!cur.moves_san.empty()) cur.moves_san.push_back(' ');
            cur.moves_san += line;}
    }
    if (!cur.moves_san.empty()) games.push_back(cur);
    return true;
}
