/* =============================================================================
   MÓDULO 23 - LECCIÓN 04: Narrow-phase con SAT (polígonos convexos)
   =============================================================================
   TEMA 3 de TEMAS.md: "Narrow-phase: SAT para polígonos convexos, circle-poly,
   Minkowski difference y GJK intro" (primera de dos lecciones).

   SEPARATING AXIS THEOREM
   -----------------------
   Dos convexos NO se intersectan si existe al menos un eje donde sus
   proyecciones no se solapan. Para polígonos, los únicos ejes que hay que
   probar son las normales de las aristas de AMBOS polígonos.

   Algoritmo:
       1. Para cada arista de A y de B:
            eje = normalizar(perpendicular(arista))
            proyectar todos los vértices de A y de B sobre el eje
            si las proyecciones no se solapan -> NO hay colisión
       2. Si en todos los ejes hay solapamiento -> HAY colisión.
          La profundidad es el MENOR solapamiento y la normal es ese eje.

   Ese "menor solapamiento" es el MTV (Minimum Translation Vector): la
   corrección más corta para separar los cuerpos. Ojo: SAT da la normal y la
   profundidad, pero NO los puntos de contacto; para eso hace falta clipping
   (o usar un manifold de Box2D).

   PROYECCIÓN
   ----------
   proyectar(v) = dot(v, eje). El intervalo de un polígono es
   [min(dot), max(dot)]. El solapamiento es
       min(maxA, maxB) - max(minA, minB)
   Si es <= 0, hay eje separador.

   ORIENTACIÓN DE LA NORMAL
   ------------------------
   SAT devuelve un eje, pero "+eje" y "-eje" son el mismo eje. Para que la
   normal apunte de A hacia B (y el impulso empuje en el sentido correcto) se
   elige el signo tal que dot(normal, centroB - centroA) > 0.
   ============================================================================= */
#include "include/fisica2d.hpp"

#include <iomanip>
#include <iostream>
#include <vector>

using f2d::Vec2;

namespace {

struct Poligono {
    std::vector<Vec2> vertices;
};

Vec2 centro(const Poligono& p) {
    Vec2 c;
    for (const auto& v : p.vertices) c += v;
    if (!p.vertices.empty()) c = c / static_cast<float>(p.vertices.size());
    return c;
}

struct Proyeccion {
    float min = 0.0f;
    float max = 0.0f;
};

Proyeccion proyectar(const Poligono& p, const Vec2& eje) {
    Proyeccion r{dot(p.vertices[0], eje), dot(p.vertices[0], eje)};
    for (const auto& v : p.vertices) {
        const float d = dot(v, eje);
        r.min = std::min(r.min, d);
        r.max = std::max(r.max, d);
    }
    return r;
}

struct ResultadoSAT {
    bool colision = false;
    Vec2 normal;
    float profundidad = 0.0f;
};

ResultadoSAT sat(const Poligono& a, const Poligono& b) {
    if (a.vertices.size() < 3 || b.vertices.size() < 3) return {};

    float menorSolape = 1e30f;
    Vec2 mejorEje;

    auto probarEjes = [&](const Poligono& p) {
        for (size_t i = 0; i < p.vertices.size(); ++i) {
            const Vec2& v0 = p.vertices[i];
            const Vec2& v1 = p.vertices[(i + 1) % p.vertices.size()];
            const Vec2 eje = f2d::normalize(f2d::perp(v1 - v0));
            if (f2d::lengthSq(eje) < f2d::kEps) continue;

            const auto pa = proyectar(a, eje);
            const auto pb = proyectar(b, eje);
            const float solape = std::min(pa.max, pb.max) - std::max(pa.min, pb.min);
            if (solape <= 0.0f) return false;  // eje separador: no colisionan
            if (solape < menorSolape) {
                menorSolape = solape;
                mejorEje = eje;
            }
        }
        return true;
    };

    if (!probarEjes(a)) return {};
    if (!probarEjes(b)) return {};

    // Orientar de A hacia B.
    if (dot(mejorEje, centro(b) - centro(a)) < 0.0f) mejorEje = -mejorEje;

    return {true, mejorEje, menorSolape};
}

Poligono caja(float hx, float hy, Vec2 centro = {}) {
    return {{{centro.x - hx, centro.y - hy},
             {centro.x + hx, centro.y - hy},
             {centro.x + hx, centro.y + hy},
             {centro.x - hx, centro.y + hy}}};
}

void imprimir(const char* nombre, const ResultadoSAT& r) {
    std::cout << nombre << ": ";
    if (!r.colision) {
        std::cout << "sin colisión\n";
        return;
    }
    std::cout << "colisión, normal=(" << r.normal.x << ", " << r.normal.y
              << "), profundidad=" << r.profundidad << "\n";
}

} // namespace

int main() {
    std::cout << std::fixed << std::setprecision(4);

    imprimir("Cajas solapadas 0.5 m", sat(caja(1.0f, 1.0f), caja(1.0f, 1.0f, {1.5f, 0.0f})));
    imprimir("Cajas separadas", sat(caja(1.0f, 1.0f), caja(1.0f, 1.0f, {4.0f, 0.0f})));
    imprimir("Cajas que se tocan", sat(caja(1.0f, 1.0f), caja(1.0f, 1.0f, {2.0f, 0.0f})));

    // Un rombo (cuadrado rotado 45°) contra una caja: el eje mínimo NO es
    // horizontal ni vertical, es la diagonal. Eso es lo que AABB no ve.
    const Poligono rombo = {{{1.2f, -1.0f}, {2.2f, 0.0f}, {1.2f, 1.0f}, {0.2f, 0.0f}}};
    imprimir("Caja vs rombo", sat(caja(1.0f, 1.0f), rombo));

    std::cout << "\nLo importante: la profundidad de SAT es el MTV. Con ella se\n"
                 "corrige la posición (Baumgarte) y con la normal se aplica el\n"
                 "impulso (lección 06). SAT no calcula puntos de contacto.\n";
    return 0;
}
