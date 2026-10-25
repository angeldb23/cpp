/* =============================================================================
   MÓDULO 06 - SOLUCIÓN 03: El turnero de acciones: despacho dinámico
   =============================================================================
   Claves de la lección 04:
   - Cada derivada REDEFINE `simular()` con override y el despacho corre
     por el tipo REAL del objeto.
   - La función genérica solo conoce `Accion*`, pero trabaja bien: es el
     punto completo del polimorfismo.
   ============================================================================= */
#include <iostream>
#include <string>
#include <vector>
#include "../../include/test_framework.hpp"

class Accion {
public:
    virtual ~Accion() = default;
    virtual std::string simular() const = 0;
    virtual int dano() const { return -1; }
    virtual bool esValida() const { return false; }
};

class Golpe : public Accion {
public:
    std::string simular() const override { return "golpe"; }
    int dano() const override { return 5; }
    bool esValida() const override { return true; }
};

class Flecha : public Accion {
public:
    std::string simular() const override { return "flecha"; }
    int dano() const override { return 3; }
    bool esValida() const override { return true; }
};

class Burla : public Accion {
public:
    std::string simular() const override { return "burla"; }
    int dano() const override { return 0; }   // la burla hiere: 0
    bool esValida() const override { return true; }
};

std::string rondaBase(const std::vector<Accion*>& acciones) {
    std::string resumen;
    bool primero = true;
    for (const Accion* a : acciones) {        // puntero base: polimorfismo
        if (!primero) resumen += "-";
        resumen += a->simular();              // version DINAMICA
        primero = false;
    }
    return resumen;
}

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
    ASSERT_EQ(total, 8);
}

TEST_CASE(TestValidas) {
    Golpe g; Flecha f; Burla b;
    ASSERT_TRUE(g.esValida());
    ASSERT_TRUE(f.esValida());
    ASSERT_TRUE(b.esValida());
}

int main() {
    std::cout << "Solucion Ejercicio 03: despacho dinamico.\n";
    return RUN_ALL_TESTS();
}
