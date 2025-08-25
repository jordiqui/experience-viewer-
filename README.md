
# Experience Viewer Pro (Win32 C++)
- **Tablero** con sprites PNG (GDI+), clic para mover, navegación ←/→.
- **Layout horizontal**: PGN a la izquierda; derecha = tablero + tabla de jugadas.
- **UCI Options (Arena/Scid-like)**: diálogo dinámico a partir de `option name ...` del motor.
- **.exp**: lectura tipo texto (CSV-like) y cruce básico por clave UCI.
- **Detección de sprites**: la aplicación avisa si faltan imágenes de piezas en `assets/`.

## Build
```bash
make -j8 CXX=clang++
# binario: build/ExperienceViewer.exe
```
Requiere MinGW-w64/Clang y GDI+ (ya enlazado con `-lgdiplus`).
