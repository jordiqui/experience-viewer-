
#pragma once
#ifdef _WIN32
#include <windows.h>
int run_gui(HINSTANCE hInstance, const wchar_t* assets_dir = nullptr);
#else
// When building with the Qt frontend we provide a real implementation of
// run_gui() in qt/gui_qt.cpp. Keeping the declaration here allows the rest of
// the codebase to invoke the GUI in a cross platform manner while still
// falling back to a no-op when Qt support is disabled.
typedef void* HINSTANCE;
int run_gui(HINSTANCE hInstance = nullptr, const wchar_t* assets_dir = nullptr);
#endif
