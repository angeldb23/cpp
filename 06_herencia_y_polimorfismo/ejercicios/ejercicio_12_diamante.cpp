/* =============================================================================
   MÓDULO 06 - EJERCICIO 12: El diamante: una sola base común
   =============================================================================
   DIFICULTAD: Alta
   PRACTICA: lección 15 (herencia virtual: el diamante compartido).

   CONSIGNA:
   El jerarcita está DADO y es el diamante ROTO (sin herencia virtual):
   la nave tiene DOS nucleos (una copia por rama), `energia` duplicada
   y upcasts a `Nucleo` imposibles (ambiguos).

   Fix:
   1. Marcá la herencia de la base común como VIRTUAL en las bases
      intermedias: `class Motores : public virtual Nucleo` y lo mismo
      para `Sensores`.
   2. La clase MÁS DERIVADA es quien construye la base virtual:
      agregá el constructor de `NaveFederada` que inicialice
      `Nucleo(77)` (la lista de inicialización con la base directa).
   3. El registro `ctorRegistro()` (dado) apunta cada constructor de
      Nucleo: el test exige UNO SOLO tras crear la nave.
   4. NO cambies Nucleo ni el registro; el resto, sí.

   COMPILACIÓN:
     g++ -std=c++20 -Wall -Wextra -I../../include \
         ejercicio_12_diamante.cpp -o ej12 && ./ej12
   ============================================================================= */
#include <iostream>
#include <vector>
#include "../../include/test_framework.hpp"

// Registro dado (no lo toques).
std::vector<int>& ctorRegistro() {
    static std::vector<int> r;
    return r;
}

// ---------------------------------------------------------------------------
// 1. Base común DADA (no la toques).
// ---------------------------------------------------------------------------
class Nucleo {
public:
    int energia = 100;
    explicit Nucleo(int e) : energia(e) {
        ctorRegistro().push_back(energia);
    }
    explicit Nucleo() : energia(100) {
        ctorRegistro().push_back(energia);
    }
};

// ---------------------------------------------------------------------------
// 2. Bases intermedias: TODO 1 (herencia virtual).
// ---------------------------------------------------------------------------
class Motores : public Nucleo {
public:
    Motores() : Nucleo(21) {}     // (se IGNORA cuando alguien más derivado
                                  //  construye la base virtual)
};
class Sensores : public Nucleo {};

// ---------------------------------------------------------------------------
// 3. La más derivada: TODO 2 (su ctor debe inicializar la base virtual).
// ---------------------------------------------------------------------------
class NaveFederada : public Motores, public Sensores {
    // TODO 2: constructor NaveFederada() que pase Nucleo(77).

public:
    // CAMBIO PERMITIDO: calificador de rama (la desambiguación clásica).
    int energiaMotores() const { return Motores::energia; }
    int energiaSensores() const { return Sensores::energia; }
};

// ============= TESTS =============
TEST_CASE(TestNucleoUnico) {
    ctorRegistro().clear();
    { NaveFederada nave; }         // la nave tiene que solo UN nucleo
    ASSERT_EQ(ctorRegistro().size(), std::size_t(1));
    ASSERT_EQ(ctorRegistro()[0], 77);          // construido por la NAVE
}

TEST_CASE(TestEnergiaCompartida) {
    NaveFederada nave;
    // Con el diamante roto: dos nucleos con 100 cada uno y 77 ni aparecen.
    ASSERT_EQ(nave.energiaMotores(), 77);
    ASSERT_EQ(nave.energiaSensores(), 77);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 12...\n";
    return RUN_ALL_TESTS();
}
