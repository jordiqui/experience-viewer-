
#pragma once
#ifdef _WIN32
#include <windows.h>
int run_gui(HINSTANCE hInstance);
#else
typedef void* HINSTANCE;
inline int run_gui(HINSTANCE) { return 0; }
#endif
