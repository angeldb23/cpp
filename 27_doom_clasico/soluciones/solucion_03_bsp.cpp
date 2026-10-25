/*
 * MÓDULO 27: Doom Clásico — De C a C++
 * SOLUCIÓN 03: BSP — Nodos, Subsectores, Segs y Recorrido Front-to-Back
 */

#include <stdint.h>
#include <stdio.h>
#include "../../include/test_framework.hpp"

// ============================================================================
// TIPOS BASE
// ============================================================================

using fixed_t = int32_t;
#define FRACUNIT 65536
using angle_t = uint16_t;

typedef struct { fixed_t x, y; } vertex_t;

// ============================================================================
// ESTRUCTURAS BSP
// ============================================================================

typedef struct {
    fixed_t x, y;
    fixed_t dx, dy;
    fixed_t bbox[2][4];
    uint16_t children[2];
} node_t;

typedef struct {
    uint16_t numsegs;
    uint16_t firstseg;
} subsector_t;

typedef struct {
    vertex_t v1, v2;
    fixed_t offset;
    angle_t angle;
    uint16_t linedef;
    uint16_t side;
    void* frontsector;
    void* backsector;
} seg_t;

#define BBOX_TOP 0
#define BBOX_BOTTOM 1
#define BBOX_LEFT 2
#define BBOX_RIGHT 3
#define CHILD_SUBSECTOR 0x8000
#define CHILD_MASK 0x7FFF

// ============================================================================
// FUNCIONES BSP
// ============================================================================

int R_PointOnSide(fixed_t px, fixed_t py, const node_t* node) {
    fixed_t dx = px - node->x;
    fixed_t dy = py - node->y;
    int64_t cross = (int64_t)dx * node->dy - (int64_t)dy * node->dx;
    return (cross <= 0) ? 0 : 1;
}

void R_RenderSubsector(int subsec_num) {
    (void)subsec_num;
}

void R_RenderBSPNode(int nodenum, const vertex_t* view,
                     const node_t* nodes, const subsector_t* subsectors) {
    if (nodenum & CHILD_SUBSECTOR) {
        R_RenderSubsector(nodenum & CHILD_MASK);
        return;
    }
    const node_t* node = &nodes[nodenum];
    int side = R_PointOnSide(view->x, view->y, node);
    R_RenderBSPNode(node->children[side], view, nodes, subsectors);
    R_RenderBSPNode(node->children[side ^ 1], view, nodes, subsectors);
}

// ============================================================================
// TESTS (NO MODIFICAR)
// ============================================================================

TEST_CASE(TestEstructuras) {
    ASSERT_EQ(sizeof(vertex_t), 8);
    ASSERT_TRUE(sizeof(node_t) >= 40);
    ASSERT_EQ(sizeof(subsector_t), 4);
    ASSERT_TRUE(sizeof(seg_t) >= 32);
    ASSERT_EQ(CHILD_SUBSECTOR, 0x8000);
    ASSERT_EQ(CHILD_MASK, 0x7FFF);
}

TEST_CASE(TestR_PointOnSide) {
    node_t node = {0, 0, 0, FRACUNIT, {{0}}, {0, 1}};
    ASSERT_EQ(R_PointOnSide(-FRACUNIT, 0, &node), 0);
    ASSERT_EQ(R_PointOnSide(FRACUNIT, 0, &node), 1);

    node_t node2 = {0, 0, FRACUNIT, 0, {{0}}, {0, 1}};
    ASSERT_EQ(R_PointOnSide(0, FRACUNIT, &node2), 0);
    ASSERT_EQ(R_PointOnSide(0, -FRACUNIT, &node2), 1);

    fixed_t norm = (fixed_t)(FRACUNIT * 0.70710678f);
    node_t node3 = {0, 0, norm, norm, {{0}}, {0, 1}};
    ASSERT_EQ(R_PointOnSide(-FRACUNIT, FRACUNIT, &node3), 0);
    ASSERT_EQ(R_PointOnSide(FRACUNIT, -FRACUNIT, &node3), 1);
}

TEST_CASE(TestR_RenderBSPNode) {
    node_t nodes[1] = {0};
    nodes[0].x = 0; nodes[0].y = 0; nodes[0].dx = 0; nodes[0].dy = FRACUNIT;
    nodes[0].children[0] = CHILD_SUBSECTOR | 0;
    nodes[0].children[1] = CHILD_SUBSECTOR | 1;

    subsector_t subsectors[2] = {{5, 0}, {3, 5}};
    vertex_t view = {-FRACUNIT, 0};

    R_RenderBSPNode(0, &view, nodes, subsectors);
    ASSERT_TRUE(true);
}

TEST_CASE(TestBBoxIndices) {
    ASSERT_EQ(BBOX_TOP, 0);
    ASSERT_EQ(BBOX_BOTTOM, 1);
    ASSERT_EQ(BBOX_LEFT, 2);
    ASSERT_EQ(BBOX_RIGHT, 3);
}

int main() {
    return RUN_ALL_TESTS();
}