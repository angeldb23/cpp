# Quiz — Módulo 14: Concurrencia y Multithreading Moderno

Respondé primero sin mirar las respuestas del final.

---

**1. La diferencia fundamental entre `std::thread` y `std::jthread` (C++20) es:**

- a) `jthread` es más rápido porque usa un pool interno
- b) `jthread` hace `join()` automático en su destructor y trae `stop_token` integrado
- c) `jthread` no permite `detach()`
- d) `jthread` solo existe en Windows

**2. ¿Qué garantiza `memory_order_relaxed` en un `std::atomic`?**

- a) Atomicidad + orden total global (seq_cst)
- b) Solo atomicidad, SIN ordenamiento respecto a otras variables
- c) Acquire-release automático
- d) Que la operación nunca falla (no es CAS)

**3. El patrón acquire-release entre dos hilos (publicador/suscriptor) requiere:**

- a) Publicador: `store(release)`, Suscriptor: `load(acquire)`
- b) Publicador: `load(acquire)`, Suscriptor: `store(release)`
- c) Ambos usan `seq_cst`
- d) Usar `mutex` en ambos lados

**4. `std::condition_variable::wait(lk, pred)` vs `wait(lk)` — la diferencia crítica es:**

- a) `wait(lk, pred)` es más lento
- b) `wait(lk)` despierta solo con `notify_one`, `wait(lk, pred)` con `notify_all`
- c) `wait(lk, pred)` reevalúa el predicado tras cada wakeup (evita spurious wakeups), `wait(lk)` NO
- d) No hay diferencia, son alias

**5. Para evitar deadlock al tomar DOS mutexes simultáneamente, la herramienta correcta (C++17) es:**

- a) `std::lock_guard(m1, m2)`
- b) `std::scoped_lock(m1, m2)` — toma ambos atómicamente
- c) `std::unique_lock(m1); std::unique_lock(m2);`
- d) `std::lock(m1); std::lock(m2);`

**6. `std::async(std::launch::async, f)` vs `std::async(f)` (policy por defecto):**

- a) Son equivalentes
- b) `launch::async` GARANTIZA hilo nuevo; el default deja a la implementación elegir (puede ser deferred)
- c) El default es más rápido porque usa thread pool
- d) `launch::async` está deprecado en C++20

**7. `std::packaged_task<void()>` sirve para:**

- a) Ejecutar tareas en un thread pool sin type-erasure
- b) Type-erasure de callables: guarda CUALQUIER callable como `void()` y provee `future`
- c) Reemplazar `std::function` en todos los casos
- d) Solo funciona con lambdas sin captura

**8. `std::latch` vs `std::barrier` (C++20) — la diferencia principal:**

- a) `latch` es reutilizable, `barrier` es single-shot
- b) `latch` es countdown single-shot; `barrier` es reutilizable multi-fase con `arrive_and_wait()`
- c) `latch` usa mutex interno, `barrier` es lock-free
- d) Son iguales, `barrier` es nombre nuevo

**9. `std::counting_semaphore<N>` — ¿qué hace `acquire()`?**

- a) Incrementa el contador en 1
- b) Decrementa el contador; si llega a 0, bloquea hasta que otro `release()` lo suba
- c) Resetea el contador a N
- d) Devuelve el valor actual sin modificar

**10. `thread_local` — ¿cuándo se inicializa y destruye la variable?**

- a) Inicialización: al arrancar el programa; destrucción: al terminar el programa
- b) Inicialización: primera vez que el hilo accede (thread-safe); destrucción: al terminar ESE hilo
- c) Inicialización: en el constructor del thread; destrucción: en `join()`
- d) Nunca se destruye: memory leak intencional

**10. `std::shared_future` vs `std::future` — diferencia clave:**

- a) `shared_future` es más rápido
- b) `future` es move-only, `get()` consume el valor; `shared_future` es copiable, `get()` devuelve `const ref` y se puede llamar múltiples veces
- c) `shared_future` solo existe en C++23
- d) `future` permite múltiples lectores, `shared_future` solo uno

**11. Para evitar FALSE SHARING en un contador compartido por múltiples hilos:**

- a) Usar `std::atomic<long long>` normal
- b) Usar `alignas(64)` en cada contador por hilo (cache line = 64 bytes)
- c) Usar `mutex` en lugar de atomic
- d) False sharing no existe en x86

**11. Copy-on-Write con `shared_ptr<const T>` + `atomic` publish (release/acquire):**

- a) Requiere mutex para lectores
- b) Lectores hacen `load(acquire)` y leen SIN locks; writer crea nueva copia + `store(release)`
- c) Solo funciona con `int`, no con structs
- d) Requiere `shared_mutex`

**12. `std::jthread` + `stop_token` para cancelación cooperativa:**

- a) `request_stop()` mata el hilo instantáneamente (como `pthread_cancel`)
- b) `request_stop()` pone flag; el hilo DEBE chequear `stop_requested()` periódicamente y salir limpio
- c) `stop_token` se usa solo para `condition_variable`
- d) `jthread` no soporta cancelación

**13. `std::execution::par` en algoritmos STL (C++17) requiere:**

- a) Iteradores forward_iterator
- b) Iteradores random_access_iterator + operaciones element-wise sin data races
- c) Solo funciona con `vector<int>`
- d) Requiere TBB instalado obligatoriamente (en GCC/libstdc++)

**14. Ley de Amdahl vs Ley de Gustafson — escalado fuerte vs débil:**

- a) Amdahl: problema fijo, más hilos; Gustafson: problema crece con hilos
- b) Son la misma ley con nombres distintos
- c) Amdahl aplica solo a GPUs, Gustafson a CPUs
- d) Gustafson predice speedup lineal infinito

**15. Checklist anti-data-race — ¿qué NO es necesario?**

- a) Proteger toda variable compartida mutable con mutex/atomic
- b) Usar `memory_order_relaxed` para publicar datos que otros leen con acquire
- c) Predicados en `cv.wait(lk, pred)` SIEMPRE
- d) Evitar `detach()` sin plan de limpieza

---

## Respuestas

1.  **b** — `jthread` = thread + join automático en dtor + stop_token integrado.
2.  **b** — `relaxed` SOLO atomicidad, sin ordenamiento (solo para contadores/stats).
3.  **a** — Publicador `release` "publica" writes previos; suscriptor `acquire` los "ve".
4.  **c** — `wait(lk, pred)` maneja spurious wakeups reevalúan pred; `wait(lk)` NO.
5.  **b** — `scoped_lock` toma múltiples mutexes atómicamente (evita deadlock).
6.  **b** — Default policy es implementation-defined (puede ser deferred); `launch::async` fuerza hilo nuevo.
7.  **b** — `packaged_task` type-erasure: callable arbitrario → `future` + `void()` uniforme.
8.  **b** — `latch` = countdown 1 uso; `barrier` = multi-fase reutilizable con `arrive_and_wait()`.
9.  **b** — `acquire()` = P (wait): decrementa, bloquea si < 0; `release()` = V (signal): incrementa.
10. **b** — `thread_local`: init on first use por hilo, dtor al terminar el hilo.
11. **b** — `future` move-only + `get()` consume; `shared_future` copiable + `get()` = `const ref` reentrante.
12. **b** — `alignas(64)` separa contadores en cache lines distintas (evita ping-pong MESI).
13. **b** — COW: writer `make_shared<const>` + `store(release)`; lectores `load(acquire)` + leer sin locks.
14. **b** — `request_stop()` = flag cooperativo; hilo chequea `stop_requested()` y sale limpio.
15. **b** — `std::execution::par` requiere random_access_iterators + element-wise sin races; en GCC necesita `-ltbb`.
16. **a** — Amdahl: strong scaling (problema fijo); Gustafson: weak scaling (problema crece).
17. **b** — `relaxed` NO sincroniza otras variables; para publicar datos hace falta release/acquire.

---

**Puntaje:**
- 14-15: Excelente — dominás concurrencia moderna
- 11-13: Muy bien — repasá memory ordering y sync primitives
- 8-10: Bien — reforzá RAII locks, futures, thread_local
- <8: Releé el módulo 14 completo y repetí los ejercicios