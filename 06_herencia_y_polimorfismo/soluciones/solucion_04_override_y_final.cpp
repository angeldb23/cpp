/* =============================================================================
   MÓDULO 06 - SOLUCIÓN 04: override y final: la firma que no coincidía
   =============================================================================
   Claves de la lección 05:
   - Sin `const` la firma NO coincide: es una función nueva que oculta
     a la base (bug silencioso). Con `override`, el compilador lo dice.
   - `override` no cambia el código: es una verificación que te cobra
     mentiras de firma.
   ============================================================================= */
#include <iostream>
#include <string>
#include "../../include/test_framework.hpp"

class Estadistica {
public:
    virtual ~Estadistica() = default;
    virtual int  valor() const { return 1; }
    virtual std::string nombre() const { return "estadistica"; }
};

class DanioFuerte : public Estadistica {
public:
    // FIX: ahora sí coincide y el compilador CONFIRMA con override:
    int valor() const override { return 99; }
};

class GuiaTuristica : public Estadistica {
public:
    std::string nombre() const override { return "guia"; }
};

class EstadisticaTexto final : public Estadistica {
public:
    std::string nombre() const override { return "texto"; }
    int valor() const override { return 7; }
};

TEST_CASE(TestValorPolimórfico) {
    DanioFuerte fuerte;
    Estadistica* e = &fuerte;
    ASSERT_EQ(e->valor(), 99);          // ahora SI corre el polimorfismo
}

TEST_CASE(TestNombreConOverride) {
    GuiaTuristica guia;
    Estadistica* e = &guia;
    ASSERT_EQ(e->nombre(), std::string("guia"));
}

TEST_CASE(TestTextoFinal) {
    EstadisticaTexto texto;
    Estadistica* e = &texto;
    ASSERT_EQ(e->nombre(), std::string("texto"));
    ASSERT_EQ(e->valor(), 7);
}

int main() {
    std::cout << "Solucion Ejercicio 04: override y final.\n";
    return RUN_ALL_TESTS();
}
