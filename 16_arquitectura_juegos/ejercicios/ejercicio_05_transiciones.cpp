/* =============================================================================
   MÓDULO 16 - EJERCICIO 05: Transiciones — Fade In/Out con Callback Mid
   =============================================================================
   DIFICULTAD: Intermedio
   APOYO: Lección 08 (Transiciones entre estados)

   CONSIGNA:
   Implementa un sistema de fade genérico:
   - Fases: None, FadeOut (0→1), FadeIn (1→0)
   - Alpha [0,1] avanza con dt/speed
   - En alpha=1 (midpoint): ejecuta callback onMid (cambio de estado real)
   - Update(dt) avanza la transición
   - IsActive() indica si hay transición en curso

   ESTRUCTURA:
   - enum class Phase { None, FadeOut, FadeIn }
   - struct Fade { Phase phase; float alpha; float speed; callback onMid; }
   - Start(callback): inicia FadeOut, alpha=0, guarda callback
   - Update(dt): alpha += dt/speed (FadeOut) o -= dt/speed (FadeIn)
   - Al llegar a 1.0 en FadeOut: phase=FadeIn, ejecuta onMid, limpia callback
   - Al llegar a 0.0 en FadeIn: phase=None

   TESTS:
   - Start -> phase=FadeOut, alpha=0
   - Update avanza alpha correctamente
   - En midpoint llama callback UNA vez
   - Completa y vuelve a None
   - Speed controla duración

   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_05_transiciones.cpp -o ej05 && ./ej05
   ============================================================================= */
#include <functional>
#include "../../include/test_framework.hpp"

enum class FadePhase { None, FadeOut, FadeIn };

inline std::ostream& operator<<(std::ostream& os, FadePhase p) {
    switch (p) {
        case FadePhase::None: return os << "None";
        case FadePhase::FadeOut: return os << "FadeOut";
        case FadePhase::FadeIn: return os << "FadeIn";
    }
    return os << "Unknown";
}

struct FadeTransition {
    FadePhase phase = FadePhase::None;
    float alpha = 0.0f;
    float speed = 1.0f;  // segundos para fade completo (0→1 o 1→0)
    std::function<void()> onMid;

    // TODO 1: Start - inicia fade out con callback
    // TU CÓDIGO AQUÍ:
    void Start(std::function<void()> callback) {
        // Pista: phase = FadePhase::FadeOut; alpha = 0; onMid = std::move(callback);
    }

    // TODO 2: Update - avanza alpha según fase
    // TU CÓDIGO AQUÍ:
    void Update(float dt) {
        // Pista:
        // if (phase == FadePhase::None) return;
        // float delta = dt / speed;
        // if (phase == FadePhase::FadeOut) {
        //     alpha += delta;
        //     if (alpha >= 1.0f) { alpha = 1.0f; phase = FadePhase::FadeIn; if (onMid) { onMid(); onMid = nullptr; } }
        // } else { // FadeIn
        //     alpha -= delta;
        //     if (alpha <= 0.0f) { alpha = 0.0f; phase = FadePhase::None; }
        // }
    }

    // TODO 3: IsActive
    // TU CÓDIGO AQUÍ:
    bool IsActive() const {
        // Pista: return phase != FadePhase::None;
        return false;
    }
};

// ============= TESTS =============
TEST_CASE(StartSetsFadeOut) {
    FadeTransition f;
    f.Start([](){});
    ASSERT_EQ(f.phase, FadePhase::FadeOut);
    ASSERT_EQ(f.alpha, 0.0f);
    ASSERT_TRUE(f.IsActive());
}

TEST_CASE(UpdateAdvancesAlphaInFadeOut) {
    FadeTransition f;
    f.speed = 2.0f;  // 2 segundos para fade completo
    f.Start([](){});
    f.Update(0.5f);  // 0.5s -> delta = 0.25
    ASSERT_EQ(f.phase, FadePhase::FadeOut);
    ASSERT_NEAR(f.alpha, 0.25f, 0.001f);
}

TEST_CASE(MidpointCallsCallbackOnce) {
    FadeTransition f;
    f.speed = 1.0f;
    int called = 0;
    f.Start([&](){ called++; });
    f.Update(1.0f);  // Llega a alpha=1
    ASSERT_EQ(called, 1);
    ASSERT_EQ(f.phase, FadePhase::FadeIn);
    ASSERT_EQ(f.alpha, 1.0f);
}

TEST_CASE(SecondCallDoesNotRecall) {
    FadeTransition f;
    f.speed = 1.0f;
    int called = 0;
    f.Start([&](){ called++; });
    f.Update(1.0f);  // midpoint
    f.Update(1.0f);  // sigue en FadeIn
    ASSERT_EQ(called, 1);  // No se llama de nuevo
}

TEST_CASE(FadeInCompletesToNone) {
    FadeTransition f;
    f.speed = 1.0f;
    f.Start([](){});
    f.Update(1.0f);  // FadeOut completo -> FadeIn
    f.Update(1.0f);  // FadeIn completo -> None
    ASSERT_EQ(f.phase, FadePhase::None);
    ASSERT_EQ(f.alpha, 0.0f);
    ASSERT_FALSE(f.IsActive());
}

TEST_CASE(SpeedControlsDuration) {
    FadeTransition f;
    f.speed = 0.5f;  // Medio segundo
    f.Start([](){});
    f.Update(0.5f);  // Debería completar FadeOut
    ASSERT_EQ(f.phase, FadePhase::FadeIn);
}

TEST_CASE(AlphaClampedAtOneAndZero) {
    FadeTransition f;
    f.speed = 1.0f;
    f.Start([](){});
    f.Update(10.0f);  // Tiempo enorme
    ASSERT_EQ(f.alpha, 0.0f);  // Debe terminar en 0
    ASSERT_EQ(f.phase, FadePhase::None);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 05 (Transiciones Fade)...\n";
    return RUN_ALL_TESTS();
}