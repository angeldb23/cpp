/*
 * MÓDULO 27: Doom Clásico — De C a C++
 * LECCIÓN 06: State Machine en Detalle y IA de Monstruos (info.c, p_enemy.c, p_pspr.c)
 *
 * TEMAS:
 * - Array states[]: definición completa, transiciones, duración, acciones
 * - Frame flags: FF_FULLBRIGHT, FF_FRAMEMASK
 * - Acciones de IA: A_Chase, A_FaceTarget, A_Look, ataques (melee/missile)
 * - Pain state, death state, resurrection (Archvile)
 * - Player weapon states (PSprites): ready, fire, flash, reload
 * - Tablas de datos: mobjinfo[] (por tipo), states[] (por estado)
 *
 * VERIFICACIÓN: info.c, p_enemy.c, p_pspr.c del source release 1997.
 * NOTA: info.c se genera parcialmente desde info.h con scripts; el array states[]
 * es GRANDE (~1000+ entradas). Aquí documentamos el PATRÓN, no cada entrada.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// ============================================================================
// 1. TIPOS BASE (reutilizados)
// ============================================================================
typedef int32_t fixed_t;
#define FRACUNIT 65536
typedef uint16_t angle_t;

#define FF_FULLBRIGHT 0x8000  // bit 15 en frame: no afecta luz
#define FF_FRAMEMASK  0x7FFF  // máscara para número de frame

// ============================================================================
// 2. STATE_T Y STATES[] (info.h, info.c)
// ============================================================================
/*
 * state_t define UN estado en la máquina de estados.
 * El array global states[] contiene TODOS los estados de TODOS los objetos.
 *
 * typedef struct {
 *     int sprite;           // sprite number (SPR_TROO, SPR_PLAY, etc.)
 *     int frame;            // frame number | flags (FF_FULLBRIGHT)
 *     int tics;             // duración en tics (0 = wait for trigger, -1 = random?)
 *     void (*action)(mobj_t*); // función de acción (puede ser NULL)
 *     int nextstate;        // índice del siguiente estado en states[]
 *     int misc1, misc2;     // parámetros extra para action
 * } state_t;
 *
 * CAMPOS:
 * - sprite: índice en sprnames[] (nombres de sprite: TROO, PLAY, BKEY, etc.)
 * - frame: 0-32767 = frame number. Bit 15 (0x8000) = FF_FULLBRIGHT (brillo completo,
 *   ignora colormap/lightlevel). Usado para ojos brillantes, proyectiles, etc.
 * - tics: cuántos tics dura este estado.
 *   - tics > 0: duración fija.
 *   - tics == 0: estado "wait" (no avanza automático; transición por evento externo).
 *   - tics == -1: en algunos ports, duración aleatoria (Doom original usa 0 para wait).
 * - action: puntero a función void action(mobj_t*). Llamada CADA tic del estado
 *   (en P_MobjThinker, ANTES de decrementar tics o DESPUÉS según versión).
 *   Acciones típicas: A_Look, A_Chase, A_FaceTarget, A_TroopAttack, A_SargAttack,
 *   A_HeadAttack, A_BruisAttack, A_SkullAttack, A_Metal, A_SpawnFly, A_Fire,
 *   A_BFGSpray, A_Explode, A_Scream, A_Fall, A_KeenDie, A_BrainPain, A_BrainScream...
 * - nextstate: índice en states[] del siguiente estado. -1 = estado terminal (remove).
 * - misc1, misc2: parámetros específicos de la acción (ej: misc1 = damage para A_Explode).
 *
 * TRANSICIÓN (en P_MobjThinker):
 *   if (mo->tics > 0) {
 *       mo->tics--;
 *       if (mo->tics == 0) {
 *           const state_t* st = &states[mo->state];
 *           if (st->action) st->action(mo);  // acción al FINAL del estado
 *           mo->state = st->nextstate;
 *           st = &states[mo->state];
 *           mo->tics = st->tics;
 *           mo->sprite = st->sprite;
 *           mo->frame = st->frame;
 *       }
 *   } else if (mo->tics == 0) {
 *       // estado wait: no avanza solo. Transición externa (ej: daño -> painstate)
 *   }
 *
 * IMPORTANTE: La acción se llama cuando tics llega a 0 (FIN del estado),
 * no al inicio. Esto significa que A_Chase se ejecuta al final de cada
 * estado de persecución, ANTES de transicionar al siguiente frame de animación.
 */

typedef struct state_s {
    int sprite;
    int frame;
    int tics;
    void (*action)(struct mobj_s*);
    int nextstate;
    int misc1, misc2;
} state_t;

// Declaración externa (definido en info.c, ~1000+ entradas)
extern state_t states[];
extern int NUMSTATES;  // número total de estados

// ============================================================================
// 3. MOBJINFO_T: DATOS ESTÁTICOS POR TIPO (info.h)
// ============================================================================
/*
 * mobjinfo_t define las propiedades BASE de cada tipo de cosa (MT_*).
 * Índices: MT_PLAYER, MT_POSSESSED, MT_SHOTGUY, MT_CHAINGUY, MT_TROOP (Imp),
 * MT_SERGEANT, MT_SHADOWS (Spectre), MT_HEAD (Cacodemon), MT_BRUISER (Baron),
 * MT_UNDEAD (Revenant), MT_FATSO (Mancubus), MT_CYBORG, MT_SPIDER (Mastermind),
 * MT_BABY (Arachnotron), MT_PAIN (Pain Elemental), MT_WOLF (Wolfenstein SS),
 * MT_KEEN (Commander Keen), MT_BOSSBRAIN, MT_BOSSSPIT, MT_BOSSTARGET...
 *
 * Cada entrada mapea a spawnstate, deathstate, etc. (índices en states[]).
 */

typedef struct mobjinfo_s {
    int doomednum;        // thing type number en mapa (0 = no spawnable)
    int spawnstate;       // estado inicial (índice en states[])
    int spawnhealth;
    int seesound;         // sound num al ver jugador (sfx_*)
    int reactiontime;     // tics antes de reaccionar (0 = instant)
    int attacksound;      // sound num al atacar
    int painchance;       // 0-255: probabilidad de entrar en painstate al recibir daño
    int painstate;        // estado al recibir daño
    int meleestate;       // estado ataque melee
    int missilestate;     // estado ataque a distancia
    int deathstate;       // estado muerte normal
    int xdeathstate;      // muerte extrema (gibs, daño extremo)
    int raisesound;       // sonido resurrección (Archvile)
    fixed_t speed;        // velocidad movimiento (units/tic)
    fixed_t radius;       // radio colisión
    fixed_t height;       // altura
    int mass;             // masa (resistencia a empuje)
    int damage;           // daño de ataque melee
    int activesound;      // sonido loop activo (ej: cacodemon hiss)
    int flags;            // MF_* flags
    int raisestate;       // estado resurrección (Archvile)
} mobjinfo_t;

extern mobjinfo_t mobjinfo[];  // array global, indexado por MT_*

// mobj_t: en Doom original es un struct enorme (p_mobj.h). Acá dejamos solo
// los campos que tocan las acciones de IA de esta lección.
struct mobj_s {
    fixed_t x = 0, y = 0, z = 0;
    angle_t angle = 0;
    int health = 0;
    int reactiontime = 0;
    int threshold = 0;
    mobj_s* target = nullptr;
    mobjinfo_t* info = nullptr;
};
using mobj_t = mobj_s;

void A_FaceTarget(mobj_t* actor);
extern int NUMMOBJTYPES;

// ============================================================================
// 4. EJEMPLO: ESTADOS DEL IMP (MT_TROOP)
// ============================================================================
/*
 * En info.c (generado), los estados del Imp (MT_TROOP) son aproximadamente:
 *
 * // Spawn / Stand
 * S_TROO_STND    = {SPR_TROO, 0, 10, A_Look,       S_TROO_STND2,  0,0},
 * S_TROO_STND2   = {SPR_TROO, 1, 10, A_Look,       S_TROO_STND,   0,0},
 * // Chase (walk)
 * S_TROO_RUN1    = {SPR_TROO, 0,  3, A_Chase,      S_TROO_RUN2,   0,0},
 * S_TROO_RUN2    = {SPR_TROO, 0,  3, A_Chase,      S_TROO_RUN3,   0,0},
 * S_TROO_RUN3    = {SPR_TROO, 1,  3, A_Chase,      S_TROO_RUN4,   0,0},
 * S_TROO_RUN4    = {SPR_TROO, 1,  3, A_Chase,      S_TROO_RUN5,   0,0},
 * S_TROO_RUN5    = {SPR_TROO, 2,  3, A_Chase,      S_TROO_RUN6,   0,0},
 * S_TROO_RUN6    = {SPR_TROO, 2,  3, A_Chase,      S_TROO_RUN7,   0,0},
 * S_TROO_RUN7    = {SPR_TROO, 3,  3, A_Chase,      S_TROO_RUN8,   0,0},
 * S_TROO_RUN8    = {SPR_TROO, 3,  3, A_Chase,      S_TROO_RUN1,   0,0},
 * // Melee attack
 * S_TROO_ATK1    = {SPR_TROO, 4,  8, A_FaceTarget, S_TROO_ATK2,   0,0},
 * S_TROO_ATK2    = {SPR_TROO, 5,  8, A_FaceTarget, S_TROO_ATK3,   0,0},
 * S_TROO_ATK3    = {SPR_TROO, 6,  6, A_TroopAttack,S_TROO_RUN1,   0,0},
 * // Missile attack (Imp lanza fireball)
 * S_TROO_MIS1    = {SPR_TROO, 7,  8, A_FaceTarget, S_TROO_MIS2,   0,0},
 * S_TROO_MIS2    = {SPR_TROO, 8,  8, A_FaceTarget, S_TROO_MIS3,   0,0},
 * S_TROO_MIS3    = {SPR_TROO, 9,  6, A_TroopAttack,S_TROO_RUN1,   0,0},
 * // Pain
 * S_TROO_PAIN    = {SPR_TROO, 10, 6, NULL,         S_TROO_RUN1,   0,0},
 * S_TROO_PAIN2   = {SPR_TROO, 11, 6, A_Pain,       S_TROO_RUN1,   0,0},
 * // Death
 * S_TROO_DIE1    = {SPR_TROO, 12, 8, A_Scream,     S_TROO_DIE2,   0,0},
 * S_TROO_DIE2    = {SPR_TROO, 13, 8, A_Fall,       S_TROO_DIE3,   0,0},
 * S_TROO_DIE3    = {SPR_TROO, 14, 6, NULL,         S_TROO_DIE4,   0,0},
 * S_TROO_DIE4    = {SPR_TROO, 15, 6, NULL,         S_TROO_DIE5,   0,0},
 * S_TROO_DIE5    = {SPR_TROO, 16, -1, NULL,        -1,            0,0},  // tics=-1 = stay forever (corpse)
 * // XDeath (gibs)
 * S_TROO_XDIE1   = {SPR_TROO, 17, 5, A_Scream,     S_TROO_XDIE2,  0,0},
 * ...
 * S_TROO_XDIE5   = {SPR_TROO, 21, -1, NULL,        -1,            0,0},
 * // Raise (resurrection by Archvile)
 * S_TROO_RAISE1  = {SPR_TROO, 16, 6, NULL,         S_TROO_RAISE2, 0,0},
 * S_TROO_RAISE2  = {SPR_TROO, 15, 6, NULL,         S_TROO_RAISE3, 0,0},
 * S_TROO_RAISE3  = {SPR_TROO, 14, 6, NULL,         S_TROO_RAISE4, 0,0},
 * S_TROO_RAISE4  = {SPR_TROO, 13, 6, A_Scream,     S_TROO_RUN1,   0,0},
 *
 * En mobjinfo[MT_TROOP]:
 *   spawnstate = S_TROO_STND
 *   seeState   = S_TROO_RUN1  (en info.h hay seeState, no en struct original? Verificar)
 *   painstate  = S_TROO_PAIN
 *   meleestate = S_TROO_ATK1
 *   missilestate = S_TROO_MIS1
 *   deathstate = S_TROO_DIE1
 *   xdeathstate = S_TROO_XDIE1
 *   raisestate = S_TROO_RAISE1
 *
 * NOTA: El struct mobjinfo_t original NO tiene seeState. La transición
 * de "ver jugador" se maneja en A_Look: si ve, cambia estado a missilestate
 * o meleestate según distancia. En source ports modernos se añadió seeState.
 */

// Sprite numbers (sprnames[] en info.c):
#define SPR_TROO  10  // Imp
#define SPR_PLAY  0   // Player
#define SPR_MISL  20  // Rocket
#define SPR_BFS1  30  // BFG shot

// State indices (ejemplos, valores reales en info.c):
#define S_TROO_STND   100
#define S_TROO_RUN1   102
#define S_TROO_ATK1   112
#define S_TROO_MIS1   115
#define S_TROO_PAIN   118
#define S_TROO_DIE1   120
#define S_TROO_XDIE1  125
#define S_TROO_RAISE1 130

// ============================================================================
// 5. ACCIONES DE IA DETALLADAS (p_enemy.c)
// ============================================================================

// A_Look: Buscar jugador en línea de vista
// Llamada cada tic en estados de "stand" (spawnstate loop).
void A_Look(mobj_t* actor) {
    // 1. Si reactiontime > 0: decrementar y volver (esperar)
    if (actor->reactiontime) {
        actor->reactiontime--;
        return;
    }

    // 2. Si threshold > 0: decrementar (retraso antes de atacar de nuevo)
    if (actor->threshold) actor->threshold--;

    // 3. Buscar target: recorrer players[] (max 4 en Doom original)
    //    Chequear P_CheckSight(actor, player->mo)
    //    Si ve: actor->target = player->mo; cambiar estado a chase
    //    En código real: for (i=0; i<MAXPLAYERS; i++) if (playeringame[i] && P_CheckSight...)
    //    Si encuentra: P_SetMobjState(actor, actor->info->meleestate) o missilestate

    // Simplificado:
    (void)actor;
}

// A_Chase: IA principal de movimiento y decisión
void A_Chase(mobj_t* actor) {
    // 1. Si no hay target o target muerto -> A_Look
    if (!actor->target || actor->target->health <= 0) {
        // P_SetMobjState(actor, actor->info->spawnstate); // volver a stand
        return;
    }

    // 2. Face target (girar hacia target)
    A_FaceTarget(actor);

    // 3. Chequear distancia para melee vs missile
    // fixed_t dist = P_AproxDistance(actor->x - actor->target->x, actor->y - actor->target->y);
    // if (dist < MELEERANGE) -> melee
    // else if (P_CheckMissileRange(actor)) -> missile

    // 4. Movimiento: P_Move(actor) -> intenta mover hacia target
    //    Maneja deslizamiento contra paredes, step up (24 unidades), drop off.

    // 5. Reacción al daño: si actor->threshold > 0, no ataca todavía
    //    painchance: al recibir daño, P_DamageMobj llama P_SetMobjState(painstate)

    (void)actor;
}

// A_FaceTarget: Girar hacia target (suavizado)
void A_FaceTarget(mobj_t* actor) {
    if (!actor->target) return;
    // angle = R_PointToAngle2(actor->x, actor->y, actor->target->x, actor->target->y);
    // actor->angle = angle; // instant turn en original; ports suavizan
}

// A_TroopAttack: Imp lanza fireball (MT_TROOPSHOT)
void A_TroopAttack(mobj_t* actor) {
    if (!actor->target) return;
    // P_SpawnMissile(actor, MT_TROOPSHOT);
    // S_StartSound(actor, sfx_impatt); // sonido ataque
}

// A_SargAttack: Demon mordida (melee)
void A_SargAttack(mobj_t* actor) {
    if (!actor->target) return;
    // if (P_CheckMeleeRange(actor)) P_DamageMobj(actor->target, actor, actor, damage);
}

// A_HeadAttack: Cacodemon melee
void A_HeadAttack(mobj_t* actor) { (void)actor; }

// A_BruisAttack: Baron melee + missile (scratch + fireball)
void A_BruisAttack(mobj_t* actor) { (void)actor; }

// A_SkullAttack: Lost Soul charge
void A_SkullAttack(mobj_t* actor) { (void)actor; }

// A_Metal: Sonido rebote (proyectil choca pared)
void A_Metal(mobj_t* actor) { (void)actor; }

// A_SpawnFly: Spawn lost soul (Pain Elemental)
void A_SpawnFly(mobj_t* actor) { (void)actor; }

// A_Fire: Disparo genérico (player weapons, chaingun, etc.)
void A_Fire(mobj_t* actor) { (void)actor; }

// A_BFGSpray: BFG tracer damage
void A_BFGSpray(mobj_t* actor) { (void)actor; }

// A_Explode: Explosión radio (rocket, barrel)
void A_Explode(mobj_t* actor) { (void)actor; }

// A_Scream: Sonido muerte
void A_Scream(mobj_t* actor) { (void)actor; }

// A_Fall: Caer al suelo (death)
void A_Fall(mobj_t* actor) { (void)actor; }

// A_Pain: Efecto pain (flash, sonido)
void A_Pain(mobj_t* actor) { (void)actor; }

// A_KeenDie: Commander Keen muere -> tag 666 trigger
void A_KeenDie(mobj_t* actor) { (void)actor; }

// ============================================================================
// 6. PLAYER WEAPON STATES (PSPRITES) — p_pspr.c
// ============================================================================
/*
 * El arma del jugador NO es un mobj_t. Es un "psprite" (player sprite)
 * renderizado encima de la vista (HUD weapon).
 *
 * player_t tiene: psprite_t psprites[NUMPSPRITES];  // NUMPSPRITES = 2 (weapon + flash)
 *
 * psprite_t (p_pspr.h):
 *   typedef struct {
 *       int state;      // índice en states[] (estados de arma)
 *       int tics;       // tics restantes
 *       int sx, sy;     // offset en pantalla
 *   } psprite_t;
 *
 * Estados de arma (states[] índices para player):
 *   S_PUNCH, S_PISTOL, S_SHOTGUN, S_CHAIN, S_MISSILE, S_PLASMA, S_BFG, S_SAW
 *   Cada uno con sub-estados: READY, DOWN, UP, FIRE, FLASH, RELOAD...
 *
 * A_WeaponReady: estado idle, chequea input para cambiar arma / disparar
 * A_Lower: bajar arma (cambio)
 * A_Raise: subir arma nueva
 * A_FirePistol, A_FireShotgun, A_FireCGun, A_FireMissile, A_FirePlasma, A_FireBFG
 * A_Flash: muzzle flash (psprite[1], estado flash, tics=1-2, FF_FULLBRIGHT)
 *
 * P_PlayerThink -> P_MovePlayer -> P_PlayerThink (psprites update)
 */

typedef struct {
    int state;
    int tics;
    int sx, sy;
} psprite_t;

#define NUMPSPRITES 2
#define PS_WEAPON 0
#define PS_FLASH  1

// Player states (subconjunto):
#define S_PUNCH_RDY   200
#define S_PISTOL_RDY  210
#define S_SHOTGUN_RDY 220

// ============================================================================
// 7. TRANSICIONES POR EVENTOS EXTERNOS (DAÑO, USO, ETC.)
// ============================================================================
/*
 * Estados con tics == 0 (wait states) NO avanzan solos.
 * Transiciones externas en P_DamageMobj, P_UseLines, etc.:
 *
 * P_DamageMobj(target, inflictor, source, damage):
 *   - target->health -= damage
 *   - if (target->health <= 0) -> P_KillMobj -> deathstate/xdeathstate
 *   - else if (target->info->painstate && P_Random() < target->info->painchance)
 *       -> P_SetMobjState(target, target->info->painstate)
 *   - target->reactiontime = 0; // wake up
 *   - target->threshold = 0;    // reset attack delay
 *
 * P_SetMobjState(mobj_t* mo, int state):
 *   mo->state = state;
 *   const state_t* st = &states[state];
 *   mo->tics = st->tics;
 *   mo->sprite = st->sprite;
 *   mo->frame = st->frame;
 *   if (st->action) st->action(mo);  // acción INMEDIATA al entrar al estado
 *
 * DIFERENCIA CRÍTICA:
 * - Transición natural (tics llega a 0): acción se llama AL FINAL del estado anterior.
 * - P_SetMobjState forzado: acción se llama AL INICIO del nuevo estado.
 *   Esto permite que painstate ejecute A_Pain inmediatamente.
 */

// ============================================================================
// 8. TESTS DE LÓGICA DE STATE MACHINE
// ============================================================================

// Mock states array para tests
state_t test_states[10];
int test_state_count = 0;

int test_state_machine(void) {
    int passed = 0, failed = 0;
    #define TEST(expr, msg) \
        do { if (expr) { passed++; } else { failed++; printf("FAIL: %s\n", msg); } } while(0)

    // Simular estados simples: A -> B -> C
    test_states[0] = (state_t){SPR_TROO, 0, 2, NULL, 1, 0,0};  // state 0: tics=2, next=1
    test_states[1] = (state_t){SPR_TROO, 1, 1, NULL, 2, 0,0};  // state 1: tics=1, next=2
    test_states[2] = (state_t){SPR_TROO, 2, -1, NULL, -1, 0,0}; // state 2: tics=-1 (stay), next=-1

    // Simular mobj_t con state machine
    typedef struct {
        int state;
        int tics;
        int sprite;
        int frame;
    } test_mobj_t;

    test_mobj_t mo = {0, 2, SPR_TROO, 0};  // start at state 0, tics=2

    // Tic 1: tics 2->1
    if (mo.tics > 0) mo.tics--;
    TEST(mo.tics == 1, "tick 1: tics 2->1");

    // Tic 2: tics 1->0 -> transicion a state 1
    if (mo.tics > 0) {
        mo.tics--;
        if (mo.tics == 0) {
            const state_t* st = &test_states[mo.state];
            mo.state = st->nextstate;
            st = &test_states[mo.state];
            mo.tics = st->tics;
            mo.sprite = st->sprite;
            mo.frame = st->frame;
        }
    }
    TEST(mo.state == 1, "transition to state 1");
    TEST(mo.tics == 1, "state 1 tics = 1");
    TEST(mo.frame == 1, "state 1 frame = 1");

    // Tic 3: state 1 tics 1->0 -> transicion a state 2
    if (mo.tics > 0) {
        mo.tics--;
        if (mo.tics == 0) {
            const state_t* st = &test_states[mo.state];
            mo.state = st->nextstate;
            st = &test_states[mo.state];
            mo.tics = st->tics;
            mo.sprite = st->sprite;
            mo.frame = st->frame;
        }
    }
    TEST(mo.state == 2, "transition to state 2");
    TEST(mo.tics == -1, "state 2 tics = -1 (stay)");
    TEST(mo.frame == 2, "state 2 frame = 2");

    // Tic 4: state 2 tics=-1 -> no change (wait forever)
    if (mo.tics > 0) { mo.tics--; }
    TEST(mo.state == 2, "state 2 stays");
    TEST(mo.tics == -1, "tics stays -1");

    // FF_FULLBRIGHT flag
    TEST((0x8000 & FF_FRAMEMASK) == 0, "FF_FULLBRIGHT masked out");
    TEST((0x8005 & FF_FRAMEMASK) == 5, "frame 5 with FF_FULLBRIGHT -> 5");

    printf("State machine tests: %d pass, %d fail\n", passed, failed);
    return failed == 0 ? 0 : 1;
}

int main(void) {
    printf("Módulo 27 - Lección 06: State Machine e IA\n");
    return test_state_machine();
}