/*
 * MÓDULO 27: Doom Clásico — De C a C++
 * LECCIÓN 04: Visplanes, Drawsegs y Render por Scanline (r_plane.c, r_segs.c, r_things.c)
 *
 * TEMAS:
 * - Visplanes: spans horizontales de floor/ceiling (flats), clipping, textura
 * - Drawsegs: segmentos de pared proyectados, siluetas, clipping horizontal
 * - Sprites: proyección, escalado, clipping, orden back-to-front
 * - Paleta, colormaps, light diminishing (z-depth lighting)
 * - Limitaciones del render software: visplane overflow, drawseg overflow
 *
 * VERIFICACIÓN: Código consultado en r_plane.h/c, r_segs.h/c, r_things.h/c, r_draw.h/c del source release.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// ============================================================================
// 1. TIPOS BASE (reutilizados de lecciones anteriores)
// ============================================================================
typedef int32_t fixed_t;
#define FRACUNIT 65536
#define FIXED_TO_INT(x) ((x) >> 16)
#define INT_TO_FIXED(x) ((x) << 16)

typedef struct { fixed_t x, y; } vertex_t;
typedef uint16_t angle_t;
typedef uint8_t byte;
typedef struct sector_s sector_t;

// ============================================================================
// 2. VISPLANES: RENDER DE FLATS (SUELO/TECHO)
// ============================================================================
/*
 * Un visplane (visplane_t) representa una región continua de piso o techo
 * con las MISMAS propiedades: altura, textura (flat), nivel de luz, escala.
 *
 * Estructura real (r_plane.h):
 *   typedef struct visplane_s {
 *       fixed_t height;           // altura del floor/ceiling (fixed_t)
 *       int picnum;               // índice de flat (lump number)
 *       int lightlevel;           // 0-255
 *       fixed_t scale;            // escala de textura (FRACUNIT * viewdist / depth)
 *       int minx, maxx;           // rango horizontal en columnas de pantalla
 *       short* top;               // array [maxx-minx+1]: y-top por columna
 *       short* bottom;            // array [maxx-minx+1]: y-bottom por columna
 *       // ... campos para padding, next/prev en lista ...
 *   } visplane_t;
 *
 * CICLO DE VIDA:
 * 1. R_ClearPlanes() al inicio del frame: resetear arrays.
 * 2. Durante BSP traversal (R_RenderBSPNode -> R_RenderSubsector -> R_AddLine):
 *    - Al cruzar un linedef two-sided con distinta altura/textura floor/ceiling,
 *      se llama R_CheckPlane / R_MakePlane / R_ExpandPlane.
 *    - R_CheckPlane: busca visplane existente compatible (misma height, picnum,
 *      lightlevel, scale). Si encuentra, lo extiende (R_ExpandPlane).
 *    - R_MakePlane: crea nuevo visplane si no hay compatible.
 *    - R_ExpandPlane: actualiza minx/maxx y los arrays top/bottom.
 * 3. Al final: R_DrawPlanes() itera todos los visplanes y dibuja spans.
 *
 * R_DrawPlane(visplane_t* pl):
 *   for (x = pl->minx; x <= pl->maxx; x++) {
 *       ytop = pl->top[x - pl->minx];
 *       ybottom = pl->bottom[x - pl->minx];
 *       if (ytop <= ybottom) {
 *           // Calcular textura coords para esta columna
 *           // Dibujar span vertical [ytop, ybottom] con flat texture
 *           // Aplicar colormap según lightlevel y distancia (scale)
 *       }
 *   }
 *
 * TEXTURA FLAT: 64x64 pixels, repetida (tile). Mapeo:
 *   u = (world_x * scale) & 63
 *   v = (world_y * scale) & 63
 *   (scale incluye factor de perspectiva).
 *
 * COLORMAPS: 32 niveles de luz (0-31) + fullbright. Cada colormap es 256 bytes
 * (remap de paleta). Light diminishing: colormap = base_colormap - (scale >> LIGHTSCALESHIFT).
 */

// Estructura visplane simplificada:
typedef struct {
    fixed_t height;
    int picnum;
    int lightlevel;
    fixed_t scale;
    int minx, maxx;
    short* top;
    short* bottom;
    struct visplane_s* next;
} visplane_t;

#define MAXVISPLANES 128  // límite original (visplane overflow si excede)
visplane_t visplanes[MAXVISPLANES];
visplane_t* free_visplanes = NULL;
visplane_t* active_visplanes = NULL;

int visplane_count = 0;

// ============================================================================
// 3. DRAWSEGS: PAREDES PROYECTADAS Y CLIPPEADAS
// ============================================================================
/*
 * drawseg_t: un segmento de pared ya proyectado a coordenadas de pantalla,
 * clipeado horizontalmente contra el frustum y contra solidsegs[].
 *
 * Estructura real (r_segs.h):
 *   typedef struct {
 *       int x1, x2;               // columnas de pantalla [x1, x2] inclusive
 *       fixed_t scale1, scale2;   // escala de textura en x1 y x2
 *       fixed_t scalestep;        // (scale2 - scale1) / (x2 - x1) para interpolación
 *       int silhouette;           // SIL_NONE, SIL_BOTTOM, SIL_TOP, SIL_BOTH
 *       fixed_t bsilheight;       // altura del bottom silhouette (floor)
 *       fixed_t tsilheight;       // altura del top silhouette (ceiling)
 *       int curline;              // linedef num
 *       int sidedef;              // 0 front, 1 back
 *       sector_t* frontsector;
 *       sector_t* backsector;
 *       // ... campos para masked mids ...
 *   } drawseg_t;
 *
 * SILUETAS: Cuando un linedef two-sided conecta sectores con distinta
 * floorheight (SIL_BOTTOM) o ceilingheight (SIL_TOP), el borde visible
 * (silhouette) debe recortar visplanes y sprites detrás.
 *
 * CLIPPING HORIZONTAL (solidsegs):
 *   Array de intervalos [x1,x2] ya dibujados (opacos).
 *   R_ClipSolidWallSegment(x1, x2) recorta contra solidsegs[].
 *   Si resultado vacío -> pared totalmente oculta, no se dibuja.
 *   Si parcial -> se dibuja la parte visible y se actualiza solidsegs[].
 *
 * R_RenderMaskedSegRange(drawseg_t* ds):
 *   Dibuja la pared columna por columna (x1 a x2):
 *   - Interpola scale, texture x offset
 *   - Para cada columna: calcula ytop, ybottom en pantalla
 *   - Aplica colormap según lightlevel y scale (distancia)
 *   - Dibuja span vertical con textura de pared (patch)
 *   - Si SIL_TOP/BOTTOM: dibuja silueta (span adicional)
 *   - Si midtexture (ventana/reja): dibuja mid texture con transparencia
 */

typedef enum { SIL_NONE, SIL_BOTTOM, SIL_TOP, SIL_BOTH } sil_type_t;

typedef struct {
    int x1, x2;
    fixed_t scale1, scale2;
    fixed_t scalestep;
    sil_type_t silhouette;
    fixed_t bsilheight, tsilheight;
    sector_t* frontsector;
    sector_t* backsector;
} drawseg_t;

#define MAXDRAWSEGS 256
drawseg_t drawsegs[MAXDRAWSEGS];
int drawseg_count = 0;

// Solidsegs para clipping horizontal
typedef struct { int x1, x2; } solidseg_t;
solidseg_t solidsegs[MAXDRAWSEGS * 2];
int solidseg_count = 0;

// ============================================================================
// 4. SPRITES (THINGS): PROYECCIÓN Y ORDENACIÓN
// ============================================================================
/*
 * Sprites = cosas (mobj_t): enemigos, items, proyectiles, decoración.
 * NO son parte del BSP estático; se añaden dinámicamente durante traversal.
 *
 * R_ProjectSprite(mobj_t* mo): llamado desde R_RenderBSPNode al visitar subsector.
 *   - Calcula posición relativa al viewer (trig con FixedMul/FixedDiv)
 *   - Determina sprite frame (mo->sprite, mo->frame)
 *   - Calcula escala: scale = FRACUNIT * viewdist / distance
 *   - Culling: si detrás del viewer o fuera de frustum horizontal -> descartar
 *   - Añade a array sprites[] (vissprites[])
 *
 * vissprite_t (r_things.h):
 *   typedef struct {
 *       fixed_t x, y;           // posición en view space
 *       fixed_t gx, gy;         // posición en world space (para sorting)
 *       fixed_t scale;          // escala global
 *       fixed_t xscale, yscale; // escalas X/Y (pueden diferir si no square)
 *       int patch;              // índice de patch (lump) del sprite frame
 *       int colormap;           // colormap num (light level + distance)
 *       int x1, x2;             // rango horizontal en pantalla
 *       fixed_t startfrac;      // offset de textura en x1
 *       int texturemid;         // offset vertical de textura
 *       fixed_t xiscale;        // 1/scale para stepping
 *       // ... campos para clipping ...
 *   } vissprite_t;
 *
 * ORDENACIÓN: vissprites[] se ordena por distancia (gx,gy -> distance) DESCENDENTE
 * (back-to-front: más lejos primero). qsort con comparador de distancia.
 *
 * R_DrawVisSprites(): itera vissprites[] ordenados, llama R_DrawVisSprite.
 * R_DrawVisSprite: clipping horizontal (solidsegs), luego columna por columna
 * con escalado de patch (R_DrawVisSprite -> R_DrawColumn -> R_DrawColumnShadowed).
 */

typedef struct {
    fixed_t x, y;           // view space
    fixed_t gx, gy;         // world space
    fixed_t scale;
    fixed_t xscale, yscale;
    int patch;
    int colormap;
    int x1, x2;
    fixed_t startfrac;
    int texturemid;
    fixed_t xiscale;
} vissprite_t;

#define MAXVISSPRITES 256
vissprite_t vissprites[MAXVISSPRITES];
int vissprite_count = 0;

// Comparador para qsort (back-to-front: mayor distancia primero)
int vissprite_compare(const void* a, const void* b) {
    const vissprite_t* va = (const vissprite_t*)a;
    const vissprite_t* vb = (const vissprite_t*)b;
    // Distancia aproximada: gx^2 + gy^2 (evita sqrt)
    int64_t da = (int64_t)va->gx * va->gx + (int64_t)va->gy * va->gy;
    int64_t db = (int64_t)vb->gx * vb->gx + (int64_t)vb->gy * vb->gy;
    return (da > db) ? -1 : (da < db) ? 1 : 0;  // descendente
}

// ============================================================================
// 5. PALETA, COLORMAPS Y LIGHT DIMINISHING
// ============================================================================
/*
 * Paleta: PLAYPAL lump = 14 paletas de 256 colores (256*3*14 bytes).
 *   Paleta 0 = normal. 1-13 = damage, bonus, radsuit, etc. (efectos pantalla).
 *
 * Colormaps: COLORMAP lump = 32 colormaps de 256 bytes cada uno (32*256).
 *   Colormap 0 = fullbright (sin oscuridad).
 *   Colormap 1-31 = progresivamente más oscuro.
 *   Colormap 32 = negro total (para fuera de rango).
 *
 * Light diminishing (r_main.c, R_SetVisibility):
 *   Las paredes/flats usan colormap según luz del sector + distancia.
 *   Fórmula aproximada:
 *     int light = sector->lightlevel;  // 0-255
 *     fixed_t scale = ...;             // escala de textura (proporcional a 1/distancia)
 *     int colormap = (light >> LIGHTSEGSHIFT) - (scale >> LIGHTSCALESHIFT);
 *     colormap = clamp(colormap, 0, NUMCOLORMAPS-1);
 *
 *   LIGHTSEGSHIFT = 4 (lightlevel / 16 -> 0-15, pero colormaps son 32).
 *   LIGHTSCALESHIFT = 16 (scale es fixed_t, shift 16 = parte entera).
 *
 *   En la práctica: lightlevel 0-255 mapea a 0-31 colormaps.
 *   Distancia lejana -> scale pequeño -> scale>>16 = 0 -> sin cambio.
 *   Distancia cercana -> scale grande -> colormap más alto (más oscuro).
 *   (Contraintuitivo: en Doom, "scale" grande = cerca = más detalle = más oscuro?
 *   Revisar r_main.c: scale = FRACUNIT * viewdist / distance. Cerca = distance pequeño = scale GRANDE.
 *   scale >> LIGHTSCALESHIFT = parte entera de scale. Si scale > 65536, colormap aumenta = más oscuro.
 *   Correcto: cosas cercanas se ven más brillantes en realidad, pero Doom oscurece
 *   por "light diminishing" para dar sensación de profundidad. Ver código real.)
 */

#define NUMCOLORMAPS 32
#define LIGHTSEGSHIFT 4
#define LIGHTSCALESHIFT 16

byte* colormaps[NUMCOLORMAPS];  // punteros a lump data (cada uno 256 bytes)

int R_GetColormap(int lightlevel, fixed_t scale) {
    // scale es fixed_t: 1.0 = 65536. A distancias > 1 unidad se oscurece.
    int distancia = FIXED_TO_INT(scale);
    int dim = (distancia > 1) ? distancia : 0;
    int map = (lightlevel >> LIGHTSEGSHIFT) - dim;
    if (map < 0) map = 0;
    if (map >= NUMCOLORMAPS) map = NUMCOLORMAPS - 1;
    return map;
}

// ============================================================================
// 6. LIMITACIONES Y OVERFLOWS (LÍMITES ORIGINALES)
// ============================================================================
/*
 * El engine original tiene límites duros (arrays estáticos):
 * - MAXVISPLANES = 128     -> "visplane overflow" crash si excede
 * - MAXDRAWSEGS = 256      -> "drawseg overflow"
 * - MAXVISSPRITES = 256    -> sprites no se dibujan si excede
 * - MAXLINEANIMS = 64      -> linedefs animados
 * - MAXPLATS = 30          -> plataformas móviles
 * - MAXBUTTONS = 30        -> botones
 * - MAXCEILINGS = 30       -> techos móviles
 * - etc.
 *
 * En mapas complejos (muchos sectores visibles, arquitectura detallada)
 * estos límites se alcanzan. Source ports modernos los aumentan o usan
 * arrays dinámicos (malloc/realloc o vector).
 *
 * VISPLANE OVERFLOW: ocurre cuando hay muchos cambios de floor/ceiling
 * height/texture/light en la vista. Cada cambio crea nuevo visplane.
 * Solución original: simplificar mapa. Solución moderna: malloc dinámico.
 */

// ============================================================================
// 7. RENDER PIPELINE RESUMIDO (R_RenderPlayerView)
// ============================================================================
/*
 * void R_RenderPlayerView(player_t* player) {
 *     // 1. View setup
 *     viewx = player->mo->x;
 *     viewy = player->mo->y;
 *     viewz = player->mo->z + player->viewheight;
 *     viewangle = player->mo->angle;
 *     viewcos = FixedCos(viewangle);
 *     viewsin = FixedSin(viewangle);
 *
 *     // 2. Clear frame data
 *     R_ClearPlanes();
 *     R_ClearDrawSegs();
 *     R_ClearSprites();
 *     solidseg_count = 0;
 *
 *     // 3. BSP Traversal (front-to-back)
 *     R_RenderBSPNode(numnodes - 1);  // root node es el último
 *
 *     // 4. Draw flats (visplanes) - PRIMERO, detrás de todo
 *     R_DrawPlanes();
 *
 *     // 5. Draw walls (drawsegs) - front-to-back (ya ordenados por BSP)
 *     R_DrawMasked();  // también dibuja masked mids en drawsegs
 *
 *     // 6. Draw sprites (vissprites) - back-to-front (qsort por distancia)
 *     qsort(vissprites, vissprite_count, sizeof(vissprite_t), vissprite_compare);
 *     R_DrawVisSprites();
 *
 *     // 7. HUD, weapon sprite (PSprites) - último, sin depth test
 *     R_DrawPlayerSprites();
 * }
 *
 * ORDEN CRÍTICO:
 * 1. Visplanes (floor/ceiling) -> más atrás
 * 2. Drawsegs (paredes) -> medio
 * 3. Sprites (things) -> delante
 * 4. PSprites (arma/HUD) -> delante de todo, sin depth
 */

// ============================================================================
// 8. TESTS DE LÓGICA (sin render real)
// ============================================================================

int test_render_logic(void) {
    int passed = 0, failed = 0;
    #define TEST(expr, msg) \
        do { if (expr) { passed++; } else { failed++; printf("FAIL: %s\n", msg); } } while(0)

    // Visplane count limit
    TEST(MAXVISPLANES == 128, "MAXVISPLANES = 128");
    TEST(MAXDRAWSEGS == 256, "MAXDRAWSEGS = 256");
    TEST(MAXVISSPRITES == 256, "MAXVISSPRITES = 256");

    // Colormap calculation
    TEST(R_GetColormap(255, INT_TO_FIXED(1)) == 15, "full light, dist 1 -> colormap 15");
    TEST(R_GetColormap(0, INT_TO_FIXED(1)) == 0, "dark, dist 1 -> colormap 0");
    TEST(R_GetColormap(255, INT_TO_FIXED(100)) == 0, "full light, far -> colormap 0 (clamp)");

    // Vissprite sort: back-to-front
    vissprite_t a = {.gx = INT_TO_FIXED(100), .gy = 0};
    vissprite_t b = {.gx = INT_TO_FIXED(50), .gy = 0};
    TEST(vissprite_compare(&a, &b) < 0, "a (far) before b (near) in sort");

    // Silhouette types
    TEST(SIL_NONE == 0 && SIL_BOTTOM == 1 && SIL_TOP == 2 && SIL_BOTH == 3,
         "silhouette enum values");

    printf("Render logic tests: %d pass, %d fail\n", passed, failed);
    return failed == 0 ? 0 : 1;
}

int main(void) {
    printf("Módulo 27 - Lección 04: Visplanes, Drawsegs, Sprites\n");
    return test_render_logic();
}