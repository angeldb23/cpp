/* =============================================================================
   MÓDULO 06 - EJERCICIO 04: override y final: la firma que no coincidía
   =============================================================================
   DIFICULTAD: Medio
   PRACTICA: lección 05 (override, final, sobrescrituras fallidas).

   CONSIGNA:
   Este ejercicio tiene un BUG YA PLANTADO (como en producción):
   `DanioFuerte::valor()` se escribió SIN `const`. La clase compila igual
   (no es una sobrescritura: es una función nueva que oculta a la base),
   pero a través de un `Estadistica*` el polimorfismo no corre y los
   tests ven el valor de la BASE: ese es el fix pendiente.

     1. Corregí `DanioFuerte`:
        - agregá `const` a la firma (la base lo exige),
        - anotá `override` para que el compilador te confirme
          que ES una sobrescritura.
     2. `GuiaTuristica::nombre()` ya tiene la firma correcta, pero está
        sin anotar: agregá únicamente `override` al final de la firma.
     3. `EstadisticaTexto` está marcada como `final` (nadie puede heredar
        de ella). Completá su `valor()` reutilizándolo: tiene que
        devolver 7 y llevar `override`.

   COMPILACIÓN:
     g++ -std=c++20 -Wall -Wextra -I../../include \
         ejercicio_04_override_y_final.cpp -o ej04 && ./ej04
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

// ---------------------------------------------------------------------------
// BUG PLANTADO: sin `const` no sobrescribe (sin `override` no avisa).
// ---------------------------------------------------------------------------
class DanioFuerte : public Estadistica {
public:
    int valor() { return 99; }        // TODO 1: const + override
};

// ---------------------------------------------------------------------------
// Firma correcta, anotación ausente.
// ---------------------------------------------------------------------------
class GuiaTuristica : public Estadistica {
public:
    std::string nombre() const { return "guia"; }   // TODO 2: solo override
};

// ---------------------------------------------------------------------------
// Clase final: cierra la jerarquía por debajo.
// ---------------------------------------------------------------------------
class EstadisticaTexto final : public Estadistica {
public:
    std::string nombre() const override { return "texto"; }

    // TODO 3: int valor() const override -> devuelve 7
    int valor() const { return 1; }                 // PLACEHOLDER
};

// ============= TESTS =============
TEST_CASE(TestValorPolimórfico) {
    DanioFuerte fuerte;
    Estadistica* e = &fuerte;
    // Si la firma quedó mal, la BASE responde 1: ahí estaba el bug.
    ASSERT_EQ(e->valor(), 99);
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
    ASSERT_EQ(e->valor(), 7);        // el override propio de la clase final
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 04...\n";
    return RUN_ALL_TESTS();
}
