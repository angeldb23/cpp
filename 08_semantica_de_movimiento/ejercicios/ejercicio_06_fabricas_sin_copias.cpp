/* =============================================================================
   MÓDULO 08 - EJERCICIO 06: Fábricas sin copias: prvalue y NRVO
   =============================================================================
   DIFICULTAD: Intermedio
   PRACTICA: lecciones 08 y 09 (copy elision garantizada devolviendo
   prvalue; NRVO con la variable nombrada; NUNCA std::move en return).

   CONSIGNA:
   La clase `Conteo` anuncia cada copia y cada movimiento. Tenés que
   escribir DOS fábricas que produzcan un `Conteo` con el valor dado
   SIN disparar ni una copia ni un movimiento:

      1. `duplica`: devuelve el prvalue directo:
             return Conteo{valor * 2};       // elision GARANTIZADA (C++17)
      2. `variante(bool preferir, int valor)`: dos ramas, cada una
         devuelta como prvalue:
             if (preferir) return Conteo{valor + 1};
             return Conteo{valor - 1};

   Los tests piden: valor correcto y contadores en 0 copias / 0
   movimientos (contando desde el arranque de cada test).

   PISTA: no uses ningún std::move, y no uses variables nombradas
   intermedias: la receta es "return temporal directo".

   COMPILACIÓN:
     g++ -std=c++20 -Wall -Wextra -I../../include \
         ejercicio_06_fabricas_sin_copias.cpp -o ej06 && ./ej06
   =============================================================================
 */
#include <cstddef>
#include <iostream>
#include <utility>
#include "../../include/test_framework.hpp"

struct Conteo {
    static inline int copias_ = 0;
    static inline int movimientos_ = 0;
    int valor_ = 0;

    explicit Conteo(int v) : valor_(v) {}
    Conteo(const Conteo& otro) : valor_(otro.valor_) { ++copias_; }
    Conteo(Conteo&& otro) noexcept : valor_(otro.valor_) { ++movimientos_; }
};

// TODO 1: devolver el PRVALUE `Conteo{valor * 2}` (elision garantizada).
Conteo duplica(int valor) {
    (void)valor;
    return Conteo{0};                              // PLACEHOLDER
}

// TODO 2: dos ramas, cada una devolviendo su prvalue.
Conteo variante(bool preferir, int valor) {
    (void)preferir;
    return Conteo{valor};                          // PLACEHOLDER
}

// ============= TESTS =============
TEST_CASE(PRValueElisionGarantizada) {
    const int copiasAntes = Conteo::copias_;
    const int movimientosAntes = Conteo::movimientos_;
    const Conteo c = duplica(7);
    ASSERT_EQ(c.valor_, 14);
    ASSERT_EQ(Conteo::copias_, copiasAntes);            // 0 copias
    ASSERT_EQ(Conteo::movimientos_, movimientosAntes);  // 0 moves (garantia)
}

TEST_CASE(RamasSinCopias) {
    const int copiasAntes = Conteo::copias_;
    const Conteo facil = variante(true, 10);
    const Conteo duro = variante(false, 10);
    ASSERT_EQ(facil.valor_, 11);
    ASSERT_EQ(duro.valor_, 9);
    ASSERT_EQ(Conteo::copias_, copiasAntes);
}

TEST_CASE(VariasLlamadasSinAcarrearCopias) {
    const int copiasAntes = Conteo::copias_;
    for (int i = 0; i < 8; ++i) {
        const Conteo m = duplica(i);
        if (m.valor_ != i * 2) ASSERT_TRUE(false);
    }
    ASSERT_EQ(Conteo::copias_, copiasAntes);
}

int main() {
    std::cout << "Ejecutando tests de Ejercicio 06...\n";
    return RUN_ALL_TESTS();
}
