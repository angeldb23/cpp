/* =============================================================================
   MÓDULO 23 - LECCIÓN 03: Sweep-and-Prune y quadtree
   =============================================================================
   TEMA 2 de TEMAS.md (segunda lección): otras dos broad-phases clásicas.

   SWEEP-AND-PRUNE (SAP)
   ---------------------
   La observación: si ordenás las cajas por su borde izquierdo (minX), dos
   cajas solo pueden solaparse si sus intervalos en X se cruzan. Se barre el
   eje con una "lista activa":

       ordenar por minX
       para cada intervalo i:
           sacar de la lista activa los que terminan antes de i.minX
           todos los que quedan en la lista activa son candidatos
           insertar i en la lista activa

   En 1D es O(N log N) por el sort + O(K) por los pares. En 2D se hace SAP
   en un eje (suele ser X) y se descarta por Y en el test de AABB. Es muy
   bueno cuando los objetos se mueven poco entre frames: se puede reutilizar
   el orden anterior (insertion sort casi ordenado).

   QUADTREE
   --------
   Se parte el espacio en 4 cuadrantes recursivamente hasta que cada hoja
   tiene pocos objetos o se llega a una profundidad máxima. Consultar un área
   solo baja por las ramas que la intersectan.
   - Bueno para densidad variable (muchos objetos chicos en una zona).
   - Malo si los objetos son enormes: un cuerpo que cruza los 4 cuadrantes
     se guarda en la raíz y se testea siempre.
   - Cuidado con la profundidad: sin tope, dos objetos casi idénticos se
     subdividen hasta el infinito.

   Este archivo implementa las dos y compara contra fuerza bruta.
   ============================================================================= */
#include "include/fisica2d.hpp"

#include <array>
#include <iomanip>
#include <iostream>
#include <memory>
#include <vector>

using f2d::AABB;
using f2d::Pair;

namespace {

struct Intervalo {
    int id;
    float minX;
    float maxX;
};

std::vector<Pair> sweepAndPrune(std::vector<Intervalo> intervalos) {
    std::sort(intervalos.begin(), intervalos.end(),
              [](const Intervalo& a, const Intervalo& b) {
                  if (a.minX != b.minX) return a.minX < b.minX;
                  return a.id < b.id;
              });

    std::vector<int> activos;  // IDs ordenados por minX
    std::vector<Pair> pares;
    for (const auto& actual : intervalos) {
        // Se retiran los que ya terminaron ANTES del inicio actual.
        // Si maxX == actual.minX se tocan: se quedan (borde inclusivo).
        activos.erase(
            std::remove_if(activos.begin(), activos.end(),
                           [&](int id) {
                               for (const auto& it : intervalos) {
                                   if (it.id == id) return it.maxX < actual.minX;
                               }
                               return true;
                           }),
            activos.end());

        for (int id : activos) {
            pares.push_back({std::min(id, actual.id), std::max(id, actual.id)});
        }
        activos.push_back(actual.id);
    }
    std::sort(pares.begin(), pares.end(),
              [](const Pair& a, const Pair& b) { return a < b; });
    return pares;
}

// ---------------------------------------------------------------------------
// Quadtree mínimo: nodos con hasta `capacidad` IDs; si se pasa, se subdivide.
// ---------------------------------------------------------------------------
struct Nodo {
    AABB area;
    std::vector<int> ids;
    std::array<std::unique_ptr<Nodo>, 4> hijos{};
    bool esHoja() const { return hijos[0] == nullptr; }
};

class Quadtree {
public:
    Quadtree(AABB area, int capacidad = 4, int maxDepth = 8)
        : raiz_(std::make_unique<Nodo>(Nodo{area, {}, {}})),
          capacidad_(capacidad),
          maxDepth_(maxDepth) {}

    void insertar(int id, const AABB& caja) {
        if (id >= static_cast<int>(cajas_.size())) cajas_.resize(id + 1);
        cajas_[id] = caja;
        insertarRec(raiz_.get(), 0, id, caja);
    }

    std::vector<int> consultar(const AABB& area) const {
        std::vector<int> out;
        consultarRec(raiz_.get(), area, out);
        std::sort(out.begin(), out.end());
        out.erase(std::unique(out.begin(), out.end()), out.end());
        return out;
    }

    const AABB& caja(int id) const { return cajas_[id]; }

private:
    void insertarRec(Nodo* nodo, int depth, int id, const AABB& caja) {
        if (!nodo->area.overlaps(caja)) return;
        if (nodo->esHoja()) {
            nodo->ids.push_back(id);
            if (static_cast<int>(nodo->ids.size()) > capacidad_ &&
                depth < maxDepth_) {
                subdividir(nodo, depth);
            }
            return;
        }
        for (auto& h : nodo->hijos) {
            if (h) insertarRec(h.get(), depth + 1, id, caja);
        }
    }

    void subdividir(Nodo* nodo, int depth) {
        const float midX = (nodo->area.minX + nodo->area.maxX) * 0.5f;
        const float midY = (nodo->area.minY + nodo->area.maxY) * 0.5f;
        nodo->hijos[0] = std::make_unique<Nodo>(Nodo{{nodo->area.minX, nodo->area.minY, midX, midY}, {}, {}});
        nodo->hijos[1] = std::make_unique<Nodo>(Nodo{{midX, nodo->area.minY, nodo->area.maxX, midY}, {}, {}});
        nodo->hijos[2] = std::make_unique<Nodo>(Nodo{{nodo->area.minX, midY, midX, nodo->area.maxY}, {}, {}});
        nodo->hijos[3] = std::make_unique<Nodo>(Nodo{{midX, midY, nodo->area.maxX, nodo->area.maxY}, {}, {}});

        const std::vector<int> viejos = std::move(nodo->ids);
        nodo->ids.clear();
        for (int id : viejos) {
            for (auto& h : nodo->hijos) {
                if (h) insertarRec(h.get(), depth + 1, id, caja(id));
            }
        }
    }

    void consultarRec(const Nodo* nodo, const AABB& area,
                      std::vector<int>& out) const {
        if (!nodo->area.overlaps(area)) return;
        for (int id : nodo->ids) {
            if (caja(id).overlaps(area)) out.push_back(id);
        }
        for (const auto& h : nodo->hijos) {
            if (h) consultarRec(h.get(), area, out);
        }
    }

    std::unique_ptr<Nodo> raiz_;
    std::vector<AABB> cajas_;
    int capacidad_;
    int maxDepth_;
};

} // namespace

int main() {
    // --- Sweep and prune 1D -------------------------------------------------
    const std::vector<Intervalo> intervalos = {
        {0, 0.0f, 2.0f},
        {1, 1.0f, 3.0f},
        {2, 4.0f, 5.0f},
        {3, 2.5f, 6.0f},
        {4, 5.0f, 7.0f},
    };
    const auto pares = sweepAndPrune(intervalos);
    std::cout << "Sweep-and-prune: " << pares.size() << " pares candidatos\n";
    for (const auto& p : pares) {
        std::cout << "  (" << p.a << ", " << p.b << ")\n";
    }

    int brutos = 0;
    for (size_t i = 0; i < intervalos.size(); ++i) {
        for (size_t j = i + 1; j < intervalos.size(); ++j) {
            if (intervalos[i].minX <= intervalos[j].maxX &&
                intervalos[j].minX <= intervalos[i].maxX) {
                ++brutos;
            }
        }
    }
    std::cout << "Fuerza bruta en 1D: " << brutos << " pares\n\n";

    // --- Quadtree -----------------------------------------------------------
    Quadtree qt(AABB(-100.0f, -100.0f, 100.0f, 100.0f), 2, 6);
    for (int i = 0; i < 40; ++i) {
        const float x = -90.0f + static_cast<float>((i * 37) % 180);
        const float y = -90.0f + static_cast<float>((i * 53) % 180);
        const AABB caja(x, y, x + 6.0f, y + 6.0f);
        qt.insertar(i, caja);
    }
    const AABB zona(0.0f, 0.0f, 40.0f, 40.0f);
    const auto enZona = qt.consultar(zona);
    std::cout << "Quadtree: " << enZona.size()
              << " cajas tocan la zona (0,0)-(40,40)\n";

    int enZonaBruto = 0;
    for (int i = 0; i < 40; ++i) {
        if (qt.caja(i).overlaps(zona)) ++enZonaBruto;
    }
    std::cout << "Fuerza bruta en la misma zona: " << enZonaBruto << "\n";
    std::cout << "Coinciden: " << (static_cast<int>(enZona.size()) == enZonaBruto ? "sí" : "NO") << "\n\n";
    std::cout << "SAP brilla con movimiento pequeño; el quadtree, con densidad\n"
                 "variable. Ambos son solo candidatos: después va SAT/impulsos.\n";
    return 0;
}
