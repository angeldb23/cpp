# 🪟 Ejecutar el curso en Windows (sin WSL)

Este documento explica cómo poner el curso de C++ y desarrollo de juegos a funcionar en **Windows puro**, usando solo las herramientas y librerías que el curso pide (SDL3, GLM, Dear ImGui, Box2D, miniaudio, CMake). No se necesita Windows Subsystem for Linux (WSL) ni entornos virtuales complejos; basta con un compilador MinGW‑w64 y el gestor de paquetes **vcpkg** (o instalar los binarios manualmente).

---

## 1. Requisitos previos

| Componente | Versión mínima | Cómo obtenerlo |
|------------|----------------|----------------|
| **Compilador** | g++ (C++20) | Install **MinGW‑w64** (versión 11 o superior). El instalador fácil es el *mingw-w64-installer* desde <https://mingw-w64.org/doku.php/download> o usar el *MSYS2* instalador y seleccionar la herramienta de 64‑bits. |
| **CMake** | 3.20+ | Descargar el instalador desde <https://cmake.org/download/> y ejecutarlo, o instalar mediante el instalador de MinGW (pacman -S cmake bajo MSYS2). |
| **vcpkg** (gestor de paquetes opcional) | último | Clonar <https://github.com/microsoft/vcpkg> y ejecutar `.\vcpkg install`. Alternativamente, usar el instalador oficial `vcpkg.exe`. |
| **Git** | cualquier versión reciente | <https://git-scm.com/download/win> |

> **Nota:** Si ya tienes Visual Studio 2022 con la carga de trabajo “Desarrollo de escritorio con C++”, puedes usar el compilador MSVC en lugar de MinGW, pero los pasos de abajo están escritos para MinGW‑w64 + g++.

---

## 2. Clonar el repositorio

Abre **PowerShell** o la línea de comandos de MinGW y ejecuta:

```powershell
git clone https://github.com/angeldb23/cpp.git
cd cpp
```

Esto creará la carpeta `cpp` con todos los módulos.

---

## 3. Instalar las dependencias del curso

El curso usa las siguientes librerías externas. En Windows la forma más sencilla es a través de **vcpkg**; si prefieres no usar vcpkg, puedes descargar los binarios/headers manualmente y apuntar CMake a ellos.

### 3.1 Con vcpkg (recomendado)

```powershell
# Desde la raíz del repositorio
.\vcpkg\bootstrap-vcpkg.bat   # primera vez solo
.\vcpkg\vcpkg install sdl3      # SDL3 (headers + libs)
.\vcpkg\vcpkg install glm       # matemáticas header‑only (vcpkg las coloca en include)
.\vcpkg\vcpkg install imgui      # Dear ImGui (tag v1.91.5‑docking)
.\vcpkg\vcpkg install box2d      # Box2D 2.4.x
.\vcpkg\vcpkg install miniaudio  # miniaudio single‑header
```

Después de cada instalación, vcpkg dejará los archivos en la carpeta `installed` de vcpkg. Para que CMake los encuentre, añade la ruta de vcpkg al `CMAKE_TOOLCHAIN_FILE` o establece la variable de entorno `VCPKG_ROOT`.

**Ejemplo de cadena de herramientas (toolchain) para CMake**:

Crea un archivo `toolchain-mingw.cmake` en la raíz:

```cmake
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_C_COMPILER i686-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER i686-w64-mingw32-g++)

# Directorio donde vcpkg instaló las librerías
set(CMAKE_PREFIX_PATH "C:/path/to/vcpkg/installed/x64-windows")
```

Luego, al configurar:

```powershell
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=toolchain-mingw.cmake
```

### 3.2 Instalación manual (sin vcpkg)

1. **SDL3**: descargar los fuentes o binarios precompilados desde <https://github.com/libsdl-org/SDL/releases> (versión 3.1.3 o superior). Descomprime y coloca `SDL3/include/` y `SDL3/lib/x64/` en rutas conocidas. En CMake puedes usar `link_directories` o `target_link_libraries` con la ruta absoluta.

2. **GLM**: simple. Copia la carpeta `glm` que viene en la distribución oficial (https://github.com/g-truc/glm) a `include/glm` dentro de tu proyecto o añádela al `INCLUDE_PATH`.

3. **Dear ImGui**: descargar la versión `v1.91.5-docking` desde <https://github.com/ocornelissen/imgui> o usar el tag que el curso necesita. Copia `imgui.cpp`, `imgui.h` y `imgui_demo.cpp` a tu proyecto; el `CMakeLists.txt` del curso ya está preparado para compilarlos como parte de las lecciones.

4. **Box2D 2.4.x**: descargar la versión 2.4.2 desde <https://box2d.org/downloads/> o usar el binario de vcpkg (recomendado). Los headers están en `box2d/box2d.h`. Asegúrate de que la librería esté enlazada (`libbox2d.a` o `box2d.lib`).

5. **miniaudio**: descargar el *single‑header* `miniaudio.h` desde <https://github.com/mikedn/miniaudio>. No requiere compilación separada; solo `#define MINIAUDIO_IMPLEMENTATION` antes de incluir el header.

Después de tener las librerías, **verifica** que el compilador las encuentre:

```powershell
g++ -std=c++20 -Wall -Wextra -I include -I path/to/SDL3/include -I path/to/glm -I path/to/imgui -I path/to/miniaudio -c src/leccion_01.cpp -o src/leccion_01.o
```

Si no hay errores, continua.

---

## 4. Compilar y ejecutar un módulo

Desde la carpeta del repositorio (Ejemplo con el modulo 28):

```powershell
# 1. Crea la carpeta build y entra en ella
mkdir build && cd build

# 2. COnfigura CMake usando la toolchain de MinGW y vcpkg
cmake -S .. -B . -DCMAKE_TOOLCHAIN_FILE=toolchain-mingw.cmake -DVCPKG_ROOT="C:/path/to/vcpkg/installed/x64-windows"

# 3. Compila
cmake --build . -j4

# 4. Corre el ejecutable (ejemplo con el modulo 1)
.\leccion_01_criterios_seleccion.exe
```

Si todo va bien, deberia abrirse la ventana de SDL3 y cerrarse cuando desees.

## 4. Expected folder structure after installation

```
curso_cpp/
│
├─ 27_doom_clasico/           ← módulo 27 (Doom clásico)
│   ├─ CMakeLists.txt
│   ├─ lecciones/
│   ├─ ejercicios/
│   └─ soluciones/
│
├─ 28_estudio_codigo_real/    ← módulo 28 (estudio de librerías)
│   ├─ CMakeLists.txt
│   ├─ lecciones/
│   ├─ ejercicios/
│   ├─ soluciones/
│   └─ cierre/
│       └─ INFORME_TECNICO.md
│
├─ 29_proyecto_final/         ← (opcional) proyecto final – ver README_windows.md
│   └─ README.md
│
├─ include/                   ← Headers del curso
├─ CMakeLists.txt             ← rRaiz (opcional, incluye modulos)
├─ README.md                  ← Descripcion del curso
├─ README_windows.md          ← Guia de instalacion del curso en Windows
├─ assets/                    ← Assets del curso, opcional
```

---

## 6. Resolución de problemas comunes



| Problema | Causa | Solución |

|---|---|---|

| `g++: command not found` | MinGW no está en el PATH | Agregá la carpeta `bin` de MinGW al `PATH` de Windows o usá la terminal de MinGW. |

| `cannot find -lSDL3` | SDL3 no está vinculada (linked) | Instalá SDL3 mediante vcpkg o agregá el archivo `SDL3.lib`/`.a` y pasá `-lSDL3` en la línea de comandos. |

| `ImGui no compiles` | Falta `IMGUI_IMPL_OPENGL_LOADER_GL3` o los headers son antiguos | Usá la versión `v1.91.5-docking` y seguí el `CMakeLists.txt`; el script ya incluye `ImGui_ImplSDL3_InitForOpenGL(window)`. |

| `Box2D errors undefined reference` | La librería no está vinculada (linked) | Si usás vcpkg: `target_link_libraries(m28_solucion box2d)`. Si es manual, agregá la ruta del `.a`/`.lib` y `-lbox2d`. |

| El programa se cierra al instante | El contexto de SDL o OpenGL no fue inicializado | Asegurate de que `main` llame a `SDL_Init(SDL_INIT_VIDEO)` y a `SDL_CreateWindowAndRenderer` (o `SDL_GPU_CreateDevice`) antes de cualquier renderizado. |

---

## 6. Pasos rápidos "todo en uno"


```powershell

# 1. Instalar MinGW‑w64 y agregarlo al PATH

# 2. Instalar vcpkg y ejecutar:

   .\vcpkg\install sdl3 glm imgui box2d miniaudio

# 3. Clonar el repositorio

git clone ... && cd curso_cpp

# 4. Configurar y compilar un módulo (ejemplo M28):

mkdir build && cd build

cmake -S .. -B . -DCMAKE_TOOLCHAIN_FILE=toolchain-mingw.cmake -DVCPKG_ROOT="C:/ruta/a/vcpkg/installed/x64-windows"

cmake --build . -j4

.\leccion_01_criterios_seleccion.exe   # probar la ventana

```

Con estos pasos tendrás el curso funcionando en Windows sin necesidad de WSL ni de una instalación completa de Linux. 

---

*Este archivo forma parte de la entrega del curso; junto con `README_curso.md` y los módulos 01‑28 constituye la documentación completa para comenzar.*
