/* =============================================================================
   MÓDULO 06 - EJERCICIO 03: El turnero de acciones: despacho dinámico
   =============================================================================
   DIFICULTAD: Básico
   PRACTICA: lección 04 (virtual, despacho por tipo dinámico, colección de
   punteros a la base).

   CONSIGNA:
   La base `Accion` y el registro `secuencia()` están DADOS.

     1. Completá las tres derivadas (los ataques de la ronda):
        - `Golpe`     : `simular()` devuelve "golpe"  (dano 5).
        - `Flecha`    : `simular()` devuelve "flecha" (dano 3).
        - `Burla`     : `simular()` devuelve "burla"  (dano 0).
        En TODAS: `esValida()` true, y el `dano()` de cada clase propio.
        Marcá siempre `override` (¿falta algo en la firma? revisá).
     2. La función `rondaBase()` recibe el vector de acciones y debe
        SIMULAR TODAS a través del PUNTERO a Accion (sin dynamic_cast):
        concatena los resultados separados por "-" y suma el dano.
        Al final se usa para el string "golpe-flecha-pausa" del test.
     3. Ocultamiento polimórfico: Burla redefine `dano()` para devolver 0
        PERO el test exige que a través de `Accion*` el valor respetado
        sea el de la BURLA (despacho dinámico, no de la base).

   COMPILACIÓN:
     g++ -std=c++20 -Wall -Wextra -I../../include \
         ejercicio_03_despacho_dinamico.cpp -o ej03 && ./ej03
   ============================================================================= */
#include <iostream>
#include <string>
#include <vector>
#include "../../include/test_framework.hpp"

// Base dada (no la toques).
class Accion {
public:
    virtual ~Accion() = default;
    virtual std::string simular() const = 0;
    virtual int dano() const { return -1; }       // sentinela "sin dano"
    virtual bool esValida() const { return false; }
};

// ---------------------------------------------------------------------------
// Tus derivadas.
// ---------------------------------------------------------------------------
class Golpe : public Accion {
public:
    // TODO: "golpe" | dano 5 | esValida true
    std::string simular() const override { return "placeholder"; }
    int dano() const override { return -1; }      // PLACEHOLDER
    bool esValida() const override { return false; }
};

class Flecha : public Accion {
public:
    // TODO: "flecha" | dano 3 | esValida true
    std::string simular() const override { return "placeholder"; }
    int dano() const override { return -1; }      // PLACEHOLDER
    bool esValida() const override { return false; }
};

class Burla : public Accion {
public:
    // TODO: "burla" | dano 0 | esValida true
    std::string simular() const override { return "placeholder"; }
    int dano() const override { return -1; }      // PLACEHOLDER
    bool esValida() const override { return false; }
};

// ---------------------------------------------------------------------------
// TODO 2: simular TODAS como Accion* y armar el resumen.
// ---------------------------------------------------------------------------
std::string rondaBase(const std::vector<Accion*>& acciones) {
    (void)acciones;
    return "todo";                                // PLACEHOLDER
}

// ============= TESTS =============
TEST_CASE(TestRondaEntera) {
    Golpe g;
    Flecha  f;
    Burla   b;
    std::vector<Accion*> ronda = {&g, &f, &b};
    ASSERT_EQ(rondaBase(ronda), std::string("golpe-flecha-burla"));
}

TEST_CASE(TestDanoPorPunteroBase) {
    Golpe g;
    Flecha  f;
    Burla   b;
    std::vector<Accion*> ronda = {&g, &f, &b};
    int total = 0;
    for (const Accion* a : ronda) total += a->dano();
    ASSERT_EQ(total, 8);                          // 5 + 3 + 0 (dinamico)
}

TEST_CASE(TestValidas) {
    Golpe g;
    Flecha  f;
    Burla   b;
    ASSERT_TRUE(g.esValida());
    ASSERT_TRUE(f.esValida());
    ASSERT_TRUE(b.esValida());
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 03...\n";
    return RUN_ALL_TESTS();
}
