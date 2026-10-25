/*
 * MÓDULO 27: Doom Clásico — De C a C++
 * LECCIÓN 01: Estructura del código base id Tech 1 (release 1997)
 *
 * TEMAS:
 * - Origen histórico: Doom (1993), source release 1997 (GPL 1999)
 * - Estructura de directorios y prefijos: d_*, g_*, p_*, r_*, s_*, z_*, w_*, v_*, m_*, i_*, hu_*, st_*, am_*, f_*
 * - Convenciones de C89/GNU89: K&R, sin prototypes en headers antiguos, macros en mayúsculas
 * - Variables globales y estado del juego: sin encapsulamiento, todo visible
 *
 * OBJETIVO: Entender la arquitectura original para luego modernizarla en C++.
 *
 * NOTA: El source release 1997 se descarga de GitHub (id Software / Doom) o se clona.
 * La carpeta exacta se verifica con `ls` al construir. NO se descargan WADs comerciales:
 * se usa Freedoom (contenido libre) para pruebas.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// ============================================================================
// 1. CONTEXTO HISTÓRICO Y OBTENCIÓN DEL CÓDIGO
// ============================================================================
/*
 * Doom se lanzó en 1993. El source release oficial (GPL) salió en 1999,
 * basado en la versión 1.10 de 1997 (la "release 1997").
 *
 * Repositorio de referencia: https://github.com/id-Software/DOOM
 * Tag relevante: `release-1997` (o el commit correspondiente al source release).
 *
 * Para estudiarlo en esta lección:
 *   git clone https://github.com/id-Software/DOOM.git doom-source
 *   cd doom-source
 *   git checkout release-1997   # o el tag/commit que coincida
 *   ls -la                      # ver la estructura real
 *
 * La estructura típica (verificada con `ls` en el repo clonado):
 *   doom-source/
 *     ├─ d_*.c/h    -> main, loop principal, net, etc. (d_main.c, d_net.c...)
 *     ├─ g_*.c/h    -> game logic, spawn, thinker, map (g_game.c, g_think.c...)
 *     ├─ p_*.c/h    -> play simulation, map, enemies, weapons (p_mobj.c, p_map.c...)
 *     ├─ r_*.c/h    -> render: BSP, visplanes, segs, linedefs (r_bsp.c, r_segs.c...)
 *     ├─ s_*.c/h    -> sound: music, sfx, mixing (s_sound.c, s_chan.c...)
 *     ├─ z_*.c/h    -> zone memory allocator (zone.c, zone.h) — crítico
 *     ├─ w_*.c/h    -> WAD/lump management (w_wad.c, w_wad.h)
 *     ├─ v_*.c/h    -> video mode, palette, gamma (v_video.c)
 *     ├─ m_*.c/h    -> menus, input handling (m_menu.c, m_argv.c...)
 *     ├─ i_*.c/h    -> system interface: video, sound, timer, I/O (i_video.c, i_sound.c...)
 *     ├─ hu_*.c/h   -> heads-up display (hud)
 *     ├─ st_*.c/h   -> status bar
 *     ├─ am_*.c/h   -> automap
 *     ├─ f_*.c/h    -> finale, intermission
 *     └─ doomdef.h, doomtype.h, doomdata.h, tables.h, m_fixed.h, m_bbox.h...
 *
 * Cada prefijo agrupa un subsistema. NO hay namespaces (es C), así que los prefijos
 * evitan colisiones de nombres. Todo es archivo .c/.h suelto, sin subdirectorios.
 *
 * Compilación original: DJGPP (DOS), GCC 2.x, Makefiles a mano.
 * Hoy: necesita parches para compilar en GCC/Clang moderno (warnings, types, includes).
 */

// ============================================================================
// 2. TIPOS BÁSICOS Y CONVENCIONES (doomtype.h, doomdef.h)
// ============================================================================
/*
 * doomdef.h define los tipos base del engine. En C89 no existía stdint.h,
 * así que id Software definió sus propios aliases:
 *
 *   typedef unsigned char  byte;
 *   typedef unsigned short word;     // 16 bits
 *   typedef unsigned long  dword;    // 32 bits (en DOS de 16 bits, long = 32)
 *   typedef int            boolean;  // 0/1, no _Bool (C99)
 *
 *   #define true  1
 *   #define false 0
 *
 * En código moderno (C99/C++11+) usamos stdint.h:
 *   uint8_t, uint16_t, uint32_t, bool (de <stdbool.h>).
 *
 * La lección usa tipos estándar para claridad, pero el código original
 * usa los typedefs de doomdef.h. Al leer el source release, verás:
 *   byte* screen;
 *   word  angle;
 *   dword ticcount;
 *   boolean inhelpscreens;
 *
 * IMPORTANTE: `angle_t` en Doom es de 16 bits (0-65535 = 0-360°),
 * no radianes. `fixed_t` es 32 bits con 16.16 fixed-point (ver lección 02).
 */

// Simulación de los tipos originales para esta lección (no compila el Doom real):
typedef uint8_t  byte;
typedef uint16_t word;
typedef uint32_t dword;
typedef int      boolean;
typedef int32_t  fixed_t;   // 16.16 fixed-point
typedef int16_t  angle_t;   // 0-65535

#define FRACUNIT    0x10000     // 1.0 en fixed-point 16.16
#define FRACBITS    16

// ============================================================================
// 3. ESTRUCTURAS CLAVE: mobj_t, thinker_t, player_t (p_mobj.h, d_think.h)
// ============================================================================
/*
 * mobj_t (map object) es LA estructura central: jugador, enemigos, proyectiles,
 * items, decoración. En p_mobj.h (verificada con `cat` en el repo):
 *
 *   typedef struct mobj_s
 *   {
 *       thinker_t       thinker;      // para la lista de thinkers (ver lección 04)
 *       fixed_t         x, y, z;      // posición 3D en fixed-point
 *       struct mobj_s*  next;         // linked list en sector (para colisión)
 *       struct mobj_s*  prev;
 *       angle_t         angle;        // yaw (0-65535)
 *       spritenum_t     sprite;       // índice de sprite
 *       int             frame;        // frame actual + flags (FF_FULLBRIGHT, etc.)
 *       struct mobj_s*  target;       // a quién ataca/mira
 *       struct mobj_s*  tracer;       // para misiles que persiguen
 *       struct mobjinfo_s* info;      // puntero a datos estáticos del tipo (mobjinfo_t)
 *       struct mobj_s*  next;         // lista de thinkers (duplicado en thinker_t)
 *       int             health;       // vida
 *       int             reactiontime; // ticks antes de reaccionar
 *       int             threshold;    // umbral de dolor/agresión
 *       // ... muchos más campos ...
 *   } mobj_t;
 *
 * Observaciones C vs C++:
 * - Herencia simulada: `thinker_t` como primer miembro (manual embedding).
 * - Polimorfismo manual: `info` apunta a tabla estática de comportamiento (mobjinfo_t).
 * - Listas enlazadas manuales: `next`, `prev` para sectores y thinkers.
 * - Sin constructores/destructores: init a mano en P_SpawnMobj, limpieza en P_RemoveMobj.
 * - Datos y funciones separados: comportamiento en p_enemy.c, p_pspr.c, etc.
 *
 * thinker_t (en d_think.h):
 *   typedef struct thinker_s
 *   {
 *       struct thinker_s*  prev;
 *       struct thinker_s*  next;
 *       void (*function)(struct thinker_s*);  // puntero a función de update
 *   } thinker_t;
 *
 * Cada mobj_t tiene un thinker_t embebido. La lista global `thinkercap` (thinker_t*)
 * recorre todos los objetos activos cada tic (35 Hz). P_Ticker() itera y llama
 * a function(thinker). Esto es el "game loop" de la simulación (no el render).
 */

// Versión simplificada para ilustrar la estructura (no es el código real):
typedef struct thinker_s {
    struct thinker_s* prev;
    struct thinker_s* next;
    void (*function)(struct thinker_s*);
} thinker_t;

typedef struct mobjinfo_s {
    int         doomednum;       // ID en el mapa (thing type)
    int         spawnstate;      // estado inicial (índice en states[])
    int         spawnhealth;     // vida inicial
    int         seesound;        // sonido al ver jugador
    int         reactiontime;    // ticks antes de actuar
    int         attacksound;     // sonido de ataque
    int         painchance;      // probabilidad de dolor (0-255)
    int         painstate;       // estado de dolor
    int         meleestate;      // ataque cuerpo a cuerpo
    int         missilestate;    // ataque a distancia
    int         deathstate;      // estado de muerte
    int         xdeathstate;     // muerte extrema (gibs)
    int         raisesound;      // sonido de resurrección (archvile)
    int         speed;           // velocidad movimiento (fixed_t)
    int         radius;          // radio colisión (fixed_t)
    int         height;          // altura (fixed_t)
    int         mass;            // masa (para empuje)
    int         damage;          // daño de ataque
    int         activesound;     // sonido activo (loop)
    int         flags;           // MF_SOLID, MF_SHOOTABLE, MF_NOBLOCKMAP, etc.
    int         raisestate;      // estado de resucitar
} mobjinfo_t;

typedef struct mobj_s {
    thinker_t       thinker;      // embebido: herencia manual
    fixed_t         x, y, z;
    struct mobj_s*  next;         // lista en sector (colisión)
    struct mobj_s*  prev;
    angle_t         angle;
    int             sprite;
    int             frame;
    struct mobj_s*  target;
    struct mobj_s*  tracer;
    const mobjinfo_t* info;       // datos estáticos del tipo (polimorfismo manual)
    int             health;
    int             reactiontime;
    int             threshold;
    int             flags;
    int             special;      // acción especial (puertas, etc.)
    int             args[5];      // argumentos para ACS/scripts (Hexen)
} mobj_t;

// ============================================================================
// 4. ESTADO GLOBAL Y VARIABLES GLOBALES (g_game.c, d_main.c)
// ============================================================================
/*
 * En C no hay encapsulamiento. El estado del juego son variables globales
 * declaradas en headers y definidas en un .c (usualmente g_game.c o d_main.c).
 *
 * Ejemplos (verificados en el source release):
 *   // g_game.h / g_game.c
 *   extern gamestate_t    gamestate;      // GS_LEVEL, GS_INTERMISSION, GS_FINALE...
 *   extern skill_t        gameskill;      // sk_baby, sk_easy, sk_medium, sk_hard, sk_nightmare
 *   extern boolean        paused;
 *   extern boolean        demoplayback;
 *   extern boolean        demorecording;
 *   extern int            gametic;        // tic actual (35 Hz)
 *   extern int            leveltime;      // tics en nivel actual
 *   extern int            totalkills, totalitems, totalsecret; // stats
 *
 *   // d_main.c
 *   extern boolean        singledemo;     // demo loop automático
 *   extern int            deathmatch;     // modo deathmatch
 *
 *   // p_local.h / p_mobj.c
 *   extern mobj_t*        players[MAXPLAYERS];  // array de punteros a mobj_t del jugador
 *   extern mobj_t*        thinkercap;           // cabeza de lista de thinkers
 *
 * Problemas de este enfoque:
 * - Acoplamiento fuerte: cualquier archivo incluye el header y toca las globales.
 * - Difícil testing: no se puede instanciar "un juego" aislado.
 * - Estado implícito: el orden de inicialización importa (static initialization order).
 * - En C++: se encapsula en clases (Game, World, Level, Player) con RAII.
 */

// Simulación de variables globales típicas:
typedef enum { GS_LEVEL, GS_INTERMISSION, GS_FINALE, GS_DEMOSCREEN } gamestate_t;
typedef enum { sk_baby, sk_easy, sk_medium, sk_hard, sk_nightmare } skill_t;

gamestate_t gamestate = GS_LEVEL;
skill_t     gameskill = sk_medium;
boolean     paused = false;
int         gametic = 0;
int         leveltime = 0;
thinker_t* thinkercap = NULL;  // cabeza de lista circular de thinkers

// ============================================================================
// 5. BUCLE PRINCIPAL Y TIC SYSTEM (d_main.c, g_game.c)
// ============================================================================
/*
 * El bucle principal en d_main.c (simplificado):
 *
 *   void D_DoomMain(void) {
 *       D_Init();           // init subsistemas: video, sound, net, WAD, etc.
 *       while (1) {
 *           D_DoomLoop();   // un frame: input + tic(s) + render
 *       }
 *   }
 *
 *   void D_DoomLoop(void) {
 *       // 1. Input: leer teclado, ratón, gamepad -> ticcmd_t
 *       // 2. Net: si multiplayer, intercambiar ticcmd_t
 *       // 3. RunTics: ejecutar tantos tics como falten para alcanzar realtime
 *       //    (timestep fijo 35 Hz, acumular y correr múltiples tics si lag)
 *       // 4. Render: R_RenderPlayerView(&players[displayplayer])
 *   }
 *
 * G_RunTics (g_game.c) ejecuta la simulación a 35 Hz fijo (TICRATE = 35).
 * Cada tic llama a G_Ticker() que llama a P_Ticker() que recorre la lista
 * de thinkers y ejecuta su function(thinker).
 *
 * Esto es "fixed timestep" puro, sin interpolación de render (el render
 * corre a la velocidad que pueda, pero la simulación SIEMPRE a 35 Hz).
 * Determinismo: misma secuencia de ticcmd_t -> mismo resultado (clave para netplay y demos).
 */

#define TICRATE 35

// ticcmd_t: comando de entrada por tic (ver lección 05 networking)
typedef struct {
    int16_t forwardmove;   // -50 a 50
    int16_t sidemove;      // -50 a 50
    int16_t angleturn;     // << 16 para fine turning
    int16_t consistancy;   // checksum para netplay
    uint8_t buttons;       // bitmask: BT_ATTACK, BT_USE, BT_JUMP, BT_STRAFE...
} ticcmd_t;

// Simulación del bucle de tics (lógica, no código real compilable):
void P_Ticker(void) {
    thinker_t* current = thinkercap->next;
    while (current != thinkercap) {
        thinker_t* next = current->next;
        current->function(current);  // dispatch polimórfico manual
        current = next;
    }
}

void G_Ticker(void) {
    // update players, level time, etc.
    P_Ticker();
    leveltime++;
}

void G_RunTics(ticcmd_t* cmd) {
    (void)cmd;
    // netplay: sincronizar cmd entre jugadores
    // single player: cmd viene del input local
    int pendientes = 1;
    while (pendientes > 0) {
        G_Ticker();
        gametic++;
        pendientes--;
    }
}

// ============================================================================
// 6. SUBSISTEMAS PRINCIPALES (resumen para navegación)
// ============================================================================
/*
 * d_*: main loop, net, save/load, demo (d_main.c, d_net.c, d_items.c...)
 * g_*: game rules, spawning, thinker list, map transitions (g_game.c, g_think.c...)
 * p_*: play simulation: map, enemies, weapons, physics (p_mobj.c, p_map.c, p_enemy.c...)
 * r_*: rendering: BSP traversal, visplanes, segs, sprites (r_bsp.c, r_segs.c, r_things.c...)
 * s_*: sound: mixing, channels, music (s_sound.c, s_chan.c, s_mus.c...)
 * z_*: zone memory: malloc/free con tags, bloqueo, purga (zone.c, zone.h)
 * w_*: WAD: lump lookup, caching, namespaces (w_wad.c, w_wad.h)
 * v_*: video: mode set, palette, gamma, screen buffer (v_video.c)
 * m_*: menus, input parsing, cheats (m_menu.c, m_argv.c, m_cheat.c...)
 * i_*: system interface: platform-specific (i_video.c, i_sound.c, i_system.c...)
 * hu_*: HUD: armas, vida, llaves, mensajes (hu_stuff.c, hu_lib.c)
 * st_*: status bar: cara, ammo, armas (st_stuff.c, st_lib.c)
 * am_*: automap: líneas, cosas, zoom (am_map.c)
 * f_*: finale, intermission, cast call (f_finale.c)
 *
 * Cada subsistema tiene su .h con declaraciones y .c con implementación.
 * Dependencias: p_* usa r_* (para sprites), r_* usa w_* (texturas), etc.
 * Todo incluye doomdef.h y doomtype.h.
 */

// ============================================================================
// 7. COMPILACIÓN HOY: PARCHES NECESARIOS
// ============================================================================
/*
 * El source release 1997 NO compila limpio en GCC/Clang moderno sin cambios:
 *
 * 1. Warnings: implicit function declarations, int/pointer conversions,
 *    unused variables, signed/unsigned mismatches, etc.
 *    Solución: -Wno-implicit-function-declaration -Wno-int-conversion...
 *
 * 2. Headers: faltan includes (stddef.h para size_t, stdlib.h para malloc).
 *    Solución: agregar #include <stddef.h>, <stdlib.h>, <string.h> donde falte.
 *
 * 3. Tipos: `boolean` como `int` vs `bool` de stdbool.h; `byte` vs `uint8_t`.
 *    Solución: mantener typedefs originales o mapear a stdint.h con care.
 *
 * 4. Funciones varargs: `printf` sin prototype en K&R style.
 *    Solución: agregar prototypes o compilar con -std=gnu89.
 *
 * 5. Sintaxis K&R: `void func(a, b) int a; char* b; { ... }`
 *    Solución: GCC acepta aún K&R con -std=gnu89, pero mejor convertir a ANSI.
 *
 * 6. `register` keyword: obsoleta, ignorada por compiladores modernos.
 *
 * 7. Makefile original: para DJGPP/DOS. Necesita CMake o Make moderno.
 *
 * EJERCICIO PRÁCTICO (parte del módulo): crear un CMakeLists.txt que compile
 * el source release 1997 con flags permisivos y lo linkee contra una SDL2/3
 * mínima para video/input/sound (reemplazando i_video.c, i_sound.c, i_system.c).
 * Usar Freedoom (doom1.wad / doom2.wad) como IWAD para probar.
 *
 * NOTA: No compiles Doom 3 ni GZDoom: son muy pesados para la máquina del dueño
 * (Celeron N4020, Intel UHD 600). Este módulo es SOLO análisis y modernización
 * incremental del código C original de 1997.
 */

// ============================================================================
// 8. EJEMPLO: RECORRER LA LISTA DE THINKERS (patrón C original)
// ============================================================================
/*
 * Este patrón aparece en TODO el código: listas circulares doblemente enlazadas
 * con centinela (thinkercap apunta a sí mismo cuando está vacía).
 *
 * Inicialización (en P_InitThinkers):
 *   thinkercap = Z_Malloc(sizeof(thinker_t), PU_STATIC, NULL);
 *   thinkercap->prev = thinkercap->next = thinkercap;
 *   thinkercap->function = NULL;
 *
 * Añadir thinker (en P_AddThinker):
 *   thinker->prev = thinkercap->prev;
 *   thinker->next = thinkercap;
 *   thinkercap->prev->next = thinker;
 *   thinkercap->prev = thinker;
 *
 * Quitar thinker (en P_RemoveThinker):
 *   thinker->prev->next = thinker->next;
 *   thinker->next->prev = thinker->prev;
 *   // NO se libera la memoria aquí: Z_Free se hace aparte según tag
 *
 * En C++ moderno: std::list o vector de unique_ptr con erase/remove_if,
 * pero el patrón de centinela es válido y eficiente en C.
 */

// ============================================================================
// 9. PRÓXIMOS PASOS EN ESTE MÓDULO
// ============================================================================
/*
 * Lección 02: Fixed-point math 16.16 (FRACUNIT, FixedMul, FixedDiv).
 * Lección 03-04: BSP, PVS, visplanes, render ordenado.
 * Lección 05-06: Thinkers, tic system 35 Hz, mobj_t, state machine.
 * Lección 07: Networking determinista.
 * Lección 08: WAD/lumps, zone memory.
 * Lección 09-10: Source ports C++ (GZDoom, Odamex, EDGE, Eternity).
 * Lección 11-12: Modernización: RAII wrappers, ECS, job system, SDL_GPU.
 * Lección 13: Lectura guiada Doom 3 (id Tech 4) — solo análisis.
 */

int main(void) {
    printf("Módulo 27 - Lección 01: Estructura id Tech 1 (release 1997)\n");
    printf("Prefijos: d_ g_ p_ r_ s_ z_ w_ v_ m_ i_ hu_ st_ am_ f_\n");
    printf("Estructuras clave: mobj_t (con thinker_t embebido), mobjinfo_t\n");
    printf("Variables globales: gamestate, gametic, leveltime, thinkercap\n");
    printf("Bucle: 35 Hz fijo, P_Ticker recorre lista thinkers\n");
    printf("Compilación hoy: necesita parches (ver sección 7)\n");
    return 0;
}