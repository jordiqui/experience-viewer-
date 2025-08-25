# Experience Viewer (C++ / Win32)

A lightweight Windows GUI app (Win32 + Common Controls) to **inspect chess engine "experience" files (`.exp`)**, open **PGN** databases, and **talk to UCI engines** for quick checks.  
Designed to compile cleanly with **MinGW-w64 (GCC 13+ / 15+)** or **Clang/LLVM** on Windows.

> No heavy GUI frameworks are required. Pure Win32 API + common controls.

---

## Features

- Open `.exp` files (text-style experience files are parsed; unknown/binary formats are shown in a hex inspector).
- Open `.pgn` databases; basic header parsing (Event/Site/Date/White/Black/Result) + moves string.
- Launch a **UCI engine** (path to exe), send `uci`, `isready`, `ucinewgame`, `position`, `go depth N`, etc.
- Set UCI options via a simple dialog (name=value pairs).
- Summary stats for experience entries (total, average score, W/D/L if present).
- Responsive UI (engine I/O handled on a worker thread).
- Single portable executable output.

> The `.exp` format is not standardised across tools. This app includes a *pluggable parser*:
> - If the file looks like **ASCII/UTF-8 text with delimited columns** (`,` or `;`), it tries to parse fields like `key,count,score,wins,draws,losses` (any subset).
> - Otherwise, it falls back to a **hex viewer** so you can still inspect unknown/binary `.exp` files.

---

## Build (Windows)

### Prerequisites
- **MSYS2 MinGW-w64** toolchain or **LLVM/Clang** with MinGW libraries.
- Common controls are part of Windows; no extra libs to install.

### Using MinGW-w64 (GCC)
```bash
cd exp_viewer_cpp
make
# or multi-core
make -j8
```

### Using Clang (with MinGW runtime)
```bash
make CXX=clang++
# or
make -j8 CXX=clang++
```

### Clean
```bash
make clean
```

The resulting executable will be: `build/ExperienceViewer.exe`

---

## Usage

- **File → Open EXP…**: load an experience file.  
  - If recognised as text, entries will populate the table with columns: *Key / Count / Score / W / D / L*.
  - If binary/unknown, a hex pane is shown.
- **File → Open PGN…**: load a PGN database. The games list shows basic headers; you can view moves in the right pane.
- **UCI → Load Engine…**: choose a UCI engine executable.
- **UCI → Send "uci"** and **UCI → Is Ready**: basic handshakes.
- **UCI → New Game**, **UCI → Go Depth 12**: quick test commands.
- **UCI → Set Options…**: enter `name=value` lines to set engine options.
- **Tools → EXP Summary**: shows totals/averages for the currently loaded experience data.
- **Help → About**: version info.

> Tip: For long engine outputs, toggle **View → Follow Output** to auto-scroll.

---

## Project Layout

```
exp_viewer_cpp/
├─ Makefile
├─ README.md
├─ LICENSE
└─ src/
   ├─ main.cpp
   ├─ app.h          ├─ app.cpp
   ├─ gui.h          ├─ gui.cpp
   ├─ utils.h        ├─ utils.cpp
   ├─ uci_engine.h   ├─ uci_engine.cpp
   ├─ pgn.h          ├─ pgn.cpp
   ├─ exp.h          ├─ exp.cpp
```

---

## Notes on `.exp` formats

Because `.exp` files vary between engines/tools, this viewer aims to be permissive:
- **Text**: lines like `key,count,score,wins,draws,losses` (order flexible; missing fields allowed). Delimiters: comma or semicolon.
- **Binary**: not parsed into semantic records (format undocumented). Displayed in hex with offset column to assist reverse-engineering.

If you have a specific `.exp` format spec, you can extend `src/exp.cpp` → `try_parse_text_like()` or add a new strategy.

---

## License

MIT — do whatever you like, attribution appreciated.
