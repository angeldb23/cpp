/* =============================================================================
   MÓDULO 17 - LECCIÓN 10: Tilemaps II — Colisión por Tile y Culling de Vista
   =============================================================================
   TEMA 8 de TEMAS.md (2da lección): Colisión por tile + culling

   CONCEPTOS CLAVE:
   - Tile sólido (collision layer): un set de GIDs que bloquea al jugador
   - Colisión por movimiento con ejes SEPARADOS: probar X y Y por separado
     permite "deslizar" por paredes en vez de atascarse en las esquinas
   - Snap + clamp: si al mover a X se solapa con un tile sólido, se recoloca
     en el borde del tile (no se "aproxima"), de lo contrario queda temblando
   - Consulta O(1): tile => convertir coordenadas de mundo a índices con
     floor(x / tileW) — NO hace falta recorrer todos los tiles
   - Culling de vista: dibuja SOLO los tiles que quedan dentro del rectángulo
     visible (convierte la cámara de módulo 16 en rango de tiles)
   - ROI: el área de juego puede ser grande (200x200 tiles); el viewport ve
     30x17; dibujar 510 tiles vs 34.000 es la diferencia entre 60 y 12 FPS

   NOTA: el culling exacto depende del zoom de Camera2D: GetScreenToWorld2D
   da el rect del mundo visible (lección 04). Aquí está encapsulado.
   ============================================================================= */

#include <iostream>
#include <vector>
#include <unordered_set>
#include <string>
#include <cmath>
#include <algorithm>

// -----------------------------------------------------------------------------
// Stubs raylib
// -----------------------------------------------------------------------------
struct Vector2 { float x, y; };
struct Rectangle { float x, y, width, height; };
struct Color { unsigned char r, g, b, a; };
struct Texture2D { int id; int width; int height; };
struct Camera2D { Vector2 offset; Vector2 target; float rotation; float zoom; };

inline void DrawTextureRec(Texture2D, Rectangle, Vector2, Color) {}
inline void DrawRectangleRec(Rectangle, Color) {}
inline void DrawRectangleLinesEx(Rectangle, float, Color) {}
inline void DrawText(const char*, int, int, int, Color) {}

// ScreenToWorld (idéntica a la lección 04): cámara inversa simple
inline Vector2 GetScreenToWorld2D(Vector2 screen, Camera2D cam) {
    return { (screen.x - cam.offset.x) / cam.zoom + cam.target.x,
             (screen.y - cam.offset.y) / cam.zoom + cam.target.y };
}

// -----------------------------------------------------------------------------
// 1. Tilemap "de juego": grilla + conjunto de tiles sólidos
// -----------------------------------------------------------------------------
class GameTilemap {
    int wTiles_ = 0, hTiles_ = 0;
    int tileW_ = 32, tileH_ = 32;
    std::vector<int> gids_;                        // capa "base" a dibujar
    std::unordered_set<int> solidGids_;            // qué GIDs bloquean
    std::vector<unsigned char> solidMap_;          // bool por tile precomputado

public:
    GameTilemap(int w, int h, int tw, int th)
        : wTiles_(w), hTiles_(h), tileW_(tw), tileH_(th) {
        gids_.assign(size_t(w) * h, 0);
        solidMap_.assign(size_t(w) * h, 0);
    }

    int WidthTiles() const { return wTiles_; }
    int HeightTiles() const { return hTiles_; }
    int TileW() const { return tileW_; }
    int TileH() const { return tileH_; }
    float WorldWidth() const { return float(wTiles_ * tileW_); }
    float WorldHeight() const { return float(hTiles_ * tileH_); }

    void SetGid(int tx, int ty, int gid) {
        if (tx < 0 || ty < 0 || tx >= wTiles_ || ty >= hTiles_) return;
        gids_[size_t(ty) * wTiles_ + tx] = gid;
        solidMap_[size_t(ty) * wTiles_ + tx] = solidGids_.count(gid) ? 1 : 0;
    }
    int Gid(int tx, int ty) const {
        if (tx < 0 || ty < 0 || tx >= wTiles_ || ty >= hTiles_) return 0;
        return gids_[size_t(ty) * wTiles_ + tx];
    }

    // Marca de GIDs sólidos; se llama ANTES de poblar el mapa (o de-todo-uno)
    void SetSolidGids(const std::vector<int>& gids) {
        solidGids_.clear();
        solidGids_.insert(gids.begin(), gids.end());
        // Precomputa el solidMap (una vez, no por frame)
        for (size_t i = 0; i < gids_.size(); ++i) {
            solidMap_[i] = solidGids_.count(gids_[i]) ? 1 : 0;
        }
    }

    // Mundo -> tile (floor para que vaya a la izquierda/arriba)
    long TileX(float worldX) const { return long(std::floor(worldX / tileW_)); }
    long TileY(float worldY) const { return long(std::floor(worldY / tileH_)); }

    // ¿La celda (tx,ty) es sólida? (fuera de mapa: SIN colisión, en juegos
    // tipo plataformas es "aire"; en laberintos con bordes, con)
    bool IsSolidCell(long tx, long ty) const {
        if (tx < 0 || ty < 0 || tx >= wTiles_ || ty >= hTiles_) return false;
        return solidMap_[size_t(ty) * wTiles_ + tx] != 0;
    }

    // ¿El rect del actor (world coords) toca algún tile sólido?
    // Costo O(1) típico: solo tiles que el rect toca (2x2 → 9 máx)
    bool OverlapsSolid(const Rectangle& aabb) const {
        const long x0 = TileX(aabb.x),          x1 = TileX(aabb.x + aabb.width);
        const long y0 = TileY(aabb.y),          y1 = TileY(aabb.y + aabb.height);
        for (long ty = y0; ty <= y1; ++ty) {
            for (long tx = x0; tx <= x1; ++tx) {
                if (IsSolidCell(tx, ty)) return true;
            }
        }
        return false;
    }

    // El rect del tile (para snap y para debug draw)
    Rectangle TileRect(long tx, long ty) const {
        return { float(tx * tileW_), float(ty * tileH_), float(tileW_), float(tileH_) };
    }

    // Culling: rango de tiles que ve la cámara (con margen)
    struct TileRange { long x0, x1, y0, y1; };
    TileRange VisibleRange(const Camera2D& cam, int screenW, int screenH, int margin = 1) const {
        // Las 4 esquinas de pantalla al mundo (el zoom/rotación hacen esto)
        Vector2 tl = GetScreenToWorld2D({0.0f,          0.0f},          cam);
        Vector2 br = GetScreenToWorld2D({float(screenW), float(screenH)}, cam);
        long x0 = TileX(tl.x) - margin, x1 = TileX(br.x) + margin;
        long y0 = TileY(tl.y) - margin, y1 = TileY(br.y) + margin;
        // Clamp al mapa (fuera del mapa NADA que dibujar)
        x0 = std::max(long(0), x0); x1 = std::min(long(wTiles_ - 1), x1);
        y0 = std::max(long(0), y0); y1 = std::min(long(hTiles_ - 1), y1);
        // Guarda: rango invertido o cámara fuera de todo => rango vacío
        if (x0 > x1) { x1 = x0 - 1; }
        if (y0 > y1) { y1 = y0 - 1; }
        return { x0, x1, y0, y1 };
    }

    int CountVisibleTiles(const Camera2D& cam, int screenW, int screenH) const {
        TileRange r = VisibleRange(cam, screenW, screenH);
        long w = std::max<long>(0, r.x1 - r.x0 + 1);
        long h = std::max<long>(0, r.y1 - r.y0 + 1);
        return int(w * h);
    }

    int TotalTiles() const { return wTiles_ * hTiles_; }
};

// -----------------------------------------------------------------------------
// 2. Movimiento con ejes separados (slide)
// -----------------------------------------------------------------------------
/* Por qué separar los ejes:
   - Si muevo (dx, dy) junto y chequeo solapamiento de UNA vez, un jugador que
     viene en diagonal a una esquina se atasca (colisión con "pared que no tocaba").
   - Si muevo SOLO X, si bloquea: snap al borde. Luego muevo SOLO Y: si
     bloquea: snap. El resultado: "sliding" (se desliza por la pared) y el
     clásico comportamiento de Mario, Celeste, Hollow Knight… */

struct Actor {
    Vector2 position { 0, 0 };   // esquina sup-izq del AABB del actor
    float width = 24.0f, height = 24.0f;
    Vector2 velocity { 0, 0 };

    Rectangle AABBRounded(float dx, float dy) const {
        return { position.x + dx, position.y + dy, width, height };
    }
};

/* SNAP: al solapar con un tile en X, llevar al actor al borde EXACTO del
   tile (no reduce progresivamente la velocidad). Resolución determinista:
   la misma entrada siempre produce la misma salida (te acordás del módulo 16:
   el determinismo es la base del gameplay y del netcode). */
void MoveAndSlideX(Actor& a, float dx, const GameTilemap& map) {
    Rectangle rec = a.AABBRounded(dx, 0);
    if (!map.OverlapsSolid(rec)) {
        a.position.x += dx;
        return;
    }
    // ¿En qué dirección me muevo? Snap al borde del tile correspondiente
    const long tileCoord = (dx > 0.0f)
        ? map.TileX(rec.x + rec.width)                      // tile que bloquea por derecha
        : map.TileX(rec.x);                                 // tile que bloquea por izquierda
    if (dx > 0.0f) {
        a.position.x = float(tileCoord * map.TileW()) - a.width;
        // EN DETALLE: hay que comprobar si rec.x + rec.width no cruce múltiples
        // tiles: si el dt es grande y el actor rápido, se atravesaría. La
        // solución robusta (CCD) es del módulo 23; para velocidades normales
        // basta limitar dx a menos de un tile por frame.
    } else {
        a.position.x = float((tileCoord + 1) * map.TileW());
    }
    a.velocity.x = 0.0f;
}

void MoveAndSlideY(Actor& a, float dy, const GameTilemap& map) {
    Rectangle rec = a.AABBRounded(0, dy);
    if (!map.OverlapsSolid(rec)) {
        a.position.y += dy;
        return;
    }
    const long tileCoord = (dy > 0.0f)
        ? map.TileY(rec.y + rec.height)                     // piso que bloquea por abajo
        : map.TileY(rec.y);                                 // techo
    if (dy > 0.0f) {
        a.position.y = float(tileCoord * map.TileH()) - a.height;
    } else {
        a.position.y = float((tileCoord + 1) * map.TileH());
    }
    a.velocity.y = 0.0f;
}

// -----------------------------------------------------------------------------
// 3. Render con culling
// -----------------------------------------------------------------------------
/* NO: recorrer TODOS los tiles y preguntar si es visible (culling por tile
   iterativo es O(N) por frame: recorrer 600 tiles para dibujar 300 deshace
   medio ahorro). SÍ: calcular el rango visible O(1) y recorrer SOLO ese rango. */
void RenderVisible(GameTilemap& map, const Texture2D& atlas,
                   const Camera2D& cam, int screenW, int screenH) {
    auto range = map.VisibleRange(cam, screenW, screenH, 1);
    for (long ty = range.y0; ty <= range.y1; ++ty) {
        for (long tx = range.x0; tx <= range.x1; ++tx) {
            int gid = map.Gid(int(tx), int(ty));
            if (gid == 0) continue;                  // vacío
            // Placeholder: rect destino (aquí se pegaría del atlas real)
            Rectangle src{ float((gid - 1) * map.TileW()), 0,
                           float(map.TileW()), float(map.TileH()) };
            Vector2 pos{ float(tx * map.TileW()), float(ty * map.TileH()) };
            DrawTextureRec(atlas, src, pos, {255,255,255,255});
            (void)pos;
        }
    }
}

// -----------------------------------------------------------------------------
// Demo
// -----------------------------------------------------------------------------
GameTilemap BuildDemoMap() {
    // 30x20 tiles de 32px => 960x640 de mundo
    GameTilemap map(30, 20, 32, 32);
    map.SetSolidGids({2, 3});        // GIDs que bloquean (piso/muros)

    // Piso (GID 2) en las filas 18-19
    for (int tx = 0; tx < 30; ++tx) {
        map.SetGid(tx, 18, 2);
        map.SetGid(tx, 19, 2);
    }
    // Muros laterales (GID 3)
    for (int ty = 0; ty < 18; ++ty) {
        map.SetGid(0, ty, 3);
        map.SetGid(29, ty, 3);
    }
    // Plataformas flotantes
    for (int tx = 4; tx < 9; ++tx)  map.SetGid(tx, 14, 3);
    for (int tx = 12; tx < 17; ++tx) map.SetGid(tx, 11, 3);
    for (int tx = 20; tx < 26; ++tx) map.SetGid(tx, 15, 3);
    return map;
}

void demo_tile_collision_and_culling() {
    std::cout << "=== Tilemap II: Colisión por Tile y Culling ===\n\n";

    GameTilemap map = BuildDemoMap();
    std::cout << "Mapa: " << map.WidthTiles() << "x" << map.HeightTiles()
              << " tiles, mundo de " << map.WorldWidth() << "x"
              << map.WorldHeight() << " px\n";

    // IsSolidCell: exactitudes
    std::cout << "Celda (0,19): " << (map.IsSolidCell(0, 19) ? "SÓLIDA" : "aire") << "\n";
    std::cout << "Celda (5,14): " << (map.IsSolidCell(5, 14) ? "SÓLIDA" : "aire") << "\n";
    std::cout << "Celda (5, 5): " << (map.IsSolidCell(5, 5) ? "SÓLIDA" : "aire") << "\n";

    // Actor cayendo al piso (tile 2 en fila 18): necesita ~1.2s a g=800
    Actor player;
    player.position = {3.0f * map.TileW(), 0.0f};
    std::cout << "\nCae 90 frames a dt=1/60 con gravedad 800:\n";
    for (int f = 0; f < 90; ++f) {
        player.velocity.y += 800.0f * (1.0f/60.0f);
        float dy = player.velocity.y * (1.0f/60.0f);
        MoveAndSlideY(player, dy, map);
    }
    std::cout << "Y tras caer: " << player.position.y
              << " (piso: " << (18 * map.TileH() - player.height) << ")\n";
    std::cout << "Llegó EXACTO al tile ante el snap: "
              << (player.position.y == 18.0f * map.TileH() - player.height ? "OK" : "BUG") << "\n";

    // Slide diagonal contra el muro izquierdo
    Actor slider;
    slider.position = {64.0f, 300.0f};
    for (int f = 0; f < 30; ++f) {
        MoveAndSlideX(slider, -8.0f, map);   // vuela hacia la izquierda
        MoveAndSlideY(slider, 0.0f, map);    // sin dy en este frame
    }
    std::cout << "Tras 30 frames contra muro: X = " << slider.position.x
              << " (borde del muro: 32)\n";

    // Culling: cámara en el centro del mapa vs fuera del mapa
    Camera2D cam;
    cam.target = {map.WorldWidth()*0.5f, map.WorldHeight()*0.5f};
    cam.offset = {320, 160};    // medio viewport de 640x320
    cam.zoom = 1.0f;
    std::cout << "\nCámara al centro: ve " << map.CountVisibleTiles(cam, 640, 320)
              << " de " << map.TotalTiles() << " tiles ("
              << (map.CountVisibleTiles(cam, 640, 320) * 100.0f) / map.TotalTiles()
              << "%)\n";
    cam.zoom = 0.5f;   // zoom out: ve el doble de área
    std::cout << "Zoom 0.5: ve " << map.CountVisibleTiles(cam, 640, 320) << " tiles\n";
    cam.zoom = 2.0f;   // zoom in: ve menos
    std::cout << "Zoom 2.0: ve " << map.CountVisibleTiles(cam, 640, 320) << " tiles\n";

    // Cámara fuera del mapa: rango vacío
    cam.target = {-5000.f, -5000.f};
    cam.zoom = 1.0f;
    std::cout << "Cámara fuera del mapa: ve "
	      << map.CountVisibleTiles(cam, 640, 320) << " tiles\n";

    std::cout << "\n=== FIN LECCIÓN 10 ===\n";
}

int main() {
    demo_tile_collision_and_culling();
    return 0;
}
