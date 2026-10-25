# QUIZ.md — Módulo 27: Doom Clásico — De C a C++

## Instrucciones
Responde cada pregunta con la mejor opción o explicación breve. Las respuestas están al final.

---

## Parte 1: Estructura id Tech 1 (Lección 01)

### 1.1 ¿Cuál es el prefijo usado para los archivos del renderer en el source release 1997?
- A) `g_*`
- B) `r_*`
- C) `p_*`
- D) `s_*`

### 1.2 En Doom, `mobj_t` embebe `thinker_t` como primer miembro. ¿Para qué sirve este patrón?
- A) Para que `mobj_t` herede de `thinker_t` en C++
- B) Para permitir cast seguro `(thinker_t*)mobj` → `mobj_t*` con offset 0 (container_of)
- C) Para ahorrar memoria al compartir el puntero `function`
- D) Para que el compilador optimice mejor

### 1.3 ¿Qué variable global controla el estado principal del juego (nivel, intermission, finale)?
- A) `gameskill`
- B) `gamestate`
- C) `gameaction`
- D) `gametic`

### 1.4 El source release 1997 NO compila limpio en GCC moderno sin parches. ¿Cuál NO es un problema típico?
- A) Declaraciones implícitas de funciones (K&R style)
- B) Falta de includes (`stddef.h`, `stdlib.h`)
- C) Uso de `bool` nativo de C99
- D) Sintaxis K&R de funciones (`void func(a, b) int a; { }`)

---

## Parte 2: Fixed-Point 16.16 (Lección 02)

### 2.1 ¿Cuál es el valor de `FRACUNIT` en fixed-point 16.16?
- A) 32768
- B) 65536
- C) 16384
- D) 131072

### 2.2 `FixedMul(a, b)` se implementa como:
- A) `(a * b) >> 8`
- B) `(a * b) >> 16`
- C) `(a * b) << 16`
- D) `(a << 16) * (b << 16)`

### 2.3 ¿Por qué `FixedDiv(a, b)` hace `(a << 16) / b` en lugar de `a / b << 16`?
- A) Para evitar overflow en la multiplicación
- B) Para preservar precisión fraccionaria ANTES de dividir
- C) Porque el compilador lo optimiza mejor
- D) Para que funcione con enteros negativos

### 2.4 La tabla `finesine` tiene 8192 entradas. ¿Cómo se indexa con `angle_t` (0-65535)?
- A) `finesine[angle]`
- B) `finesine[angle >> 3]`
- C) `finesine[angle & 8191]`
- D) `finesine[(angle >> 3) & 8191]`

### 2.5 ¿Qué hace `ApproxDistance(dx, dy)` en Doom?
- A) `sqrt(dx*dx + dy*dy)` exacto
- B) `max(dx, dy) + min(dx, dy)/2` (aproximación rápida, error < 12%)
- C) `abs(dx) + abs(dy)` (distancia Manhattan)
- D) `FixedDiv(dx, dy)`

---

## Parte 3: BSP y Mapa (Lección 03)

### 3.1 En un `node_t`, el campo `children[2]` usa el bit 15 como flag. ¿Qué significa `children[i] & 0x8000`?
- A) El hijo es otro nodo interno
- B) El hijo es un subsector (hoja del BSP)
- C) El hijo está vacío
- D) El hijo es un portal

### 3.2 `R_PointOnSide(px, py, node)` determina si el viewer está en el lado front o back. ¿Qué producto cruz usa?
- A) `dx * node->dx + dy * node->dy`
- B) `dx * node->dy - dy * node->dx`
- C) `dx * node->dx - dy * node->dy`
- D) `dy * node->dx - dx * node->dy`

### 3.3 El recorrido BSP es **front-to-back**. ¿Por qué?
- A) Para dibujar sprites primero
- B) Para minimizar overdraw (paredes cercanas ocultan lejanas)
- C) Porque el BSP está construido así
- D) Para que los visplanes salgan ordenados

### 3.4 Un `subsector_t` contiene:
- A) Lista de vertexes y linedefs
- B) `numsegs` y `firstseg` (índice al array global `segs[]`)
- C) Referencia al sector padre y bounding box
- D) Lista de things (mobj_t) en ese subsector

---

## Parte 4: Visplanes, Drawsegs, Sprites (Lección 04)

### 4.1 Un `visplane` representa:
- A) Un sector completo del mapa
- B) Una región continua de piso/techo con MISMA altura, textura, luz y escala
- C) Un segmento de pared visible
- D) Un sprite proyectado en pantalla

### 4.2 ¿Cuál es el límite original de visplanes (`MAXVISPLANES`)?
- A) 64
- B) 128
- C) 256
- D) 512

### 4.3 Los `drawsegs` son segmentos de pared ya proyectados y clipeados. ¿Qué estructura evita overdraw horizontal?
- A) `visplane_t`
- B) `solidseg_t` (intervalos [x1,x2] ya dibujados)
- C) `vissprite_t`
- D) `drawseg_t` mismo

### 4.4 ¿En qué orden se dibujan los visplanes, drawsegs y sprites?
- A) Sprites → Drawsegs → Visplanes
- B) Visplanes → Drawsegs → Sprites
- C) Drawsegs → Visplanes → Sprites
- D) Visplanes → Sprites → Drawsegs

### 4.5 `vissprite_compare` para `qsort` ordena:
- A) Front-to-back (cerca primero)
- B) Back-to-front (lejos primero)
- C) Por textura (agrupar por patch)
- D) Por ángulo relativo al viewer

---

## Parte 5: Thinkers y Tic System (Lección 05)

### 5.1 La lista de thinkers usa una **centinela** (`thinkercap`). ¿Cómo se inicializa?
- A) `thinkercap->next = thinkercap->prev = NULL`
- B) `thinkercap->next = thinkercap->prev = thinkercap`
- C) `thinkercap = NULL`
- D) `thinkercap->function = P_Ticker`

### 5.2 `P_Ticker()` recorre la lista y llama `current->function(current)`. ¿Qué pasa si un thinker se elimina durante la iteración?
- A) Crash por puntero inválido
- B) Se salta el siguiente thinker
- C) No pasa nada: se guardó `next` antes de llamar a la función
- D) El bucle se rompe

### 5.3 El tic rate de Doom es **35 Hz fijo**. ¿Por qué es crítico para determinismo?
- A) Porque `P_Random()` usa tabla de 256 bytes
- B) Porque la física usa fixed-point
- C) Misma secuencia de `ticcmd_t` → mismo resultado bitwise (demos, netplay)
- D) Porque el monitor refresca a 35 Hz

### 5.4 `BACKUPTICS = 12` en el sistema de backup de gamestate. ¿Para qué sirve?
- A) Guardar 12 niveles de undo
- B) Permitir re-simulación cuando llegan ticcmds tardíos (latencia red)
- C) Guardar 12 slots de savegame
- D) Buffer de 12 frames para interpolación

---

## Parte 6: State Machine (Lección 06)

### 6.1 En `state_t`, el campo `frame` usa el bit 15 (`0x8000`) como flag `FF_FULLBRIGHT`. ¿Qué hace?
- A) Hace el sprite transparente
- B) Hace el sprite brillante completo (ignora colormap/lightlevel)
- C) Duplica el frame horizontalmente
- D) Marca el frame como último de la animación

### 6.2 Diferencia clave entre `P_SetMobjState()` (forzado) y transición natural (tics llega a 0):
- A) En forzado, la acción se llama AL INICIO del nuevo estado; en natural, AL FINAL del anterior
- B) En forzado, no se llama la acción; en natural sí
- C) En forzado, se salta el siguiente estado
- D) No hay diferencia

### 6.3 Un estado con `tics = -1` (o 0 según versión) significa:
- A) Duración aleatoria
- B) Estado terminal (elimina el mobj)
- C) Estado "wait": no avanza solo, espera evento externo (daño, trigger)
- D) Duración infinita (cadáver, ítem en suelo)

### 6.4 ¿Qué acción se llama cuando un monstruo recibe daño y `P_Random() < painchance`?
- A) `A_Look`
- B) `A_Chase`
- C) `A_Pain` (transición a `painstate`)
- D) `A_Scream`

---

## Parte 7: Networking (Lección 07)

### 7.1 Doom original usa modelo **peer-to-peer lockstep**. ¿Qué implica?
- A) Un servidor autoritario simula y envía snapshots
- B) Todos los nodos simulan TODO; mismos inputs → mismo resultado; el más lento frena a todos
- C) Clientes predicen, servidor reconcilia
- D) Solo el host simula, otros solo renderizan

### 7.2 `ticcmd_t` contiene `consistancy` (checksum). ¿Para qué?
- A) Verificar que el packet no se corrompió en red
- B) Detectar DESYNC: comparar checksum local vs remoto del estado de juego
- C) Comprimir el comando
- D) Identificar al jugador

### 7.3 ¿Cuántos jugadores máximo soporta Doom vanilla (`MAXNETNODES`)?
- A) 2
- B) 4
- C) 8
- D) 16

### 7.4 ¿Qué pasa si un jugador tiene lag y sus `ticcmd_t` llegan tarde?
- A) El juego continúa y se resync después
- B) **Todos los jugadores se congelan** ("waiting for player X") hasta que llega el comando
- C) Se usa predicción local
- C) Se descarta el comando tardío

---

## Parte 8: WAD y Zone Memory (Lección 08)

### 8.1 El header WAD tiene 12 bytes: `identification[4]`, `numlumps`, `infotableofs`. ¿Qué valores puede tener `identification`?
- A) "WAD1" o "WAD2"
- B) "IWAD" o "PWAD"
- C) "DOOM" o "DOOM2"
- D) "LUMP" o "DATA"

### 8.2 Los namespaces (prefijos) evitan colisiones. ¿Qué namespace usan los flats (pisos/techos)?
- A) `S_`
- B) `P_`
- C) `F_`
- D) `M_`

### 8.3 Zone Memory tags: `PU_CACHE` significa:
- A) Bloque permanente (nunca se libera)
- B) Bloque ligado al nivel (se libera al cambiar de mapa)
- C) **Bloque de cache purgable** (se libera bajo presión de memoria)
- D) Bloque para savegames

### 8.4 `Z_Malloc` devuelve `user` pointer (justo después del header `memblock_t`). ¿Cómo recupera `Z_Free` el header?
- A) Busca en tabla hash global
- B) `block = (memblock_t*)((byte*)ptr - sizeof(memblock_t))`
- C) El header está al final del bloque
- D) Usa el campo `user` del bloque anterior

---

## Parte 9: Source Ports (Lecciones 09 y 13)

### 9.1 GZDoom usa **ZScript**. ¿Qué es?
- A) Lenguaje de scripting compilado a bytecode con VM JIT, similar a C++/Java
- B) Formato de mapa extendido (UDMF)
- C) Sistema de partículas
- D) Renderer Vulkan

### 9.2 Odamex usa arquitectura **client-server autoritario**. ¿Qué NO hace?
- A) Servidor simula autoridad
- B) Clientes predicen movimiento local
- C) Interpolación de entidades entre snapshots
- D) **Peer-to-peer lockstep** (eso es Doom vanilla)

### 9.3 EDGE-Classic usa **COAL** para scripting. Característica principal:
- A) Interpretado en runtime (como Lua)
- B) **Compilado a bytecode, tipado estático, clases/herencia** (C-like)
- C) Solo data-driven (como EDF)
- D) Transpilado a C++

### 9.4 Eternity Engine tiene **portales linked** y **attached surfaces**. ¿Qué permiten?
- A) Multijugador masivo
- B) **Geometría no-euclidiana: habitaciones sobre habitaciones, loops espaciales, mid-textures 3D**
- C) Shaders personalizados
- D) Física de ragdoll

### 9.5 Chocolate Doom busca **fidelidad vanilla EXACTA**. ¿En qué estándar C compila?
- A) C99
- B) C11
- C) **C89 (ANSI C)**
- D) C++98

---

## Parte 10: Modernización (Lecciones 10 y 14)

### 10.1 Patrón **Strangler Fig** para migrar legacy C a C++:
- A) Reescribir todo de una vez (big bang)
- B) **Identificar frontera → interfaz C++ → implementar nueva → redirigir → testear → eliminar legacy**
- C) Mantener C y añadir C++ encima sin tocar lo viejo
- D) Compilar C como C++ y arreglar warnings

### 10.2 Wrapper RAII `ZoneBlock` para `Z_Malloc/Z_Free`. ¿Qué hace el destructor?
- A) `Z_ChangeTag(ptr, PU_STATIC)`
- B) **`if (owned_) Z_Free(ptr_)`**
- C) `free(ptr_)` directo
- D) Nada, el zone memory se limpia solo al salir

### 10.3 ECS (Entity Component System) vs `mobj_t` monolítico. Ventaja principal:
- A) Menos líneas de código
- B) **Cache locality (SoA) + paralelización de systems independientes + modularidad**
- C) Más fácil de debuggear
- D) Compatible con C89

### 10.4 `JobSystem` para paralelizar thinkers. ¿Cuál es el desafío principal en Doom?
- A) Crear threads es lento
- B) **Dependencias de datos: thinkers interactúan (colisión, target, RNG global)**
- C) No hay suficientes cores en Celeron N4020
- D) `P_Ticker` ya es bastante rápido

### 10.5 **SDL_GPU** (SDL3) abstrae:
- A) Solo Vulkan
- B) **Vulkan / Metal / DirectX 12** (backend portable)
- C) OpenGL 3.3 + Vulkan
- D) Solo software rendering

---

## Parte 11: Doom 3 / id Tech 4 (Lección 11)

### 11.1 Concepto clave del renderer de Doom 3: **Interaction Tables**. ¿Qué es una "interaction"?
- A) Interacción entre jugador y NPC
- B) **Interacción entre una luz y una superficie (light + surface + material)**
- C) Colisión entre dos cuerpos rígidos
- D) Evento de script

### 11.2 **Shadow Volumes** (stencil shadows) en Doom 3:
- A) Shadow mapping con depth texture
- B) **Extruir silueta de casters → geometría de volumen → stencil buffer increment/decrement**
- C) PCF (Percentage Closer Filtering) en fragment shader
- D) Ray-tracing por GPU

### 11.3 `idEntity` en Doom 3 vs `mobj_t` en Doom 1:
- A) `idEntity` es struct C plano, `mobj_t` es clase C++
- B) **`idEntity` es clase C++ con herencia virtual, scripting via VM; `mobj_t` es struct C con function pointers**
- C) Son equivalentes, solo cambia nombre
- D) `idEntity` no tiene física, `mobj_t` sí

### 11.4 ¿Por qué NO se compila Doom 3 / BFG en la máquina del dueño (Celeron N4020)?
- A) Falta código fuente
- B) **Son MUY PESADOS (500K+ líneas, dependencias Qt, OpenAL, etc.), compilan en horas**
- C) Requieren Windows 10+
- D) No tienen licencia GPL

---

## Parte 12: Subsistemas (Lección 12)

### 12.1 Audio en Doom 1: mezcla por **software** (s_chan.c). ¿Cuántos canales en vanilla?
- A) 4
- B) **8**
- C) 16
- D) 32

### 12.2 `PLAYPAL` lump contiene 14 paletas de 256 colores. ¿Para qué sirve la paleta 0?
- A) Efecto damage (rojo)
- B) **Paleta normal (juego estándar)**
- C) Radsuit (azul)
- D) Powerup invulnerabilidad

### 12.3 El automap (TAB) NO usa BSP para dibujar. ¿Qué dibuja?
- A) Solo el sector actual
- B) **Todos los linedefs de sectores "known" (visited), things, puertas, secretos**
- C) Solo los visplanes visibles
- D) El BSP completo pero en 2D

---

## Parte 13: Asset Pipeline y Testing (Lección 14)

### 13.1 **Golden Frames** testing:
- A) Comparar FPS entre runs
- B) **Render frame con seed fijo → guardar PNG → CI compara hash/RMS/PSNR contra golden**
- C) Testear solo shaders
- D) Verificar uso de memoria

### 13.2 **Deterministic simulation test**:
- A) Correr juego y ver si crashea
- B) **Ejecutar N tics con seed fijo → hash estado final → comparar bitwise entre máquinas/compiladores**
- C) Testear solo fixed-point math
- D) Verificar que `P_Random` devuelve valores aleatorios

### 13.3 VFS (Virtual File System) en asset pipeline moderno. Implementaciones típicas:
- A) Solo directorios sueltos
- B) **PhysicalFS + ZipFS (PK3) + PackFS (custom) + MemoryFS + CompositeFS (prioridad mod > base > dlc)**
- C) Solo WAD
- D) Solo HTTP streaming

---

## RESPUESTAS

| Pregunta | Respuesta | Explicación breve |
|----------|-----------|-------------------|
| 1.1 | B | `r_*`: renderer (r_bsp.c, r_segs.c, r_things.c...) |
| 1.2 | B | Herencia manual en C: `thinker_t` como primer miembro permite `container_of` / cast directo offset 0 |
| 1.3 | B | `gamestate_t gamestate`: GS_LEVEL, GS_INTERMISSION, GS_FINALE, GS_DEMOSCREEN |
| 1.4 | C | Doom usa `typedef int boolean` + `#define true 1`, NO `bool` de C99 |
| 2.1 | B | `FRACUNIT = 1 << 16 = 65536` (1.0 en 16.16) |
| 2.2 | B | `FixedMul = (int64_t)a * b >> 16` |
| 2.3 | B | Shift ANTES de dividir preserva bits fraccionarios |
| 2.4 | D | `angle_t` 16 bits → 8192 entradas: `angle >> 3` (divide 8) + máscara 13 bits |
| 2.5 | B | `max + min/2`: rápido, error < 12%, usado en IA, sonido, colisión |
| 3.1 | B | Bit 15 = 1 → subsector (hoja); `child & 0x7FFF` = índice |
| 3.2 | B | `cross = dx*node->dy - dy*node->dx`; ≤0 = front (izquierda del vector) |
| 3.3 | B | Front-to-back: paredes cercanas en `drawsegs` primero, `solidsegs` rechaza lejanas |
| 3.4 | B | Subsector = lista de segs (índice + count); convexos, sin partición interna |
| 4.1 | B | Visplane = región continua misma altura/textura/luz/escala; se fusionan adyacentes |
| 4.2 | B | `MAXVISPLANES = 128`; overflow = crash en vanilla |
| 4.3 | B | `solidseg_t` = intervalos [x1,x2] ya dibujados opacos; clipping horizontal |
| 4.4 | B | 1. Visplanes (floor/ceiling) → 2. Drawsegs (paredes) → 3. Sprites (things) |
| 4.5 | B | Back-to-front: painter's algorithm para sprites (transparencia, no z-buffer) |
| 5.1 | B | Centinela circular: `next = prev = self` cuando vacía |
| 5.2 | C | `next = current->next` ANTES de llamar function; remoción segura |
| 5.3 | C | Determinismo: misma secuencia ticcmds → mismo estado bitwise (demos, netplay) |
| 5.4 | B | Backup circular 12 tics: re-simular desde tic tardío cuando llega comando real |
| 6.1 | B | `FF_FULLBRIGHT` (bit 15): ojos brillantes, proyectiles, ignora lightlevel |
| 6.2 | A | Forzado: acción al ENTRAR; natural (tics=0): acción al SALIR del estado anterior |
| 6.3 | C | `tics = 0` = wait state (trigger externo); `tics = -1` = stay forever (cadáver) |
| 6.4 | C | `P_DamageMobj` → `P_SetMobjState(target, painstate)` si `P_Random() < painchance` |
| 7.1 | B | P2P lockstep: todos simulan todo, determinismo, el más lento frena a todos (4 max) |
| 7.2 | B | Consistancy = checksum estado; mismatch = DESYNC (pausa, "waiting for...") |
| 7.3 | B | `MAXNETNODES = 4` en vanilla; source ports aumentan |
| 7.4 | B | Sin predicción en vanilla: lag = freeze total ("waiting for player X") |
| 8.1 | B | `IWAD` = datos base (doom.wad); `PWAD` = patch/addon (override por nombre) |
| 8.2 | C | `F_` = flats (64x64 raw); `S_` sprites, `P_` patches, `DS_` sounds, `D_` music |
| 8.3 | C | `PU_CACHE`: purgable bajo presión; `Z_FreeTags(PU_CACHE, 0)` libera no bloqueados |
| 8.4 | B | Header `memblock_t` antes de user data; `Z_Free` resta `sizeof(memblock_t)` |
| 9.1 | A | ZScript: VM JIT, clases, herencia, bytecode; reemplaza DECORATE/ACS |
| 9.2 | D | Odamex = client-server; vanilla = P2P lockstep |
| 9.3 | B | COAL: tipado estático, clases, compilado a bytecode, VM propia |
| 9.4 | B | Portales linked + attached surfaces = geometría 3D real no-euclidiana |
| 9.5 | C | Chocolate Doom: C89, SDL2, bitwise-identical a vanilla 1.9 |
| 10.1 | B | Strangler Fig: migración incremental subsistema a subsistema |
| 10.2 | B | RAII: destructor llama `Z_Free` si `owned_`; move semantics transfiere ownership |
| 10.3 | B | SoA = cache locality; systems paralelos si no comparten datos; añadir/quitar components |
| 10.4 | B | RNG global, estado compartido, colisiones → requiere double-buffering o islas |
| 10.5 | B | SDL_GPU: un API, backends Vulkan/Metal/DX12 automáticos |
| 11.1 | B | Interaction = (light, surface, material) → draw call; tabla construida cada frame |
| 11.2 | B | Stencil shadows: extruir silueta → shadow volume → stencil inc/dec → light pass where stencil=0 |
| 11.3 | B | idEntity: C++ class, virtual, script thread; mobj_t: C struct, function ptr, states[] |
| 11.4 | B | ~500K líneas, dependencias pesadas, horas de compile; solo lectura guiada |
| 12.1 | B | 8 canales vanilla mixing software DMX; ports modernos 16-32+ |
| 12.2 | B | Paleta 0 = normal; 1-13 = efectos (damage, bonus, radsuit, etc.) |
| 12.3 | B | Dibuja linedefs de sectores visited, things, puertas, secretos; no BSP |
| 13.1 | B | Golden frames: headless render + hash/RMS compare en CI; detecta regressions visuales |
| 13.2 | B | Determinismo: fixed seed → bitwise identical state hash (positions, health, RNG) |
| 13.3 | B | CompositeFS con prioridad: mod.pk3 > base.pk3 > dlc.pk3 > loose files |

---

## Puntuación
- **45-52**: ¡Experto en id Tech 1 y modernización!
- **38-44**: Sólido conocimiento, repasa networking y state machine
- **30-37**: Buena base, profundiza en BSP/visplanes y source ports
- **<30**: Relee lecciones 01-14 y reintenta

---

## Ejercicio Práctico Final (No evaluado en quiz)
> **Análisis Guiado**: Clona `id-Software/DOOM` (tag `release-1997`), compila con CMake propio (parcheando warnings), ejecuta con `freedoom1.wad`. Documenta en `docs/doom1_analysis.md`:
> 1. Estructura de `mobj_t`, `thinker_t`, `state_t` en el código real
> 2. Flujo de `P_Ticker` → `P_MobjThinker` → state machine
> 3. Diferencias con lo visto en lecciones (campos extra, macros, etc.)
> 4. Compilar Doom 3 (`id-Software/DOOM3`) NO; solo leer `game/Entity.h`, `renderer/tr_main.cpp`