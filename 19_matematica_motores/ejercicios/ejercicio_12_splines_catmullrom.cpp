/* =============================================================================
   MÓDULO 19 - EJERCICIO 12: Splines Catmull-Rom y longitud de arco
   =============================================================================
   DIFICULTAD: Avanzado
   APOYO: Lección 15 (Hermite con tangentes de los vecinos, el clonado de
   extremos y la tabla de distancias para moverse a VELOCIDAD CONSTANTE).
   El lerp/easing puro es la lección 14: NO lo repetimos acá — la mezcla
   por componente ya ES un lerp, y acá lo usás con el t de la curva.

   CONSIGNA:

     Vec3 puntoCatmullRom(Vec3 p1, Vec3 p2, Vec3 m0, Vec3 m1, float t)
        -> la cúbica de Hermite:
           h00 =  2t³ - 3t² + 1     h10 = t³ - 2t² + t
           h01 = -2t³ + 3t²         h11 = t³ - t²
           q(t) = h00*p1 + h10*m0 + h01*p2 + h11*m1   con t en [0..1]

     Vec3 evaluarCamino(std::vector<Vec3> const& q, float s)
        -> s en "unidades de segmento" (s=1.5 => 2do segmento, mitad):
           clamp de s a [0, N-1]; i = (int)s y forzá i <= N-2;
           t = s - i (la FRACCIÓN dentro del segmento)
           vecinos (SIEMPRE en rango): p0 = q[max(i-1, 0)]
                                       p3 = q[min(i+2, N-1)]
           m0 = (q[i+1] - p0) * 0.5f;   m1 = (p3 - q[i]) * 0.5f
           devolvé puntoCatmullRom(q[i], q[i+1], m0, m1, t)

     std::vector<float> tablaDeArco(std::vector<Vec3> const& q,
                                    int muestrasPorSegmento)
        -> distancias ACUMULADAS: arrancá con {0.0f} y por cada segmento
           muestreá con K pasos: acum.push_back(acum.back() +
           largo(puntoActual - puntoAnterior)).
           (el tamaño de la tabla = K*(N-1) + 1 y la ÚLTIMA entrada es
            la LONGITUD TOTAL del camino)

     Vec3 puntoADistancia(std::vector<Vec3> const& q,
                          std::vector<float> const& acum,
                          int muestrasPorSegmento, float distancia)
        -> "dame el punto del camino que está a TANTOS metros":
           1) RUTAS CÍCLICAS: distancia = fmod(distancia, total)
              (si total == 0, devolvé q[0])
           2) BUSQUEDA BINARIA del intervalo: hallá el j con
              acum[j] <= distancia < acum[j+1]
           3) fracción intra-intervalo: frac = (d - acum[j])/(acum[j+1]-acum[j])
           4) sParam = (j + frac) / muestrasPorSegmento  y llamá
              evaluarCamino(q, sParam)
           (la búsqueda binaria: lo = 0, hi = last; mientras lo+1 < hi,
            mid = (lo+hi)/2; si acum[mid] <= d: lo = mid; sino hi = mid)

   TESTS (no tocar): el MISMO zig-zag de la lección 15 (4 waypoints:
   (0,0) (4,0) (8,3) (12,0)); los valores de la tabla son de referencia.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_12_... -o ej12 && ./ej12
   ============================================================================= */
#include <cmath>
#include <vector>
#include "../../include/test_framework.hpp"
#include "../include/mate3d.hpp"

using namespace mate;

// Ayudas DADA (del ejercicio 01 / lección 03):
static float productoEscalar(Vec3 a, Vec3 b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}
static float largoDe(Vec3 a) {
    return std::sqrt(productoEscalar(a, a));
}
// DADA: el camino de la lección 15
static std::vector<Vec3> caminoDeLeccion() {
    return { Vec3{ 0.0f, 0.0f, 0.0f},
             Vec3{ 4.0f, 0.0f, 0.0f},
             Vec3{ 8.0f, 3.0f, 0.0f},
             Vec3{12.0f, 0.0f, 0.0f} };
}

// ===== TU CÓDIGO (mirá la LECCIÓN 15) =====
Vec3 puntoCatmullRom(Vec3 p1, Vec3 p2, Vec3 m0, Vec3 m1, float t) {
    (void)p1; (void)p2; (void)m0; (void)m1; (void)t;
    return {0.0f, 0.0f, 0.0f};   // TODO 1: las 4 funciones de Hermite (basis)
}

Vec3 evaluarCamino(std::vector<Vec3> const& q, float s) {
    (void)q; (void)s;
    return {0.0f, 0.0f, 0.0f};   // TODO 2: el segmento i, vecinos clonados,
                                 //        tangentes de la mitad y TODO 1
}

std::vector<float> tablaDeArco(std::vector<Vec3> const& q,
                               int muestrasPorSegmento) {
    (void)q; (void)muestrasPorSegmento;
    return {0.0f};               // TODO 3: distancias acumuladas
}

Vec3 puntoADistancia(std::vector<Vec3> const& q,
                     std::vector<float> const& acum,
                     int muestrasPorSegmento, float distancia) {
    (void)q; (void)acum; (void)muestrasPorSegmento; (void)distancia;
    return {0.0f, 0.0f, 0.0f};   // TODO 4: wrap + BINARIA + fracción + TODO 2
}

// ============================= TESTS (no tocar) =============================
TEST_CASE(WaypointsExactos) {
    auto c = caminoDeLeccion();
    // en s ENTERO los waypoints se tocan EXACTO (CR pasa POR los puntos):
    ASSERT_TRUE(casiIgualVec3(evaluarCamino(c, 1.0f), Vec3{4.0f, 0.0f, 0.0f}, 1e-3f));
    ASSERT_TRUE(casiIgualVec3(evaluarCamino(c, 2.0f), Vec3{8.0f, 3.0f, 0.0f}, 1e-3f));
}

TEST_CASE(MitadDelPrimerSegmento) {
    auto c = caminoDeLeccion();
    // valor de la lección 15: la curva "se aparta" ya desde el arranque
    // (la tangente en (4,0) apunta hacia (8,3)):
    Vec3 m = evaluarCamino(c, 0.5f);
    ASSERT_TRUE(casiIgual(m.x, 1.75f, 1e-3f));
    ASSERT_TRUE(casiIgual(m.y, -0.1875f, 1e-3f));
}

TEST_CASE(ClampElLimite) {
    auto c = caminoDeLeccion();
    ASSERT_TRUE(casiIgualVec3(evaluarCamino(c, -3.0f), Vec3{0.0f, 0.0f, 0.0f}, 1e-4f));
    ASSERT_TRUE(casiIgualVec3(evaluarCamino(c, 9.0f),  Vec3{12.0f, 0.0f, 0.0f}, 1e-4f));
}

TEST_CASE(TablaYLongitudTotal) {
    auto c = caminoDeLeccion();
    std::vector<float> acum = tablaDeArco(c, 10);
    ASSERT_EQ((int)acum.size(), 31);            // 3 segmentos * 10 + la 0
    ASSERT_TRUE(casiIgual(acum.front(), 0.0f, 1e-6f));
    ASSERT_TRUE(casiIgual(acum.back(), 14.2008f, 0.05f));   // el TOTAL
}

TEST_CASE(PuntoAVelocidadConstante) {
    auto c = caminoDeLeccion();
    std::vector<float> acum = tablaDeArco(c, 10);
    // a 0.5 unidades del punto de partida:
    Vec3 a = puntoADistancia(c, acum, 10, 0.5f);
    ASSERT_TRUE(casiIgual(a.x, 0.4941f, 0.01f));
    ASSERT_TRUE(casiIgual(a.y, -0.0418f, 0.01f));
    // a 13 unidades de arco (ya en el 3er segmento, bajando el montón):
    Vec3 b = puntoADistancia(c, acum, 10, 13.0f);
    ASSERT_TRUE(casiIgual(b.x, 11.1349f, 0.02f));
    ASSERT_TRUE(casiIgual(b.y, 0.8273f, 0.02f));
}

TEST_CASE(WrapUniversalDeRutaCiclica) {
    // rutas que CICLAN (patrullando): el ENEMIGO recorre y recorre:
    auto c = caminoDeLeccion();
    std::vector<float> acum = tablaDeArco(c, 10);
    float total = acum.back();                  // ≈ 14.2
    // d = total + 11.8 es NOMÁS una vuelta extra al acumulador: el fmod lo
    // deja en 11.8 y el punto es EXACTO el mismo que pedí directo:
    Vec3 b = puntoADistancia(c, acum, 10, total + 11.8f);
    Vec3 ref = puntoADistancia(c, acum, 10, 11.8f);
    ASSERT_TRUE(casiIgualVec3(b, ref, 1e-3f));
    // dos vueltas y media (30 = 2 * total + 1.5876):
    Vec3 c2 = puntoADistancia(c, acum, 10, 30.0f);
    ASSERT_TRUE(casiIgual(c2.x, 1.5857f, 0.03f));
    ASSERT_TRUE(casiIgual(c2.y, -0.1727f, 0.03f));
}

int main() {
    std::cout << "EJERCICIO 12: splines Catmull-Rom con longitud de arco (lección 15)\n";
    int r = RUN_ALL_TESTS();
    if (r != 0) std::cout << "Sin resolver: los tests están fallando (OK).\n";
    return r;
}
