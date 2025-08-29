
# Experience Viewer Pro (Win32 C++)
- **Tablero** con sprites PNG (GDI+), clic para mover, navegación ←/→.
- **Layout horizontal**: PGN a la izquierda; derecha = tablero + tabla de jugadas.
- **UCI Options (Arena/Scid-like)**: diálogo dinámico a partir de `option name ...` del motor.
- **.exp**: lectura tipo texto (CSV-like) y cruce básico por clave UCI.
- **Detección de sprites**: la aplicación avisa si faltan imágenes de piezas en `assets/`.

## Build
Este proyecto se orienta a la plataforma Windows y debe compilarse con un
toolchain MinGW-w64.  En sistemas Unix, asegúrate de tener instalado el
compilador cruzado y especificarlo a `make` mediante `CXX`:

```bash
make -j8 CXX=x86_64-w64-mingw32-g++
# binario: build/ExperienceViewer.exe
```
Los flags específicos de Win32 se aplican automáticamente cuando se detecta un
compilador MinGW.
