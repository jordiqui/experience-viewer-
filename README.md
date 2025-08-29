
# Experience Viewer Pro (C++)
- **Tablero** con sprites PNG (GDI+), clic para mover, navegación ←/→ (solo Windows).
- **Layout horizontal**: PGN a la izquierda; derecha = tablero + tabla de jugadas.
- **UCI Options (Arena/Scid-like)**: diálogo dinámico a partir de `option name ...` del motor.
  El parsing de estas opciones está disponible en cualquier plataforma.
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

### Qt (experimental)

Los componentes basados en Qt aún son experimentales. Se incluye un prototipo
que reemplaza el tablero y el diálogo de opciones UCI por widgets nativos.
Requiere tener instalados Qt5 y `pkg-config`. Para activar la compilación de
estos componentes:

```bash
make USE_QT=1
```

El comando compila los fuentes ubicados en el directorio `qt/` utilizando las
bibliotecas de Qt detectadas por `pkg-config` y define la macro `USE_QT`. Al
ejecutar esta variante, la opción **Configure Options…** abrirá un diálogo Qt
para modificar los parámetros del motor.

## Demo web con chess.js

Se agregó un ejemplo minimal en el directorio `web/` que utiliza las
dependencias `chess.js` y `cm-chessboard` para visualizar un tablero de
ajedrez.

1. Instalar las dependencias JavaScript:

   ```bash
   npm install
   ```

2. Abrir `web/index.html` en un navegador. Dependiendo del navegador, puede
   requerirse servir el archivo desde un servidor estático para que se
   resuelvan correctamente los módulos importados.

