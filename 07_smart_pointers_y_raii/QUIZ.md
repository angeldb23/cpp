# Quiz — Módulo 07: Smart Pointers, RAII y Ciclos de Vida

Respondé primero sin mirar las respuestas del final. Cada pregunta tiene UNA opción correcta.

---

**1. ¿Qué significa exactamente RAII?**

- a) Reservar memoria antes del main y liberarla al final
- b) La adquisición del recurso ES la inicialización de un objeto y la liberación corre con su destructor
- c) Un recolector de basura de C++ muy rápido
- d) Un patrón para evitar constructores

**2. Al salir del bloque `{ Buffer a; Buffer b; } ¿en qué orden se destruyen?`

- a) a, luego b (orden de creación)
- b) b, luego a (reversa)
- c) Depende del optimizador
- d) Se destruyen a la vez

**3. En una clase con tres miembros declarados (texturas_, buffer_, shaders_), el destructor generado...**

- a) Corre el cuerpo primero; luego destruye miembros en orden inverso a su declaración
- b) Destruye los miembros primero y después el cuerpo
- c) Destruye los miembros en el orden de la lista de inicialización
- d) Depende del algoritmo de allocación

**4. Una excepción cruza una función que tiene un objeto RAII local. El destructor...**

- a) NO corre: el recurso queda fugado mientras no se atrape
- b) Corre igual (stack unwinding) cuando el frame se desarma
- c) Corre solo si atrapás por valor
- d) Corre al terminar el main siempre

**5. `sizeof(std::unique_ptr<T>)` en 64 bits suele ser...**

- a) 16 bytes
- b) 8 bytes: igual que un puntero crudo
- c) 4 bytes
- d) Depende de sizeof(T)

**6. ¿Cuál NO compila y por qué?**

- a) `std::unique_ptr<T> copia = otro;` → no se puede copiar (propiedad exclusiva)
- b) `otro->metodo();` → no existe la flecha
- c) `*otro = T{};` → no se puede desreferenciar
- d) `otro.reset();` → reset no existe

**7. `p.get()` en un unique_ptr devuelve...**

- a) El puntero crudo, que sirve para observar (NO para liberar ni para hacer otro dueño)
- b) Una copia dueña del recurso
- c) El control block
- d) Un weak_ptr

**8. En `vector<std::unique_ptr<Enemigo>>, `erase(v.begin())`...**

- a) Solo suelta el puntero: el Enemigo sigue vivo en memoria
- b) Destruye el Enemigo en ese momento (el unique_ptr del slot muere y libera)
- c) Mueve el Enemigo al final y lo libera al salir del scope
- d) No compila: los contenedores no admiten unique_ptr

**9. El patrón para pasarle "la única textura" a una función que la DIBUJA es...**

- a) `const std::unique_ptr<Textura>&`
- b) `std::unique_ptr<Textura> copiaPorValor` (siempre)
- c) devolver un `Textura` por valor
- d) `std::weak_ptr<Textura>` obligatoriamente

**10. Las firmas del módulo dicen: `const T*` / `T*` crudo significa...**

- a) que la función es dueña del recurso y lo libera al final
- b) observación o uso transitorio, sin propiedad; puede ser nullptr en la variante opcional
- c) que el recurso no puede morir jamás
- d) un bug de diseño siempre

**11. Un shared_ptr puede informar use_count == 3 para un mismo objeto. ¿Qué afirmación es correcta?**

- a) El contador es global del proceso: comparten todos los shared_ptr de la app
- b) Cada par copia del MISMO origen cuenta un dueño: 1 original + 2 copias del mismo bloque
- c) shared_ptr no tiene contadores
- d) Es UB: los shared_ptr no se copian

**12. `shared_ptr<T> p(new T(...))` frente a `make_shared<T>(...)`: la diferencia clave es...**

- a) Ninguna
- b) make_shared hace UNA allocación (objeto + bloque juntos) y elimina la ventana de fuga entre new y la adopción
- c) make_shared es más lento siempre
- d) p(new T) es la única segura con excepciones

**13. Un ciclo A->B->A con shared_ptr sucede porque...**

- a) los contadores llegan a 0 pero el deleters falla
- b) cada uno sostiene al otro (strong 2 y 2): ningún destructor llega a 0 y nada corre
- c) el stack unwinding no corre
- d) make_shared lanza bad_weak_ptr

**14. weak_ptr.expired() frente a weak_ptr.lock(): la señal del patrón correcto para USAR el recurso es...**

- a) expired() y después lock(): las dos en secuencia
- b) lock() directo: decide y sostiene en UN paso atómico (expired solo sirve para diagnóstico/limpieza)
- c) expired() solo: si da false, desreferenciás
- d) comparar weak con nullptr entre las dos llamadas

**15. Un caché de texturas que devuelve shared_ptr de cada sprite pero guarda el caché como weak: ¿qué pasa cuando el ÚLTIMO sprite muere?**

- a) La textura no muere nunca (el weak la sostiene)
- b) La textura muere (el weak NO sostiene): la próxima petición recarga (lock falla)
- c) Error: los weak_ptr prohíben que el objeto muera
- d) El shared_ptr del gestor vacía el vector

**16. Deleter personalizado en unique_ptr<FILE, D>: su trabajo es...**

- a) liberar la memoria del FILE* con delete
- b) hacer fclose (el "liberar" correcto del recurso FILE)
- c) convertir el FILE* a void*
- d) nada: no se puede personalizar el deleter

**17. `std::shared_ptr<Tarea>(this)` adentro de un método de Tarea es un bug porque...**

- a) this no es un puntero válido
- b) crea un SEGUNDO bloque de control para el mismo objeto: doble delete
- c) lanza bad_weak_ptr si hay dueños
- d) enciende el contador en 0

**18. ¿Cuándo es válido shared_from_this()?**

- a) Siempre, desde el constructor
- b) Solo cuando el objeto YA es administrado por algún shared_ptr (y si el tipo heredó de enable_shared_from_this)
- c) Solo en objetos del stack
- d) Solo si el objeto tiene virtual destructor

**19. ¿Qué hace un POOL de balas en vez de new/delete por bala?**

- a) Mueve las balas al heap global con unique_ptr
- b) Pre-aloca N balas y las recicla: spawn toma un slot de la free-list y liberar lo devuelve
- c) Usa std::shared_ptr para que nadie la libere
- d) Reserva memoria nueva en cada frame y el RAII libera el frame entero

**20. ¿Cuál caso pide un ARENA (bloque grande + reseteo) en vez de RAII por objeto?**

- a) El administrador del "jugador" que vive todo el juego
- b) Cientos de objetos temporales por frame que mueren TODOS juntos
- c) Un archivo de configuración
- d) Un mutex lock

---

## Respuestas

1. **b** — la adquisición es la inicialización; la liberación corre con el destructor del objeto.
2. **b** — reversa: último en nacer, primero en morir (L02).
3. **a** — cuerpo del dtor primero; miembros en inversa DECLARACIÓN (no de la lista de init).
4. **b** — stack unwinding: los dtors corren con cada frame desarmado (L03).
5. **b** — un solo puntero + deleter stateless: 8 bytes (L04).
6. **a** — no se puede copiar: es la característica clave de la propiedad exclusiva; `->`, `*` y `reset` existen.
7. **a** — get() para observación y pasaje a APIs no dueñas; nunca delete ni doble dueño.
8. **b** — el vector es dueño de la colección: el erase destruye el slot y con él el objeto YA (L04).
9. **a** — const&: leer sin propiedad; unique_ptr por valor implicaría adopción (L05).
10. **b** — el crudo es "uso transitorio" u "observador garantizado"; el nullptr opcional es la variante con estado (L10-L11).
11. **b** — el mismo origen compartido (copias del mismo shared): tres dueños de un bloque (L06).
12. **b** — una allocación y sin ventana entre new y la adopción (L07).
13. **b** — strong counts 2 y 2: nadie libera (L08).
14. **b** — lock() es "decide y sostiene" en un paso; expired()+lock() tiene ventana de carrera (L09).
15. **b** — la caché auto-vaciante: muere cuando el último sprite muere; el slot queda de rastro (L09).
16. **b** — el deleter define la forma de liberar: para un FILE*, fclose (y su flush) (L12).
17. **b** — dos bloques de control, doble delete: la solución es enable_shared_from_this (L13).
18. **b** — requiere bloque preexistente (objeto ya compartido) y herencia correcta (L13).
19. **b** — pre-alocación + free-list: los objetos se RECICLAN; el pool es el único dueño (L14).
20. **b** — oleadas por frame: el RAII por objeto sobra: toda la LOTE va en una sola allocación (L14).
