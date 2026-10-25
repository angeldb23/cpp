/* =============================================================================
   MÓDULO 06 - SOLUCIÓN 11: Dos bases, un mismo nombre: desambiguar
   =============================================================================
   Claves de la lección 14:
   - El nombre repetido no es un error: es la AMBIGÜEDAD que la derivada
     tiene que resolver (calificando por base).
   - Los upcasts a cada base conviven; no "se pisan".
   ============================================================================= */
#include <iostream>
#include <string>
#include "../../include/test_framework.hpp"

class Arquero {
public:
    virtual ~Arquero() = default;
    std::string presentarse() const { return "arquero"; }
};

class Vigia {
public:
    virtual ~Vigia() = default;
    std::string presentarse() const { return "vigia"; }
};

class Explorador : public Arquero, public Vigia {
public:
    // La derivada DECIDE: califica a cada base y arma su propia respuesta.
    std::string presentarse() const
    {
        return "explorador(" + Arquero::presentarse() + "+" +
               Vigia::presentarse() + ")";
    }
};

TEST_CASE(TestAmbiguoResueltoPorLaDerivada) {
    Explorador e;
    ASSERT_EQ(e.presentarse(), std::string("explorador(arquero+vigia)"));
}

TEST_CASE(TestUpcastACadaBase) {
    Explorador e;
    const Arquero* a = &e;
    const Vigia*   v = &e;
    ASSERT_EQ(a->presentarse(), std::string("arquero"));
    ASSERT_EQ(v->presentarse(), std::string("vigia"));
}

TEST_CASE(TestCalificadaSiempreFunciona) {
    Explorador e;
    ASSERT_EQ(e.Arquero::presentarse(), std::string("arquero"));
    ASSERT_EQ(e.Vigia::presentarse(), std::string("vigia"));
}

int main() {
    std::cout << "Solucion Ejercicio 11: herencia multiple.\n";
    return RUN_ALL_TESTS();
}
