# Quiz — Módulo 15: CMake Profesional, Sanitizers, Cache y Rendimiento

Respondé primero sin mirar las respuestas del final.

---

**1. En CMake moderno, `target_include_directories(mi_lib PUBLIC include/)` hace que:**

- a) Solo `mi_lib` vea los headers en `include/`
- b) `mi_lib` Y sus consumidores vean los headers en `include/`
- c) Solo los consumidores vean los headers (mi_lib no)
- d) No hace nada, es sintaxis inválida

**2. La diferencia entre `PUBLIC`, `PRIVATE` e `INTERFACE` en `target_link_libraries`:**

- a) Son sinónimos, funcionan igual
- b) `PUBLIC` propaga a consumidores, `PRIVATE` no, `INTERFACE` solo para header-only
- c) `PRIVATE` propaga, `PUBLIC` no
- d) Solo existe `PUBLIC`

**3. Para una librería header-only (como nlohmann/json), el scope correcto es:**

- a) PUBLIC
- b) PRIVATE
- c) INTERFACE
- d) No se usa target_link_libraries

**4. `FetchContent` vs `find_package` — diferencia clave:**

- a) Son lo mismo
- b) `FetchContent` descarga fuentes en configure time; `find_package` busca instalado
- c) `find_package` descarga de GitHub
- d) `FetchContent` solo funciona con vcpkg

**5. En `CMakePresets.json`, la sección `configurePresets` define:**

- a) Cómo compilar (build)
- b) Cómo configurar (cmake): generator, binaryDir, cacheVariables
- c) Cómo testear (ctest)
- d) Solo el nombre del proyecto

**5. `ccache` acelera compilaciones porque:**

- a) Compila en paralelo automáticamente
- b) Cachea resultados de compilación y reusa si inputs son idénticos
- c) Usa un compiler más rápido
- d) Elimina warnings

**7. `target_precompile_headers(mi_lib PRIVATE <vector> "pch.h")` sirve para:**

- a) Precompilar headers pesados y reducir tiempo de compilación
- b) Instalar headers
- c) Generar documentation
- d) Hacer link time optimization

**8. Unity Builds (`UNITY_BUILD ON`) unen varios `.cpp` en uno. ¿Cuál es un riesgo?**

- a) Siempre más lento
- b) Rompe `anonymous namespace` y variables `static` en .cpp
- c) No funciona con C++
- d) Solo funciona en MSVC

**9. `mold` linker es más rápido que `lld`/`gold` porque:**

- a) Está escrito en Rust y usa paralelismo agresivo
- b) Es mágico
- c) Solo funciona en Windows
- d) Elimina optimizaciones

**10. `-fsanitize=address,undefined` habilita:**

- a) Solo AddressSanitizer
- b) Solo UndefinedBehaviorSanitizer
- c) AddressSanitizer + UndefinedBehaviorSanitizer (compatibles)
- d) ThreadSanitizer

**10. ThreadSanitizer (`-fsanitize=thread`) detecta:**

- a) Memory leaks
- b) Data races (race conditions) y deadlocks
- c) Buffer overflows
- d) Division by zero

**11. `perf record -g ./exe` graba:**

- a) Solo tiempo total
- b) Perfil estadístico con call graph (-g)
- c) Solo memory leaks
- d) Solo CPU cycles

**11. En Google Benchmark, `BENCHMARK(func)->Range(8, 1024)->RangeMultiplier(2)` prueba:**

- a) Solo 8 y 1024
- b) 8, 16, 32, 64, 128, 256, 512, 1024 (x2 cada step)
- c) Todos los números entre 8 y 1024
- d) Solo potencias de 10

**12. `perf stat ./exe` muestra por defecto:**

- a) Solo tiempo
- b) cycles, instructions, cache-references, cache-misses, branch-misses, IPC
- c) Solo memory leaks
- d) Solo function names

**13. `ccache` acelera rebuilds porque:**

- a) Usa más RAM
- b) Cachea resultados de compilación y reusa si source+flags+compiler son idénticos
- c) Compila en GPU
- d) Desactiva optimizaciones

**13. `target_precompile_headers(mi_lib PRIVATE <vector> "pch.h")` requiere:**

- a) CMake 3.10+
- b) CMake 3.16+
- c) CMake 3.20+
- d) Solo funciona en MSVC

**14. False sharing ocurre cuando:**

- a) Dos hilos leen la misma variable
- b) Dos hilos escriben en variables DISTINTAS que caen en la MISMA cache line (64 bytes)
- c) Un hilo lee y otro escribe la misma variable
- d) Un hilo accede a memoria liberada

**14. Para evitar false sharing en contadores por hilo:**

- a) Usar `std::mutex`
- b) `alignas(64)` en cada contador (cache line = 64 bytes)
- c) `std::atomic` solo
- d) No se puede evitar

**15. AoS vs SoA — Structure of Arrays (SoA) es mejor para cache porque:**

- a) Usa menos memoria
- b) Acceso contiguo al iterar un solo campo (stride = tamaño del campo, no del struct completo)
- c) Es más bonito
- d) Funciona solo en C

**15. Para evitar copias innecesarias de strings en parámetros read-only:**

- a) `const std::string&`
- b) `std::string_view` (zero-copy, no dueño)
- c) `std::string&&`
- d) `const char*`

---

## Respuestas

1.  **b** — `PUBLIC` propaga a consumidores (interface + implementación).
2.  **b** — `PUBLIC` propaga, `PRIVATE` no, `INTERFACE` solo interface (header-only).
3.  **c** — `INTERFACE` para header-only (no hay binario que linkear).
4.  **b** — `FetchContent` descarga en configure time; `find_package` busca instalado.
5.  **b** — `configurePresets` = configuración (generator, binaryDir, cacheVariables).
6.  **b** — Cachea compilaciones previas; reusa si source+flags+compiler idénticos.
7.  **a** — Precompila headers pesados; reduce tiempo de compilación.
8.  **b** — Rompe `anonymous namespace` / `static` en .cpp (compartidos en unity build).
9.  **a** — Escrito en Rust, paralelismo agresivo, 2-5x más rápido que lld/gold.
10. **c** — ASan + UBSan son compatibles y complementarios.
11. **b** — Data races (condiciones de carrera) y deadlocks.
12. **b** — Perfil estadístico con call graph (`-g`).
13. **b** — `RangeMultiplier(2)` multiplica por 2 cada step.
14. **b** — cycles, instructions, cache-references, cache-misses, branch-misses, IPC.
15. **b** — Cachea compilaciones previas; reusa si inputs idénticos.
16. **b** — CMake 3.16+ introdujo `target_precompile_headers()`.
16. **b** — Hilos escriben en vars distintas pero misma cache line (64B) -> ping-pong MESI.
17. **b** — `alignas(64)` separa contadores en cache lines distintas.
17. **b** — SoA: stride = sizeof(field), acceso contiguo al iterar un campo.
18. **b** — `std::string_view`: zero-copy view, no dueño, ideal para read-only params.

---

**Puntaje:**
- 14-15: Excelente — dominás CMake moderno, sanitizers, profiling y DOD
- 11-13: Muy bien — repasá sanitizers y data-oriented design
- 8-10: Bien — reforzá CMake moderno y profiling
- <8: Releé el módulo 15 completo y repetí los ejercicios