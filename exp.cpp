
#include "exp.h"
#include "utils.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <cctype>

static void split_line(const std::string& line, std::vector<std::string>& out){
    out.clear();
    std::string cur;
    for(char c: line){
        if (c==',' || c==';' || c==':' || isspace((unsigned char)c)){
            if (!cur.empty()) { out.push_back(cur); cur.clear(); }
        } else cur.push_back(c);
    }
    if (!cur.empty()) out.push_back(cur);
}

bool ExpDatabase::load(const std::wstring& path){
    source_path = path;
    items.clear();
    raw_hex.clear();
    // naive: try to read as text first
    std::string text;
    if (read_file_text(path, text)){
        std::istringstream f(text);
        std::string line;
        while (std::getline(f,line)){
            if (line.empty() || line[0]=='#') continue;
            std::vector<std::string> v; split_line(line, v);
            if (v.empty()) continue;
            ExpEntry e; e.key = v[0];
            for (size_t i=1; i<v.size(); ++i){
                const std::string& tok = v[i];
                if (tok=="count" && i+1<v.size()) { e.count = std::stoi(v[++i]); }
                else if (tok=="eval" && i+2<v.size() && v[i+1]=="cp") { e.score = std::stod(v[i+2]); i+=2; }
                else if (tok=="quality" && i+1<v.size()) { e.quality = std::stoi(v[++i]); }
                else if (i==1) { e.count = std::stoi(tok); }
                else if (i==2) { e.score = std::stod(tok); }
                else if (i==3) { e.wins  = std::stoi(tok); }
                else if (i==4) { e.draws = std::stoi(tok); }
                else if (i==5) { e.losses= std::stoi(tok); }
            }
            items.push_back(std::move(e));
        }
        is_text_like = true;
        return true;
    }
    // fallback: binary hex
    std::vector<unsigned char> bin;
    if (!read_file_binary(path, bin)) return false;
    // simple hex
    static const char* hexd = "0123456789ABCDEF";
    std::ostringstream oss;
    for (size_t i=0;i<bin.size();++i){
        unsigned char c = bin[i];
        oss<<hexd[(c>>4)&0xF]<<hexd[c&0xF];
        if (i%16==15) oss<<"\n"; else oss<<" ";
    }
    raw_hex = oss.str();
    is_text_like = false;
    return true;
}

bool ExpDatabase::save() const{
    if (!is_text_like || source_path.empty()) return false;
    std::string out = "# key,count,score,quality,wins,draws,losses\n";
    for (auto& e: items){
        out += e.key + "," + std::to_string(e.count) + "," + std::to_string(e.score) + "," +
               std::to_string(e.quality) + "," + std::to_string(e.wins) + "," +
               std::to_string(e.draws) + "," + std::to_string(e.losses) + "\n";
    }
#ifdef _WIN32
    HANDLE h = CreateFileW(source_path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h==INVALID_HANDLE_VALUE) return false;
    DWORD wrote=0; BOOL ok = WriteFile(h, out.data(), (DWORD)out.size(), &wrote, nullptr);
    CloseHandle(h);
    return ok == TRUE;
#else
    std::ofstream f(wide_to_utf8(source_path));
    if(!f) return false;
    f << out;
    return true;
#endif
}

ExpStats ExpDatabase::compute_stats() const{
    ExpStats st{};
    st.entries = (int)items.size();
    if (items.empty()) return st;
    long long sumCnt=0; double sumScore=0.0; long long w=0,d=0,l=0;
    for (auto& e: items){
        sumCnt += e.count;
        sumScore += e.score;
        w += e.wins; d += e.draws; l += e.losses;
    }
    st.total_count = sumCnt;
    st.avg_score = sumScore / items.size();
    st.total_wins = w; st.total_draws = d; st.total_losses = l;
    return st;
}
