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
git clone https://github.com/tuusuario/curso_cpp.git
cd curso_cpp
```

Esto creará la carpeta `curso_cpp` con todos los módulos, el script `verificar.sh` y los archivos de configuración.

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

If no errors, continue.

---

## 4. Compilar y ejecutar un módulo

From the repository folder (example with module 28):

```powershell
# 1. Create build folder and enter it
mkdir build && cd build

# 2. Configure CMake (using MinGW toolchain and vcpkg)
cmake -S .. -B . -DCMAKE_TOOLCHAIN_FILE=toolchain-mingw.cmake -DVCPKG_ROOT="C:/path/to/vcpkg/installed/x64-windows"

# 3. Compile
cmake --build . -j4

# 4. Run the executable (example lesson 01)
.\leccion_01_criterios_seleccion.exe
```

If all goes well, the SDL3 window should open and the program should close when you close the window.

### 4.1 Probar la verificación (`verificar.sh`)

The original `verificar.sh` is a **bash** script and assumes a Linux environment. In Windows you can:

- **Run it inside MSYS2** (if you have it installed) – it will behave like Linux.
- **Translate the logic to PowerShell or a `.bat` script** – the essential part is compiling each solution and checking whether the binary fails or passes. A simplified example:

```powershell
# verify.ps1 (example)
$mod = "28"
$tmp = "$env:TEMP\verify_$mod"
Remove-Item $tmp -ErrorAction SilentlyContinue
$solutions = Get-ChildItem "$mod/soluciones/*.cpp"
foreach ($sol in $solutions) {
    $base = [System.IO.Path]::GetFileNameWithoutExtension($sol.Name)
    $bin = "$tmp\$base.exe"
    # Compile
    g++ -std=c++20 -Wall -Wextra -I..\include $sol.FullName -o $bin
    # Run with 10‑second timeout
    $null = & timesout 10 $bin
    if ($LastExitCode -eq 0) {
        Write-Host "$base: OK_SOL"
    } else {
        Write-Host "$base: FALLA test"
    }
}
```

Adjust the paths according to your installation.

> **Recommendation:** If you already have MSYS2 installed (see section 1), simply run `./verificar.sh 28` from the MSYS2 terminal; no need to adapt the script.

---

## 5. Expected folder structure after installation

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
├─ include/                   ← own course headers
├─ verify.sh                  ← bash script (run in MSYS2 or translate)
├─ CMakeLists.txt             ← root (optional, includes modules)
├─ PROGRESO.md
├─ LEEME_PRIMERO.md
├─ DECISIONES.md
├─ README_curso.md            ← course description
├─ README_windows.md          ← Windows installation guide
├─ assets/                    ← optional, course assets
```

---

## 6. Common problem‑solving

| Problem | Cause | Solution |
|---|---|---|
| `g++: command not found` | MinGW not in PATH | Add MinGW `bin` folder to Windows `PATH` or use the MinGW terminal. |
| `cannot find -lSDL3` | SDL3 not linked | Install SDL3 via vcpkg or add the `SDL3.lib`/`.a` and pass `-lSDL3` on the command line. |
| `ImGui no compiles` | Missing `IMGUI_IMPL_OPENGL_LOADER_GL3` or old headers | Use version `v1.91.5-docking` and follow the `CMakeLists.txt`; the script already includes `ImGui_ImplSDL3_InitForOpenGL(window)`. |
| `Box2D errors undefined reference` | Library not linked | If using vcpkg: `target_link_libraries(m28_solucion box2d)`. If manual, add the `.a`/`.lib` path and `-lbox2d`. |
| Program closes instantly | SDL or OpenGL context not initialized | Ensure `main` calls `SDL_Init(SDL_INIT_VIDEO)` and `SDL_CreateWindowAndRenderer` (or `SDL_GPU_CreateDevice`) before any render. |
| `verificar.sh` reports “SALTA: libreria faltante” | Missing library or version mismatch | Install the library indicated (via vcpkg or manual) and re‑run. |

---

## 6. Quick “all‑in‑one” steps

```powershell
# 1. Install MinGW‑w64 and add to PATH
# 2. Install vcpkg and run:
   .\vcpkg\install sdl3 glm imgui box2d miniaudio
# 3. Clone the repo
git clone ... && cd curso_cpp
# 4. Configure and compile a module (example M28):
mkdir build && cd build
cmake -S .. -B . -DCMAKE_TOOLCHAIN_FILE=toolchain-mingw.cmake -DVCPKG_ROOT="C:/path/to/vcpkg/installed/x64-windows"
cmake --build . -j4
.\leccion_01_criterios_seleccion.exe   # test the window
# 5. (optional) Run tests:
#    launch `./verify.sh 28` inside MSYS2
```

With these steps you’ll have the course running on Windows without WSL or a full Linux install. If you encounter specific difficulties, see the “Problem‑solving” section or open an issue in the repository.

---
*This file forms part of the course delivery; together with `README_curso.md` and the modules 01‑28 it constitutes the complete documentation to get started.*