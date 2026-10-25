/* =============================================================================
   MÓDULO 23 - LECCIÓN 02: Broad-phase con spatial hash (grid uniforme)
   =============================================================================
   TEMA 2 de TEMAS.md: "Colisión broad-phase: spatial hash (grid uniforme),
   sweep-and-prune y quadtree" (primera de dos lecciones).

   QUÉ ES LA BROAD-PHASE
   ---------------------
   Con N cuerpos, probar todos los pares cuesta N*(N-1)/2. Con 10.000 cuerpos
   son ~50 millones de tests por frame: imposible. La broad-phase descarta
   rápido los pares que NI SE ACERCAN, usando cajas alineadas a los ejes
   (AABB). La narrow-phase (SAT, círculo-polígono) solo recibe unos pocos
   candidatos.

   SPATIAL HASH / GRID UNIFORME
   ----------------------------
   - Se elige un tamaño de celda, típicamente ~2 veces el cuerpo promedio.
   - Cada AABB se inserta en TODAS las celdas que toca.
   - Dos cuerpos solo pueden chocar si comparten al menos una celda.
   - Se deduplican los pares (dos cajas pueden compartir 4 celdas y no hay
     que resolver la colisión 4 veces).

   VENTAJAS: O(1) por inserción y consulta, ideal para cuerpos de tamaño
   parecido y densidad uniforme.
   DESVENTAJAS: si hay un cuerpo gigante (un jefe, un tilemap entero), toca
   muchísimas celdas. Ahí conviene un quadtree o separar capas estáticas.

   COORDENADAS NEGATIVAS
   ---------------------
   `int(v/cell)` trunca hacia cero y rompe el grid: -0.5 y 0.5 caen en la
   celda 0. Hay que usar `floor(v/cell)`. Lo vas a necesitar en el ejercicio.

   Este archivo compara el grid contra fuerza bruta y verifica que devuelvan
   exactamente los mismos pares.
   ============================================================================= */
#include "include/fisica2d.hpp"

#include <iomanip>
#include <iostream>
#include <map>
#include <random>
#include <set>
#include <utility>
#include <vector>

using f2d::AABB;
using f2d::Pair;

namespace {

struct Cuerpo {
    int id = 0;
    AABB caja;
};

class SpatialHash {
public:
    explicit SpatialHash(float cellSize) : cellSize_(cellSize) {
        if (cellSize_ <= f2d::kEps) cellSize_ = 1.0f;
    }

    void clear() {
        celdas_.clear();
        cajas_.clear();
    }

    void insertar(int id, const AABB& caja) {
        cajas_.push_back({id, caja});
        const int x0 = f2d::floorDiv(caja.minX, cellSize_);
        const int x1 = f2d::floorDiv(caja.maxX, cellSize_);
        const int y0 = f2d::floorDiv(caja.minY, cellSize_);
        const int y1 = f2d::floorDiv(caja.maxY, cellSize_);
        for (int y = y0; y <= y1; ++y) {
            for (int x = x0; x <= x1; ++x) {
                celdas_[{x, y}].push_back(id);
            }
        }
    }

    std::vector<Pair> pares() const {
        std::set<Pair> unicos;
        for (const auto& [clave, ids] : celdas_) {
            (void)clave;
            for (size_t i = 0; i < ids.size(); ++i) {
                for (size_t j = i + 1; j < ids.size(); ++j) {
                    const Pair p{std::min(ids[i], ids[j]), std::max(ids[i], ids[j])};
                    if (!cajasSeTocan(p.a, p.b)) continue;
                    unicos.insert(p);
                }
            }
        }
        return {unicos.begin(), unicos.end()};
    }

    size_t celdas() const { return celdas_.size(); }

private:
    bool cajasSeTocan(int a, int b) const {
        for (const auto& c : cajas_) {
            if (c.id == a) {
                for (const auto& d : cajas_) {
                    if (d.id == b) return c.caja.overlaps(d.caja);
                }
                return false;
            }
        }
        return false;
    }

    float cellSize_;
    std::map<std::pair<int, int>, std::vector<int>> celdas_;
    std::vector<Cuerpo> cajas_;
};

std::vector<Pair> fuerzaBruta(const std::vector<Cuerpo>& cuerpos) {
    std::vector<Pair> out;
    for (size_t i = 0; i < cuerpos.size(); ++i) {
        for (size_t j = i + 1; j < cuerpos.size(); ++j) {
            if (cuerpos[i].caja.overlaps(cuerpos[j].caja)) {
                out.push_back({std::min(cuerpos[i].id, cuerpos[j].id),
                               std::max(cuerpos[i].id, cuerpos[j].id)});
            }
        }
    }
    std::sort(out.begin(), out.end(), [](const Pair& a, const Pair& b) {
        return a < b;
    });
    return out;
}

} // namespace

int main() {
    std::mt19937 rng(1234);  // semilla fija: salida reproducible
    std::uniform_real_distribution<float> pos(-40.0f, 40.0f);
    std::uniform_real_distribution<float> lado(1.0f, 4.0f);

    std::vector<Cuerpo> cuerpos;
    for (int i = 0; i < 300; ++i) {
        const float cx = pos(rng);
        const float cy = pos(rng);
        const float hx = lado(rng) * 0.5f;
        const float hy = lado(rng) * 0.5f;
        cuerpos.push_back({i, AABB(cx - hx, cy - hy, cx + hx, cy + hy)});
    }

    constexpr float cellSize = 5.0f;
    SpatialHash grid(cellSize);
    for (const auto& c : cuerpos) grid.insertar(c.id, c.caja);

    const auto paresGrid = grid.pares();
    const auto paresBrutos = fuerzaBruta(cuerpos);

    std::cout << "Cuerpos: " << cuerpos.size()
              << " | celdas ocupadas: " << grid.celdas() << "\n";
    std::cout << "Pares por fuerza bruta: " << paresBrutos.size() << "\n";
    std::cout << "Pares por spatial hash: " << paresGrid.size() << "\n";
    std::cout << "¿Coinciden? " << (paresGrid == paresBrutos ? "sí" : "NO")
              << "\n\n";

    std::cout << "Tests de AABB: fuerza bruta = N*(N-1)/2 = "
              << (cuerpos.size() * (cuerpos.size() - 1) / 2) << "\n";
    std::cout << "Con grid solo se testean los pares que comparten celda.\n";
    std::cout << "El grid NO reemplaza a la narrow-phase: solo descarta.\n\n";

    // El tamaño de celda importa: celdas de 1 m con cuerpos de 4 m hacen que
    // cada cuerpo toque 16+ celdas y el grid pierda sentido.
    SpatialHash gridFino(1.0f);
    for (const auto& c : cuerpos) gridFino.insertar(c.id, c.caja);
    std::cout << "Misma escena con celdas de 1 m: " << gridFino.celdas()
              << " celdas ocupadas (más insertions y más memoria).\n";
    std::cout << "Regla práctica: celda ~2x el tamaño típico de cuerpo.\n";
    return 0;
}
