/* =============================================================================
   MÓDULO 06 - EJERCICIO 06: Destructor virtual: el delete que no avisaba
   =============================================================================
   DIFICULTAD: Medio
   PRACTICA: lección 07 (por qué la base polimórfica necesita dtor virtual).

   OJO: este ejercicio CONTIENE UB a propósito (el dtor no virtual).
   Comportamiento típico sin sanitizer: el dtor derivado no se llama y
   el registro queda cojo. Con -fsanitize=address verás
   "new-delete-type-mismatch" al correr el esqueleto, en el epicentro
   del problema. La SOLUCIÓN deja de tener UB.

   CONSIGNA:
   La base `Cofre` está DADA con un BUG (su destructor no es virtual:
   la línea está comentada tal cual va la versión correcta).

     1. Arreglalo: hacé el destructor de `Cofre` VIRTUAL.
     2. El destructor derivado `~CofreTesoro()` ya hace su trabajo:
        registramos la línea "~CofreTesoro" y después "~Cofre" (por
        orden inverso automático). Los tests esperan EXACTAMENTE:
        ["~CofreTesoro", "~Cofre"] al hacer `delete` POR LA BASE.
     3. Ojo con la firma del dtor derivado: si lo anotás bien, también
        conviene dejar explícito que sobrescribe: usá `override` en
        `~CofreTesoro()`.

   COMPILACIÓN:
     g++ -std=c++20 -Wall -Wextra -I../../include \
         ejercicio_06_destructor_virtual.cpp -o ej06 && ./ej06
   ============================================================================= */
#include <iostream>
#include <string>
#include <vector>
#include "../../include/test_framework.hpp"

std::vector<std::string>& registroDtor() {
    static std::vector<std::string> r;
    return r;
}

// ---------------------------------------------------------------------------
// Base DADA, con el bug de producción: dtor SIN virtual.
// ---------------------------------------------------------------------------
class Cofre {
public:
    // TODO 1: hacé este destructor VIRTUAL (una palabra):
    ~Cofre() {
        // con UB lo corregido: registro de la base al destruirse
        registroDtor().push_back("~Cofre");
    }
    // (sin virtual, `delete cofreBase` NO llega a ~CofreTesoro: UB)
};

class CofreTesoro : public Cofre {
public:
    ~CofreTesoro() {
        registroDtor().push_back("~CofreTesoro");
        // TODO 3: al agregar el override te asegurás que el dtor base ES virtual
    }
};

// ============= TESTS =============
TEST_CASE(TestDtorPorBase) {
    Cofre* c = new CofreTesoro{};
    registroDtor().clear();
    delete c;                                   // ¿llama al derivado?
    ASSERT_EQ(registroDtor().size(), std::size_t(2));
    ASSERT_EQ(registroDtor()[0], std::string("~CofreTesoro"));   // primero el real
    ASSERT_EQ(registroDtor()[1], std::string("~Cofre"));         // luego la base
}

TEST_CASE(TestDtorPorTipoReal) {
    registroDtor().clear();
    { CofreTesoro local{}; }                    // dtor directo: siempre funciona
    ASSERT_EQ(registroDtor().size(), std::size_t(2));
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 06...\n";
    return RUN_ALL_TESTS();
}
