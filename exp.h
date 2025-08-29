
#pragma once
#include <string>
#include <vector>

struct ExpEntry {
    std::string key;  // typically UCI like e2e4
    int count = 0;
    double score = 0.0; // cp
    int quality = 0;
    int wins = 0, draws = 0, losses = 0;
};

struct ExpStats {
    int entries=0;
    long long total_count=0;
    double avg_score=0.0;
    long long total_wins=0, total_draws=0, total_losses=0;
};

struct ExpDatabase {
    std::wstring source_path;
    std::vector<ExpEntry> items;
    bool is_text_like = true;
    std::string raw_hex;

    bool load(const std::wstring& path);
    bool save() const; // CSV-like only
    ExpStats compute_stats() const;
};
