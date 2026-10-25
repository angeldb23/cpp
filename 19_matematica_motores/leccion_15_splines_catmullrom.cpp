/* =============================================================================
   MÓDULO 19 - LECCIÓN 15: Curvas II — splines Catmull-Rom y longitud de arco
   =============================================================================
   TEMA 8 de TEMAS.md (2da lección): splines cúbicas (Catmull-Rom), evaluación
   y longitud de arco aproximada.

   ¿Por qué Catmull-Rom y no Bezier? Porque CR pasa POR los puntos que le
   das: en un camino de juego ("los enemigos recorren esta ruta") los puntos
   SON los waypoints que dibujaste en el editor. Con Bezier el camino casi
   nunca toca los puntos de control (y eso confunde al editor del módulo 24).

   == CATMULL-ROM = HERMITE CON TANGENTES AUTOMÁTICAS =========================

   El segmento entre p1 y p2 (con vecinos p0 y p3) usa las tangentes:

       m0 = 0.5 * (p2 - p0)      -- tangente en p1 (los VECINOS la definen)
       m1 = 0.5 * (p3 - p1)      -- tangente en p2

   y evalúa la cúbica de Hermite:

       q(t) = h00(t)*p1 + h10(t)*m0 + h01(t)*p2 + h11(t)*m1     con t en [0..1]

       h00 =  2t³ - 3t² + 1        h10 = t³ - 2t² + t
       h01 = -2t³ + 3t²            h11 = t³ - t²

   Propiedad clave (es lo que hace "fluir" el camino): la tangente en p_i es
   PARALELA a (p_{i+1} - p_{i-1}) — la curva entra y sale por la dirección
   que marcan los vecinos.

   == ARMAR EL CAMINO COMPLETO ===============================================

   Dados los waypoints q0..qN, el segmento i (de q_i a q_{i+1}) usa:

       p0 = q[max(i-1, 0)]      -- vecino izquierdo (si no hay, CLONA q0)
       p1 = q[i]
       p2 = q[i+1]
       p3 = q[min(i+2, N)]      -- vecino derecho (si no hay, CLONA qN)

   Clonar los extremos es lo estándar y barato: la curva de los primeros
   2 puntos queda casi una recta (y en un juego de ruta de 2 puntos eso
   ES lo que se quiere).

   == LONGITUD DE ARCO APROXIMADA (la parte importante del gameplay) ==========

   El parámetro t NO es proporcional a la distancia recorrida (la velocidad
   del punto depende de qué tan "estirada" esté la curva en ese t). Para
   seguir el camino a VELOCIDAD CONSTANTE se precomputa una tabla:

       1) Muestrear cada segmento con K pasos (p. ej. 10): posición
          q(t_j) y distancia entre muestras consecutivas.
       2) Acumular: distAja[j+1] = distAja[j] + largo(q_{j+1} - q_j)
          (terminando en la LONGITUD TOTAL del camino).
       3) "Punto a la distancia s": buscar en la tabla el segmento
          [j, j+1] que contiene s y mapear lerp del t dentro del segmento.

   Con K=10 por segmento el error es < 1% (suficiente para juegos; la
   reparamétrica "arc-length exacta" con integral es análisis, no jugo).

   == DEMO ====================================================================

   Camino de 4 waypoints en zig-zag; la demo imprimirá la tabla de
   distancias acumuladas y las posiciones de un ente que avanza a velocidad
   constante (frame a frame, con dt del timestep fijo del módulo 16).
   ============================================================================= */

#include "include/mate3d.hpp"
#include <cmath>
#include <cstdio>
#include <iostream>
#include <vector>

using namespace mate;

// ==== PRIMITIVAS LOCALES ===================================================
float productoEscalar(Vec3 a, Vec3 b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}
float largo(Vec3 a) { return std::sqrt(productoEscalar(a, a)); }
// ===========================================================================

// FUNCIÓN CENTRAL: Catmull-Rom/Hermite de un segmento (t en [0..1]).
Vec3 puntoCatmullRom(Vec3 p1, Vec3 p2, Vec3 m0, Vec3 m1, float t) {
    float t2 = t * t, t3 = t2 * t;
    float h00 =  2.0f*t3 - 3.0f*t2 + 1.0f;
    float h10 =  t3 - 2.0f*t2 + t;
    float h01 = -2.0f*t3 + 3.0f*t2;
    float h11 =  t3 - t2;
    return p1*h00 + m0*h10 + p2*h01 + m1*h11;
}

// La spline completa sobre los waypoints (con clonado de extremos).
Vec3 evaluarCamino(std::vector<Vec3> const& q, float s) {
    // s en [0, N-1]: "por qué segmento voy" (s entero) + t local.
    int N = (int)q.size();
    s = std::fmax(0.0f, std::fmin(s, (float)(N - 1)));
    int i = (int)s;
    if (i >= N - 1) i = N - 2;
    float t = s - (float)i;
    Vec3 a = q[std::max(i - 1, 0)];
    Vec3 b = q[i];
    Vec3 e = q[i + 1];
    Vec3 f = q[std::min(i + 2, N - 1)];
    Vec3 m0 = (e - a) * 0.5f;
    Vec3 m1 = (f - b) * 0.5f;
    return puntoCatmullRom(b, e, m0, m1, t);
}

// TABLA DE DISTANCIAS ACUMULADAS (muestras uniformes por segmento).
std::vector<float> tablaDeArco(std::vector<Vec3> const& q, int muestrasPorSegmento) {
    std::vector<float> acum;
    acum.push_back(0.0f);
    int N = (int)q.size();
    for (int i = 0; i < N - 1; ++i) {
        Vec3 a = q[std::max(i - 1, 0)];
        Vec3 b = q[i], e = q[i + 1];
        Vec3 f = q[std::min(i + 2, N - 1)];
        Vec3 m0 = (e - a) * 0.5f;
        Vec3 m1 = (f - b) * 0.5f;
        Vec3 anterior = b;
        for (int k = 1; k <= muestrasPorSegmento; ++k) {
            float t = (float)k / (float)muestrasPorSegmento;
            Vec3 punto = puntoCatmullRom(b, e, m0, m1, t);
            acum.push_back(acum.back() + largo(punto - anterior));
            anterior = punto;
        }
    }
    return acum;
}

// DEL ARCO AL t: en qué muestra/segmento cae la distancia s (búsqueda lineal
// de la demo; el ejercicio 12 se pide con búsqueda binaria del índice).
Vec3 puntoADistancia(std::vector<Vec3> const& q,
                     std::vector<float> const& acum,
                     int muestrasPorSegmento, float distanciaObjetivo) {
    float total = acum.back();
    distanciaObjetivo = std::fmax(0.0f, std::fmin(distanciaObjetivo, total));
    int j = 0;
    while (j + 1 < (int)acum.size() && acum[j + 1] < distanciaObjetivo) ++j;
    // dentro de la muestra j: fracción t entre la muestra j y la j+1
    float fraccion = (distanciaObjetivo - acum[j]) / (acum[j + 1] - acum[j]);
    // s total en unidades de segmento: muestras 0..K por cada segmento
    float sParam = ((float)j + fraccion) / (float)muestrasPorSegmento;
    return evaluarCamino(q, sParam);
}

// ==== DEMO ================================================================
int main() {
    std::vector<Vec3> camino = {
        { 0.0f,  0.0f, 0.0f},
        { 4.0f,  0.0f, 0.0f},
        { 8.0f,  3.0f, 0.0f},
        {12.0f,  0.0f, 0.0f},
    };

    std::cout << "== PROPIEDAD: TANGENTE = DIRECCION ENTRE VECINOS ==\n";
    // en (t=0.5) del segmento 0..1 la curva va "a lo ancho" (4 waypoints recta)
    Vec3 mitad = evaluarCamino(camino, 0.5f);
    std::printf("mitad del primer segmento: (%.3f, %.3f, %.3f)\n",
                mitad.x, mitad.y, mitad.z);
    std::cout << "(baja un poco: la tangente en la esquina (4,0,0) apunta\n"
                 " hacia arriba (8,3): la curva empieza a girar temprano)\n";

    std::cout << "\n== TABLA DE ARCO (4 muestras por segmento) ==\n";
    auto acum = tablaDeArco(camino, 4);
    std::printf("longitud total ≈ %.4f\n", acum.back());
    for (size_t j = 0; j < acum.size(); ++j)
        std::printf("muestra %zu: dist=%7.4f\n", j, acum[j]);

    std::cout << "\n== SEGUIMIENTO: avanza a constante ==\n";
    float velocidad = 2.0f;                     // unidades por segundo
    float dt = 1.0f / 60.0f;                    // timestep fijo del módulo 16
    float dist = 0.0f;
    for (int frame = 0; frame < 6; ++frame) {
        dist += velocidad * dt;
        Vec3 p = puntoADistancia(camino, acum, 4, dist);
        std::printf("frame %d: dist=%6.3f pos=(%7.3f, %7.3f)\n",
                    frame, dist, p.x, p.y);
    }
    std::cout << "\n(el 'puntoADistancia' de la demo recorre la tabla línea a\n"
                 "línea; el ejercicio 12 pide búsqueda binaria y wrap-around\n"
                 "para rutas que ciclan)\n";

    std::cout << "=== FIN LECCIÓN 15 ===\n";
    return 0;
}
