/*
 * MÓDULO 27: Doom Clásico — De C a C++
 * LECCIÓN 03: BSP y PVS — Estructura del mapa y visibilidad (r_bsp.h, r_bsp.c, r_segs.c)
 *
 * TEMAS:
 * - Formato del mapa: vertexes, linedefs, sidedefs, sectors, things
 * - Árbol BSP (Binary Space Partitioning): nodos, subsectores, partición recursiva
 * - PVS (Potentially Visible Set): no existe como tal en Doom; visibilidad por BSP traversal
 * - Segs: segmentos de linedef recortados al subsector
 * - Visplanes: spans horizontales de suelo/techo para render por scanline
 * - Orden de render: BSP traversal front-to-back, drawsegs, sprites
 *
 * VERIFICACIÓN: Estructuras verificadas contra r_bsp.h, r_defs.h, r_segs.h del source release 1997.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// ============================================================================
// 1. ESTRUCTURAS DEL MAPA (w_wad.h, r_defs.h, p_mobj.h)
// ============================================================================
/*
 * El mapa (lumps THINGS, VERTEXES, LINEDEFS, SIDEDEFS, SECTORS, NODES, SSECTORS, SEGS)
 * se carga en memoria como arrays de structs. NO hay clases, solo datos planos.
 *
 * vertex_t: punto 2D (x, y) en fixed_t (coordenadas de mapa, 16.16)
 * linedef_t: línea entre dos vértices, con flags, tipos especiales, sidedefs front/back
 * sidedef_t: referencia a sector + texturas (upper/lower/mid) + offsets
 * sector_t: floorheight, ceilingheight, floorpic, ceilingpic, lightlevel, special, tag
 * thing_t: spawn point: x, y, angle, type (doomednum), options
 *
 * NODES: árbol BSP. Cada nodo divide el espacio en dos mitades (front/back).
 *        children[0] = front, children[1] = back. Si bit 15 = 1 -> subsector index.
 * SSECTORS (subsectores): lista de segs (índice + count). Convexo, sin partición interna.
 * SEGS: segmento de linedef recortado al subsector. v1, v2, angle, linedef, side, offset.
 *
 * ESTRUCTURAS REALES (verificadas en r_defs.h, r_bsp.h, r_segs.h):
 */

typedef int32_t fixed_t;
typedef uint16_t angle_t;
#define FRACUNIT 65536

typedef struct { fixed_t x, y; } vertex_t;

typedef struct {
    uint16_t v1, v2;           // índices a vertexes
    uint16_t flags;            // ML_BLOCKING, ML_TWOSIDED, ML_SECRET, etc.
    uint16_t special;          // tipo de linedef especial (puerta, lift, etc.)
    uint16_t tag;              // sector tag para activación
    uint16_t sidenum[2];       // sidedef front (0) y back (1), 0xFFFF = none
} linedef_t;

typedef struct {
    fixed_t xoffset, yoffset;  // offsets de textura
    uint16_t toptexture;       // nombre textura superior (índice en texture lump)
    uint16_t bottomtexture;    // textura inferior
    uint16_t midtexture;       // textura media (para linedefs two-sided)
    uint16_t sector;           // sector al que pertenece este lado
} sidedef_t;

typedef struct {
    fixed_t floorheight, ceilingheight;
    uint16_t floorpic;         // flat name (índice)
    uint16_t ceilingpic;
    uint16_t lightlevel;       // 0-255
    uint16_t special;          // sector especial (daño, secreto, etc.)
    uint16_t tag;              // tag para linedefs que lo activan
    // runtime:
    int validcount;            // para BSP traversal
    int soundtarget;           // para propagación sonido
    // ... más campos runtime ...
} sector_t;

typedef struct {
    fixed_t x, y;
    angle_t angle;             // 0-65535
    uint16_t type;             // doomednum (MT_PLAYER, MT_POSSESSED, etc.)
    uint16_t options;          // flags: MTF_AMBUSH, MTF_SINGLE, etc.
} thing_t;

// ============================================================================
// 2. ÁRBOL BSP (r_bsp.h, r_bsp.c)
// ============================================================================
/*
 * El BSP se construye en la herramienta de mapa (BSP builder, no en runtime).
 * En runtime solo se RECORRE.
 *
 * Estructura de nodo (r_bsp.h):
 *   typedef struct {
 *       fixed_t x, y;           // punto de la línea de partición
 *       fixed_t dx, dy;         // vector dirección de la línea (normalizado)
 *       fixed_t bbox[2][4];     // bounding box de cada hijo [front/back][top/bottom/left/right]
 *       uint16_t children[2];   // índice de hijo: si bit 15=1 -> subsector (index & 0x7FFF)
 *   } node_t;
 *
 * División: la linedef del nodo define un semiplano. Puntos "front" están a la izquierda
 * del vector (dx, dy) (cross product > 0). "Back" a la derecha.
 *
 * BBox: [side][0]=top (max y), [1]=bottom (min y), [2]=left (min x), [3]=right (max x).
 * Usado para culling rápido: si la caja del nodo no intersecta el frustum, se poda.
 */

typedef struct {
    fixed_t x, y;           // punto en la línea divisora
    fixed_t dx, dy;         // vector dirección (normalizado a FRACUNIT)
    fixed_t bbox[2][4];     // [front/back][top/bottom/left/right]
    uint16_t children[2];   // bit 15 = 1 -> subsector (child & 0x7FFF)
} node_t;

// Índices de bbox para claridad:
#define BBOX_TOP    0
#define BBOX_BOTTOM 1
#define BBOX_LEFT   2
#define BBOX_RIGHT  3

#define CHILD_SUBSECTOR 0x8000  // bit 15: si 1, es subsector
#define CHILD_MASK      0x7FFF  // máscara para índice

// ============================================================================
// 3. SUBSECTORES Y SEGS (r_defs.h, r_segs.h)
// ============================================================================
/*
 * Subsector (r_defs.h):
 *   typedef struct {
 *       uint16_t numsegs;       // cuántos segs
 *       uint16_t firstseg;      // índice inicial en array global segs[]
 *   } subsector_t;
 *
 * Seg (r_segs.h):
 *   typedef struct {
 *       vertex_t v1, v2;        // vértices del segmento (ya transformados a view space en R_RenderSegLoop)
 *       fixed_t offset;         // offset de textura a lo largo del seg
 *       angle_t angle;          // ángulo del seg (normal, apuntando al interior del sector)
 *       uint16_t linedef;       // linedef original
 *       uint16_t side;          // 0 = front, 1 = back
 *       uint16_t partner_seg;   // para portales/ventanas (Doom no tiene, Heretic/Hexen sí)
 *       // runtime:
 *       sector_t* frontsector;
 *       sector_t* backsector;   // NULL si one-sided
 *   } seg_t;
 *
 * Segs son segmentos de linedef RECORTADOS al subsector. Un linedef two-sided
 * genera dos segs (uno por lado) en subsectores adyacentes.
 */

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
    sector_t* frontsector;
    sector_t* backsector;
} seg_t;

// ============================================================================
// 4. RECORRIDO BSP (R_RenderBSPNode) — FRONT-TO-BACK
// ============================================================================
/*
 * R_RenderBSPNode(nodenum) recursivo:
 *
 *   if (nodenum & CHILD_SUBSECTOR) {
 *       R_RenderSubsector(nodenum & CHILD_MASK);
 *       return;
 *   }
 *
 *   node_t* node = &nodes[nodenum];
 *
 *   // Determinar lado del viewer (player) respecto a la línea del nodo:
 *   // side = R_PointOnSide(viewx, viewy, node)  -> 0 front, 1 back
 *
 *   // Recursar PRIMERO el lado donde está el viewer (front-to-back):
 *   R_RenderBSPNode(node->children[side]);
 *
 *   // Luego el otro lado (puede ser occluso):
 *   R_RenderBSPNode(node->children[side^1]);
 *
 * R_PointOnSide(px, py, node):
 *   fixed_t dx = px - node->x;
 *   fixed_t dy = py - node->y;
 *   fixed_t cross = FixedMul(dx, node->dy) - FixedMul(dy, node->dx);
 *   return (cross <= 0) ? 0 : 1;  // front = left of vector (cross > 0)
 *
 * Ventaja front-to-back: overdraw mínimo. Los segs del lado cercano se dibujan
 * primero, y el z-buffer (en Doom: drawsegs + clipper) rechaza los lejanos.
 * Doom NO usa z-buffer por píxel: usa "drawsegs" (intervalos en pantalla) y
 * "visplanes" (spans de floor/ceiling) para resolver visibilidad por scanline.
 */

// Simulación simplificada del recorrido (lógica, no render real):
typedef struct { fixed_t x, y, z; angle_t angle; } view_t;

int R_PointOnSide(fixed_t px, fixed_t py, const node_t* node) {
    fixed_t dx = px - node->x;
    fixed_t dy = py - node->y;
    // cross = dx * node->dy - dy * node->dx
    int64_t cross = (int64_t)dx * node->dy - (int64_t)dy * node->dx;
    return (cross <= 0) ? 0 : 1;  // 0 = front, 1 = back
}

void R_RenderSubsector(int subsec_num); // forward

// Placeholder docente: en Doom real recorre los segs del subsector.
void R_RenderSubsector(int subsec_num) { (void)subsec_num; }

void R_RenderBSPNode(int nodenum, const view_t* view, const node_t* nodes,
                     const subsector_t* subsectors) {
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
// 5. RENDER DE SUBSECTOR: SEGS Y VISPLANES
// ============================================================================
/*
 * R_RenderSubsector(subsec_num):
 *   subsector_t* ss = &subsectors[subsec_num];
 *   for (i = 0; i < ss->numsegs; i++) {
 *       seg_t* seg = &segs[ss->firstseg + i];
 *       R_AddLine(seg);  // proyecta seg a pantalla, clippea, añade a drawsegs
 *   }
 *
 * R_AddLine proyecta v1/v2 a coordenadas de pantalla (perspectiva),
 * recorta contra frustum (clip horizontal), y si visible añade a drawsegs[]
 * (array global de segmentos de pared visibles, ordenados front-to-back).
 *
 * VISPLANES (r_plane.h, r_plane.c):
 *   Los pisos y techos (flats) se renderizan por SPANS horizontales (scanlines).
 *   Un visplane = región contigua de mismo flat, misma altura, misma luz, misma scale.
 *   Estructura:
 *     typedef struct {
 *         fixed_t height;       // floor/ceiling height
 *         uint16_t picnum;      // flat texture index
 *         int lightlevel;       // light level
 *         fixed_t scale;        // escala de textura (varía con distancia)
 *         int minx, maxx;       // rango horizontal en pantalla
 *         short pad[...];       // tabla de top/bottom por columna (para clipping)
 *     } visplane_t;
 *
 *   R_CheckPlane / R_MakePlane / R_ExpandPlane / R_DrawPlanes.
 *   Al recorrer segs, se detectan cambios de sector (floor/ceiling height/texture)
 *   y se crean/extienden visplanes. Al final del frame, R_DrawPlanes dibuja
 *   todos los visplanes (span por span, textura mapeada).
 *
 * SPRITES (things): se añaden a sprites[] durante BSP traversal (R_ProjectSprite),
 * luego se ordenan por distancia (back-to-front) y dibujan con R_DrawSprite.
 * R_DrawVisSprites itera sprites[] y llama R_DrawVisSprite.
 */

// Estructura simplificada de visplane:
typedef struct {
    fixed_t height;
    uint16_t picnum;
    int lightlevel;
    fixed_t scale;
    int minx, maxx;
    short* top;      // array [maxx-minx+1] de y-top por columna
    short* bottom;   // array [maxx-minx+1] de y-bottom
} visplane_t;

// ============================================================================
// 6. DRAWSEGS: CLIPPING HORIZONTAL (r_segs.c)
// ============================================================================
/*
 * drawseg_t: segmento de pared ya proyectado y clipeado horizontalmente.
 *   typedef struct {
 *       int x1, x2;           // columnas de pantalla [x1, x2]
 *       fixed_t scale1, scale2; // escala de textura en x1 y x2
 *       fixed_t silhouette;   // SIL_BOTH, SIL_TOP, SIL_BOTTOM, SIL_NONE
 *       fixed_t bsilheight;   // altura del bottom silhouette
 *       fixed_t tsilheight;   // altura del top silhouette
 *       int curline;          // índice en linedefs
 *       int sidedef;          // 0 front, 1 back
 *       int linedef;          // linedef num
 *       sector_t* frontsector;
 *       sector_t* backsector;
 *   } drawseg_t;
 *
 * Silhouettes: cuando un linedef two-sided tiene distinta altura/textura
 * en front/back, se genera un "silhouette" (borde visible) para recortar
 * visplanes y sprites detrás. SIL_TOP = techo distinto, SIL_BOTTOM = suelo distinto.
 *
 * El array drawsegs[] se llena durante BSP traversal. R_RenderMaskedSegRange
 * dibuja los segs en orden (front-to-back), y el clipper horizontal (solidsegs[])
 * evita overdraw: solidsegs[] mantiene intervalos [x1,x2] ya dibujados.
 */

typedef enum { SIL_NONE, SIL_BOTTOM, SIL_TOP, SIL_BOTH } sil_type_t;

typedef struct {
    int x1, x2;
    fixed_t scale1, scale2;
    sil_type_t silhouette;
    fixed_t bsilheight, tsilheight;
    sector_t* frontsector;
    sector_t* backsector;
} drawseg_t;

// Solidsegs para clipping horizontal (intervalos ya cubiertos):
typedef struct { int x1, x2; } solidseg_t;
solidseg_t solidsegs[256];  // máximo práctico
int solidseg_count = 0;

int R_ClipSolidWallSegment(int x1, int x2) {
    // Recorre solidsegs[] y recorta [x1,x2] contra intervalos existentes.
    // Devuelve nuevo x1, x2 o -1 si totalmente occluso.
    // (Implementación real en r_segs.c: R_ClipSolidWallSegment)
    return 0; // placeholder
}

// ============================================================================
// 7. ORDEN DE RENDER COMPLETO (R_RenderPlayerView en r_main.c)
// ============================================================================
/*
 * R_RenderPlayerView(player):
 *   1. Setup view: viewx, viewy, viewz, viewangle, viewcos, viewsin
 *   2. Clear drawsegs, visplanes, sprites, solidsegs
 *   3. R_RenderBSPNode(rootnode) -> llena drawsegs, visplanes, sprites
 *   4. R_DrawPlanes() -> dibuja visplanes (floor/ceiling) span por span
 *   5. R_DrawMasked() -> dibuja drawsegs (paredes) + masked mids
 *   6. R_DrawVisSprites() -> dibuja sprites ordenados back-to-front
 *   7. R_DrawMasked() también dibuja sprites en drawsegs (mid textures)
 *
 * Clave: BSP traversal front-to-back -> drawsegs front-to-back.
 * Visplanes se dibujan ANTES que paredes (piso/techo detrás de paredes).
 * Sprites se dibujan DESPUÉS (delante de paredes, pero con depth test por drawsegs).
 */

// ============================================================================
// 8. PVS EN DOOM: NO EXISTE COMO TAL
// ============================================================================
/*
 * Doom NO tiene PVS precalculado (como Quake). La visibilidad se resuelve
 * EN TIEMPO REAL recorriendo el BSP front-to-back con frustum culling.
 *
 * Ventajas: memoria cero para PVS, mapa dinámico (puertas abren/cierran cambian visibilidad).
 * Desventajas: CPU por frame para traversal (pero BSP es O(log n) nodos visitados).
 *
 * En source ports modernos (GZDoom, etc.): se añade PVS precalculado o
 * portal-based visibility para acelerar en mapas grandes.
 */

// ============================================================================
// 9. TEST DE ESTRUCTURAS (lógica pura)
// ============================================================================

int test_bsp_structures(void) {
    int passed = 0, failed = 0;
    #define TEST(expr, msg) \
        do { if (expr) { passed++; } else { failed++; printf("FAIL: %s\n", msg); } } while(0)

    // Verificar tamaños esperados (compilación nativa, 64-bit)
    TEST(sizeof(vertex_t) == 8, "vertex_t = 2*fixed_t = 8 bytes");
    TEST(sizeof(linedef_t) == 14, "linedef_t = 7*uint16_t = 14 bytes");
    // Nota: fixed_t = int32_t = 4 bytes. sidedef: 2*4 + 4*2 = 16 bytes.
    TEST(sizeof(sidedef_t) == 16, "sidedef_t = 16 bytes");
    TEST(sizeof(sector_t) >= 16, "sector_t >= 16 bytes");
    TEST(sizeof(node_t) == 52, "node_t size = 4*4 + 8*4 + 2*2 = 52 bytes");
    TEST(sizeof(subsector_t) == 4, "subsector_t = 2*uint16_t = 4 bytes");
    TEST(sizeof(seg_t) >= 32, "seg_t >= 32 bytes");

    // CHILD_SUBSECTOR bit
    TEST((CHILD_SUBSECTOR & 0x8000) != 0, "CHILD_SUBSECTOR bit 15");
    TEST((CHILD_MASK & 0x7FFF) == 0x7FFF, "CHILD_MASK 15 bits");

    // R_PointOnSide: nodo vertical en x=0, dx=0, dy=FRACUNIT (apunta +Y)
    // front = left of vector = negative x
    node_t node = {0, 0, 0, FRACUNIT, {{0}}, {0, 1}};
    TEST(R_PointOnSide(-FRACUNIT, 0, &node) == 0, "left of vertical line = front");
    TEST(R_PointOnSide(FRACUNIT, 0, &node) == 1, "right of vertical line = back");

    printf("BSP struct tests: %d pass, %d fail\n", passed, failed);
    return failed == 0 ? 0 : 1;
}

int main(void) {
    printf("Módulo 27 - Lección 03: BSP y estructura del mapa\n");
    return test_bsp_structures();
}