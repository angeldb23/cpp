/* =============================================================================
   MÓDULO 23 - LECCIÓN 05: Círculo vs polígono, Minkowski y GJK
   =============================================================================
   TEMA 3 de TEMAS.md (segunda lección): circle-poly, Minkowski difference y
   una introducción a GJK.

   CÍRCULO VS POLÍGONO CONVEXO
   ---------------------------
   Es más simple que SAT porque un círculo no tiene aristas: se busca el punto
   del polígono más cercano al centro.
     - Si el centro está afuera: distancia = |centro - punto|. Hay colisión si
       distancia <= radio. La normal va del polígono al círculo y la
       penetración es radio - distancia.
     - Si el centro está adentro: el punto más cercano está en una arista; la
       penetración es radio + distancia a esa arista y la normal apunta hacia
       afuera (del centro al punto de la arista).
   Para polígonos convexos alcanza con recorrer los segmentos y quedarse con
   el mínimo. Para no convexos hay que descomponer o usar un BVH.

   DIFERENCIA DE MINKOWSKI
   -----------------------
   A ⊖ B = { a - b : a ∈ A, b ∈ B }. Dos cuerpos colisionan si el origen
   (0,0) pertenece a A ⊖ B. Para un círculo y un polígono, la diferencia de
   Minkowski es el polígono "inflado" por el radio: eso explica por qué el
   test de distancia al punto más cercano funciona.

   GJK (GILBERT-JOHNSON-KEERSTI)
   -----------------------------
   En vez de construir la diferencia de Minkowski entera, GJK usa solo una
   función de soporte:
       soporte(A, dirección) = el vértice de A más lejano en esa dirección.
   Con tres puntos (un simplex) va preguntando si el origen quedó adentro y
   en qué dirección seguir. Funciona para cualquier convexo, incluidos los
   curvas. GJK dice SI hay colisión; para la penetración hace falta EPA
   (Expanding Polytope Algorithm), que es otro tema.
   Para la mayoría de los juegos 2D, SAT + circle-poly alcanza y es más
   fácil de depurar.
   ============================================================================= */
#include "include/fisica2d.hpp"

#include <iomanip>
#include <iostream>
#include <limits>
#include <vector>

using f2d::Vec2;

namespace {

struct Poligono {
    std::vector<Vec2> vertices;
};

struct Circulo {
    Vec2 centro;
    float radio = 0.0f;
};

struct Contacto {
    bool hay = false;
    Vec2 normal;       // del polígono hacia el círculo (dirección de empuje)
    float profundidad = 0.0f;
    Vec2 punto;        // punto más cercano del polígono
};

Vec2 puntoMasCercanoEnSegmento(const Vec2& p, const Vec2& a, const Vec2& b) {
    const Vec2 ab = b - a;
    const float den = f2d::lengthSq(ab);
    if (den <= f2d::kEps) return a;
    const float t = f2d::clampf(f2d::dot(p - a, ab) / den, 0.0f, 1.0f);
    return a + ab * t;
}

bool puntoDentroDeConvexo(const Poligono& p, const Vec2& q) {
    bool positivo = false;
    bool negativo = false;
    for (size_t i = 0; i < p.vertices.size(); ++i) {
        const Vec2& a = p.vertices[i];
        const Vec2& b = p.vertices[(i + 1) % p.vertices.size()];
        const float c = f2d::cross(b - a, q - a);
        if (c > f2d::kEps) positivo = true;
        if (c < -f2d::kEps) negativo = true;
        if (positivo && negativo) return false;
    }
    return true;
}

Contacto circuloVsPoligono(const Circulo& c, const Poligono& p) {
    if (p.vertices.size() < 3) return {};

    Vec2 mejorPunto = p.vertices[0];
    float mejorDist2 = std::numeric_limits<float>::max();
    for (size_t i = 0; i < p.vertices.size(); ++i) {
        const Vec2 q = puntoMasCercanoEnSegmento(c.centro, p.vertices[i],
                                                 p.vertices[(i + 1) % p.vertices.size()]);
        const float d2 = f2d::lengthSq(c.centro - q);
        if (d2 < mejorDist2) {
            mejorDist2 = d2;
            mejorPunto = q;
        }
    }

    const bool dentro = puntoDentroDeConvexo(p, c.centro);
    const float dist = std::sqrt(mejorDist2);

    if (dentro) {
        // normal hacia afuera: del centro al borde más cercano
        const Vec2 n = f2d::normalize(mejorPunto - c.centro);
        return {true, n, c.radio + dist, mejorPunto};
    }
    if (dist > c.radio) return {};
    const Vec2 n = f2d::normalize(c.centro - mejorPunto);
    return {true, n, c.radio - dist, mejorPunto};
}

// --- GJK conceptual: función de soporte ------------------------------------
Vec2 soporte(const Poligono& p, const Vec2& direccion) {
    Vec2 mejor = p.vertices[0];
    float mejorDot = f2d::dot(mejor, direccion);
    for (const auto& v : p.vertices) {
        const float d = f2d::dot(v, direccion);
        if (d > mejorDot) {
            mejorDot = d;
            mejor = v;
        }
    }
    return mejor;
}

Poligono caja(float hx, float hy) {
    return {{{-hx, -hy}, {hx, -hy}, {hx, hy}, {-hx, hy}}};
}

void imprimir(const char* nombre, const Contacto& c) {
    std::cout << nombre << ": ";
    if (!c.hay) {
        std::cout << "sin contacto\n";
        return;
    }
    std::cout << "normal=(" << c.normal.x << ", " << c.normal.y
              << "), profundidad=" << c.profundidad
              << ", punto=(" << c.punto.x << ", " << c.punto.y << ")\n";
}

} // namespace

int main() {
    std::cout << std::fixed << std::setprecision(4);
    const Poligono cuadrado = caja(1.0f, 1.0f);

    imprimir("Círculo a la derecha (r=0.5, x=1.2)", circuloVsPoligono({{1.2f, 0.0f}, 0.5f}, cuadrado));
    imprimir("Círculo en el centro (r=0.5)", circuloVsPoligono({{0.0f, 0.0f}, 0.5f}, cuadrado));
    imprimir("Círculo lejos (r=0.5, x=3)", circuloVsPoligono({{3.0f, 0.0f}, 0.5f}, cuadrado));
    imprimir("Círculo en la esquina (1.3, 1.3)", circuloVsPoligono({{1.3f, 1.3f}, 0.5f}, cuadrado));
    imprimir("Círculo justo tocando (x=1.5)", circuloVsPoligono({{1.5f, 0.0f}, 0.5f}, cuadrado));

    std::cout << "\nSoporte del cuadrado en (1, 1): ("
              << soporte(cuadrado, {1.0f, 1.0f}).x << ", "
              << soporte(cuadrado, {1.0f, 1.0f}).y << ")\n";
    std::cout << "GJK usaría 3 de esos puntos para decidir si el origen está\n"
                 "en la diferencia de Minkowski. Nosotros nos quedamos con el\n"
                 "test directo círculo-polígono, que es exacto y barato.\n";
    return 0;
}
