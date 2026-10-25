/*
 * MÓDULO 27: Doom Clásico — De C a C++
 * SOLUCIÓN 04: Visplanes, Drawsegs, Sprites y Colormaps
 */

#include <stdint.h>
#include "../../include/test_framework.hpp"

// ============================================================================
// TIPOS BASE
// ============================================================================

using fixed_t = int32_t;
#define FRACUNIT 65536
#define FIXED_TO_INT(x) ((x) >> 16)
#define INT_TO_FIXED(x) ((x) << 16)
using angle_t = uint16_t;

// ============================================================================
// VISPLANES
// ============================================================================

typedef struct {
    fixed_t height;
    int picnum;
    int lightlevel;
    fixed_t scale;
    int minx, maxx;
    short* top;
    short* bottom;
} visplane_t;

#define MAXVISPLANES 128

int R_GetColormap(int lightlevel, fixed_t scale) {
    int base = (lightlevel == 0) ? 0 : ((lightlevel - 1) >> 4);
    int dist = FIXED_TO_INT(scale);
    int map = base - (dist - 1);
    if (map < 0) map = 0;
    if (map >= 32) map = 31;
    return map;
}

// ============================================================================
// DRAWSEGS
// ============================================================================

typedef enum { SIL_NONE, SIL_BOTTOM, SIL_TOP, SIL_BOTH } sil_type_t;

typedef struct {
    int x1, x2;
    fixed_t scale1, scale2;
    fixed_t scalestep;
    sil_type_t silhouette;
    fixed_t bsilheight, tsilheight;
    void* frontsector;
    void* backsector;
} drawseg_t;

#define MAXDRAWSEGS 256

typedef struct { int x1, x2; } solidseg_t;

// ============================================================================
// SPRITES
// ============================================================================

typedef struct {
    fixed_t x, y;
    fixed_t gx, gy;
    fixed_t scale;
    int patch;
    int colormap;
    int x1, x2;
} vissprite_t;

#define MAXVISSPRITES 256

int vissprite_compare(const void* a, const void* b) {
    const vissprite_t* va = (const vissprite_t*)a;
    const vissprite_t* vb = (const vissprite_t*)b;
    int64_t da = (int64_t)va->gx * va->gx + (int64_t)va->gy * va->gy;
    int64_t db = (int64_t)vb->gx * vb->gx + (int64_t)vb->gy * vb->gy;
    return (da > db) ? -1 : (da < db) ? 1 : 0;
}

// ============================================================================
// TESTS
// ============================================================================

TEST_CASE(TestVisplane) {
    ASSERT_TRUE(sizeof(visplane_t) >= 32);
    ASSERT_EQ(MAXVISPLANES, 128);
}

TEST_CASE(TestR_GetColormap) {
    ASSERT_EQ(R_GetColormap(255, FRACUNIT), 15);
    ASSERT_EQ(R_GetColormap(0, FRACUNIT), 0);
    ASSERT_EQ(R_GetColormap(255, 100*FRACUNIT), 0);
    ASSERT_EQ(R_GetColormap(128, FRACUNIT), 7);
}

TEST_CASE(TestDrawsegSilhouette) {
    ASSERT_EQ(SIL_NONE, 0);
    ASSERT_EQ(SIL_BOTTOM, 1);
    ASSERT_EQ(SIL_TOP, 2);
    ASSERT_EQ(SIL_BOTH, 3);
    ASSERT_EQ(MAXDRAWSEGS, 256);
}

TEST_CASE(TestVissprite) {
    ASSERT_TRUE(sizeof(vissprite_t) >= 32);
    ASSERT_EQ(MAXVISSPRITES, 256);
}

TEST_CASE(TestVisspriteCompare) {
    vissprite_t a = {0, 0, INT_TO_FIXED(100), 0, 0, 0, 0, 0, 0};
    vissprite_t b = {0, 0, INT_TO_FIXED(50), 0, 0, 0, 0, 0, 0};
    ASSERT_TRUE(vissprite_compare(&a, &b) < 0);
    ASSERT_TRUE(vissprite_compare(&b, &a) > 0);
    ASSERT_EQ(vissprite_compare(&a, &a), 0);
}

int main() {
    return RUN_ALL_TESTS();
}