/*
 * MÓDULO 27: Doom Clásico — De C a C++
 * EJERCICIO 02: Fixed-Point 16.16 — FixedMul, FixedDiv, Trig Tables
 *
 * BASADO EN: Lección 02 (fixed_point)
 *
 * OBJETIVO: Implementar aritmética fixed-point 16.16 y tabla de senos/cosenos.
 */

#include <stdint.h>
#include <math.h>
#include <limits.h>
#include "../../include/test_framework.hpp"

// ============================================================================
// CONSTANTES Y TIPOS
// ============================================================================

using fixed_t = int32_t;
#define FRACBITS 16
#define FRACUNIT (1 << FRACBITS)  // 65536

#define ANG90 (FRACUNIT / 4)      // 16384
#define ANG180 (FRACUNIT / 2)     // 32768
#define ANG270 (3 * FRACUNIT / 4) // 49152
#define ANG360 FRACUNIT           // 65536

#define FINESINE_SIZE 8192
#define FINESINE_MASK (FINESINE_SIZE - 1)

// Tabla de senos (precalculada)
static fixed_t finesine[FINESINE_SIZE];

// ============================================================================
// MACROS DE CONVERSIÓN
// ============================================================================

#define FIXED_TO_INT(x) ((x) >> FRACBITS)
#define INT_TO_FIXED(x) ((x) << FRACBITS)
#define FIXED_TO_FLOAT(x) ((float)(x) / (float)FRACUNIT)
#define FLOAT_TO_FIXED(x) ((fixed_t)((x) * (float)FRACUNIT))

// ============================================================================
// OPERACIONES FIXED-POINT
// ============================================================================

fixed_t FixedMul(fixed_t a, fixed_t b) {
    // TODO: implementar multiplicación de punto fijo 16.16 con shift
    (void)a; (void)b;
    return 0;
}

fixed_t FixedDiv(fixed_t a, fixed_t b) {
    // TODO: implementar división de punto fijo 16.16 con shift
    (void)a; (void)b;
    return 0;
}

fixed_t FixedAbs(fixed_t x) { return (x >= 0) ? x : -x; }
fixed_t FixedMin(fixed_t a, fixed_t b) { return (a < b) ? a : b; }
fixed_t FixedMax(fixed_t a, fixed_t b) { return (a > b) ? a : b; }

// ============================================================================
// TABLA TRIGONOMÉTRICA
// ============================================================================

void InitFineSine(void) {
    for (int i = 0; i < FINESINE_SIZE; i++) {
        double rad = (2.0 * M_PI * i) / FINESINE_SIZE;
        finesine[i] = FLOAT_TO_FIXED(sin(rad));
    }
}

fixed_t FixedSin(uint16_t angle) {
    return finesine[(angle >> 3) & FINESINE_MASK];
}

fixed_t FixedCos(uint16_t angle) {
    return finesine[((angle + ANG90) >> 3) & FINESINE_MASK];
}

// ============================================================================
// APLICACIONES DOOM
// ============================================================================

void MoveForward(fixed_t* x, fixed_t* y, uint16_t angle, fixed_t speed) {
    *x += FixedMul(speed, FixedCos(angle));
    *y += FixedMul(speed, FixedSin(angle));
}

fixed_t ApproxDistance(fixed_t dx, fixed_t dy) {
    dx = FixedAbs(dx);
    dy = FixedAbs(dy);
    if (dx < dy) { fixed_t t = dx; dx = dy; dy = t; }
    return dx + (dy >> 1);
}

fixed_t CalcSpriteScale(fixed_t distance, fixed_t viewingdistance) {
    if (distance == 0) return INT_MAX;
    return FixedDiv(viewingdistance, distance);
}

// ============================================================================
// TESTS (NO MODIFICAR)
// ============================================================================

TEST_CASE(TestConstantes) {
    ASSERT_EQ(FRACBITS, 16);
    ASSERT_EQ(FRACUNIT, 65536);
    ASSERT_EQ(ANG90, 16384);
    ASSERT_EQ(ANG180, 32768);
    ASSERT_EQ(ANG270, 49152);
    ASSERT_EQ(ANG360, 65536);
    ASSERT_EQ(FINESINE_SIZE, 8192);
    ASSERT_EQ(FINESINE_MASK, 8191);
}

TEST_CASE(TestConversiones) {
    ASSERT_EQ(INT_TO_FIXED(5), 5 * FRACUNIT);
    ASSERT_EQ(FIXED_TO_INT(INT_TO_FIXED(5)), 5);
    ASSERT_EQ(FIXED_TO_INT(INT_TO_FIXED(-10)), -10);
    ASSERT_NEAR(FLOAT_TO_FIXED(1.5f), (fixed_t)(1.5f * FRACUNIT), 2);
    ASSERT_NEAR(FIXED_TO_FLOAT(FRACUNIT) * 1000, 1000.0f, 1.0f);
}

TEST_CASE(TestFixedMul) {
    ASSERT_EQ(FixedMul(FRACUNIT, FRACUNIT), FRACUNIT);
    ASSERT_EQ(FixedMul(INT_TO_FIXED(2), INT_TO_FIXED(3)), INT_TO_FIXED(6));
    ASSERT_EQ(FixedMul(FRACUNIT/2, FRACUNIT/2), FRACUNIT/4);
    ASSERT_EQ(FixedMul(-FRACUNIT, FRACUNIT/2), -(FRACUNIT/2));
}

TEST_CASE(TestFixedDiv) {
    ASSERT_EQ(FixedDiv(INT_TO_FIXED(6), INT_TO_FIXED(3)), INT_TO_FIXED(2));
    ASSERT_EQ(FixedDiv(FRACUNIT, INT_TO_FIXED(2)), FRACUNIT/2);
    ASSERT_EQ(FixedDiv(FRACUNIT/2, FRACUNIT/4), INT_TO_FIXED(2));
    ASSERT_EQ(FixedDiv(FRACUNIT, 0), INT_MAX);
}

TEST_CASE(TestFixedAbsMinMax) {
    ASSERT_EQ(FixedAbs(-FRACUNIT), FRACUNIT);
    ASSERT_EQ(FixedMin(10, 20), 10);
    ASSERT_EQ(FixedMax(10, 20), 20);
}

TEST_CASE(TestTrig) {
    InitFineSine();
    ASSERT_EQ(FixedSin(0), 0);
    ASSERT_EQ(FixedSin(ANG90), FRACUNIT);
    ASSERT_EQ(FixedSin(ANG180), 0);
    ASSERT_EQ(FixedSin(ANG270), -FRACUNIT);
    ASSERT_EQ(FixedCos(0), FRACUNIT);
    ASSERT_EQ(FixedCos(ANG90), 0);
    ASSERT_EQ(FixedCos(ANG180), -FRACUNIT);
}

TEST_CASE(TestPitagorasAprox) {
    InitFineSine();
    fixed_t s = FixedSin(ANG90/3);
    fixed_t c = FixedCos(ANG90/3);
    fixed_t sum_sq = FixedMul(s, s) + FixedMul(c, c);
    ASSERT_NEAR(sum_sq, (fixed_t)FRACUNIT, 100);
}

TEST_CASE(TestMoveForward) {
    fixed_t x = 0, y = 0;
    MoveForward(&x, &y, 0, INT_TO_FIXED(10));
    ASSERT_EQ(x, INT_TO_FIXED(10));
    ASSERT_EQ(y, 0);

    x = 0; y = 0;
    MoveForward(&x, &y, ANG90, INT_TO_FIXED(10));
    ASSERT_EQ(x, 0);
    ASSERT_EQ(y, INT_TO_FIXED(10));
}

TEST_CASE(TestApproxDistance) {
    fixed_t dist = ApproxDistance(INT_TO_FIXED(3), INT_TO_FIXED(4));
    ASSERT_TRUE(dist >= INT_TO_FIXED(4));
    ASSERT_TRUE(dist <= INT_TO_FIXED(6));
}

TEST_CASE(TestCalcSpriteScale) {
    fixed_t scale = CalcSpriteScale(INT_TO_FIXED(100), INT_TO_FIXED(200));
    ASSERT_EQ(scale, INT_TO_FIXED(2));
}

int main() {
    return RUN_ALL_TESTS();
}