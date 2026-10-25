/* =============================================================================
   MÓDULO 17 - SOLUCIÓN 05: AnimatedStateMachine — Transiciones con Condición
   =============================================================================
   Resuelve ejercicio_05_fsm.cpp. La clave de la FSM de la lección 03 es el
   ORDEN de las transiciones: la primera condición verdadera gana (por eso
   "Hurt" registrada primero reemplaza a "Attack" que vendría después).
   Los dead-ends (estado sin transición de salida) vuelven a sí mismas.

   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra solucion_05_fsm.cpp -o sol && ./sol
   ============================================================================= */
#include <string>
#include <vector>
#include <functional>
#include <iostream>
#include "../../include/test_framework.hpp"

// ============================ TIPOS (no tocar) ===============================
struct Transition {
    std::string from;
    std::string to;
    std::function<bool()> cond;
};

struct RC {
    bool moving = false;
    bool attacking = false;
    bool attackDone = false;
    bool hurt = false;
};

// ============================ SOLUCIÓN =======================================
std::string FSMUpdate(const std::vector<Transition>& trans,
                      const RC& rc, const std::string& current) {
    // TODO: solución. Recorre EN ORDEN (mismo orden de la lista = prioridad).
    // (rc queda expuesto en la firma para condiciones externas al contexto)
    (void)rc;
    for (const auto& t : trans) {
        if (t.from != current) continue;   // no aplica a este estado
        if (!t.cond()) continue;           // la condición dio false
        return t.to;                       // LA PRIMERA que dispara gana
    }
    return current;   // dead-end o ninguna condición verdadera: mantiene
}

// Transiciones canónicas de los tests:
std::vector<Transition> CanonTransitions(const RC& rc) {
    std::vector<Transition> t;
    t.push_back({"Idle",    "Hurt",   [&rc]{ return rc.hurt; }});
    t.push_back({"Walk",    "Hurt",   [&rc]{ return rc.hurt; }});
    t.push_back({"Attack",  "Hurt",   [&rc]{ return rc.hurt; }});
    t.push_back({"Idle",    "Attack", [&rc]{ return rc.attacking; }});
    t.push_back({"Walk",    "Attack", [&rc]{ return rc.attacking; }});
    t.push_back({"Attack",  "Idle",   [&rc]{ return rc.attackDone && !rc.attacking; }});
    t.push_back({"Idle",    "Walk",   [&rc]{ return rc.moving; }});
    t.push_back({"Walk",    "Idle",   [&rc]{ return !rc.moving; }});
    return t;
}

// ============================ TESTS (iguales) ================================
TEST_CASE(IdleAWalkConMovimiento) {
    RC rc; rc.moving = true;
    auto trans = CanonTransitions(rc);
    ASSERT_EQ(FSMUpdate(trans, rc, "Idle"), "Walk");
}

TEST_CASE(WalkAIdleSinMovimiento) {
    RC rc; rc.moving = false;
    auto trans = CanonTransitions(rc);
    ASSERT_EQ(FSMUpdate(trans, rc, "Walk"), "Idle");
}

TEST_CASE(AttackInterrumpeHurtTienePrioridad) {
    RC rc; rc.attacking = true; rc.hurt = true;
    auto trans = CanonTransitions(rc);
    ASSERT_EQ(FSMUpdate(trans, rc, "Idle"), "Hurt");
}

TEST_CASE(AttackNoSaleSinAttackDone) {
    RC rc; rc.attacking = true; rc.attackDone = false;
    auto trans = CanonTransitions(rc);
    ASSERT_EQ(FSMUpdate(trans, rc, "Attack"), "Attack");
}

TEST_CASE(AttackSaleConAttackDone) {
    RC rc; rc.attacking = false; rc.attackDone = true;
    auto trans = CanonTransitions(rc);
    ASSERT_EQ(FSMUpdate(trans, rc, "Attack"), "Idle");
}

TEST_CASE(StateMuertoEsDeadEnd) {
    RC rc; rc.moving = true;
    auto trans = CanonTransitions(rc);
    ASSERT_EQ(FSMUpdate(trans, rc, "GameOver"), "GameOver");
}

int main() {
    std::cout << "Solución 05: AnimatedStateMachine\n";
    return RUN_ALL_TESTS();
}
