
#pragma once
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#else
using UINT = unsigned int;
#endif

std::wstring utf8_to_wide(const std::string& s);
std::string  wide_to_utf8(const std::wstring& ws);

bool read_file_text(const std::wstring& path, std::string& out);
bool read_file_binary(const std::wstring& path, std::vector<unsigned char>& out);

std::wstring open_file_dialog(const wchar_t* filter, const wchar_t* title);
void show_message(const std::wstring& text, const std::wstring& caption,
                  UINT flags = 0);

std::string hex_dump(const std::vector<unsigned char>& data,
                     size_t bytes_per_line = 16);
