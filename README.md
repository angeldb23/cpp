# 🎓 Curso de C++ y Desarrollo de Juegos (Módulos 01‑28)

## ¿Qué es este curso?
Un recorrido estructurado de **28 módulos** que lleva al estudiante desde los fundamentos de C++ hasta la construcción de un mini motor 2D y un juego de plataformas. Cada módulo está diseñado para que se **aprenda haciendo**: se presentan lecciones con código comentado, ejercicios que deben resolverse (los tests fallan hasta que los resuelva) y una solución que pasa los tests. Al final de cada módulo hay un **QUIZ.md** con preguntas de repaso.

## Estructura del curso
- **Módulos numerados del 01 al 28**. Cada módulo vive en su carpeta `01_fundamentos`, `02_funciones`, … `28_estudio_codigo_real`.
- Dentro de cada módulo:
  - `lecciones/` – archivos `.cpp`/`.hpp` que explican el tema.
  - `ejercicios/` – pequeños problemas de lógica pura con `test_framework.hpp`.
  - `soluciones/` – implementaciones completas que deben pasar los tests.
  - `QUIZ.md` – 20‑38 preguntas de opción múltiple o respuesta corta.
- **Herramientas comunes**:
  - Compilador **g++** (C++20) o **MinGW‑w64** en Windows.
  - **CMake** 3.20+ para compilar y generar los tests (CTest).
- **Dependencias externas** (sólo en módulos avanzados):
  - GLM (matemáticas), Dear ImGui (interfaz), Box2D 2.4.x (física), miniaudio (audio), SDL3 (ventana + input + audio + GPU).

## ¿Qué se espera que el usuario aprenda?
- **C++ moderno** (C++17/20): tipos fuertes, plantillas, RAII, punteros inteligentes, conceptos.
- **Arquitectura de software**: separación entre motor, juego y herramientas; uso de fachadas y encapsulamiento.
- **Programación de bajo nivel**: manejo de memoria, punteros, estructuras de datos lineales y árboles.
- **Desarrollo de juegos 2D**: bucle de juego, entrada de teclado/mouse, renderizado con OpenGL 3.3 core (o SDL_GPU), física básica (AABB/SAT o Box2D), audio con miniaudio, UI con Dear ImGui.
- **Testing y calidad**: escribir tests unitarios que fallen sin la solución, usar sanitizers (ASan/UBSan), compilar con `-Wall -Wextra`.
- **Gestión de dependencias**: usar CMake + FetchContent para traer librerías con tags verificados; nunca usar “master”.

## Cómo está organizado el trabajo
1. **Leer la lección** – entender el código y los conceptos.
2. **Intentar el ejercicio** – los tests están pensados para fallar; el objetivo es hacerlos pasar.
3. **Comparar con la solución** – aprender las buenas prácticas y los trucos de la librería.
4. **Resolver el QUIZ** – verificar la asimilación teórica.
5. **Ejecutar `./verificar.sh <modulo>`** – obtener un informe rápido (OK_SOL, OK_EJ, fallos, saltos).
6. **Actualizar `PROGRESO.md` y `LEEME_PRIMERO.md`** – llevar la cuenta de lo completado.

## Normas y reglas del curso
- **No hay código de solución entregado previamente**; cada uno debe escribir su propia solución basándose en la lección.
- **Los tests deben fallar sin resolver**; un ejercicio que ya pasa sin resolver se considera un error.
- **Usar CMake** para compilar; el script `verificar.sh` usa `g++ -std=c++20 -Wall -Wextra -Iinclude`.
- **Dependencias**: los módulos 16‑28 introducen librerías externas; se descargan con CMake FetchContent y tags fijos (nunca `master`).
- **Comportamiento en Windows**: el curso está probado en Linux, pero el código es multiplataforma; en Windows se puede usar MinGW‑w64 o Visual Studio con las mismas librerías (SDL3, GLM, ImGui, Box2D, miniaudio). Véase `README_windows.md` para instrucciones detalladas.

## Evaluación
- Cada módulo se verifica con `./verificar.sh <NN>`.
- El resultado muestra: número de soluciones OK, ejercicios OK, avisos, saltos y fallos.
- Un módulo se considera **completo** cuando `verificar.sh` devuelve `VERIFICACION_OK` (0 fallas) y los tests de CTest pasan.
- El curso se considera **terminado** cuando todos los módulos 01‑28 muestran ese resultado.

## Cómo comenzar
1. Clonar el repositorio.
2. Ejecutar `./estado.sh` para ver el estado actual.
3. Empezar por el módulo 01 y avanzar en orden; cada módulo depende de los anteriores.
4. Si se queda atascado, revisar la lección, la solución y los tests; luego intentar de nuevo.
5. Cuando termine los 28 módulos, habrá cubierto desde C++ básico hasta la construcción de un mini motor 2D y un juego de plataformas.

## Contacto / Dudas
- Revisar los archivos `DECISIONES.md` y `TEMAS.md` para decisiones técnicas.
- Abrir un issue en el repositorio o consultar al profesor/a.

---
*Este archivo forma parte de la entrega del curso; junto con `README_windows.md` y los módulos 01‑28 constituye la entrega completa*.
