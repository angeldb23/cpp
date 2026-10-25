/* =============================================================================
   MÓDULO 16 - SOLUCIÓN 04: Game States — Pila de Estados
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

    void Push(std::unique_ptr<IGameState> state) {
        if (!stack_.empty()) stack_.back()->Exit();
        stack_.push_back(std::move(state));
        stack_.back()->Enter();
    }

    void Pop() {
        if (!stack_.empty()) {
            stack_.back()->Exit();
            stack_.pop_back();
            if (!stack_.empty()) stack_.back()->Enter();
        }
    }

    void Change(std::unique_ptr<IGameState> state) {
        Pop();
        Push(std::move(state));
    }

    IGameState* Current() {
        return stack_.empty() ? nullptr : stack_.back().get();
    }
    bool Empty() const {
        return stack_.empty();
    }
};

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
    ASSERT_EQ(sm.Current()->Name(), "A");
}

TEST_CASE(PushSecondCallsExitThenEnter) {
    StateManager sm;
    int enter = 0, exit = 0; float upd = 0;
    sm.Push(std::make_unique<TestState>("A", &enter, &exit, &upd));
    sm.Push(std::make_unique<TestState>("B", &enter, &exit, &upd));
    ASSERT_EQ(enter, 2);
    ASSERT_EQ(exit, 1);
    ASSERT_EQ(sm.Current()->Name(), "B");
}

TEST_CASE(PopCallsExitAndResumesPrevious) {
    StateManager sm;
    int enter = 0, exit = 0; float upd = 0;
    sm.Push(std::make_unique<TestState>("A", &enter, &exit, &upd));
    sm.Push(std::make_unique<TestState>("B", &enter, &exit, &upd));
    sm.Pop();
    ASSERT_EQ(exit, 2);
    ASSERT_EQ(enter, 3);
    ASSERT_EQ(sm.Current()->Name(), "A");
}

TEST_CASE(ChangeReplacesTop) {
    StateManager sm;
    int enter = 0, exit = 0; float upd = 0;
    sm.Push(std::make_unique<TestState>("A", &enter, &exit, &upd));
    sm.Change(std::make_unique<TestState>("C", &enter, &exit, &upd));
    ASSERT_EQ(exit, 1);
    ASSERT_EQ(enter, 2);
    ASSERT_EQ(sm.Current()->Name(), "C");
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
    sm.Current()->Update(0.016f);
    sm.Current()->Update(0.016f);
    ASSERT_NEAR(upd, 0.032f, 0.001f);
}

int main() {
    std::cout << "Solución 04 (Game States Stack)...\n";
    return RUN_ALL_TESTS();
}