/* =============================================================================
   MÓDULO 17 - EJERCICIO 05: AnimatedStateMachine — Transiciones con Condición
   =============================================================================
   DIFICULTAD: Intermedio
   APOYO: Lección 03 (Animation State Machine y transiciones)

   CONSIGNA:
   Implementa FSMUpdate: dado el set de transiciones, el estado actual y las
   "condiciones de juego" (moving, attacking, hurt, attackingDone), devuelve
   el estado NUEVO en tiempo actual.

   REGLAS (evaluación EN ORDEN, como la lección 03):
   - Se recorren las transiciones en el orden en que fueron registradas.
   - La primera transición con from == estadoActual y cond() == true
     decide el estado nuevo. Se devuelve INMEDIATAMENTE.
   - Si ninguna aplica, el estado se mantiene.
   - Detalle de prioridad clásico: la transición "Attack -> Idle" solo dispara
     cuando el ataque TERMINÓ (attackDone). La de "Hurt" tiene prioridad
     SIEMPRE que exista (hurt == true) porque aparece ANTES en la lista.
   - Los estados que NO existen como "from" en ninguna transición (ej estado
     "GameOver") no tienen salida: devolver el mismo estado (dead-end).

   TESTS (no tocar): idle->walk, walk->idle, prioridad de hurt y el
   exit-time del ataque (solo sale con attackDone).

   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_05_fsm.cpp -o ej && ./ej
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

// ============================ TU CÓDIGO AQUÍ ================================
// TODO: evalúa transiciones EN ORDEN y devuelve el estado nuevo
std::string FSMUpdate(const std::vector<Transition>& trans,
                      const RC& rc, const std::string& current) {
    // Pistas:
    // 1) recorrer trans en orden
    // 2) if (t.from == current && t.cond()) return t.to;
    // 3) si ninguna dispara: return current;
    return current;   // placeholder
}

// Transiciones canónicas que usan los tests (con prioridad en el orden):
//   Hurt     : cualquier -> Hurt      cond: rc.hurt
//   Attack   : Idle/Walk -> Attack    cond: rc.attacking
//   Idle     : Attack -> Idle        cond: (attackDone && !attacking)
//   Walk     : Idle <-> Walk         cond: moving / !moving
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

// ============================ TESTS (no tocar) ==============================
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
    // hurt aparece ANTES en la lista: gana
    ASSERT_EQ(FSMUpdate(trans, rc, "Idle"), "Hurt");
}

TEST_CASE(AttackNoSaleSinAttackDone) {
    RC rc; rc.attacking = true; rc.attackDone = false;
    auto trans = CanonTransitions(rc);
    ASSERT_EQ(FSMUpdate(trans, rc, "Attack"), "Attack");   // mantiene
}

TEST_CASE(AttackSaleConAttackDone) {
    RC rc; rc.attacking = false; rc.attackDone = true;
    auto trans = CanonTransitions(rc);
    ASSERT_EQ(FSMUpdate(trans, rc, "Attack"), "Idle");
}

TEST_CASE(StateMuertoEsDeadEnd) {
    RC rc; rc.moving = true;
    auto trans = CanonTransitions(rc);
    // "GameOver" no es "from" de ninguna transición: siempre mantiene
    ASSERT_EQ(FSMUpdate(trans, rc, "GameOver"), "GameOver");
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 05 (FSM)...\n";
    return RUN_ALL_TESTS();
}
