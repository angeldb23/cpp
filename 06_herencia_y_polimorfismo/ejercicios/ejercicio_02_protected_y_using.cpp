/* =============================================================================
   MÓDULO 06 - EJERCICIO 02: protected, name hiding y `using`
   =============================================================================
   DIFICULTAD: Básico
   PRACTICA: lección 03 (protected, ocultamiento de nombres con `using`,
   heredando constructores con `using Base::Base;`).

   CONSIGNA:
   La base `Escaner` está DADA (no la toques). Tiene dos atributos
   protected y dos sobrecargas de `leer`.

     1. Completá la derivada `EscanerVisor`:
        a) Constructor: usá `using Escaner::Escaner;` para heredar el
           constructor del padre (una sola línea; no escribas un ctor).
           PISTA: el `using` va en el scope público de la derivada.
        b) `void calibrar()`: un método de la derivada queLEE el dato
           protected `sensibilidad` (solo puede, por ser protected) y lo
           deja en 100 si ya no lo está. Guardá el resultado en
           `sensibilidad`.
        c) Ocultamiento: la derivada define su propia
           `std::string leer(const char* etiqueta)`.
           Con esa definición, las `leer` del padre QUEDAN OCULTAS:
           agregá el `using Escaner::leer;` para revivirlas.
   La base marca registro en `historialLeidos()` cada vez que alguien
   llama a `leer`: los tests lo usan para verificar qué versión corre.

   COMPILACIÓN:
     g++ -std=c++20 -Wall -Wextra -I../../include \
         ejercicio_02_protected_y_using.cpp -o ej02 && ./ej02
   ============================================================================= */
#include <iostream>
#include <string>
#include <vector>
#include "../../include/test_framework.hpp"

class Escaner {
protected:
    int sensibilidad = 50;
    int canales = 4;

public:
    Escaner(int sens_, int can_) : sensibilidad(sens_), canales(can_) {}
    virtual ~Escaner() = default;

    std::string leer(int canal) const {
        historial().push_back("Esc::leer(int)");
        return "canal" + std::to_string(canal);
    }
    std::string leer(const char* etiqueta) const {
        historial().push_back("Esc::leer(const char*)");
        return std::string("marca:") + etiqueta;
    }
    int getSensibilidad() const { return sensibilidad; }

    static std::vector<std::string>& historial() {
        static std::vector<std::string> h;
        return h;
    }
};

// ---------------------------------------------------------------------------
// Tu clase derivada.
// ---------------------------------------------------------------------------
class EscanerVisor : public Escaner {
public:
    // TODO 1a: reemplazá ESTE constructor manual por `using Escaner::Escaner;`
    //          (el using debe quedar en el scope público de la derivada).
    EscanerVisor(int sens_, int can_) : Escaner(sens_, can_) {}  // PLACEHOLDER

    // TODO 1c: reviví las sobrecargas de `leer` del padre con using.
    //          (va DESPUÉS de definir tu propia `leer` para que el
    //          using no tape el ocultamiento que vos creaste.)

    // Definición propia de la derivada (ya está: no la borres):
    std::string leer(const char* etiqueta) {
        historial().push_back("Visor::leer(const char*)");
        return "visor[" + std::string(etiqueta) + "]";
    }

    // TODO 1c (fase 1): esta sobrecarga es un PLACEHOLDER que hace que el
    // ejercicio compile sin el using. Cuando agregues `using Escaner::leer;`,
    // BORRÁ esta función por completo (el using la trae del padre).
    std::string leer(int canal) {
        historial().push_back("Visor::placeholder(int)");
        (void)canal;
        return "placeholder";
    }

    // TODO 1b: calibrar() aclimata el dato protected:
    void calibrar() {
        // TODO: si sensibilidad != 100, dejarla en 100. (placeholder: nada)
    }
};

// ============= TESTS =============
TEST_CASE(TestCtorHeredado) {
    EscanerVisor v(1, 2);                 // funciona solo con el using heredado
    ASSERT_EQ(v.getSensibilidad(), 1);
    ASSERT_EQ(v.leer(1), std::string("canal1"));   // sobrecarga del padre viva
}

TEST_CASE(TestSobrecargasRevividas) {
    Escaner::historial().clear();
    EscanerVisor v(60, 6);
    ASSERT_EQ(v.leer(3), std::string("canal3"));
    ASSERT_EQ(v.leer("radar"), std::string("visor[radar]"));
    ASSERT_EQ(Escaner::historial()[0], std::string("Esc::leer(int)"));
    ASSERT_EQ(Escaner::historial()[1], std::string("Visor::leer(const char*)"));
}

TEST_CASE(TestCalibrar) {
    EscanerVisor v(20, 2);
    v.calibrar();
    ASSERT_EQ(v.getSensibilidad(), 100);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 02...\n";
    return RUN_ALL_TESTS();
}
