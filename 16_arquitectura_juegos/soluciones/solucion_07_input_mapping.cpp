/* =============================================================================
   MÓDULO 16 - SOLUCIÓN 07: Input Mapping — Polling + Action Mapping
   ============================================================================= */
#include <array>
#include <functional>
#include "../../include/test_framework.hpp"

enum class Action { MoveLeft, MoveRight, Jump, Attack, Pause, Count };
constexpr size_t ActionCount = static_cast<size_t>(Action::Count);

struct KeyboardSim {
    std::array<bool, 256> keys{};
    void Press(int key) { if (key >= 0 && key < 256) keys[key] = true; }
    void Release(int key) { if (key >= 0 && key < 256) keys[key] = false; }
    bool IsDown(int key) const { return (key >= 0 && key < 256) ? keys[key] : false; }
};

struct InputMapping {
    int key;
    Action action;
};

struct InputState {
    std::array<bool, ActionCount> current{};
    std::array<bool, ActionCount> previous{};
};

void UpdateInput(InputState& state, const InputMapping* mappings, size_t mapCount, const KeyboardSim& kb) {
    state.previous = state.current;
    for (size_t i = 0; i < mapCount; ++i) {
        state.current[static_cast<size_t>(mappings[i].action)] = kb.IsDown(mappings[i].key);
    }
}

bool IsPressed(const InputState& state, Action a) {
    size_t idx = static_cast<size_t>(a);
    return state.current[idx] && !state.previous[idx];
}

bool IsReleased(const InputState& state, Action a) {
    size_t idx = static_cast<size_t>(a);
    return !state.current[idx] && state.previous[idx];
}

bool IsHeld(const InputState& state, Action a) {
    return state.current[static_cast<size_t>(a)];
}

// ============= TESTS =============
const InputMapping GAME_MAPPINGS[] = {
    {'A', Action::MoveLeft},
    {'D', Action::MoveRight},
    {' ', Action::Jump},
    {'J', Action::Attack},
    {27, Action::Pause},
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
    kb.Press(' ');
    UpdateInput(state, GAME_MAPPINGS, 5, kb);
    ASSERT_TRUE(IsPressed(state, Action::Jump));
    ASSERT_TRUE(IsHeld(state, Action::Jump));
    UpdateInput(state, GAME_MAPPINGS, 5, kb);
    ASSERT_FALSE(IsPressed(state, Action::Jump));
    ASSERT_TRUE(IsHeld(state, Action::Jump));
}

TEST_CASE(ReleasedOnKeyUp) {
    InputState state;
    KeyboardSim kb;
    kb.Press('D');
    UpdateInput(state, GAME_MAPPINGS, 5, kb);
    UpdateInput(state, GAME_MAPPINGS, 5, kb);
    kb.Release('D');
    UpdateInput(state, GAME_MAPPINGS, 5, kb);
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
    kb.Press(27);
    UpdateInput(state, GAME_MAPPINGS, 5, kb);
    ASSERT_TRUE(IsPressed(state, Action::Pause));
}

int main() {
    std::cout << "Solución 07 (Input Mapping)...\n";
    return RUN_ALL_TESTS();
}