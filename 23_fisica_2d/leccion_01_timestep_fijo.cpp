/* =============================================================================
   MÓDULO 23 - LECCIÓN 01: Física en timestep fijo
   =============================================================================
   TEMA 1 de TEMAS.md: "Física en timestep fijo: por qué separado del render,
   interpolación de estado (alpha) y determinismo".

   IDEA CENTRAL
   ------------
   El render corre "cuando puede" (60 Hz, 144 Hz, o a los saltos porque el
   sistema se trabó). La física NO puede depender de eso: si el resultado
   cambia según los FPS, dos partidas no se pueden reproducir y una replay no
   coincide. La solución clásica es el acumulador de tiempo:

       accumulator += frameTime (clampeado);
       while (accumulator >= fixedDt) {
           step(fixedDt);          // SIEMPRE el mismo dt
           accumulator -= fixedDt;
       }
       alpha = accumulator / fixedDt;   // para interpolar el dibujo

   `frameTime` se clampea (por ejemplo a 0.25 s). Si no, un stall de 2 segundos
   dispara 120 pasos de golpe y el juego se congela más: la "espiral de la
   muerte". Clampear pierde tiempo real, pero mantiene el juego vivo.

   DETERMINISMO
   ------------
   Con `fixedDt` constante y el mismo orden de operaciones, la simulación da
   los mismos bits en la misma máquina. Con `dt` variable, cada frame suma una
   cantidad distinta de error de integración y el estado diverge.

   INTERPOLACIÓN (alpha)
   ---------------------
   La física quedó "atrás" del tiempo real: el último paso fue en t y el
   render está en t + alpha*fixedDt. Para que se vea suave, se dibuja
   `posicion = anterior*(1-alpha) + actual*alpha`. La lógica (colisiones,
   daño, IA) usa el estado actual; solo el dibujo interpola.

   Este archivo NO abre ventanas: imprime la comparación numérica.
   ============================================================================= */
#include "include/fisica2d.hpp"

#include <cmath>
#include <iomanip>
#include <iostream>
#include <vector>

using f2d::Vec2;

namespace {

struct Proyectil {
    Vec2 pos;
    Vec2 vel;
};

// Integración semi-implícita de Euler: primero velocidad, después posición.
// Es la que usa Box2D y la mayoría de los motores 2D.
void integrar(Proyectil& p, float dt) {
    constexpr float gravedad = -9.8f;
    p.vel.y += gravedad * dt;
    p.pos += p.vel * dt;
}

struct Acumulador {
    float dtFijo = 1.0f / 120.0f;  // 120 pasos por segundo
    float maxFrame = 0.25f;        // techo anti-espiral
    float resto = 0.0f;
    int pasos = 0;

    void avanzar(Proyectil& p, float frameTime) {
        if (dtFijo <= 0.0f) return;              // configuración inválida
        frameTime = f2d::clampf(frameTime, 0.0f, maxFrame);
        resto += frameTime;
        while (resto >= dtFijo) {
            integrar(p, dtFijo);
            resto -= dtFijo;
            ++pasos;
        }
    }

    float alpha() const {
        return dtFijo > 0.0f ? f2d::clampf(resto / dtFijo, 0.0f, 1.0f) : 0.0f;
    }
};

} // namespace

int main() {
    // Tiempos de frame irregulares que suman exactamente 1 segundo.
    const std::vector<float> frames = {
        0.004f, 0.009f, 0.017f, 0.033f, 0.008f, 0.021f,
        0.016f, 0.050f, 0.007f, 0.012f, 0.025f, 0.019f,
        0.011f, 0.030f, 0.014f, 0.022f, 0.013f, 0.018f,
        0.016f, 0.015f, 0.009f, 0.017f, 0.024f, 0.011f,
        0.016f, 0.016f, 0.016f, 0.016f, 0.016f, 0.016f,
    };

    Proyectil fijo{{0.0f, 0.0f}, {10.0f, 10.0f}};
    Proyectil variable = fijo;
    Acumulador acc;

    float tiempoTotal = 0.0f;
    for (float ft : frames) {
        acc.avanzar(fijo, ft);
        integrar(variable, ft);  // el error didáctico: física atada al frame
        tiempoTotal += ft;
    }

    std::cout << std::fixed << std::setprecision(4);
    std::cout << "Física en timestep fijo (dt = " << acc.dtFijo << " s)\n";
    std::cout << "Tiempo real acumulado: " << tiempoTotal << " s\n";
    std::cout << "Pasos fijos ejecutados: " << acc.pasos << "\n";
    std::cout << "Alpha del próximo render: " << acc.alpha() << "\n\n";
    std::cout << "                 x          y\n";
    std::cout << "Fijo        " << fijo.pos.x << "   " << fijo.pos.y << "\n";
    std::cout << "Variable    " << variable.pos.x << "   " << variable.pos.y << "\n";
    std::cout << "Diferencia  " << (fijo.pos.x - variable.pos.x) << "   "
              << (fijo.pos.y - variable.pos.y) << "\n\n";

    std::cout << "Con dt variable, cada frame integra con un dt distinto: el\n"
                 "resultado depende de los FPS. Con dt fijo, no.\n";

    // Un frame gigante se clampea: no se simulan 2 segundos de una vez.
    Proyectil p2{{0.0f, 0.0f}, {0.0f, 0.0f}};
    Acumulador acc2;
    acc2.avanzar(p2, 1000.0f);
    std::cout << "Frame de 1000 s clampeado a " << acc2.maxFrame
              << " s -> " << acc2.pasos << " pasos (no 120000).\n";

    // Alpha: el resto que no llegó a un paso fijo.
    Proyectil p3{{0.0f, 0.0f}, {0.0f, 0.0f}};
    Acumulador acc3;
    acc3.avanzar(p3, 0.012f);
    std::cout << "Frame de 0.012 s con dt 1/120 -> pasos=" << acc3.pasos
              << ", alpha=" << acc3.alpha() << " (se dibuja interpolado).\n";
    return 0;
}
