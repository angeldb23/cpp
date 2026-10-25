# MÓDULO 16 — QUIZ: Arquitectura de Juegos: Game Loop y Timesteps

Respuestas al final.

---

## Preguntas Conceptuales

**1. ¿Cuál es la diferencia fundamental entre un game loop con timestep variable y uno con timestep fijo?**
A) El variable usa `GetFrameTime()`, el fijo usa `std::chrono`
B) El variable pasa `dt` real a Update; el fijo pasa siempre el mismo `fixedDt` y usa un acumulador
C) El variable corre a 60 FPS; el fijo corre a 30 FPS
D) No hay diferencia práctica

**2. En el patrón de "Fix Your Timestep" (Gaffer on Games), ¿para qué sirve el acumulador (accumulator)?**
A) Guardar la posición anterior del jugador para interpolación
B) Acumular frame times y ejecutar tantos steps de física fijos como quepan
C) Contar cuántos frames han pasado desde el inicio
D) Limitar el FPS máximo

**3. ¿Qué es `alpha` en la interpolación de render y qué rango tiene?**
A) Opacidad del fade; [0, 255]
B) Factor de interpolación entre tick anterior y actual; [0, 1)
C) Tiempo transcurrido en segundos; [0, ∞)
D) Velocidad de la animación; [0, 1]

**4. ¿Por qué `std::chrono::steady_clock` es preferible a `system_clock` para game loops?**
A) Es más rápido
B) Es monótono: nunca salta hacia atrás por ajustes de reloj (NTP, usuario)
C) Tiene mayor resolución
D) Es el único que soporta `duration<float>`

**5. ¿Cuál es el propósito del "clamp" del frame time (ej. `fminf(dt, 0.25f)`)?**
A) Limitar el FPS a 4
B) Evitar "spiral of death": si un frame tarda mucho (debugger, alt-tab), no ejecutar miles de steps de física de golpe
C) Forzar timestep fijo
D) Sincronizar con VSync

**6. En una pila de estados (State Stack), ¿qué ocurre al hacer `Push(PauseState)` sobre `PlayingState`?**
A) `PlayingState` se destruye y `PauseState` toma su lugar
B) `PlayingState.Exit()`, luego `PauseState.Enter()`; `PlayingState` queda debajo, pausado
C) Ambos corren en paralelo
D) `PauseState` reemplaza a `PlayingState` en la pila

**7. ¿Qué ventaja tiene el mapeo de acciones (Action Mapping) sobre leer teclas directamente en el juego?**
A) Es más rápido
B) Permite reconfigurar teclas, soportar gamepad/teclado simultáneo, testing sin hardware, replay y network
C) Usa menos memoria
D) Es obligatorio en raylib

**8. En Struct of Arrays (SoA), ¿por qué `UpdatePhysics` itera solo `pos` y `vel` y no toda la entidad?**
A) Porque `health` y `type` no existen en SoA
B) Cache locality: trae solo los datos que usa (16 bytes/entidad vs 48+ en AoS), mejor aprovechamiento de cache lines
C) Porque el compilador lo optimiza automáticamente
D) SoA no permite acceder a otros campos

**9. ¿Qué es "false sharing" y cómo se evita en contadores atómicos por hilo?**
A) Dos hilos leen la misma variable; se evita con `mutex`
B) Dos hilos escriben en variables distintas pero en la misma cache line (64B), causando ping-pong; se evita con `alignas(64)` o padding
C) Un hilo escribe mientras otro lee; se evita con `atomic`
D) Compartir punteros entre hilos; se evita con `shared_ptr`

**10. ¿Qué hace `__builtin_prefetch(&entities[i+1].hot, 0, 3)` en un loop de update?**
A) Carga la entidad siguiente en L1 cache antes de necesitarla (read, alta temporalidad)
B) Guarda la entidad en disco
C) Marca la entidad para recolección de basura
D) Fuerza escritura inmediata a RAM

**11. En un Resource Manager con handles RAII y reference counting, ¿cuándo se llama al `Unload`?**
A) Al hacer `Clear()` del manager
B) Cuando el último handle (copia o original) se destruye y el refcount llega a 0
C) Al cambiar de estado
D) Cada frame

**12. ¿Qué métrica revela problemas que el FPS promedio oculta?**
A) Frame time mínimo
B) Percentiles (p95, p99): muestran la cola de distribución (spikes)
C) Frame time máximo
D) FPS instantáneo

**13. En la interpolación de render con timestep fijo, ¿qué posición se usa cuando `alpha ≈ 0`?**
A) `prevPos` (tick anterior)
B) `currPos` (tick actual)
C) Promedio de ambas
D) Posición inicial

**14. ¿Por qué `duration<float>` y no `duration<double>` para game loops?**
A) `float` es más preciso
B) Consistencia con GPU/raylib (Vector2, Matrix usan float), evita casts y warnings; precisión suficiente para ms
C) `double` no compila en C++20
D) `float` es más rápido en CPU

**15. ¿Cuál es el orden correcto de miembros en un struct para minimizar padding?**
A) Alfabético
B) Grande a pequeño (double → int → char → char)
C) Pequeño a grande (char → char → int → double)
D) Agrupados por tipo semántico

**16. En SoA con swap-with-last para eliminar entidades, ¿por qué NO se incrementa el índice `i` tras un `RemoveAt(i)`?**
A) Porque el elemento movido desde el final a `i` aún no se ha procesado
B) Porque `count` ya decrementó
C) Para evitar saltar elementos
D) Ambas A y C

**17. ¿Qué información guarda un `FadeTransition` en su callback `onMid`?**
A) El color del fade
B) La acción a ejecutar exactamente en el midpoint (alpha=1), típicamente el cambio real de estado
C) La duración total
D) El estado anterior

**18. ¿Qué hace `std::chrono::duration_cast<std::chrono::milliseconds>(seconds(1.5))`?**
A) Redondea a 1500 ms
B) Trunca a 1500 ms (hacia cero)
C) Error de compilación
D) Devuelve 1.5 ms

**19. En el game loop con timestep fijo, ¿por qué se guarda `prevState` antes de cada `Update`?**
A) Para poder hacer undo
B) Para interpolar render entre `prevState` y `currState` usando `alpha`
C) Para detectar cambios
D) Para logging

**20. ¿Cuál es la diferencia entre `IsKeyDown`, `IsKeyPressed` y `IsKeyReleased` en raylib?**
A) Ninguna, son alias
B) Down = true mientras se sostiene; Pressed = true SOLO frame de bajada; Released = true SOLO frame de subida
C) Down = evento; Pressed = estado; Released = callback
D) Solo existe `IsKeyDown`

---

## Preguntas de Análisis de Código

**21. ¿Qué imprime este código?**
```cpp
using namespace std::chrono_literals;
auto dt = 16ms + 4ms;  // milliseconds
std::chrono::duration<float> dt_f = dt;
std::cout << dt_f.count();
```
A) 20
B) 0.02
C) 20.0
D) Error de compilación

**22. Dado este State Manager, ¿cuál es la salida?**
```cpp
StateManager sm;
sm.Push(Menu);    // Enter Menu
sm.Push(Playing); // Exit Menu, Enter Playing
sm.Pop();         // Exit Playing, Enter Menu
sm.Pop();         // Exit Menu
```
A) Enter Menu, Enter Playing, Exit Playing, Enter Menu, Exit Menu
B) Enter Menu, Exit Menu, Enter Playing, Exit Playing, Enter Menu, Exit Menu
C) Enter Menu, Enter Playing, Exit Menu, Exit Playing
D) Enter Menu, Enter Playing, Exit Playing, Exit Menu

**23. En este SoA, ¿qué valor tiene `p.Count()` tras el Update?**
```cpp
ParticleSoA p;
p.Add(0,0, 0,0, 0.05f); // life 50ms
p.Update(0.01f);        // 10ms
p.Update(0.05f);        // 50ms más
```
A) 1
B) 0
C) 2
D) Undefined behavior

**24. ¿Qué devuelve `Percentile(50)` en un FrameProfiler con samples [10, 20, 30, 40, 50]?**
A) 30 (mediana)
B) 30.0
C) 20
D) 50

**25. En este ResourceHandle, ¿cuántas veces se llama a `UnloadTextureSim`?**
```cpp
ResourceManager rm;
{
    auto h1 = rm.GetOrLoad("a.png");
    auto h2 = h1;
} // scope ends
rm.Clear();
```
A) 0
B) 1 (en Clear)
C) 2
D) 3

---

## Respuestas

1.  **B** — Timestep variable pasa dt real; fijo usa acumulador + fixedDt constante.
2.  **B** — Acumula frameTime y ejecuta `while (accumulator >= fixedDt) { Update(fixedDt); accumulator -= fixedDt; }`.
3.  **B** — Alpha = accumulator / fixedDt; rango [0, 1). 0 = tick anterior, ~1 = tick actual.
4.  **B** — `steady_clock` es monótono; `system_clock` puede saltar (NTP, cambio hora manual).
5.  **B** — Sin clamp, un spike de 200ms con fixedDt=16ms ejecutaría 12 steps de física de golpe → spiral of death.
6.  **B** — Push llama Exit al anterior y Enter al nuevo; el anterior queda debajo (pause over game).
7.  **B** — Desacopla input físico de lógica de juego; reconfigurable, multi-device, testable, replay, network.
8.  **B** — SoA: arrays contiguos por campo. Update physics solo toca pos/vel → cache lines llenas de datos útiles.
9.  **B** — False sharing: cores invalidan cache line compartida. Solución: `alignas(64)` separa a líneas distintas.
10. **A** — Prefetch read (0), alta temporalidad (3) → trae siguiente `hot` a L1 antes de usarlo.
11. **B** — RAII: último destructor (refcount=0) llama deleter. Cache con `shared_ptr` o `weak_ptr` según política.
12. **B** — Promedio esconde spikes. p99 = peor 1% revela frames lentos percibidos como lag.
13. **B** — Alpha≈0 significa "acabamos de hacer un tick", render = estado actual (no previo).
14. **B** — GPU/raylib usan float. `duration<float>` evita narrowing y matches API gráfica.
15. **B** — Ordenar por alineación descendente minimiza padding (double 8, int 4, char 1, char 1 = 16B vs 24B).
16. **D** — Swap mueve último a `i`; ese elemento nuevo en `i` debe procesarse en misma iteración.
17. **B** — `onMid` se ejecuta en alpha=1 (FadeOut completo), justo antes de FadeIn. Ideal para `ChangeState`.
18. **B** — `duration_cast` trunca hacia cero. 1.5s = 1500ms exacto aquí, pero 1.501s → 1500ms.
19. **B** — Interpolación: `render = Lerp(prev, curr, alpha)`. Sin prevState no hay de dónde interpolar.
20. **B** — Down: continuo; Pressed: edge rising; Released: edge falling.

21. **B** — `16ms + 4ms = 20ms` (milliseconds). `duration<float>` convierte a segundos: 0.02.
22. **A** — Push(Menu): Enter Menu. Push(Playing): Exit Menu, Enter Playing. Pop(): Exit Playing, Enter Menu. Pop(): Exit Menu.
23. **B** — Life inicial 0.05. Tras 0.01: life=0.04 (viva). Tras 0.05: life=-0.01 → muere, swap-remove, count=0.
24. **A** — Mediana de 5 elementos ordenados = elemento central (índice 2) = 30.
25. **B** — `h1` y `h2` comparten ownership (refcount=2). Al salir del scope, ambos destruidos → refcount=0 → unload. `Clear()` ve cache vacía (weak/expired) o shared ya destruido → 1 unload total.