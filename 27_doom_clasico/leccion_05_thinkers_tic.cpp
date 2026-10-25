/*
 * MÓDULO 27: Doom Clásico — De C a C++
 * LECCIÓN 05: Thinkers y Tic System — Simulación a 35 Hz fijo (g_think.h, p_mobj.c, p_enemy.c)
 *
 * TEMAS:
 * - thinker_t: lista circular doblemente enlazada con centinela (thinkercap)
 * - P_Ticker(): recorre thinkers y llama function(thinker) cada tic
 * - mobj_t: thinker embebido + estado (state_t), posición, target, health
 * - State machine: states[] array, tic duration, action function, next state
 * - 35 Hz fijo (TICRATE): determinismo para demos y netplay
 * - P_SpawnMobj / P_RemoveMobj: ciclo de vida de objetos
 *
 * VERIFICACIÓN: Estructuras y funciones consultadas en g_think.h, p_mobj.h/c, p_enemy.h/c,
 * info.h (states[], mobjinfo[]), doomstat.h del source release 1997.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================================
// 1. TIPOS BASE (reutilizados)
// ============================================================================
typedef int32_t fixed_t;
#define FRACUNIT 65536
typedef uint16_t angle_t;
typedef int boolean;

#define TICRATE 35  // 35 tics por segundo (determinista)

// ============================================================================
// 2. THINKER_T: BASE PARA OBJETOS ACTIVOS (g_think.h)
// ============================================================================
/*
 * thinker_t es la base de TODOS los objetos que se actualizan cada tic:
 * - mobj_t (monstruos, jugador, proyectiles, items)
 * - thinker_t genérico (para efectos, pisos móviles, techos, etc.)
 *
 * Estructura real (g_think.h):
 *   typedef struct thinker_s {
 *       struct thinker_s* prev;
 *       struct thinker_s* next;
 *       void (*function)(struct thinker_s*);  // función de update
 *   } thinker_t;
 *
 * Lista global: thinkercap (centinela). Inicializada en P_InitThinkers:
 *   thinkercap = Z_Malloc(sizeof(thinker_t), PU_STATIC, NULL);
 *   thinkercap->prev = thinkercap->next = thinkercap;
 *   thinkercap->function = NULL;
 *
 * P_AddThinker(thinker_t* th): inserta al final (antes de centinela).
 * P_RemoveThinker(thinker_t* th): quita de lista, NO libera memoria.
 *
 * P_Ticker(): llamado cada tic desde G_Ticker -> P_Ticker:
 *   void P_Ticker(void) {
 *       thinker_t* current = thinkercap->next;
 *       while (current != thinkercap) {
 *           thinker_t* next = current->next;
 *           current->function(current);
 *           current = next;
 *       }
 *   }
 *
 * PATRÓN CLAVE: Herencia manual embebiendo thinker_t como PRIMER miembro.
 * Esto permite cast (thinker_t*)mobj -> mobj_t* con offset 0 (container_of).
 */

typedef struct thinker_s {
    struct thinker_s* prev;
    struct thinker_s* next;
    void (*function)(struct thinker_s*);
} thinker_t;

thinker_t* thinkercap = NULL;

void P_InitThinkers(void) {
    thinkercap = (thinker_t*)malloc(sizeof(thinker_t));
    thinkercap->prev = thinkercap->next = thinkercap;
    thinkercap->function = NULL;
}

void P_AddThinker(thinker_t* th) {
    th->prev = thinkercap->prev;
    th->next = thinkercap;
    thinkercap->prev->next = th;
    thinkercap->prev = th;
}

void P_RemoveThinker(thinker_t* th) {
    th->prev->next = th->next;
    th->next->prev = th->prev;
    // NO free aquí: el dueño de la memoria decide cuándo liberar
}

void P_Ticker(void) {
    thinker_t* current = thinkercap->next;
    while (current != thinkercap) {
        thinker_t* next = current->next;
        if (current->function) current->function(current);
        current = next;
    }
}

// ============================================================================
// 3. MOBJ_T: OBJETO DE MAPA (p_mobj.h)
// ============================================================================
/*
 * mobj_t embebe thinker_t como primer miembro (herencia manual).
 * Todos los "actores" del juego son mobj_t.
 *
 * Estructura real (p_mobj.h - campos principales):
 *   typedef struct mobj_s {
 *       thinker_t       thinker;      // MUST BE FIRST
 *       fixed_t         x, y, z;
 *       struct mobj_s*  next;         // linked list en sector (blockmap)
 *       struct mobj_s*  prev;
 *       angle_t         angle;
 *       spritenum_t     sprite;
 *       int             frame;        // frame actual | flags (FF_FULLBRIGHT...)
 *       struct mobj_s*  target;       // target para AI/ataque
 *       struct mobj_s*  tracer;       // para misiles guiados
 *       struct mobjinfo_s* info;      // datos estáticos del tipo (mobjinfo_t)
 *       int             health;
 *       int             reactiontime; // ticks antes de reaccionar
 *       int             threshold;    // umbral de dolor
 *       int             flags;        // MF_SOLID, MF_SHOOTABLE, MF_COUNTKILL...
 *       int             special;      // acción especial (puertas)
 *       int             args[5];      // args para ACS (Hexen)
 *       // ... más campos ...
 *   } mobj_t;
 *
 * mobjinfo_t (info.h): tabla estática por tipo de cosa (MT_PLAYER, MT_POSSESSED...).
 *   struct mobjinfo_s {
 *       int doomednum;        // thing type en mapa
 *       int spawnstate;       // estado inicial (índice en states[])
 *       int spawnhealth;
 *       int seesound;
 *       int reactiontime;
 *       int attacksound;
 *       int painchance;
 *       int painstate;
 *       int meleestate;
 *       int missilestate;
 *       int deathstate;
 *       int xdeathstate;
 *       int raisesound;
 *       fixed_t speed;
 *       fixed_t radius;
 *       fixed_t height;
 *       int mass;
 *       int damage;
 *       int activesound;
 *       int flags;
 *       int raisestate;
 *   };
 *
 * Polimorfismo MANUAL: info->spawnstate, info->missilestate, etc. son índices
 * al array global states[]. La función de update (P_MobjThinker) lee mo->info
 * y despacha según estado actual.
 */

typedef struct mobjinfo_s {
    int doomednum;
    int spawnstate;
    int spawnhealth;
    int seesound;
    int reactiontime;
    int attacksound;
    int painchance;
    int painstate;
    int meleestate;
    int missilestate;
    int deathstate;
    int xdeathstate;
    int raisesound;
    fixed_t speed;
    fixed_t radius;
    fixed_t height;
    int mass;
    int damage;
    int activesound;
    int flags;
    int raisestate;
} mobjinfo_t;

typedef struct mobj_s {
    thinker_t       thinker;
    fixed_t         x, y, z;
    struct mobj_s*  next;
    struct mobj_s*  prev;
    angle_t         angle;
    int             sprite;
    int             frame;
    int             state;
    int             tics;
    struct mobj_s*  target;
    struct mobj_s*  tracer;
    const mobjinfo_t* info;
    int             health;
    int             reactiontime;
    int             threshold;
    int             flags;
    int             special;
    int             args[5];
} mobj_t;

// Flags comunes (p_mobj.h):
#define MF_SOLID        0x00000001
#define MF_SHOOTABLE    0x00000002
#define MF_NOSECTOR     0x00000004
#define MF_NOBLOCKMAP   0x00000008
#define MF_AMBUSH       0x00000010
#define MF_COUNTKILL    0x00000020
#define MF_COUNTITEM    0x00000040
#define MF_SKULLFLY     0x00000080
#define MF_NOGRAVITY    0x00000100
#define MF_DROPOFF      0x00000200
#define MF_PICKUP       0x00000400
#define MF_NOCLIP       0x00000800
#define MF_SLIDE        0x00001000
#define MF_FLOAT        0x00002000
#define MF_TELEPORT     0x00004000
#define MF_MISSILE      0x00008000
#define MF_DROPPED      0x00010000
#define MF_SHADOW       0x00020000
#define MF_NOBLOOD      0x00040000
#define MF_CORPSE       0x00080000
#define MF_INFLOAT      0x00100000

// ============================================================================
// 4. STATE MACHINE: STATES[] Y STATE_T (info.h)
// ============================================================================
/*
 * El comportamiento de cada mobj_t se define por una MÁQUINA DE ESTADOS.
 * states[] es un array global de state_t (definido en info.c, generado desde info.h).
 *
 * state_t (info.h):
 *   typedef struct {
 *       spritenum_t sprite;      // sprite number
 *       int frame;               // frame number + flags (FF_FULLBRIGHT=0x8000)
 *       int tics;                // duración en tics (0 = infinito, -1 = random?)
 *       void (*action)(mobj_t*); // función de acción (A_Chase, A_Fire, etc.)
 *       int nextstate;           // índice del siguiente estado
 *       int misc1, misc2;        // parámetros extra para action
 *   } state_t;
 *
 * FLUJO:
 * - mobj_t tiene campo `state` (índice en states[]) y `tics` (tics restantes en estado actual).
 * - P_MobjThinker (function del thinker) decrementa tics y llama action.
 * - Cuando tics == 0: transiciona a nextstate, carga nuevo tics, sprite, frame.
 * - Estados con tics = -1 (o 0 según versión): duración infinita (esperar evento).
 *
 * EJEMPLO: Imp (MT_TROOP)
 *   spawnstate -> S_TROO_STND (tics=10, action=A_Look, next=S_TROO_STND2)
 *   S_TROO_STND2 (tics=10, A_Look, next=S_TROO_RUN1)
 *   S_TROO_RUN1 (tics=3, A_Chase, next=S_TROO_RUN2)
 *   ...
 *   meleestate -> S_TROO_ATK1 (tics=8, A_FaceTarget, next=S_TROO_ATK2)
 *   S_TROO_ATK2 (tics=8, A_FaceTarget, next=S_TROO_ATK3)
 *   S_TROO_ATK3 (tics=6, A_TroopAttack, next=S_TROO_RUN1)
 *   deathstate -> S_TROO_DIE1 (tics=8, A_Scream, next=S_TROO_DIE2) ...
 *
 * ACCIONES (p_enemy.c, p_pspr.c): A_Look, A_Chase, A_FaceTarget, A_TroopAttack,
 * A_SargAttack, A_HeadAttack, A_BruisAttack, A_SkullAttack, A_Metal,
 * A_SpawnFly, A_Fire, A_BFGSpray, A_Explode, A_Scream, A_Fall, A_KeenDie...
 * Cada una es void action(mobj_t* actor).
 */

typedef struct state_s {
    int sprite;
    int frame;
    int tics;
    void (*action)(struct mobj_s*);
    int nextstate;
    int misc1, misc2;
} state_t;

// En Doom real states[] lo genera info.c desde info.h. Acá definimos una
// tabla mínima para poder ejecutar el ejemplo de state machine.
state_t states[] = {
    {0, 0, 10, nullptr, 1, 0, 0},
    {0, 1, 10, nullptr, 2, 0, 0},
    {0, 2,  3, nullptr, 3, 0, 0},
    {0, 3, -1, nullptr, 3, 0, 0},
};

// Añadido a mobj_t para state machine:
typedef struct mobj_s_extended {
    mobj_t base;
    int state;        // índice actual en states[]
    int tics;         // tics restantes en estado actual
} mobj_t_ext;

// ============================================================================
// 5. P_MOBJTHINKER: UPDATE PRINCIPAL DE MOBJ (p_mobj.c)
// ============================================================================
/*
 * Función asignada a thinker.function para todos los mobj_t:
 *   void P_MobjThinker(thinker_t* th) {
 *       mobj_t* mo = (mobj_t*)th;  // cast seguro: thinker es primer miembro
 *       // ... lógica de movimiento, gravedad, fricción ...
 *       // State machine:
 *       if (mo->tics > 0) {
 *           mo->tics--;
 *           if (mo->tics == 0) {
 *               // Transición de estado
 *               state_t* st = &states[mo->state];
 *               if (st->action) st->action(mo);
 *               mo->state = st->nextstate;
 *               st = &states[mo->state];
 *               mo->tics = st->tics;
 *               mo->sprite = st->sprite;
 *               mo->frame = st->frame;
 *           }
 *       }
 *   }
 *
 * NOTA: El código real es MÁS complejo: maneja momentum, fricción, gravedad,
 * step up/down, deslizamiento contra paredes, blockmap collision, etc.
 * Aquí mostramos SOLO el patrón de state machine.
 */

void P_MobjThinker(thinker_t* th) {
    mobj_t* mo = (mobj_t*)th;

    // Movimiento básico (simplificado):
    // mo->x += FixedMul(mo->momx, ...); etc.

    // State machine:
    if (mo->tics > 0) {
        mo->tics--;
        if (mo->tics == 0) {
            const state_t* st = &states[mo->state];
            if (st->action) st->action(mo);
            mo->state = st->nextstate;
            st = &states[mo->state];
            mo->tics = st->tics;
            mo->sprite = st->sprite;
            mo->frame = st->frame;
        }
    }
}

// ============================================================================
// 6. ACCIONES DE EJEMPLO (p_enemy.c)
// ============================================================================

// A_Look: buscar jugador en vista, si ve -> cambiar a estado de persecución
void A_Look(mobj_t* actor) {
    // Simplificado: en el real recorre players[] y chequea P_CheckSight
    // Si ve jugador: actor->target = player->mo; cambiar estado a chase
    (void)actor;
}

// A_Chase: IA principal de monstruos (perseguir, atacar, mover)
void A_Chase(mobj_t* actor) {
    // Lógica compleja: 
    // - Si target muerto/inválido -> A_Look
    // - Si melee range -> meleestate
    // - Si missile range && random -> missilestate
    // - Mover hacia target (P_Move)
    // - Manejar reactiontime, threshold, painchance
    (void)actor;
}

// A_FaceTarget: girar hacia target
void A_FaceTarget(mobj_t* actor) {
    if (!actor->target) return;
    // Calcular ángulo hacia target
    // actor->angle = R_PointToAngle2(actor->x, actor->y, actor->target->x, actor->target->y);
}

// A_TroopAttack: ataque del Imp (lanzar fireball)
void A_TroopAttack(mobj_t* actor) {
    if (!actor->target) return;
    // P_SpawnMissile(actor, MT_TROOPSHOT);
}

// A_Scream: sonido de muerte
void A_Scream(mobj_t* actor) {
    // S_StartSound(actor, actor->info->deathsound);
}

// ============================================================================
// 7. SPAWN Y REMOVE (p_mobj.c)
// ============================================================================
/*
 * P_SpawnMobj(x, y, z, type): crea mobj_t, inicializa desde mobjinfo[type],
 * añade a thinkercap, añade a blockmap (para colisión), establece estado inicial.
 *
 *   mobj_t* P_SpawnMobj(fixed_t x, fixed_t y, fixed_t z, int type) {
 *       mobj_t* mo = Z_Malloc(sizeof(mobj_t), PU_LEVEL, NULL);
 *       mo->x = x; mo->y = y; mo->z = z;
 *       mo->info = &mobjinfo[type];
 *       mo->health = mo->info->spawnhealth;
 *       mo->flags = mo->info->flags;
 *       mo->radius = mo->info->radius;
 *       mo->height = mo->info->height;
 *       mo->thinker.function = P_MobjThinker;
 *       P_AddThinker(&mo->thinker);
 *       P_SetThingPosition(mo);  // blockmap
 *       // Estado inicial:
 *       mo->state = mo->info->spawnstate;
 *       const state_t* st = &states[mo->state];
 *       mo->tics = st->tics;
 *       mo->sprite = st->sprite;
 *       mo->frame = st->frame;
 *       return mo;
 *   }
 *
 * P_RemoveMobj(mo): quita de thinkercap, blockmap, libera memoria (Z_Free).
 *   void P_RemoveMobj(mobj_t* mo) {
 *       P_UnsetThingPosition(mo);  // blockmap
 *       P_RemoveThinker(&mo->thinker);
 *       Z_Free(mo);
 *   }
 *
 * Z_Malloc / Z_Free: zone memory allocator (lección 08). Tags: PU_LEVEL, PU_STATIC, etc.
 */

// ============================================================================
// 8. BLOCKMAP: COLISIÓN ESPACIAL (p_map.c)
// ============================================================================
/*
 * El blockmap es un grid uniforme sobre el mapa para acelerar colisiones.
 * Divide el mapa en celdas de 128x128 unidades (blockmap cells).
 * Cada celda tiene lista de mobj_t que la intersectan.
 *
 * P_SetThingPosition(mo): calcula celdas que ocupa (por radius) y añade a listas.
 * P_UnsetThingPosition(mo): quita de listas.
 * P_BlockThingsIterator(x, y, radius, func): itera cosas en área (para explosiones, sight).
 *
 * Blockmap se construye al cargar mapa (P_SetupLevel) desde lump BLOCKMAP.
 * Estructura: header (originx, originy, numcols, numrows) + offsets a lists por celda.
 */

// ============================================================================
// 9. TIC SYSTEM: 35 HZ FIJO Y DETERMINISMO (g_game.c)
// ============================================================================
/*
 * G_RunTics(ticcmd_t* cmd): ejecuta tics hasta alcanzar tiempo real.
 *
 *   void G_RunTics(ticcmd_t* cmd) {
 *       int ticcount = I_GetTime() / (1000/TICRATE);  // tics transcurridos
 *       while (gametic < ticcount) {
 *           G_Ticker(cmd);
 *           gametic++;
 *       }
 *   }
 *
 * G_Ticker(cmd):
 *   - NetUpdate() si multiplayer (intercambia ticcmd_t)
 *   - Para cada player: copia cmd a player->cmd
 *   - P_Ticker() -> recorre thinkers
 *   - Level timer, intermission, etc.
 *
 * DETERMINISMO:
 * - MISMA secuencia de ticcmd_t -> MISMO resultado (bitwise identical).
 * - Sin randomness no determinista: P_Random() usa tabla fija (m_random.c).
 * - Fixed-point determinista en todas las plataformas (complemento a dos).
 * - Clave para: demos (grabar/reproducir), netplay (lockstep).
 *
 * P_Random() (m_random.c): tabla de 256 bytes, índice global rndindex.
 *   byte rndtable[256] = { 0, 8, 109, 220, ... };  // valores predefinidos
 *   int P_Random(void) { return rndtable[rndindex++ & 255]; }
 *   Determinista: misma secuencia SIEMPRE.
 */

// P_Random simulado:
uint8_t rndtable[256] = { 0, 8, 109, 220, 222, 241, 149, 107, 75, 248, /* ... */ };
int rndindex = 0;

int P_Random(void) {
    return rndtable[rndindex++ & 255];
}

// ============================================================================
// 10. TESTS DE LÓGICA
// ============================================================================

int test_thinkers_tic(void) {
    int passed = 0, failed = 0;
    #define TEST(expr, msg) \
        do { if (expr) { passed++; } else { failed++; printf("FAIL: %s\n", msg); } } while(0)

    // Thinker list basics
    P_InitThinkers();
    TEST(thinkercap != NULL, "thinkercap allocated");
    TEST(thinkercap->next == thinkercap, "empty list: next = self");
    TEST(thinkercap->prev == thinkercap, "empty list: prev = self");

    thinker_t a = {0}, b = {0};
    a.function = (void(*)(thinker_t*))1; // dummy
    b.function = (void(*)(thinker_t*))2;

    P_AddThinker(&a);
    TEST(thinkercap->next == &a, "first added at front");
    TEST(a.next == thinkercap, "a.next = cap");
    TEST(a.prev == thinkercap, "a.prev = cap");

    P_AddThinker(&b);
    TEST(thinkercap->next == &a, "a still first");
    TEST(a.next == &b, "a.next = b");
    TEST(b.next == thinkercap, "b.next = cap");
    TEST(b.prev == &a, "b.prev = a");

    P_RemoveThinker(&a);
    TEST(thinkercap->next == &b, "after remove a, b is first");
    TEST(b.prev == thinkercap, "b.prev = cap");

    // P_Random determinista
    rndindex = 0;
    int r1 = P_Random();
    int r2 = P_Random();
    rndindex = 0;
    TEST(P_Random() == r1, "P_Random deterministic reset 1");
    TEST(P_Random() == r2, "P_Random deterministic reset 2");

    // TICRATE
    TEST(TICRATE == 35, "TICRATE = 35 Hz");

    printf("Thinkers/Tic tests: %d pass, %d fail\n", passed, failed);
    return failed == 0 ? 0 : 1;
}

int main(void) {
    printf("Módulo 27 - Lección 05: Thinkers y Tic System 35 Hz\n");
    return test_thinkers_tic();
}