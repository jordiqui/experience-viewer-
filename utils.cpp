
#include "utils.h"
#include <sstream>
#include <iomanip>
#ifdef _WIN32
#include <commdlg.h>
#else
#include <fstream>
#include <codecvt>
#include <locale>
#include <iostream>
#endif

std::wstring utf8_to_wide(const std::string& s){
    if (s.empty()) return L"";
#ifdef _WIN32
    int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), nullptr, 0);
    std::wstring ws(len, 0);
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), ws.data(), len);
    return ws;
#else
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
    return conv.from_bytes(s);
#endif
}
std::string wide_to_utf8(const std::wstring& ws){
    if (ws.empty()) return std::string();
#ifdef _WIN32
    int len = WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), (int)ws.size(), nullptr, 0, nullptr, nullptr);
    std::string s(len, 0);
    WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), (int)ws.size(), s.data(), len, nullptr, nullptr);
    return s;
#else
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
    return conv.to_bytes(ws);
#endif
}

bool read_file_text(const std::wstring& path, std::string& out){
#ifdef _WIN32
    HANDLE hFile = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr,
                               OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) return false;
    DWORD size = GetFileSize(hFile, nullptr);
    if (size == INVALID_FILE_SIZE) { CloseHandle(hFile); return false; }
    std::vector<char> buf(size);
    DWORD read = 0; BOOL ok = ReadFile(hFile, buf.data(), size, &read, nullptr);
    CloseHandle(hFile);
    if (!ok) return false;
    out.assign(buf.begin(), buf.begin()+read);
    return true;
#else
    std::ifstream f(wide_to_utf8(path));
    if(!f) return false;
    std::stringstream ss; ss<<f.rdbuf();
    out = ss.str();
    return true;
#endif
}

bool read_file_binary(const std::wstring& path, std::vector<unsigned char>& out){
#ifdef _WIN32
    HANDLE hFile = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr,
                               OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) return false;
    DWORD size = GetFileSize(hFile, nullptr);
    if (size == INVALID_FILE_SIZE) { CloseHandle(hFile); return false; }
    out.resize(size);
    DWORD read = 0; BOOL ok = ReadFile(hFile, out.data(), size, &read, nullptr);
    CloseHandle(hFile);
    if (!ok) return false;
    out.resize(read);
    return true;
#else
    std::ifstream f(wide_to_utf8(path), std::ios::binary);
    if(!f) return false;
    std::vector<char> buf((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    out.assign(buf.begin(), buf.end());
    return true;
#endif
}

std::wstring open_file_dialog(const wchar_t* filter, const wchar_t* title){
#ifdef _WIN32
    wchar_t buffer[MAX_PATH] = L"";
    OPENFILENAMEW ofn{}; ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFilter = filter; ofn.lpstrFile = buffer; ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST; ofn.lpstrTitle = title;
    if (GetOpenFileNameW(&ofn)) return buffer;
    return L"";
#else
    (void)filter; (void)title; return L"";
#endif
}

void show_message(const std::wstring& text, const std::wstring& caption, UINT flags){
#ifdef _WIN32
    MessageBoxW(nullptr, text.c_str(), caption.c_str(), flags);
#else
    std::wcerr << caption << L": " << text << std::endl;
#endif
}

std::string hex_dump(const std::vector<unsigned char>& data, size_t bytes_per_line){
    std::ostringstream oss;
    size_t n = data.size();
    for (size_t i = 0; i < n; i += bytes_per_line) {
        oss << std::setw(8) << std::setfill('0') << std::hex << i << "  ";
        for (size_t j = 0; j < bytes_per_line; ++j) {
            if (i + j < n) oss << std::setw(2) << std::setfill('0') << std::hex << (int)data[i+j] << " ";
            else oss << "   ";
        }
        oss << " ";
        for (size_t j = 0; j < bytes_per_line; ++j) {
            if (i + j < n) {
                unsigned char c = data[i+j];
                oss << (c >= 32 && c < 127 ? (char)c : '.');
            }
        }
        oss << "\n";
    }
    return oss.str();
}
