#include <windows.h>
#include "gui.h"
#include <shlwapi.h>

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
    wchar_t mod[MAX_PATH]; GetModuleFileNameW(nullptr, mod, MAX_PATH);
    PathRemoveFileSpecW(mod);
    SetCurrentDirectoryW(mod);
    return run_gui(hInstance);
}
