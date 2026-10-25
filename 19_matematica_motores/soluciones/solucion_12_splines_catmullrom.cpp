/* =============================================================================
   SOLUCIÓN 12: splines Catmull-Rom y longitud de arco (lección 15)
   =============================================================================
   - puntoCatmullRom: la cúbica de HERMITE con las funciones de base — CR
     es "Hermite con tangentes automáticas de los vecinos".
   - evaluarCamino: cada segmento i define sus tangentes con los vecinos
     (clonando extremos al llegar al borde): m0 = 0.5*(q[i+1] - q[i-1]),
     m1 = 0.5*(q[i+2] - q[i]).
   - tablaDeArco: muestreoUniforme del segmento y acumulado de lados;
     la ÚLTIMA entrada es la longitud TOTAL.
   - puntoADistancia: fmod para rutas cíclicas + BÚSQUEDA BINARIA del
     intervalo [acum[j], acum[j+1]) + fracción --> sParam --> evaluarCamino.
     Con s en unidades de arco la velocidad es CONSTANTE (el t de la curva
     "avanza más rápido" donde la curva está comprimida; la tabla lo corrige).
   ============================================================================= */
#include <cmath>
#include <vector>
#include "../../include/test_framework.hpp"
#include "../include/mate3d.hpp"

using namespace mate;

static float productoEscalar(Vec3 a, Vec3 b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}
static float largoDe(Vec3 a) {
    return std::sqrt(productoEscalar(a, a));
}

static std::vector<Vec3> caminoDeLeccion() {
    return { Vec3{ 0.0f, 0.0f, 0.0f},
             Vec3{ 4.0f, 0.0f, 0.0f},
             Vec3{ 8.0f, 3.0f, 0.0f},
             Vec3{12.0f, 0.0f, 0.0f} };
}

// ===== SOLUCIÓN =====
Vec3 puntoCatmullRom(Vec3 p1, Vec3 p2, Vec3 m0, Vec3 m1, float t) {
    float t2 = t * t, t3 = t2 * t;
    float h00 =  2.0f*t3 - 3.0f*t2 + 1.0f;
    float h10 =  t3 - 2.0f*t2 + t;
    float h01 = -2.0f*t3 + 3.0f*t2;
    float h11 =  t3 - t2;
    return p1*h00 + m0*h10 + p2*h01 + m1*h11;
}

Vec3 evaluarCamino(std::vector<Vec3> const& q, float s) {
    int N = (int)q.size();
    s = std::fmax(0.0f, std::fmin(s, (float)(N - 1)));
    int i = (int)s;
    if (i >= N - 1) i = N - 2;
    float t = s - (float)i;
    Vec3 a = q[std::max(i - 1, 0)];                 // vecino izq (clonado)
    Vec3 b = q[i];
    Vec3 e = q[i + 1];
    Vec3 f = q[std::min(i + 2, N - 1)];             // vecino der (clonado)
    Vec3 m0 = (e - a) * 0.5f;                       // tangente en b
    Vec3 m1 = (f - b) * 0.5f;                       // tangente en e
    return puntoCatmullRom(b, e, m0, m1, t);
}

std::vector<float> tablaDeArco(std::vector<Vec3> const& q,
                               int muestrasPorSegmento) {
    std::vector<float> acum;
    acum.push_back(0.0f);
    int N = (int)q.size();
    for (int i = 0; i < N - 1; ++i) {
        int im = std::max(i - 1, 0), ip = std::min(i + 2, N - 1);
        Vec3 a = q[im], b = q[i], e = q[i + 1], f = q[ip];
        Vec3 m0 = (e - a) * 0.5f, m1 = (f - b) * 0.5f;
        Vec3 anterior = b;
        for (int k = 1; k <= muestrasPorSegmento; ++k) {
            float t = (float)k / (float)muestrasPorSegmento;
            Vec3 punto = puntoCatmullRom(b, e, m0, m1, t);
            acum.push_back(acum.back() + largoDe(punto - anterior));
            anterior = punto;
        }
    }
    return acum;
}

Vec3 puntoADistancia(std::vector<Vec3> const& q,
                     std::vector<float> const& acum,
                     int muestrasPorSegmento, float distancia) {
    float total = acum.back();
    if (total <= 0.0f) return q.empty() ? Vec3{} : q[0];
    distancia = std::fmod(distancia, total);        // rutas cíclicas
    if (distancia < 0.0f) distancia += total;
    // BÚSQUEDA BINARIA: hallar j con acum[j] <= d < acum[j+1]:
    int lo = 0, hi = (int)acum.size() - 1;
    while (lo + 1 < hi) {
        int mid = (lo + hi) / 2;
        if (acum[mid] <= distancia) lo = mid; else hi = mid;
    }
    float fraccion = (distancia - acum[lo]) / (acum[hi] - acum[lo]);
    float sParam = ((float)lo + fraccion) / (float)muestrasPorSegmento;
    return evaluarCamino(q, sParam);
}

// ============================= TESTS (del ejercicio — sin cambios) ==========
TEST_CASE(WaypointsExactos) {
    auto c = caminoDeLeccion();
    ASSERT_TRUE(casiIgualVec3(evaluarCamino(c, 1.0f), Vec3{4.0f, 0.0f, 0.0f}, 1e-3f));
    ASSERT_TRUE(casiIgualVec3(evaluarCamino(c, 2.0f), Vec3{8.0f, 3.0f, 0.0f}, 1e-3f));
}

TEST_CASE(MitadDelPrimerSegmento) {
    auto c = caminoDeLeccion();
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
    ASSERT_EQ((int)acum.size(), 31);
    ASSERT_TRUE(casiIgual(acum.front(), 0.0f, 1e-6f));
    ASSERT_TRUE(casiIgual(acum.back(), 14.2008f, 0.05f));
}

TEST_CASE(PuntoAVelocidadConstante) {
    auto c = caminoDeLeccion();
    std::vector<float> acum = tablaDeArco(c, 10);
    Vec3 a = puntoADistancia(c, acum, 10, 0.5f);
    ASSERT_TRUE(casiIgual(a.x, 0.4941f, 0.01f));
    ASSERT_TRUE(casiIgual(a.y, -0.0418f, 0.01f));
    Vec3 b = puntoADistancia(c, acum, 10, 13.0f);
    ASSERT_TRUE(casiIgual(b.x, 11.1349f, 0.02f));
    ASSERT_TRUE(casiIgual(b.y, 0.8273f, 0.02f));
}

TEST_CASE(WrapUniversalDeRutaCiclica) {
    auto c = caminoDeLeccion();
    std::vector<float> acum = tablaDeArco(c, 10);
    float total = acum.back();                  // ≈ 14.2
    Vec3 b = puntoADistancia(c, acum, 10, total + 11.8f);
    Vec3 ref = puntoADistancia(c, acum, 10, 11.8f);
    ASSERT_TRUE(casiIgualVec3(b, ref, 1e-3f));
    Vec3 c2 = puntoADistancia(c, acum, 10, 30.0f);
    ASSERT_TRUE(casiIgual(c2.x, 1.5857f, 0.03f));
    ASSERT_TRUE(casiIgual(c2.y, -0.1727f, 0.03f));
}

int main() {
    std::cout << "SOLUCIÓN 12: Catmull-Rom + longitud de arco (todos DEBEN pasar)\n";
    int r = RUN_ALL_TESTS();
    if (r != 0) std::cout << "ERROR: la solución debe pasar todos los tests.\n";
    return r;
}
