/* =============================================================================
   MÓDULO 06 - EJERCICIO 11: Dos bases, un mismo nombre: desambiguar
   =============================================================================
   DIFICULTAD: Medio
   PRACTICA: lección 14 (herencia múltiple: dos subobjetos, ambigüedad
   de nombres, upcast a cada base).

   CONSIGNA:
   Las dos bases están DADAS y tienen EL MISMO nombre de método:
   `presentarse()`. `Explorador` hereda de las dos.

   1. Completá `Explorador::presentarse()` para que devuelva
      "explorador(arquero+vigia)". Así la derivada DECIDE y el llamador
      no tiene que calificar: la desambiguación dentro de la derivada
      (llamando a cada base por su nombre calificado) es la manera
      limpia de resolverla.
   2. Los tests también comprueban los upcasts a cada base: si el
      esqueleto hereda bien, funcionan solos.
   3. NO cambies las bases: la ambigüedad es el punto del ejercicio.

   COMPILACIÓN:
     g++ -std=c++20 -Wall -Wextra -I../../include \
         ejercicio_11_herencia_multiple.cpp -o ej11 && ./ej11
   ============================================================================= */
#include <iostream>
#include <string>
#include "../../include/test_framework.hpp"

// Bases dadas (no las toques): OJO, mismo nombre de método.
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

// ---------------------------------------------------------------------------
// La derivada con dos bases: el nombre repetido genera ambigüedad.
// ---------------------------------------------------------------------------
class Explorador : public Arquero, public Vigia {
public:
    // TODO 1: desambiguá DENTRO de la derivada:
    //         Arquero::presentarse() y Vigia::presentarse() formateadas.
    std::string presentarse() const
    {
        return "placeholder";                  // PLACEHOLDER
    }
};

// ============= TESTS =============
TEST_CASE(TestAmbiguoResueltoPorLaDerivada) {
    Explorador e;
    ASSERT_EQ(e.presentarse(), std::string("explorador(arquero+vigia)"));
}

TEST_CASE(TestUpcastACadaBase) {
    Explorador e;
    const Arquero* a = &e;      // primera base
    const Vigia*   v = &e;      // segunda base
    ASSERT_EQ(a->presentarse(), std::string("arquero"));
    ASSERT_EQ(v->presentarse(), std::string("vigia"));
}

TEST_CASE(TestCalificadaSiempreFunciona) {
    Explorador e;
    ASSERT_EQ(e.Arquero::presentarse(), std::string("arquero"));
    ASSERT_EQ(e.Vigia::presentarse(), std::string("vigia"));
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 11...\n";
    return RUN_ALL_TESTS();
}
