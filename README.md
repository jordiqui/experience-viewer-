
# Experience Viewer Pro (C++)
- **Tablero** con sprites PNG (GDI+), clic para mover, navegación ←/→ (solo Windows).
- **Layout horizontal**: PGN a la izquierda; derecha = tablero + tabla de jugadas.
- **UCI Options (Arena/Scid-like)**: diálogo dinámico a partir de `option name ...` del motor.
- **.exp**: lectura tipo texto (CSV-like) y cruce básico por clave UCI.
- **Detección de sprites**: la aplicación avisa si faltan imágenes de piezas en `assets/` (listando los archivos faltantes) y permite especificar la ruta mediante la variable de entorno `EV_ASSETS_DIR`.

## Build
La aplicación puede compilarse tanto en Linux (modo consola) como en Windows
con MinGW-w64 (modo gráfico).

### Linux
La compilación por defecto genera una versión **de consola** que no muestra piezas ni el diálogo UCI.

```bash
make
# binario: build/experience-viewer (modo consola)
./build/experience-viewer datos.exp
```

Para disponer de tablero y diálogo UCI en Linux es necesario compilar con Qt.
Instalar dependencias (Debian/Ubuntu):

```bash
sudo apt-get update
sudo apt-get install qtbase5-dev qt5-qmake pkg-config
```

Compilación y ejecución con Qt:

```bash
make USE_QT=1
# binario: build/experience-viewer (Qt)
./build/experience-viewer datos.exp

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

Los componentes basados en Qt aún son experimentales. El comando
`make USE_QT=1` compila los fuentes del directorio `qt/` utilizando las
bibliotecas detectadas por `pkg-config` e inicia una interfaz gráfica simple
inspirada en [jfxchess](https://github.com/asdfjkl/jfxchess) que muestra un
tablero y una lista de jugadas.

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

