/*
 * MÓDULO 27: Doom Clásico — De C a C++
 * SOLUCIÓN 06: State Machine — states[], Transiciones, Acciones, Pain/Death
 */

#include <stdint.h>
#include "../../include/test_framework.hpp"

// ============================================================================
// TIPOS BASE
// ============================================================================

using fixed_t = int32_t;
#define FRACUNIT 65536

#define FF_FULLBRIGHT 0x8000
#define FF_FRAMEMASK 0x7FFF

// ============================================================================
// STATE_T
// ============================================================================

typedef struct {
    int sprite;
    int frame;
    int tics;
    void (*action)(void*);
    int nextstate;
    int misc1, misc2;
} state_t;

state_t test_states[5] = {0};
#define states test_states

// ============================================================================
// MOBJ_T CON STATE MACHINE
// ============================================================================

typedef struct {
    int state;
    int tics;
    int sprite;
    int frame;
    int health;
} mobj_t;

// ============================================================================
// ACCIONES
// ============================================================================

void A_Look(void* actor) { (void)actor; }
void A_Chase(void* actor) { (void)actor; }
void A_FaceTarget(void* actor) { (void)actor; }
void A_TroopAttack(void* actor) { (void)actor; }
void A_Scream(void* actor) { (void)actor; }
void A_Fall(void* actor) { (void)actor; }
void A_Pain(void* actor) { (void)actor; }

// ============================================================================
// FUNCIONES STATE MACHINE
// ============================================================================

void P_SetMobjState(mobj_t* mo, int state) {
    mo->state = state;
    const state_t* st = &states[state];
    mo->tics = st->tics;
    mo->sprite = st->sprite;
    mo->frame = st->frame;
    if (st->action) st->action(mo);
}

void P_MobjThinker(mobj_t* mo) {
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
// TESTS
// ============================================================================

TEST_CASE(TestFlags) {
    ASSERT_EQ(FF_FULLBRIGHT, 0x8000);
    ASSERT_EQ(FF_FRAMEMASK, 0x7FFF);
}

TEST_CASE(TestP_SetMobjState) {
    test_states[1] = (state_t){0, 1, 1, NULL, 2, 0, 0};
    mobj_t mo = {0, 0, 0, 0, 100};
    P_SetMobjState(&mo, 1);
    ASSERT_EQ(mo.state, 1);
    ASSERT_EQ(mo.tics, 1);
    ASSERT_EQ(mo.frame, 1);
}

TEST_CASE(TestP_MobjThinker_Natural) {
    test_states[0] = (state_t){0, 0, 2, NULL, 1, 0, 0};
    test_states[1] = (state_t){0, 1, 1, NULL, 2, 0, 0};
    test_states[2] = (state_t){0, 2, -1, NULL, -1, 0, 0};

    mobj_t mo = {0, 2, 0, 0, 100};

    P_MobjThinker(&mo);
    ASSERT_EQ(mo.tics, 1);
    ASSERT_EQ(mo.state, 0);

    P_MobjThinker(&mo);
    ASSERT_EQ(mo.state, 1);
    ASSERT_EQ(mo.tics, 1);
    ASSERT_EQ(mo.frame, 1);

    P_MobjThinker(&mo);
    ASSERT_EQ(mo.state, 2);
    ASSERT_EQ(mo.tics, -1);
    ASSERT_EQ(mo.frame, 2);

    P_MobjThinker(&mo);
    ASSERT_EQ(mo.state, 2);
    ASSERT_EQ(mo.tics, -1);
}

TEST_CASE(TestPainDeath) {
    mobj_t mo = {0, 0, 0, 0, 100};
    mo.health = 50;
    P_SetMobjState(&mo, 3);
    ASSERT_EQ(mo.state, 3);

    mo.health = 0;
    P_SetMobjState(&mo, 4);
    ASSERT_EQ(mo.state, 4);
}

int main() {
    return RUN_ALL_TESTS();
}