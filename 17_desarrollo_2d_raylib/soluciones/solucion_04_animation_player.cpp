/* =============================================================================
   MÓDULO 17 - SOLUCIÓN 04: AnimationPlayer — Timer, Loop, Once y Ping-Pong
   =============================================================================
   Resuelve ejercicio_04_animation_player.cpp. Dos reglas de oro del timer:
   (1) el resto del timer SE PRESERVA (timer -= duración, nunca = 0) para no
   perder tiempo en frames veloces; (2) ping-pong maneja dirección con un solo
   int (el "incremento" cambia de signo en los extremos).

   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra solucion_04_animation_player.cpp -o sol && ./sol
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
    int direction = 1;

    // ============================ SOLUCIÓN ===================================
    void Update(float dt) {
        if (!clip || finished || clip->durations.empty()) return;
        timer += dt;

        const int n = int(clip->durations.size());
        while (true) {
            const float dur = clip->durations[size_t(frame)];
            if (timer < dur) break;         // sigue dentro del frame actual
            timer -= dur;                   // el resto se PRESERVA (regla 1)

            switch (clip->mode) {
                case LoopMode::Loop:
                    frame = (frame + 1) % n;
                    break;

                case LoopMode::Once:
                    if (frame + 1 < n) {
                        ++frame;
                    } else {
                        finished = true;    // al terminar se congela en el ulti
                        return;
                    }
                    break;

                case LoopMode::PingPong: {
                    // el incremento cambia de SIGNO en los extremos (regla 2):
                    // llega a n-1 => rebota hacia atrás; vuelve a 0 => adelante
                    frame += direction;
                    if (frame >= n - 1) { frame = n - 1; direction = -1; }
                    else if (frame <= 0) { frame = 0; direction = 1; }
                    break;
                }
            }
            if (finished && clip->mode != LoopMode::PingPong) break;
        }
    }
};

// ============================ TESTS (iguales) ================================
TEST_CASE(AvanceConPreservacionDeResto) {
    AnimationClip clip{{0.1f, 0.2f}};
    AnimationPlayer p;
    p.clip = &clip;
    p.Update(0.15f);
    ASSERT_EQ(p.frame, 1);
    ASSERT_NEAR(p.timer, 0.05f, 0.001f);
}

TEST_CASE(LoopCircularYVuelveAEmpezar) {
    AnimationClip clip{{0.1f, 0.1f}};
    AnimationPlayer p;
    p.clip = &clip;
    p.Update(0.15f);
    ASSERT_EQ(p.frame, 1);
    p.Update(0.06f);
    ASSERT_EQ(p.frame, 0);
    ASSERT_NEAR(p.timer, 0.01f, 0.002f);
}

TEST_CASE(LoopDeMuchasVueltasIguales) {
    AnimationClip clip{{0.1f, 0.2f, 0.3f}};
    AnimationPlayer p;
    p.clip = &clip;
    p.Update(1.4f);
    ASSERT_EQ(p.frame, 1);
    ASSERT_NEAR(p.timer, 0.1f, 0.001f);
}

TEST_CASE(OnceSeFrenaYNuncaPasaDelUltimoFrame) {
    AnimationClip clip{{0.1f, 0.2f}, LoopMode::Once};
    AnimationPlayer p;
    p.clip = &clip;
    p.Update(0.5f);
    ASSERT_TRUE(p.finished);
    p.Update(1.0f);
    ASSERT_EQ(p.frame, 1);
    ASSERT_TRUE(p.finished);
}

TEST_CASE(PingPongRebotaYVuelve) {
    AnimationClip clip{{0.1f, 0.1f, 0.1f}, LoopMode::PingPong};
    AnimationPlayer p;
    p.clip = &clip;
    p.Update(0.35f);
    ASSERT_EQ(p.frame, 1);
    ASSERT_NEAR(p.timer, 0.05f, 0.002f);
    ASSERT_EQ(p.direction, -1);
    p.Update(0.1f);
    ASSERT_EQ(p.frame, 0);
    ASSERT_NEAR(p.timer, 0.05f, 0.002f);
    ASSERT_EQ(p.direction, 1);
    p.Update(0.2f);
    ASSERT_EQ(p.frame, 2);
    ASSERT_EQ(p.direction, -1);
}

int main() {
    std::cout << "Solución 04: AnimationPlayer\n";
    return RUN_ALL_TESTS();
}
