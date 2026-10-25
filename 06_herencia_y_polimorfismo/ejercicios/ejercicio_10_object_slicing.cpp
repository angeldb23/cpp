/* =============================================================================
   MÓDULO 06 - EJERCICIO 10: Object slicing: la firma que rebana al boss
   =============================================================================
   DIFICULTAD: Medio
   PRACTICA: lección 13 (pasar por valor REBANA; por referencia o puntero
   conserva el polimorfismo).

   CONSIGNA:
   La jerarquía está DADA (no la toques). Hay DOS funciones con firma
   mala: tu trabajo es CAMBIAR FIRMAS (concretamente, nada de tocar la
   jerarquía) para que el polimorfismo sobreviva:

   1. `int atacar(Enemigo e)` (por valor): el parámetro se COPIA a un
      Enemigo y el boss queda rebanado. Cambiá la firma a
      `int atacar(const Enemigo& e)` y que devuelva `e.danoBase()`
      (el mismo cuerpo con la firma buena).
   2. `int danioTotal(std::vector<Enemigo> lista)`: el copia del
      argumento rebana a todos y recorre la parte base. Cambiala a
      `int danioTotal(const std::vector<Enemigo*>& lista)` y recorré
      punteros (los tests TE PASAN un vector<Enemigo*>).
   3. El clip final: con la firma por valor, el mismo test pasa con
      "danios chicos" (1, etc.): el bug es SILENCIOSO. Con la firma
      por referencia, el polimorfismo corre y el boss vuelve a pegar.

   PISTA: es un ejercicio de firmas: la solución es una línea por
   función, más el cuerpo que recorre punteros en el segundo caso.

   COMPILACIÓN:
     g++ -std=c++20 -Wall -Wextra -I../../include \
         ejercicio_10_object_slicing.cpp -o ej10 && ./ej10
   ============================================================================= */
#include <iostream>
#include <vector>
#include "../../include/test_framework.hpp"

// ---------------------------------------------------------------------------
// Jerarquía DADA (no la toques).
// ---------------------------------------------------------------------------
class Enemigo {
public:
    virtual ~Enemigo() = default;
    virtual int danoBase() const { return 1; }
};

class Zombi : public Enemigo {
public:
    int danoBase() const override { return 4; }
};

class Boss : public Enemigo {
public:
    int danoBase() const override { return 90; }
};

// ---------------------------------------------------------------------------
// FIRMAS ROTAS (cambiá firmas; el cuerpo cuerpo igual mínimo).
// ---------------------------------------------------------------------------
int atacar(Enemigo e) {                       // TODO 1: const Enemigo& e
    return e.danoBase();
}

int danioTotal(std::vector<Enemigo> lista) {  // TODO 2: ELIMINÁ esta versión
    int total = 0;
    for (const auto& e : lista) total += e.danoBase();
    return total;
}

// TODO 2 (firma buena): los tests usan ESTA; completá el cuerpo real
// (sumar danoBase() recorriendo punteros; hoy es placeholder):
int danioTotal(const std::vector<Enemigo*>& lista) {
    (void)lista;
    return -1;
}

// ============= TESTS =============
TEST_CASE(TestAtacarConservaPolimorfismo) {
    Boss boss;
    Zombi  z;
    ASSERT_EQ(atacar(boss), 90);   // con la firma por valor: 1 (rebanado)
    ASSERT_EQ(atacar(z), 4);
}

TEST_CASE(TestDanioTotalConPunteros) {
    Boss boss;
    Zombi  z1;
    Zombi  z2;
    std::vector<Enemigo*> ronda = {&boss, &z1, &z2};
    ASSERT_EQ(danioTotal(ronda), 98);          // 90 + 4 + 4
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 10...\n";
    return RUN_ALL_TESTS();
}
