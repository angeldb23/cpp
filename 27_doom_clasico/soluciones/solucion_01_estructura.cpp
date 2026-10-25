/*
 * MÓDULO 27: Doom Clásico — De C a C++
 * SOLUCIÓN 01: Estructura id Tech 1 — Tipos, Prefijos y Variables Globales
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "../../include/test_framework.hpp"

// ============================================================================
// TIPOS BASE (doomdef.h, doomtype.h)
// ============================================================================

using fixed_t = int32_t;
#define FRACUNIT 65536
using angle_t = uint16_t;

#define ANG90 (FRACUNIT / 4)      // 16384
#define ANG180 (FRACUNIT / 2)     // 32768
#define ANG270 (3 * FRACUNIT / 4) // 49152
#define ANG360 FRACUNIT           // 65536

// ============================================================================
// ESTRUCTURAS (p_mobj.h, d_think.h)
// ============================================================================

typedef struct thinker_s {
    struct thinker_s* prev;
    struct thinker_s* next;
    void (*function)(struct thinker_s*);
} thinker_t;

typedef struct mobjinfo_s {
    int doomednum;
    int spawnstate;
    int spawnhealth;
    fixed_t speed;
    fixed_t radius;
    fixed_t height;
    int mass;
    int damage;
    int flags;
} mobjinfo_t;

typedef struct mobj_s {
    thinker_t thinker;  // PRIMER MIEMBRO: herencia manual
    fixed_t x, y, z;
    struct mobj_s* next;
    struct mobj_s* prev;
    angle_t angle;
    int sprite;
    int frame;
    struct mobj_s* target;
    struct mobj_s* tracer;
    const mobjinfo_t* info;
    int health;
    int reactiontime;
    int threshold;
    int flags;
    fixed_t radius;
    fixed_t height;
} mobj_t;

// ============================================================================
// VARIABLES GLOBALES (g_game.c, d_main.c)
// ============================================================================

typedef enum { GS_LEVEL, GS_INTERMISSION, GS_FINALE, GS_DEMOSCREEN } gamestate_t;
typedef enum { sk_baby, sk_easy, sk_medium, sk_hard, sk_nightmare } skill_t;

gamestate_t gamestate = GS_LEVEL;
skill_t gameskill = sk_medium;
bool paused = false;
int gametic = 0;
int leveltime = 0;
thinker_t* thinkercap = NULL;

// ============================================================================
// MACROS DE CONVERSIÓN (m_fixed.h)
// ============================================================================

#define FIXED_TO_INT(x) ((x) >> 16)
#define INT_TO_FIXED(x) ((x) << 16)

// ============================================================================
// FUNCIONES BÁSICAS
// ============================================================================

void P_InitThinkers(void) {
    if (!thinkercap) {
        thinkercap = (thinker_t*)malloc(sizeof(thinker_t));
    }
    thinkercap->prev = thinkercap->next = thinkercap;
    thinkercap->function = NULL;
}

void P_CleanupThinkers(void) {
    if (thinkercap) {
        free(thinkercap);
        thinkercap = NULL;
    }
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
}

void DummyMobjThinker(thinker_t* th) { (void)th; }

mobj_t* P_SpawnMobj(fixed_t x, fixed_t y, fixed_t z, const mobjinfo_t* info) {
    mobj_t* mo = (mobj_t*)malloc(sizeof(mobj_t));
    mo->x = x; mo->y = y; mo->z = z;
    mo->info = info;
    mo->health = info->spawnhealth;
    mo->flags = info->flags;
    mo->radius = info->radius;
    mo->height = info->height;
    mo->thinker.function = DummyMobjThinker;
    P_AddThinker(&mo->thinker);
    return mo;
}

// ============================================================================
// TESTS (NO MODIFICAR)
// ============================================================================

TEST_CASE(TestTiposBase) {
    ASSERT_EQ(sizeof(fixed_t), 4);
    ASSERT_EQ(FRACUNIT, 65536);
    ASSERT_EQ(sizeof(angle_t), 2);
    ASSERT_EQ(ANG90, 16384);
    ASSERT_EQ(ANG180, 32768);
    ASSERT_EQ(ANG270, 49152);
    ASSERT_EQ(ANG360, 65536);
}

TEST_CASE(TestThinkerT) {
    ASSERT_TRUE(sizeof(thinker_t) >= 3*sizeof(void*));
}

TEST_CASE(TestMobjInfoT) {
    ASSERT_TRUE(sizeof(mobjinfo_t) >= 9*sizeof(int));
}

TEST_CASE(TestMobjTEmbeThink) {
    mobj_t mo = {0};
    thinker_t* th = (thinker_t*)&mo;
    ASSERT_TRUE(th == (thinker_t*)&mo);
}

TEST_CASE(TestVariablesGlobales) {
    ASSERT_EQ(gamestate, GS_LEVEL);
    ASSERT_EQ(gameskill, sk_medium);
    ASSERT_FALSE(paused);
    ASSERT_EQ(gametic, 0);
    ASSERT_EQ(leveltime, 0);
}

TEST_CASE(TestMacrosConversion) {
    ASSERT_EQ(INT_TO_FIXED(5), 5 * FRACUNIT);
    ASSERT_EQ(FIXED_TO_INT(INT_TO_FIXED(5)), 5);
    ASSERT_EQ(FIXED_TO_INT(INT_TO_FIXED(-10)), -10);
}

TEST_CASE(TestP_InitThinkers) {
    P_InitThinkers();
    ASSERT_TRUE(thinkercap != NULL);
    ASSERT_EQ(thinkercap->next, thinkercap);
    ASSERT_EQ(thinkercap->prev, thinkercap);
}

TEST_CASE(TestP_AddThinker) {
    thinker_t a = {0}, b = {0};
    P_AddThinker(&a);
    ASSERT_EQ(thinkercap->next, &a);
    ASSERT_EQ(a.next, thinkercap);
    ASSERT_EQ(a.prev, thinkercap);

    P_AddThinker(&b);
    ASSERT_EQ(thinkercap->next, &a);
    ASSERT_EQ(a.next, &b);
    ASSERT_EQ(b.next, thinkercap);
}

TEST_CASE(TestP_RemoveThinker) {
    P_InitThinkers();
    // Use dynamically allocated thinkers to ensure stable addresses
    thinker_t* a = (thinker_t*)malloc(sizeof(thinker_t));
    thinker_t* b = (thinker_t*)malloc(sizeof(thinker_t));
    a->prev = a->next = NULL; a->function = (void(*)(thinker_t*))1;
    b->prev = b->next = NULL; b->function = (void(*)(thinker_t*))2;

    P_AddThinker(a);
    P_AddThinker(b);
    P_RemoveThinker(a);
    ASSERT_EQ(thinkercap->next, b);
    ASSERT_EQ(b->prev, thinkercap);

    P_RemoveThinker(b);
    free(a); free(b);
}

TEST_CASE(TestP_SpawnMobj) {
    P_InitThinkers();
    mobjinfo_t test_info = {1, 0, 100, INT_TO_FIXED(8), INT_TO_FIXED(20), INT_TO_FIXED(56), 100, 10, 0};
    mobj_t* spawned = P_SpawnMobj(INT_TO_FIXED(100), INT_TO_FIXED(200), 0, &test_info);
    ASSERT_TRUE(spawned != NULL);
    if (spawned) {
        ASSERT_EQ(spawned->x, INT_TO_FIXED(100));
        ASSERT_EQ(spawned->y, INT_TO_FIXED(200));
        ASSERT_EQ(spawned->info, &test_info);
        ASSERT_EQ(spawned->health, 100);
        ASSERT_EQ(spawned->flags, 0);
        ASSERT_TRUE(spawned->thinker.function != NULL);
    }
    free(spawned);
    P_CleanupThinkers();
}

int main() {
    return RUN_ALL_TESTS();
}