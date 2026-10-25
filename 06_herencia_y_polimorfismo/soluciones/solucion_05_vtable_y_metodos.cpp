/* =============================================================================
   MÓDULO 06 - SOLUCIÓN 05: La mesa de botones: qué responde cada llamada
   =============================================================================
   Claves de la lección 06:
   - La llamada VIRTUAL mira el vptr del objeto: resuelve por tipo real.
   - La NO virtual mira el tipo de la expresión: siempre la base.
   - Tamaño: una jerarquía sin datos extra no agranda los objetos (un
     vptr por subobjeto, ya pagado en la base).
   ============================================================================= */
#include <iostream>
#include <string>
#include "../../include/test_framework.hpp"

class Mesa {
public:
    virtual ~Mesa() = default;

    std::string rueda() const {           // NO virtual: llamada directa
        return "mesa";
    }
    virtual std::string resuelve() const {  // virtual: vtable
        return "mesa";
    }
};

class Segadora : public Mesa {
public:
    // Redefinida SIN virtual en la derivada: la palabra "virtual" es
    // HEREDABLE, pero lo que cuenta es que el PADRE no la puso.
    std::string rueda() const { return "segadora"; }

    std::string resuelve() const override { return "segadora"; }
};

TEST_CASE(TestResuelveDinamico) {
    Segadora s;
    Mesa* m = &s;
    ASSERT_EQ(m->resuelve(), std::string("segadora"));
}

TEST_CASE(TestRuedaEstatico) {
    Segadora s;
    Mesa* m = &s;
    ASSERT_EQ(m->rueda(), std::string("mesa"));
}

TEST_CASE(TestRuedaDirecto) {
    Segadora s;
    ASSERT_EQ(s.rueda(), std::string("segadora"));
}

TEST_CASE(TestCostoMemoria) {
    ASSERT_EQ(sizeof(Segadora), sizeof(Mesa));
}

int main() {
    std::cout << "Solucion Ejercicio 05: vtable, despacho y tamaño.\n";
    return RUN_ALL_TESTS();
}
