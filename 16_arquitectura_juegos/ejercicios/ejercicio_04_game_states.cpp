/* =============================================================================
   MÓDULO 16 - EJERCICIO 04: Game States — Pila de Estados (Menu, Playing, Pause)
   =============================================================================
   DIFICULTAD: Intermedio
   APOYO: Lección 07 (Estados de juego y pila)

   CONSIGNA:
   Implementa un State Manager con pila (stack) de estados:
   - Push(state): apila nuevo estado, llama Exit() al anterior, Enter() al nuevo
   - Pop(): desapila, llama Exit() al actual, Enter() al anterior (resume)
   - Change(state): Pop() + Push() atómico
   - Current(): acceso al estado tope
   - Empty(): true si stack vacía (juego debe terminar)

   ESTADOS MÍNIMOS:
   - MenuState: Enter() cuenta++, Exit() cuenta++
   - PlayingState: Enter/Exit cuentan, Update(dt) acumula dt
   - PauseState: Enter/Exit cuentan, Update no hace nada

   TESTS:
   - Push Menu -> Push Playing -> Pop -> Current = Menu
   - Enter/Exit llamados en orden correcto
   - Change reemplaza estado tope
   - Empty() tras Pop final

   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_04_game_states.cpp -o ej04 && ./ej04
   ============================================================================= */
#include <vector>
#include <memory>
#include <string>
#include "../../include/test_framework.hpp"

struct IGameState {
    virtual ~IGameState() = default;
    virtual void Enter() = 0;
    virtual void Exit() = 0;
    virtual void Update(float) = 0;
    virtual std::string Name() const = 0;
};

struct StateManager {
    std::vector<std::unique_ptr<IGameState>> stack_;

    // TODO 1: Push - apila estado, llama Exit anterior, Enter nuevo
    // TU CÓDIGO AQUÍ:
    void Push(std::unique_ptr<IGameState> state) {
        // Pista: if (!stack_.empty()) stack_.back()->Exit();
        //        stack_.push_back(std::move(state));
        //        stack_.back()->Enter();
    }

    // TODO 2: Pop - desapila, llama Exit, Enter al anterior si existe
    // TU CÓDIGO AQUÍ:
    void Pop() {
        // Pista: if (!stack_.empty()) { stack_.back()->Exit(); stack_.pop_back(); if (!stack_.empty()) stack_.back()->Enter(); }
    }

    // TODO 3: Change - Pop + Push atómico
    // TU CÓDIGO AQUÍ:
    void Change(std::unique_ptr<IGameState> state) {
        // Pista: Pop(); Push(std::move(state));
    }

    // TODO 4: Current y Empty
    // TU CÓDIGO AQUÍ:
    IGameState* Current() {
        // Pista: return stack_.empty() ? nullptr : stack_.back().get();
        return nullptr;
    }
    bool Empty() const {
        // Pista: return stack_.empty();
        return true;
    }
};

// Estados de prueba con contadores
struct TestState : IGameState {
    std::string name_;
    int* enterCount_;
    int* exitCount_;
    float* updateAccum_;

    TestState(std::string n, int* e, int* x, float* u)
        : name_(std::move(n)), enterCount_(e), exitCount_(x), updateAccum_(u) {}

    void Enter() override { if (enterCount_) ++*enterCount_; }
    void Exit() override { if (exitCount_) ++*exitCount_; }
    void Update(float dt) override { if (updateAccum_) *updateAccum_ += dt; }
    std::string Name() const override { return name_; }
};

// ============= TESTS =============
TEST_CASE(PushCallsEnter) {
    StateManager sm;
    int enter = 0, exit = 0; float upd = 0;
    sm.Push(std::make_unique<TestState>("A", &enter, &exit, &upd));
    ASSERT_EQ(enter, 1);
    ASSERT_EQ(exit, 0);
    auto* curr = sm.Current();
    ASSERT_TRUE(curr != nullptr);  // Sin implementar: Current() devuelve nullptr
    if (curr) ASSERT_EQ(curr->Name(), "A");
}

TEST_CASE(PushSecondCallsExitThenEnter) {
    StateManager sm;
    int enter = 0, exit = 0; float upd = 0;
    sm.Push(std::make_unique<TestState>("A", &enter, &exit, &upd));
    sm.Push(std::make_unique<TestState>("B", &enter, &exit, &upd));
    ASSERT_EQ(enter, 2);  // A y B
    ASSERT_EQ(exit, 1);   // A salió
    auto* curr = sm.Current();
    ASSERT_TRUE(curr != nullptr);
    if (curr) ASSERT_EQ(curr->Name(), "B");
}

TEST_CASE(PopCallsExitAndResumesPrevious) {
    StateManager sm;
    int enter = 0, exit = 0; float upd = 0;
    sm.Push(std::make_unique<TestState>("A", &enter, &exit, &upd));
    sm.Push(std::make_unique<TestState>("B", &enter, &exit, &upd));
    sm.Pop();
    ASSERT_EQ(exit, 2);  // B sale, A vuelve a entrar
    ASSERT_EQ(enter, 3);
    auto* curr = sm.Current();
    ASSERT_TRUE(curr != nullptr);
    if (curr) ASSERT_EQ(curr->Name(), "A");
}

TEST_CASE(ChangeReplacesTop) {
    StateManager sm;
    int enter = 0, exit = 0; float upd = 0;
    sm.Push(std::make_unique<TestState>("A", &enter, &exit, &upd));
    sm.Change(std::make_unique<TestState>("C", &enter, &exit, &upd));
    ASSERT_EQ(exit, 1);  // A sale
    ASSERT_EQ(enter, 2); // C entra
    auto* curr = sm.Current();
    ASSERT_TRUE(curr != nullptr);
    if (curr) ASSERT_EQ(curr->Name(), "C");
    ASSERT_EQ(sm.Empty(), false);
}

TEST_CASE(PopToEmpty) {
    StateManager sm;
    int enter = 0, exit = 0; float upd = 0;
    sm.Push(std::make_unique<TestState>("A", &enter, &exit, &upd));
    sm.Pop();
    ASSERT_TRUE(sm.Empty());
    ASSERT_EQ(sm.Current(), nullptr);
}

TEST_CASE(UpdateAccumulates) {
    StateManager sm;
    int enter = 0, exit = 0; float upd = 0;
    sm.Push(std::make_unique<TestState>("A", &enter, &exit, &upd));
    auto* curr = sm.Current();
    ASSERT_TRUE(curr != nullptr);
    if (curr) {
        curr->Update(0.016f);
        curr->Update(0.016f);
    }
    ASSERT_NEAR(upd, 0.032f, 0.001f);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 04 (Game States Stack)...\n";
    return RUN_ALL_TESTS();
}