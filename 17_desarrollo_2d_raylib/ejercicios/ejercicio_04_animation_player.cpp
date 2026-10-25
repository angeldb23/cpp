/* =============================================================================
   MÓDULO 17 - EJERCICIO 04: AnimationPlayer — Timer, Loop, Once y Ping-Pong
   =============================================================================
   DIFICULTAD: Intermedio
   APOYO: Lección 03 (Animaciones: frame data, timer, modos de looping)

   CONSIGNA:
   Completa AnimationPlayer::Update(dt) para que reproduzca un clip según su
   modo de repeat:

   REGLAS:
   - El timer acumula dt. Cuando acumula >= duración del frame actual:
     se consume esa duración y avanza de frame (y el RESTO del timer se
     PRESERVA: timer -= duración, no timer = 0).
   - Loop: los frames ciclan (n-1 → 0).
   - Once: al terminar el ÚLTIMO frame, finished = true y NO avanza más.
   - PingPong: 0→1→2...→n-1→(n-2)→...→0, y volviendo a 0 vuelve a avanzar.
     El "rebote" sigue el orden de la dirección actual.
   - Si el clip ya terminó (Once y finished), Update no hace nada.

   El clip es una lista de duraciones por frame (sin raylib ni textura:
   la parte gráfica no entra al test).

   TESTS (no tocar): avance con preservación de resto, loop circular,
   once se frena, y ping-pong rebota 2 veces.

   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_04_animation_player.cpp -o ej && ./ej
   ============================================================================= */
#include <vector>
#include <cstddef>
#include <iostream>
#include "../../include/test_framework.hpp"

// ============================ TIPOS (no tocar) ===============================
enum class LoopMode { Loop, Once, PingPong };

struct AnimationClip {
    std::vector<float> durations;
    LoopMode mode = LoopMode::Loop;
};

struct AnimationPlayer {
    const AnimationClip* clip = nullptr;
    int frame = 0;
    float timer = 0.0f;
    bool finished = false;
    int direction = 1;   // PingPong: +1 avanza, -1 retrocede

    // TODO: actualiza frame/timer/finished según el modo del clip
    void Update(float dt) {
        (void)dt;
        // Pistas:
        // 1) si !clip || finished => no hacer nada
        // 2) timer += dt; mientras timer >= duración del frame actual:
        //       timer -= duración; y:
        //        Loop:    frame = (frame + 1) % n
        //        Once:    si frame+1 < n: ++frame; sino: finished = true, break
        //        PingPong: avanzar/retroceder según direction, con rebote en
        //                  los extremos (0 y n-1)
    }
};

// ============================ TESTS (no tocar) ==============================
TEST_CASE(AvanceConPreservacionDeResto) {
    AnimationClip clip{{0.1f, 0.2f}};
    AnimationPlayer p;
    p.clip = &clip;
    p.Update(0.15f);   // cruza el frame 0 (0.1) y sobra 0.05
    ASSERT_EQ(p.frame, 1);
    ASSERT_NEAR(p.timer, 0.05f, 0.001f);   // el resto NO se tira
}

TEST_CASE(LoopCircularYVuelveAEmpezar) {
    AnimationClip clip{{0.1f, 0.1f}};
    AnimationPlayer p;
    p.clip = &clip;
    p.Update(0.15f);   // frame 0->1, timer 0.05
    ASSERT_EQ(p.frame, 1);
    p.Update(0.06f);   // cruza frame 1 (0.1) con 0.05 + 0.06 = 0.11 > 0.1 => vuelve a 0
    ASSERT_EQ(p.frame, 0);
    ASSERT_NEAR(p.timer, 0.01f, 0.002f);
}

TEST_CASE(LoopDeMuchasVueltasIguales) {
    AnimationClip clip{{0.1f, 0.2f, 0.3f}};
    AnimationPlayer p;
    p.clip = &clip;
    // Duración total 0.6; 1.4s = 2 vueltas completas (1.2) + 0.2 de sobra.
    // Con 0.2 restante: cruza frame 0 (0.1) y queda timer 0.1 en el frame 1.
    p.Update(1.4f);
    ASSERT_EQ(p.frame, 1);
    ASSERT_NEAR(p.timer, 0.1f, 0.001f);
}

TEST_CASE(OnceSeFrenaYNuncaPasaDelUltimoFrame) {
    AnimationClip clip{{0.1f, 0.2f}, LoopMode::Once};
    AnimationPlayer p;
    p.clip = &clip;
    p.Update(0.5f);   // cruza ambos frames y termina
    ASSERT_TRUE(p.finished);
    p.Update(1.0f);
    ASSERT_EQ(p.frame, 1);   // sigue en el último: no hay wrap
    ASSERT_TRUE(p.finished);
}

TEST_CASE(PingPongRebotaYVuelve) {
    AnimationClip clip{{0.1f, 0.1f, 0.1f}, LoopMode::PingPong};
    AnimationPlayer p;
    p.clip = &clip;
    // 0.35s = 3.5 cruzadas de 0.1: 0->1->2 (rebote) ->1
    p.Update(0.35f);
    ASSERT_EQ(p.frame, 1);
    ASSERT_NEAR(p.timer, 0.05f, 0.002f);
    ASSERT_EQ(p.direction, -1);   // acaba de rebotar en el frame 2
    p.Update(0.1f);    // cruza frame 1 => en 0 rebota y vuelve a avanzar
    ASSERT_EQ(p.frame, 0);
    ASSERT_NEAR(p.timer, 0.05f, 0.002f);
    ASSERT_EQ(p.direction, 1);
    p.Update(0.2f);    // cruza 0->1->2 denunciando rebote otra vez
    ASSERT_EQ(p.frame, 2);
    ASSERT_EQ(p.direction, -1);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 04 (AnimationPlayer)...\n";
    return RUN_ALL_TESTS();
}
