/* =============================================================================
   MÓDULO 23 - LECCIÓN 14: Debug draw y perfilado de la física
   =============================================================================
   TEMA 9 de TEMAS.md: "Debug draw: shapes, contact points, normals, AABBs,
   joint anchors y perfilado de island count".

   POR QUÉ DIBUJAR LA FÍSICA
   -------------------------
   El render normal miente: un sprite puede estar 20 cm corrido de su collider
   y no se nota. El debug draw muestra lo que el motor realmente cree:
     - shapes (cajas, círculos, polígonos)
     - AABBs de broad-phase
     - puntos de contacto y normales
     - anchors de joints
     - centros de masa y ejes locales
   Es la herramienta número uno para depurar "por qué me quedo trabado".

   ARQUITECTURA
   ------------
   No conviene que el sistema de física llame a `DrawLine` de raylib/SDL/GL:
   así lo acoplás a un backend. Se genera una lista de comandos (líneas,
   puntos, texto) y el renderer la dibuja. Eso permite:
     - testear la lista sin abrir ventana (lo que hacemos acá),
     - apagar el debug draw en release,
     - tener varios backends (consola, OpenGL, ImGui).

   CONTACTOS Y NORMALES
   --------------------
   Cada contacto dibuja:
     - un punto en el punto de contacto,
     - una línea corta en la dirección de la normal (desde A hacia B).
   Si la normal apunta para el lado equivocado, el impulso empuja al cuerpo
   hacia adentro y el objeto se hunde: se ve inmediatamente.

   AABBs
   -----
   El AABB es la caja alineada a los ejes que envuelve al shape. Se calcula
   rotando el shape y tomando min/max. En un OBB rotado 45°, el AABB es más
   grande que el shape: por eso la broad-phase puede dar falsos positivos.

   ISLAND COUNT
   ------------
   Box2D agrupa los cuerpos conectados por contactos/joints en "islas" y las
   simula en paralelo (o las duerme si están quietas). Si el island count sube
   sin parar, hay cuerpos que nunca se duermen (jitter, contactos que
   aparecen/desaparecen). Medir `GetProfile().islandCount` por frame es un
   indicador temprano de que algo está mal.

   PERFILADO
   ---------
   Se mide el tiempo del Step con `steady_clock` (nunca `system_clock`) y se
   reportan promedio, p95 y máximo. Un promedio bajo puede esconder spikes de
   30 ms que se sienten como tirones.
   ============================================================================= */
#include "include/fisica2d.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <vector>

using f2d::Vec2;

namespace {

struct Segmento {
    Vec2 a;
    Vec2 b;
};

struct Punto {
    Vec2 p;
    Vec2 normal;
};

struct ComandosDebug {
    std::vector<Segmento> lineas;
    std::vector<Punto> contactos;
    std::vector<Vec2> anclas;
    int cajas = 0;
    int circulos = 0;

    void caja(Vec2 centro, float hx, float hy, float angulo) {
        const float c = std::cos(angulo);
        const float s = std::sin(angulo);
        auto rot = [&](Vec2 p) {
            return Vec2(centro.x + p.x * c - p.y * s,
                        centro.y + p.x * s + p.y * c);
        };
        const Vec2 v0 = rot({-hx, -hy});
        const Vec2 v1 = rot({hx, -hy});
        const Vec2 v2 = rot({hx, hy});
        const Vec2 v3 = rot({-hx, hy});
        lineas.push_back({v0, v1});
        lineas.push_back({v1, v2});
        lineas.push_back({v2, v3});
        lineas.push_back({v3, v0});
        ++cajas;
    }

    void circulo(Vec2 centro, float radio, int segmentos = 12) {
        for (int i = 0; i < segmentos; ++i) {
            const float a0 = 6.2831853f * static_cast<float>(i) / segmentos;
            const float a1 = 6.2831853f * static_cast<float>(i + 1) / segmentos;
            lineas.push_back({{centro.x + std::cos(a0) * radio, centro.y + std::sin(a0) * radio},
                              {centro.x + std::cos(a1) * radio, centro.y + std::sin(a1) * radio}});
        }
        ++circulos;
    }

    void aabb(const f2d::AABB& b) {
        const Vec2 v0{b.minX, b.minY};
        const Vec2 v1{b.maxX, b.minY};
        const Vec2 v2{b.maxX, b.maxY};
        const Vec2 v3{b.minX, b.maxY};
        lineas.push_back({v0, v1});
        lineas.push_back({v1, v2});
        lineas.push_back({v2, v3});
        lineas.push_back({v3, v0});
    }

    void contacto(Vec2 punto, Vec2 normal, float escala = 0.5f) {
        contactos.push_back({punto, normal});
        lineas.push_back({punto, punto + normal * escala});
    }

    void ancla(Vec2 p) { anclas.push_back(p); }
};

f2d::AABB aabbDeCajaRotada(Vec2 centro, float hx, float hy, float angulo) {
    const float c = std::fabs(std::cos(angulo));
    const float s = std::fabs(std::sin(angulo));
    const float ex = c * hx + s * hy;
    const float ey = s * hx + c * hy;
    return {centro.x - ex, centro.y - ey, centro.x + ex, centro.y + ey};
}

struct ResultadoPerfil {
    double promedioMs = 0.0;
    double p95Ms = 0.0;
    double maxMs = 0.0;
};

ResultadoPerfil perfilar(int pasos, float dt) {
    std::vector<double> tiempos;
    tiempos.reserve(pasos);
    Vec2 pos{0.0f, 10.0f};
    Vec2 vel{0.0f, 0.0f};
    for (int i = 0; i < pasos; ++i) {
        const auto t0 = std::chrono::steady_clock::now();
        // Simulación mínima para tener algo que medir.
        vel.y -= 9.8f * dt;
        pos += vel * dt;
        if (pos.y < 0.0f) {
            pos.y = 0.0f;
            vel.y = -vel.y * 0.2f;
        }
        const auto t1 = std::chrono::steady_clock::now();
        tiempos.push_back(std::chrono::duration<double, std::milli>(t1 - t0).count());
    }
    std::sort(tiempos.begin(), tiempos.end());
    double suma = 0.0;
    for (double t : tiempos) suma += t;
    return {suma / pasos, tiempos[static_cast<size_t>(pasos * 0.95)], tiempos.back()};
}

} // namespace

int main() {
    std::cout << std::fixed << std::setprecision(3);

    ComandosDebug debug;
    debug.caja({0.0f, 1.0f}, 1.0f, 0.5f, 0.0f);
    debug.caja({2.0f, 1.0f}, 0.5f, 0.5f, 0.785398f);  // 45°
    debug.circulo({-2.0f, 1.0f}, 0.7f);
    debug.aabb(aabbDeCajaRotada({2.0f, 1.0f}, 0.5f, 0.5f, 0.785398f));
    debug.contacto({0.5f, 0.5f}, {1.0f, 0.0f});
    debug.contacto({1.5f, 0.5f}, {0.0f, 1.0f});
    debug.ancla({0.0f, 0.0f});
    debug.ancla({2.0f, 0.5f});

    std::cout << "Comandos de debug:\n"
              << "  cajas=" << debug.cajas
              << " círculos=" << debug.circulos
              << " líneas=" << debug.lineas.size()
              << " contactos=" << debug.contactos.size()
              << " anclas=" << debug.anclas.size() << "\n";

    std::cout << "  AABB de la caja rotada 45°: min=("
              << aabbDeCajaRotada({2.0f, 1.0f}, 0.5f, 0.5f, 0.785398f).minX << ", "
              << aabbDeCajaRotada({2.0f, 1.0f}, 0.5f, 0.5f, 0.785398f).minY
              << ") max=("
              << aabbDeCajaRotada({2.0f, 1.0f}, 0.5f, 0.5f, 0.785398f).maxX << ", "
              << aabbDeCajaRotada({2.0f, 1.0f}, 0.5f, 0.5f, 0.785398f).maxY
              << ")\n";
    std::cout << "  El AABB rotado mide " << (0.5f * std::sqrt(2.0f) * 2.0f)
              << " m de lado; la caja real mide 1 m.\n\n";

    const auto perfil = perfilar(1000, 1.0f / 60.0f);
    std::cout << "Perfil de 1000 pasos:\n"
              << "  promedio=" << perfil.promedioMs << " ms"
              << " p95=" << perfil.p95Ms << " ms"
              << " max=" << perfil.maxMs << " ms\n";
    std::cout << "Un p95 muy por encima del promedio delata spikes.\n\n";

    std::cout << "Debug draw = lista de comandos sin backend. En el juego, esa\n"
                 "lista se dibuja con líneas de OpenGL/SDL y se apaga con una tecla.\n";
    return 0;
}
