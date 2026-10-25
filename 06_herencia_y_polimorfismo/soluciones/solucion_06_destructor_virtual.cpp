/* =============================================================================
   MÓDULO 06 - SOLUCIÓN 06: Destructor virtual: el delete que no avisaba
   =============================================================================
   Claves de la lección 07:
   - Una base con destrucción por puntero: dtor virtual ALWAYS.
   - La palabra `virtual` en el dtor habilita el "deleting dtor": el
     delete del tamano correcto del tipo real.
   - Orden de dtors: primero el derivado, luego la base.
   ============================================================================= */
#include <iostream>
#include <string>
#include <vector>
#include "../../include/test_framework.hpp"

std::vector<std::string>& registroDtor() {
    static std::vector<std::string> r;
    return r;
}

class Cofre {
public:
    virtual ~Cofre() {                    // FIX: virtual -> delete por base
        registroDtor().push_back("~Cofre");
    }
    // Ya no es UB: el dtor del tipo real corre y destruye TODO el objeto.
};

class CofreTesoro : public Cofre {
public:
    ~CofreTesoro() override {             // override deja claro que sobrescribe
        registroDtor().push_back("~CofreTesoro");
    }
};

TEST_CASE(TestDtorPorBase) {
    Cofre* c = new CofreTesoro{};
    registroDtor().clear();
    delete c;
    ASSERT_EQ(registroDtor().size(), std::size_t(2));
    ASSERT_EQ(registroDtor()[0], std::string("~CofreTesoro"));
    ASSERT_EQ(registroDtor()[1], std::string("~Cofre"));
}

TEST_CASE(TestDtorPorTipoReal) {
    registroDtor().clear();
    { CofreTesoro local{}; }
    ASSERT_EQ(registroDtor().size(), std::size_t(2));
}

int main() {
    std::cout << "Solucion Ejercicio 06: dtor virtual.\n";
    return RUN_ALL_TESTS();
}
