
# Experience Viewer Pro (Win32 C++)
- **Tablero** con sprites PNG (GDI+), clic para mover, navegación ←/→.
- **Layout horizontal**: PGN a la izquierda; derecha = tablero + tabla de jugadas.
- **UCI Options (Arena/Scid-like)**: diálogo dinámico a partir de `option name ...` del motor.
- **.exp**: lectura tipo texto (CSV-like) y cruce básico por clave UCI.
- **Detección de sprites**: la aplicación avisa si faltan imágenes de piezas en `assets/`.

## Build
```bash
# con clang++
make -j8 CXX=clang++
# o con g++ de MinGW-w64
make -j8 CXX=x86_64-w64-mingw32-g++
# binario: build/ExperienceViewer.exe
```
Requiere MinGW-w64 y GDI+ (ya enlazado con `-lgdiplus`).
