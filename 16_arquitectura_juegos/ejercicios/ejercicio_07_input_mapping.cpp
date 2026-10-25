/* =============================================================================
   MÓDULO 16 - EJERCICIO 07: Input Mapping — Polling + Action Mapping + Edge Detection
   =============================================================================
   DIFICULTAD: Intermedio
   APOYO: Lección 11 (Input polling, mapeo a acciones lógicas)

   CONSIGNA:
   Implementa sistema de input con action mapping:
   - Mapping: tecla física -> acción lógica
   - InputState: current[Action], previous[Action]
   - UpdateInput(): copia current->previous, polltea teclas, actualiza current
   - IsPressed/Released/Held(action): edge detection

   ACCIONES:
   enum Action { MoveLeft, MoveRight, Jump, Attack, Pause, Count };

   MAPEO (fijo para tests):
   - KEY_A / LEFT  -> MoveLeft
   - KEY_D / RIGHT -> MoveRight
   - KEY_SPACE     -> Jump
   - KEY_J         -> Attack
   - KEY_ESCAPE    -> Pause

   TESTS:
   - Press key -> Held=true, Pressed=true (solo frame 1)
   - Release key -> Released=true (solo frame 1), Held=false
   - Multiple keys simultáneos
   - Previous frame correcto

   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_07_input_mapping.cpp -o ej07 && ./ej07
   ============================================================================= */
#include <array>
#include <functional>
#include "../../include/test_framework.hpp"

enum class Action { MoveLeft, MoveRight, Jump, Attack, Pause, Count };
constexpr size_t ActionCount = static_cast<size_t>(Action::Count);

// Simulación de teclas (para tests deterministas)
struct KeyboardSim {
    std::array<bool, 256> keys{};
    void Press(int key) { if (key >= 0 && key < 256) keys[key] = true; }
    void Release(int key) { if (key >= 0 && key < 256) keys[key] = false; }
    bool IsDown(int key) const { return (key >= 0 && key < 256) ? keys[key] : false; }
};

struct InputMapping {
    int key;        // tecla física
    Action action;  // acción lógica
};

struct InputState {
    std::array<bool, ActionCount> current{};
    std::array<bool, ActionCount> previous{};
};

// TODO 1: UpdateInput - copia previous, polltea mappings
// TU CÓDIGO AQUÍ:
void UpdateInput(InputState& state, const InputMapping* mappings, size_t mapCount, const KeyboardSim& kb) {
    // Pista: state.previous = state.current;
    //        for each mapping: state.current[action] = kb.IsDown(key);
}

// TODO 2: Edge detection helpers
// TU CÓDIGO AQUÍ:
bool IsPressed(const InputState& state, Action a) {
    // Pista: current && !previous
    return false;
}

bool IsReleased(const InputState& state, Action a) {
    // Pista: !current && previous
    return false;
}

bool IsHeld(const InputState& state, Action a) {
    // Pista: current
    return false;
}

// ============= TESTS =============
const InputMapping GAME_MAPPINGS[] = {
    {'A', Action::MoveLeft},
    {'D', Action::MoveRight},
    {' ', Action::Jump},
    {'J', Action::Attack},
    {27, Action::Pause},  // ESC = 27
};

TEST_CASE(UpdateInputSetsCurrent) {
    InputState state;
    KeyboardSim kb;
    kb.Press('A');
    UpdateInput(state, GAME_MAPPINGS, 5, kb);
    ASSERT_TRUE(IsHeld(state, Action::MoveLeft));
    ASSERT_FALSE(IsHeld(state, Action::MoveRight));
}

TEST_CASE(PressedOnlyFirstFrame) {
    InputState state;
    KeyboardSim kb;
    kb.Press(' ');  // Space

    // Frame 1
    UpdateInput(state, GAME_MAPPINGS, 5, kb);
    ASSERT_TRUE(IsPressed(state, Action::Jump));
    ASSERT_TRUE(IsHeld(state, Action::Jump));

    // Frame 2 (still held)
    UpdateInput(state, GAME_MAPPINGS, 5, kb);
    ASSERT_FALSE(IsPressed(state, Action::Jump));
    ASSERT_TRUE(IsHeld(state, Action::Jump));
}

TEST_CASE(ReleasedOnKeyUp) {
    InputState state;
    KeyboardSim kb;
    kb.Press('D');
    UpdateInput(state, GAME_MAPPINGS, 5, kb);  // Frame 1: press
    UpdateInput(state, GAME_MAPPINGS, 5, kb);  // Frame 2: hold
    kb.Release('D');
    UpdateInput(state, GAME_MAPPINGS, 5, kb);  // Frame 3: release
    ASSERT_TRUE(IsReleased(state, Action::MoveRight));
    ASSERT_FALSE(IsHeld(state, Action::MoveRight));
}

TEST_CASE(MultipleKeysIndependent) {
    InputState state;
    KeyboardSim kb;
    kb.Press('A');
    kb.Press('J');
    UpdateInput(state, GAME_MAPPINGS, 5, kb);
    ASSERT_TRUE(IsHeld(state, Action::MoveLeft));
    ASSERT_TRUE(IsHeld(state, Action::Attack));
    ASSERT_FALSE(IsHeld(state, Action::Jump));
}

TEST_CASE(PreviousFrameCopiedCorrectly) {
    InputState state;
    KeyboardSim kb;
    kb.Press('A');
    UpdateInput(state, GAME_MAPPINGS, 5, kb);
    ASSERT_TRUE(state.current[static_cast<size_t>(Action::MoveLeft)]);
    ASSERT_FALSE(state.previous[static_cast<size_t>(Action::MoveLeft)]);

    kb.Release('A');
    UpdateInput(state, GAME_MAPPINGS, 5, kb);
    ASSERT_FALSE(state.current[static_cast<size_t>(Action::MoveLeft)]);
    ASSERT_TRUE(state.previous[static_cast<size_t>(Action::MoveLeft)]);
}

TEST_CASE(EscapeMapsToPause) {
    InputState state;
    KeyboardSim kb;
    kb.Press(27);  // ESC
    UpdateInput(state, GAME_MAPPINGS, 5, kb);
    ASSERT_TRUE(IsPressed(state, Action::Pause));
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 07 (Input Mapping)...\n";
    return RUN_ALL_TESTS();
}