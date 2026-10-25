/*
 * MÓDULO 27: Doom Clásico — De C a C++
 * SOLUCIÓN 05: Thinkers y Tic System — Lista Circular, P_Ticker, 35 Hz
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "../../include/test_framework.hpp"

// ============================================================================
// TIPOS BASE
// ============================================================================

using fixed_t = int32_t;
#define FRACUNIT 65536
using angle_t = uint16_t;
#define TICRATE 35

// ============================================================================
// THINKER_T
// ============================================================================

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
}

void P_Ticker(void) {
    thinker_t* current = thinkercap->next;
    while (current != thinkercap) {
        thinker_t* next = current->next;
        if (current->function) current->function(current);
        current = next;
    }
}

void P_CleanupThinkers(void) {
    // Free all thinkers in list except centinela
    thinker_t* current = thinkercap->next;
    while (current != thinkercap) {
        thinker_t* next = current->next;
        free(current);
        current = next;
    }
    // Free centinela
    free(thinkercap);
    thinkercap = NULL;
}

// ============================================================================
// TIC SYSTEM
// ============================================================================

typedef struct {
    int8_t forwardmove;
    int8_t sidemove;
    int16_t angleturn;
    int16_t consistancy;
    uint8_t buttons;
} ticcmd_t;

int gametic = 0;
int leveltime = 0;

void G_Ticker(void) {
    P_Ticker();
    leveltime++;
}

void G_RunTics(int ticcount) {
    while (gametic < ticcount) {
        G_Ticker();
        gametic++;
    }
}

// ============================================================================
// P_RANDOM
// ============================================================================

uint8_t rndtable[256] = {
    0, 8, 109, 220, 222, 241, 149, 107, 75, 248, 254, 140, 16, 66,
    74, 21, 211, 47, 80, 242, 154, 27, 205, 128, 161, 89, 77, 36,
    95, 110, 85, 48, 212, 140, 211, 47, 80, 242, 154, 27, 205, 128,
    161, 89, 77, 36, 95, 110, 85, 48, 212, 140, 211, 47, 80, 242,
    154, 27, 205, 128, 161, 89, 77, 36, 95, 110, 85, 48, 212, 140,
    211, 47, 80, 242, 154, 27, 205, 128, 161, 89, 77, 36, 95, 110,
    85, 48, 212, 140, 211, 47, 80, 242, 154, 27, 205, 128, 161, 89,
    77, 36, 95, 110, 85, 48, 212, 140, 211, 47, 80, 242, 154, 27,
    205, 128, 161, 89, 77, 36, 95, 110, 85, 48, 212, 140, 211, 47,
    80, 242, 154, 27, 205, 128, 161, 89, 77, 36, 95, 110, 85, 48,
    212, 140, 211, 47, 80, 242, 154, 27, 205, 128, 161, 89, 77, 36,
    95, 110, 85, 48, 212, 140, 211, 47, 80, 242, 154, 27, 205, 128,
    161, 89, 77, 36, 95, 110, 85, 48, 212, 140, 211, 47, 80, 242,
    154, 27, 205, 128, 161, 89, 77, 36, 95, 110, 85, 48, 212, 140,
    211, 47, 80, 242, 154, 27, 205, 128, 161, 89, 77, 36, 95, 110,
    85, 48, 212, 140, 211, 47, 80, 242, 154, 27, 205, 128, 161, 89,
    77, 36, 95, 110, 85, 48, 212, 140, 211, 47, 80, 242, 154, 27,
    205, 128, 161, 89, 77, 36
};
int rndindex = 0;

int P_Random(void) {
    return rndtable[rndindex++ & 255];
}

// ============================================================================
// TESTS
// ============================================================================

TEST_CASE(TestThinkerList) {
    P_InitThinkers();
    ASSERT_TRUE(thinkercap != NULL);
    ASSERT_EQ(thinkercap->next, thinkercap);
    ASSERT_EQ(thinkercap->prev, thinkercap);

    // Use dynamically allocated thinkers to avoid stack lifetime issues
    thinker_t* a = (thinker_t*)malloc(sizeof(thinker_t));
    thinker_t* b = (thinker_t*)malloc(sizeof(thinker_t));
    thinker_t* c = (thinker_t*)malloc(sizeof(thinker_t));
    a->prev = a->next = NULL; a->function = (void(*)(thinker_t*))1;
    b->prev = b->next = NULL; b->function = (void(*)(thinker_t*))2;
    c->prev = c->next = NULL; c->function = (void(*)(thinker_t*))3;

    P_AddThinker(a);
    ASSERT_EQ(thinkercap->next, a);
    ASSERT_EQ(a->next, thinkercap);

    P_AddThinker(b);
    ASSERT_EQ(thinkercap->next, a);
    ASSERT_EQ(a->next, b);
    ASSERT_EQ(b->next, thinkercap);

    P_AddThinker(c);
    ASSERT_EQ(thinkercap->next, a);
    ASSERT_EQ(a->next, b);
    ASSERT_EQ(b->next, c);
    ASSERT_EQ(c->next, thinkercap);

    P_RemoveThinker(b);
    free(b);
    ASSERT_EQ(a->next, c);
    ASSERT_EQ(c->prev, a);

    P_CleanupThinkers();
}

TEST_CASE(TestTicSystem) {
    P_InitThinkers();
    ASSERT_EQ(TICRATE, 35);
    ASSERT_EQ(gametic, 0);
    ASSERT_EQ(leveltime, 0);

    G_Ticker();
    ASSERT_EQ(leveltime, 1);

    G_RunTics(5);
    ASSERT_EQ(gametic, 5);

    P_CleanupThinkers();
}

TEST_CASE(TestPRandom) {
    rndindex = 0;
    int r1 = P_Random();
    int r2 = P_Random();
    ASSERT_EQ(r1, 0);
    ASSERT_EQ(r2, 8);
    rndindex = 0;
    ASSERT_EQ(P_Random(), r1);
    ASSERT_EQ(P_Random(), r2);
}

int main() {
    return RUN_ALL_TESTS();
}