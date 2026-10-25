/* =============================================================================
   MÓDULO 06 - SOLUCIÓN 12: El diamante: una sola base común
   =============================================================================
   Claves de la lección 15:
   - `virtual` se marca en las bases INTERMEDIAS (Motores y Sensores):
     no basta la palabra en la derivada final.
   - La base virtual la construye SIEMPRE la clase más derivada: su
     lista de inicialización manda (las llamadas de B/C se ignoran).
   - Con UN nucleao, los upcast a Nucleo dejan de ser ambiguos.
   ============================================================================= */
#include <iostream>
#include <vector>
#include "../../include/test_framework.hpp"

std::vector<int>& ctorRegistro() {
    static std::vector<int> r;
    return r;
}

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

// FIX 1: la palabra `virtual` en AMBAS intermediarias:
class Motores : public virtual Nucleo {
public:
    Motores() : Nucleo(21) {}     // se ignora al construir la clase final
};
class Sensores : public virtual Nucleo {};

class NaveFederada : public Motores, public Sensores {
public:
    // FIX 2: la MÁS DERIVADA construye la base virtual (de una sola vez):
    NaveFederada() : Nucleo(77) {}

    int energiaMotores() const { return Motores::energia; }
    int energiaSensores() const { return Sensores::energia; }
};

TEST_CASE(TestNucleoUnico) {
    ctorRegistro().clear();
    { NaveFederada nave; }
    ASSERT_EQ(ctorRegistro().size(), std::size_t(1));
    ASSERT_EQ(ctorRegistro()[0], 77);
}

TEST_CASE(TestEnergiaCompartida) {
    NaveFederada nave;
    ASSERT_EQ(nave.energiaMotores(), 77);
    ASSERT_EQ(nave.energiaSensores(), 77);
    // Ahora el upcast a la base común TAMBIEN es posible (sin ambigüedad):
    Nucleo* nucleo = &nave;
    ASSERT_EQ(nucleo->energia, 77);
}

int main() {
    std::cout << "Solucion Ejercicio 12: herencia virtual.\n";
    return RUN_ALL_TESTS();
}
