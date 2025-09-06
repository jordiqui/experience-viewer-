#ifdef _WIN32
#include <windows.h>
#include "gui.h"
#include <shlwapi.h>

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
    wchar_t mod[MAX_PATH]; GetModuleFileNameW(nullptr, mod, MAX_PATH);
    PathRemoveFileSpecW(mod);
    SetCurrentDirectoryW(mod);
    return run_gui(hInstance);
}
#else
#include "exp.h"
#include "utils.h"
#include "gui.h"
#include <iostream>

int main(int argc, char** argv){
#if USE_QT
    (void)argc; (void)argv;
    return run_gui(nullptr);
#else
    if (argc < 2){
        std::cerr << "Usage: " << argv[0] << " <file.exp>\n";
        return 1;
    }
    ExpDatabase db;
    if (!db.load(utf8_to_wide(argv[1]))){
        std::cerr << "Failed to load " << argv[1] << "\n";
        return 1;
    }
    for (auto& e : db.items){
        std::cout << e.key << " (" << e.count << ")\n";
    }
    return 0;
#endif
}
#endif
