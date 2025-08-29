
#pragma once
#include <string>
#include <vector>
#include <map>

struct PgnGame {
    std::map<std::string,std::string> tags;
    std::string moves_san; // raw moves segment
};

struct PgnDatabase {
    std::vector<PgnGame> games;
    bool load(const std::wstring& path);
};
