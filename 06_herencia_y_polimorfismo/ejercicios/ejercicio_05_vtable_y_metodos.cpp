/* =============================================================================
   MÓDULO 06 - EJERCICIO 05: La mesa de botones: qué responde cada llamada
   =============================================================================
   DIFICULTAD: Medio
   PRACTICA: lección 06 (vtable/vptr, llamada directa vs indirecta,
   methods estáticos vs virtuales).

   CONSIGNA:
   La base `Mesa` está DADA con una llamada NO virtual ("rueda()") y una
   virtual ("resuelve()"). Completá la derivada `Segadora` y el test de
   costos:

     1. `Segadora::resuelve()` -> "segadora" (override correcto).
     2. `Segadora::rueda()` existe (igual firma) pero NO es virtual:
        a través de un `Mesa*`, la respuesta SIEMPRE será de la base
        (despacho estático). El test te comprueba ese comportamiento.
     3. Test de memoria: pese a ser derivada, sizeof(Segadora) no debería
        superar sizeof(Mesa): el vptr ya está en la base y la derivada
        no agrega datos. (si agregás atributos el test falla por diseño)

   Ayuda mental: la llamada virtual mira el vptr del OBJETO; la no
   virtual mira el TIPO de la variable/la expresión. El caso 2 es el
   recordatorio de por qué el polimorfismo no es automático.

   COMPILACIÓN:
     g++ -std=c++20 -Wall -Wextra -I../../include \
         ejercicio_05_vtable_y_metodos.cpp -o ej05 && ./ej05
   ============================================================================= */
#include <iostream>
#include <string>
#include "../../include/test_framework.hpp"

// Base dada (no la toques).
class Mesa {
public:
    virtual ~Mesa() = default;

    std::string rueda() const {          // NO virtual: siempre la de la base
        return "mesa";
    }
    virtual std::string resuelve() const {
        return "mesa";
    }
};

// ---------------------------------------------------------------------------
// Tu derivada.
// ---------------------------------------------------------------------------
class Segadora : public Mesa {
public:
    // TODO 2: redefiní rueda() igual que la base (pero sin virtual:
    //         en la DERIVADA la palabra no hace falta; el punto es que
    //         el PADRE no la puso).
    // std::string rueda() const { return "segadora"; }

    // TODO 1: resuelve() -> "segadora"
    std::string resuelve() const override { return "mesa"; }  // PLACEHOLDER
};

// ============= TESTS =============
TEST_CASE(TestResuelveDinamico) {
    Segadora s;
    Mesa* m = &s;
    ASSERT_EQ(m->resuelve(), std::string("segadora"));   // mira el objeto
}

TEST_CASE(TestRuedaEstatico) {
    Segadora s;
    Mesa* m = &s;
    // NO virtual: mira el TIPO de la variable (Mesa*), no el objeto:
    ASSERT_EQ(m->rueda(), std::string("mesa"));
}

TEST_CASE(TestRuedaDirecto) {
    Segadora s;
    // por la derivada directa: es SU versión
    ASSERT_EQ(s.rueda(), std::string("segadora"));
}

TEST_CASE(TestCostoMemoria) {
    // La derivada no agrega datos: el vptr ya venía con la base.
    ASSERT_EQ(sizeof(Segadora), sizeof(Mesa));
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 05...\n";
    return RUN_ALL_TESTS();
}
