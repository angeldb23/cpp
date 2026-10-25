/* =============================================================================
   MÓDULO 17 - LECCIÓN 07: Broad-Phase con Grid Uniforme (Spatial Hash)
   =============================================================================
   TEMA 6 de TEMAS.md (2da lección): Colisiones 2D — broad-phase con grid uniforme

   EL PROBLEMA:
   Con N objetos, testear TODOS contra TODOS es O(N²). Con 1.000 enemigos son
   499.500 pares POR FRAME. El 99,999% de esos pares están a kilómetros de
   distancia y nunca se van a tocar.

   LA IDEA (broad-phase):
   Dividir el mundo en una grilla de celdas y solo testear pares que comparten
   (o son vecinos de) una misma celda. Un objeto grande puede ocupar varias
   celdas: se registra en todas las que toca.

   Terminología del patrón, dividir en 2 fases:
   - BROAD phase: "¿parece posible que toquen?" -> respuestas baratas (grilla,
     sweep-and-prune, quadtree). Da un SUBCONJUNTO chico de pares candidatos.
   - NARROW phase: "¿ Realmente tocan, con qué penetración?" -> tests exactos
     de la lección 06 (AABB, círculo, closest point).

   COSTO ESPERADO:
   - Spatial hash: O(N) insertar + O(K) pares candidatos (K depende de la
     densidad local, NO del tamaño total del mundo) + query O(n.º de celdas tocadas).
   - Con celda del tamaño aprox de 2 objetos promedio y densidad pareja, los
     candidatos caen a ~2-5% de N².

   DETALLES DE IMPLEMENTACIÓN (las trampas que matan a los principiantes):
   1. Tamaño de grilla: celdas más chicas -> menos candidatos por celda pero
      objetos registrados en más celdas. Más grandes -> más candidatos.
      Regla práctica: cell = 2 x (tamaño promedio del objeto).
   2. La clave de celda (cx, cy) debe manejar coordenadas NEGATIVAS sin
      colapsar: si se empaca "(cx << 16) | cy" con enteros con signo, la celda
      (-1, 0) puede caer en el mismo valor que (15, 0). Solución: convertir
      cada coordenada a uint32_t ANTES del empacado (ver CellKey).
   3. Los pares candidatos se duplican: el mismo par aparece por cada celda
      que ambos comparten; hay que deduplicar. Truco: normalizar el par como
      (min, max) por orden de índice, o marcar pares vistos en un set.
   4. NO alocar memoria en el frame: en un motor real la grilla reutiliza
      vectores y el buffer de candidatos se limpia (clear) cada frame.

   NOTA: Código compila sin raylib (stubs). Compara N² vs grilla con counters.
   ============================================================================= */

#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <cmath>
#include <algorithm>
#include <cstdint>

// -----------------------------------------------------------------------------
// Stubs mínimos
// -----------------------------------------------------------------------------
struct Vector2 { float x, y; };
struct Rectangle { float x, y, width, height; };

inline float RectMinX(const Rectangle& r) { return r.x; }
inline float RectMaxX(const Rectangle& r) { return r.x + r.width; }
inline float RectMinY(const Rectangle& r) { return r.y; }
inline float RectMaxY(const Rectangle& r) { return r.y + r.height; }
inline bool RectsOverlap(const Rectangle& a, const Rectangle& b) {
    if (RectMaxX(a) < RectMinX(b) || RectMaxX(b) < RectMinX(a)) return false;
    if (RectMaxY(a) < RectMinY(b) || RectMaxY(b) < RectMinY(a)) return false;
    return true;
}
inline Vector2 GetRectCenter(const Rectangle& r) {
    return { r.x + r.width * 0.5f, r.y + r.height * 0.5f };
}

// -----------------------------------------------------------------------------
// 1. Spatial Hash Grid (mapa de celda -> índices de objetos)
// -----------------------------------------------------------------------------
class SpatialHashGrid {
    float cellSize_;
    // Celda (cx, cy) -> índices de entidades registradas ahí
    // La clave es int64 = (cy << 32) | (cx & 0xFFFFFFFF): packing de dos ints.
    // Esto es 1 lookup único por celda en un unordered_map (mucho mejor que
    // hacer pair<int,int> como key, aunque también funciona).
    std::unordered_map<uint64_t, std::vector<size_t>> cells_;

    static uint64_t CellKey(int cx, int cy) {
        // Convierte cada int a SIN SIGNO antes del shift: (uint32_t|-1) es
        // 4294967295, no 0xFFFFFFFF truncado con signo. Un solo uint64 compone
        // ambas coordenadas sin colisiones negativas vs positivas.
        return (static_cast<uint64_t>(static_cast<uint32_t>(cy)) << 32)
             |  static_cast<uint32_t>(cx);
    }

public:
    explicit SpatialHashGrid(float cellSize) : cellSize_(cellSize) {}
    float CellSize() const { return cellSize_; }

    // Índice de celda de una coordenada de mundo (floor para negativos)
    long CellOf(float worldX) const {
        return static_cast<long>(std::floor(worldX / cellSize_));
    }

    // Registra el AABB de la entidad index. TODAS las celdas que el AABB toca.
    void Insert(const Rectangle& aabb, size_t index) {
        const long x0 = CellOf(RectMinX(aabb));
        const long x1 = CellOf(RectMaxX(aabb) - 0.0001f);  // borde incluido
        const long y0 = CellOf(RectMinY(aabb));
        const long y1 = CellOf(RectMaxY(aabb) - 0.0001f);
        for (long cy = y0; cy <= y1; ++cy) {
            for (long cx = x0; cx <= x1; ++cx) {
                cells_[CellKey(cx, cy)].push_back(index);
            }
        }
    }

    void Clear() { cells_.clear(); }
    size_t CellsUsedCount() const { return cells_.size(); }
    size_t EntriesCount() const {
        size_t total = 0;
        for (const auto& [_, vec] : cells_) total += vec.size();
        return total;
    }

    // Celdas que toca un AABB: helper interno para queries y diagnóstico.
    struct CellRange { long x0, x1, y0, y1; };
    CellRange CellsOfRect(const Rectangle& aabb) const { 
        return { CellOf(RectMinX(aabb)), CellOf(RectMaxX(aabb) - 0.0001f),
                 CellOf(RectMinY(aabb)), CellOf(RectMaxY(aabb) - 0.0001f) };
    }

    // Query: devuelve los ÍNDICES de entidades (sin repetir) que coparticipan
    // alguna celda con el rect. BROAD phase pura: candidatos, no colisiones.
    void QueryIndices(const Rectangle& area, std::vector<size_t>& outIndices) const {
        CellRange range = CellsOfRect(area);
        std::unordered_set<size_t> seen;
        outIndices.clear();
        for (long cy = range.y0; cy <= range.y1; ++cy) {
            for (long cx = range.x0; cx <= range.x1; ++cx) {
                auto it = cells_.find(CellKey(cx, cy));
                if (it == cells_.end()) continue;
                for (size_t idx : it->second) {
                    if (seen.insert(idx).second) {   // DST when inserted
                        outIndices.push_back(idx);
                    }
                }
            }
        }
    }
};

// -----------------------------------------------------------------------------
// 2. Broad + Narrow: la tubería completa del frame
// -----------------------------------------------------------------------------
/* ORDEN REAL EN EL FRAME (según el game loop del módulo 16):
   1. Update de movimiento (integrar posiciones).
   2. Clear + Insert de todos los AABBs en la grilla (broad cache del frame).
   3. Por cada entidad, query de vecinos, dedup, y NARROW phase exacta.
   4. Resolución (MTV de la lección 06) + fixes de fuerza (piso, muros).

   La grilla se RECONSTRUYE cada frame porque las entidades se mueven.
   Con objetos casi estáticos se puede UPGRADE con "dirty flag" (sólo insertar
   los que CAMBIARON de celda); eso es un tema avanzado del módulo 23. */

// Formación de pares únicos: (min, max) por orden de índice para evitar
// el duplicado del mismo par visto desde varias celdas.
struct PairHash {
    // Packing de dos size_t en un solo hash con xor-mix estilo murmur3
    size_t operator()(const std::pair<size_t, size_t>& p) const {
        uint64_t x = (static_cast<uint64_t>(p.first) << 32) | static_cast<uint32_t>(p.second);
        x ^= x >> 33; x *= 0xff51afd7ed55819dULL;
        x ^= x >> 33; x *= 0xc4ceb9fe1a85ec53ULL;
        x ^= x >> 33;
        return static_cast<size_t>(x);
    }
};

void CollectCollisionPairs(const std::vector<Rectangle>& aabbs,
                           SpatialHashGrid& grid,
                           std::vector<std::pair<size_t, size_t>>& outPairs) {
    grid.Clear();
    for (size_t i = 0; i < aabbs.size(); ++i) grid.Insert(aabbs[i], i);

    outPairs.clear();
    std::unordered_set<std::pair<size_t, size_t>, PairHash> seenPairs;
    std::vector<size_t> candidates;
    for (size_t i = 0; i < aabbs.size(); ++i) {
        grid.QueryIndices(aabbs[i], candidates);
        for (size_t j : candidates) {
            if (j == i) continue;                 // self-collision off
            if (!RectsOverlap(aabbs[i], aabbs[j])) continue;  // narrow here
            size_t lo = std::min(i, j), hi = std::max(i, j);
            if (seenPairs.insert({lo, hi}).second) {
                outPairs.emplace_back(lo, hi);
            }
        }
    }
}

// -----------------------------------------------------------------------------
// 3. Demo: verificación numérica O(N²) vs grilla
// -----------------------------------------------------------------------------
std::vector<Rectangle> GenerateEntities(size_t n, unsigned seed) {
    // Generador LCG determinista (mismo dataset en toda la sesión)
    std::vector<Rectangle> out;
    out.reserve(n);
    uint32_t s = seed;
    auto nextRand = [&s]() {
        s = s * 1664525u + 1013904223u;
        return static_cast<float>(s % 1000) / 1000.0f;
    };
    for (size_t i = 0; i < n; ++i) {
        float x = nextRand() * 1000.0f;
        float y = nextRand() * 1000.0f;
        float w = 10.0f + nextRand() * 30.0f;   // 10-40 px
        float h = 10.0f + nextRand() * 30.0f;
        out.push_back(Rectangle{x, y, w, h});
    }
    return out;
}

/// Compare N² brute versus grid: num pares && propuestas.
void demo_broad_phase() {
    std::cout << "=== Broad-Phase con Grid Uniforme ===\n\n";

    const size_t N = 300;
    auto aabbs = GenerateEntities(N, 42);

    // O(N²): contar pares P
    long brutePairs = 0, bruteChecks = 0;
    for (size_t i = 0; i < N; ++i) {
        for (size_t j = i + 1; j < N; ++j) {
            ++bruteChecks;
            if (RectsOverlap(aabbs[i], aabbs[j])) ++brutePairs;
        }
    }
    std::cout << "O(N²): checks=" << bruteChecks << " pares=" << brutePairs << "\n";

    // Grilla: mismo resultado con menos CHEQUEOS de narrow phase.
    SpatialHashGrid grid(60.0f);   // ~2 veces el diámetro medio (~25px)
    std::vector<std::pair<size_t, size_t>> pairs;
    CollectCollisionPairs(aabbs, grid, pairs);
    std::cout << "Grilla: pares=" << pairs.size()
              << " celdas=" << grid.CellsUsedCount()
              << " entries=" << grid.EntriesCount() << "\n";

    // La GARANTÍA importante: la grilla es una optimización de la búsqueda;
    // los resultados deben ser IGUALES al algoritmo de fuerzenia bruta:
    std::cout << "Pares iguales: " << (pairs.size() == static_cast<size_t>(brutePairs)
                                        ? "SI (la grilla NO pierde colisiones)"
                                        : "NO (BUG)") << "\n";

    // Query: solo una zona pequeña del mundo (radio de interés)
    std::vector<size_t> near;
    Rectangle area{0, 0, 200, 200};
    grid.QueryIndices(area, near);
    std::cout << "Entidades en area (200x200): " << near.size()
              << " de " << N << " (~" << (near.size() * 100) / N << "% del total)\n";
    std::cout << "La zona son " << 200.0f / grid.CellSize() << " celdas de lado\n";
}

int main() {
    demo_broad_phase();
    return 0;
}
