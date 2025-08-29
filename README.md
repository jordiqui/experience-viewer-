
# Experience Viewer Pro (C++)
- **Tablero** con sprites PNG (GDI+), clic para mover, navegación ←/→ (solo Windows).
- **Layout horizontal**: PGN a la izquierda; derecha = tablero + tabla de jugadas.
- **UCI Options (Arena/Scid-like)**: diálogo dinámico a partir de `option name ...` del motor.
- **.exp**: lectura tipo texto (CSV-like) y cruce básico por clave UCI.
- **Detección de sprites**: la aplicación avisa si faltan imágenes de piezas en `assets/`.

## Build
La aplicación puede compilarse tanto en Linux (modo consola) como en Windows
con MinGW-w64 (modo gráfico).

### Linux
```bash
make
# binario: build/experience-viewer

# verificar (placeholder)
make check

# verificar distribución (placeholder)
make distcheck
```

### Windows (cross-compile)
```bash
make CXX=x86_64-w64-mingw32-g++
# binario: build/ExperienceViewer.exe
```
Los flags específicos de Win32 se aplican automáticamente cuando se detecta un
toolchain MinGW.
