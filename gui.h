
#pragma once
#ifdef _WIN32
#include <windows.h>
int run_gui(HINSTANCE hInstance, const wchar_t* assets_dir = nullptr);
#else
typedef void* HINSTANCE;
inline int run_gui(HINSTANCE, const wchar_t* = nullptr) { return 0; }
#endif
