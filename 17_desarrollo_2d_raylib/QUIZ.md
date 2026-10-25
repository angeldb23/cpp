# MÓDULO 17 — QUIZ: Desarrollo 2D con raylib (Sprites, Audio, Física)

Respuestas al final.

---

## Preguntas Conceptuales

**1. ¿Por qué un wrapper RAII de `Texture2D` debe definir move constructor/move assignment (y sin copias)?**
A) Porque `Texture2D` no se puede copiar a nivel de GPU
B) Para transferir la propiedad del id y dejar el origen "neutro", evitando doble `UnloadTexture` en el destructor
C) Porque raylib requiere mover la textura entre hilos
D) Porque las copias son más lentas que los movimientos

**2. ¿Qué hace el wrapper RAII en el destructor cuando aún es propietario del recurso?**
A) Nada: el usuario debe llamar `Unload` manualmente siempre
B) Llama a la función de descarga (`UnloadTexture`, `UnloadSound`...) solo si el id es válido
C) Reinicia el id a un valor por defecto
D) Espera a que el GC libere la memoria

**3. En un atlas de sprites de NxM frames, ¿cómo se calcula el rect fuente del frame i?**
A) Se guardan las coordenadas en un archivo .idx
B) `sx = (i % cols) * fw; sy = (i / cols) * fh` con `fw = texW/cols`, `fh = texH/rows`
C) Todos los frames están en la fila 0
D) Se divide la textura en diagonal

**4. ¿Para qué sirve el "origen" (source rect) al dibujar un sprite del atlas?**
A) Centrar la textura completa
B) Decidir QUÉ parte del atlas se dibuja: el sub-rect del frame; y (con destino) también el pivote respecto al cual se escala/rota
C) Definir el color del tint
D) Ajustar la resolución de la ventana

**5. ¿Cómo se logra un "flash blanco de daño" sin textura extra?**
A) Redibujando el sprite con transparencia total
B) Mezclando el color base con blanco vía lerp sobre `tint` (en raylib, tinte con `255` alto casi aclara; técnica empírica) durante el timer de invulnerabilidad
C) Cambiando el cursor del sistema
D) Escalando el sprite al doble por un frame

**6. En la animación por frames, ¿por qué el acumulador del timer se resta en vez de "reiniciar con dt"?**
A) Porque dt es siempre mayor que el frame time
B) Para que no se pierda fracción de tiempo entre frames: exhaustiva con dt variable se mantiene la cadencia; el residuo se preserva
C) Para que la animación vaya más rápido
D) Porque raylib no permite asignar el timer

**7. En la animación con ping-pong, ¿cómo se invierte la dirección al llegar al último frame?**
A) Se descarta la mitad de los frames
B) Se cambia el signo del avance: `dir = -dir` en los extremos y el índice `idx += dir` recorre ida y vuelta
C) Se reinicia siempre a 0
D) Se duplica el primer y último frame

**8. En la FSM de animación (Idle/Walk/Attack), ¿por qué las transiciones se evalúan EN ORDEN y la primera que dispara gana?**
A) Para ahorrar memoria
B) Porque el orden define prioridad: "hurt" interrumpe a "attack" solo si su transición está antes en la lista
C) Porque raylib itera en reversa
D) Porque las FSM no tienen estados complejos

**9. ¿Qué problema resuelve el "seguimiento suave" (lerp) de la cámara sobre el seguimiento duro a la posición del jugador?**
A) La cámara barata de calcular
B) Elimina el "snap" por frame: `target = Lerp(target, playerPos, k*dt)` percibido como cámara flotante sin sobresaltos
C) Evita que la cámara salga de la ventana
D) Compensa el tearing

**10. Al hacer clamp con límites de mundo, ¿por qué si `minX > maxX` se igualan ambos al centro del mundo?**
A) Es un bug de raylib
B) Porque el mundo es más chico que la vista: no hay recorrido horizontal y la cámara se centra (evita oscilar)
C) Para que la cámara no se mueva nunca
D) Para activar el zoom automático

**11. ¿Cuál es la diferencia entre cargar un sonido con `LoadSound` y un audio en stream (`AudioStream`)?**
A) No hay diferencia
B) `LoadSound` decodifica TODO a RAM/VRAM (memoria fija, latencia mínima); el stream lee bloques bajo demanda (memoria baja, para música larga), requiriendo alimentar buffers
C) El stream solo reproduce wav
D) `LoadSound` solo funciona con archivos del atlas

**12. ¿Qué problema del pooling de sonidos resuelve `SetSoundPitch`/volúmenes por copia y una cola de "canales"?**
A) La pérdida de audio en VSync
B) Que disparar el MISMO sonido 10 veces en un frame lo reinicia/corta: se agarran varias copias del pool y se dan vuelta (robin) para poder superponer
C) El pitch se pierde al guardar
D) Que miniaudio no soporta volumen

**13. En AABB (rect vs rect), ¿cuándo hay overlap en un eje?**
A) Solo si los centros coinciden
B) Si `a.x < b.x + b.w && b.x < a.x + a.w` (estricto: tocar exactamente en el borde NO es overlap en ese eje)
C) Si los rectángulos son del mismo tamaño
D) Siempre que el rect b esté al lado

**14. Para "point in rect", ¿por qué el borde inclusivo ( igualdad ) SÍ cuenta a diferencia del AABB overlap?**
A) Es una decisión del compilador
B) Porque es una indicación puntual del jugador sobre una celda exacta: `tx == floor(px/TILE)` con el borde tocando cuenta la celda del borde (coincide con cómo se convierten los bordes del tilemap)
C) Porque los floats no comparan igual que los enteros
D) No cuenta: es igual al AABB

**15. En el broad-phase con grid uniforme, ¿para qué sirve el "tamaño de celda"?**
A) Es el tamaño del sprite más grande del juego
B) Una celda ~ 2x el tamaño del cuerpo promedio: los pares candidatos solo se prueban entre vecinos de celdas que comparten entidades
C) Es siempre 32 px
D) Define el tamaño del mundo

**16. Al recolectar pares candidatos del grid, ¿por qué se descartan duplicados (a,b) / (b,a)?**
A) Porque los objetos son únicamente identificables por su id
B) Porque la misma pareja puede salir en celdas distintas o en la celda de a y la de b; sin dedupe se probaría la narrow-phase dos veces (y en colisiones finas duplicaría el impulso)
C) Porque el grid no permite pares
D) Para ahorrar pantalla

**17. En el pool de partículas con swap-remove, ¿por qué el swap debe procesarse de nuevo la MISMA i y no avanzar i?**
A) Para que el pool no se desordene por edad
B) Porque el elemento traído desde el final aún no fue evaluado: si se avanza, se lo salta ese frame
C) Porque swap-remove es O(n log n)
D) Porque así lo exige std::vector

**18. En un tilemap de Tiled con GIDs, ¿qué significa un GID de 0 en la capa de tiles?**
A) Tile con rotación
B) Tile vacío (aire): no se dibuja y no colisiona
C) El primer tile del tileset
D) Un error de parseo

**19. Para la colisión por tile (solids por GID), ¿por qué se usa `floor(...)/TILE` en los bordes del rect y NO `ceil`?**
A) Por performance
B) Para que "tocar exactamente con el borde inferior" cuente la celda del piso (consistencia con el resto del juego: caer sobre una celda se detecta de inmediato, no 1 px después)
C) Porque ceil falla con negativos
D) Es indistinto: ceil y floor dan lo mismo en tilemaps

**20. ¿Cómo se calcula el rango de tiles visibles (culling de vista) a partir de la cámara?**
A) Se dibujan TODOS los tiles y raylib descarta los sobrantes
B) Rect visible mundo = `cam.Target - cam.Offset/zoom` a `+ Offset/zoom` (más margen); tiles `x0 = floor(worldMinX/TILE)` a `x1 = floor(worldMaxX/TILE)`
C) Se pregunta a cada tile si es visible con un raycast
D) Se fija arbitrariamente en 100 tiles

**21. En la consola en pantalla (ScreenConsole), ¿por qué el ring usa un `std::deque` con push_back + pop_front?**
A) deque es lo único que soporta strings
B) El orden de inserción (viejo→nuevo) queda gratis para Snapshot, y descartar por el frente es O(1): exactamente lo que un ring de "últimas N líneas" necesita
C) Porque vector no puede crecer
D) Para confundir a los lectores

**22. En la consola con lifetime, ¿por qué Update resta dt a TODAS las entradas y purga con remove_if?**
A) Para sincronizar con el reloj del sistema
B) Las entradas son independientes (cada una nace cuando se la loguea): decaimiento uniforme + `erase(remove_if(... life<=0))` expulsa a las caducadas manteniendo el orden
C) Porque los floats no se pueden comparar con 0
D) Para que las líneas viejas siempre se eliminen antes que las nuevas

**23. ¿Qué ventaja tiene dibujar TODOS los overlays de debug (colisiones, pivots, grid, FPS) con una sola pasada de "debug layer" después del render normal?**
A) Ahorra texturas
B) Un lugar único para activar/desactivar (tecla en toggle y/o flag): el juego normal no se modifica y el debug no ensucia la lógica de juego
C) Es obligatorio en raylib
D) Evita el tearing

**24. ¿Por qué el formato de línea "[cat] msg" con filtro por categoría es útil en la consola de debug?**
A) Para que los strings ocupen menos
B) Permite silenciar una categoría entera ("spawnea 300 partículas" no tapa los mensajes de colisión) filtrando por prefijo exacto, lo que resulta legible en pantalla en una corrida real
C) Porque miniaudio lo exige
D) Porque comparamos floats con ==

**25. Al cerrar la partida "Vampire Survivors lite" del módulo, ¿qué decisión se tomó y por qué?**
A) Se construyó como lección 12
B) Igual que el cierre "Asteroids" del módulo 16: el cierre NO se construye como archivo; las piezas ya están construidas pieza a pieza (colisiones, partículas, tilemap, audio, cámara, debug) a lo largo de 11 lecciones y 12 ejercicios
C) Se omitió de TEMAS.md
D) Se reemplazó por un segundo módulo de audio

---

## Respuestas

1.  **B** — El move deja el origen con id 0 (neutro): el destructor del temporal no descarga el recurso que ya es del destino.
2.  **B** — Guard: `if (tex.id > 0) UnloadTexture(tex);` — id 0 significa "no propietario".
3.  **B** — Grilla: fila = i / cols, col = i % cols; el tamaño del frame sale de dividir la textura.
4.  **B** — El source rect se saca del atlas; el pivote/destination decide transformación.
5.  **B** — Lerping el tint hacia blanco durante el tiempo de "flash" y de vuelta; no se necesitan texturas dobles.
6.  **B** — El acumulador preserva el residuo (timer += dt; while (timer >= frameTime) { next; timer -= frameTime; }) para cadencia estable con dt variable.
7.  **B** — Ping-pong: invierte el signo en los extremos y avanza `idx += dir`; sin condiciones intermedias.
8.  **B** — El índice 0 de la lista es la máxima prioridad: hurt primero, luego attack, luego locomoción.
9.  **B** — Lerp exponencial: la cámara "persigue" al jugador con suavidad; el factor k*dt hace el suavizado independiente del framerate.
10. **B** — Si el mundo cabe entero en la vista, no hay clamp válido: clamp del centro evita el vaivén.
11. **B** — Loading completo = fijo y simple; streaming = memoria constante para música larga, requiere refill de buffers desde archivo.
12. **B** — Un mismo Stream de sonido sin pool se "corta": el pool de copias permite superposición (¡pooling del tema 5!).
13. **B** — Test clásico de overlap por eje con `<` estricto; el borde-compartido no colisiona en AABB puro.
14. **B** — Point-in-rect es inclusivo en bordes, consistente con la conversión celda = floor(coordenada/TILE).
15. **B** — Celda ~ 2x el cuerpo tipo: cualquier par que colisione cae en celdas vecinas o compartidas.
16. **B** — Dedupe (id menor primero + hash set o compare) evita narrow-phase repetida y dobles impulsos.
17. **B** — En swap-remove hay que re-evaluar `i` (sin ++i) porque el traído del final no pasó el test.
18. **B** — GID 0 = vacío: se omite en draw y no es sólido en colisión.
19. **B** — floor en bordes => tocar-en-borde cuenta la celda a la que se llega: snap coherente con move-and-slide del ejercicio 11.
20. **B** — Se deriva el rect visible de target/offset/zoom y se itera solo y0..y1, x0..x1 (con margen 1 tile).
21. **B** — deque: orden de inserción natural + pop_front O(1); es el ring lineal de consola.
22. **B** — Decaimiento por entrada + purge con el idiom erase-remove; el orden viejo→nuevo se conserva.
23. **B** — La capa de debug es una pasada post-render con flags; el juego queda limpio y el debug centralizado.
24. **B** — El prefijo `[cat]` permite agrupar/filtrar sin cambiar la API del Log.
25. **B** — Precedente del módulo 16 (Asteroids): los cierres no se construyen como archivos; se documentan.
