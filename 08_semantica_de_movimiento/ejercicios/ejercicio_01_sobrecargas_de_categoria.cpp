/* =============================================================================
   MÓDULO 08 - EJERCICIO 01: Sobrecargas que se eligen por valor-categoría
   =============================================================================
   DIFICULTAD: Básico
   PRACTICA: lecciones 01 y 02 (lvalues/rvalues, la tabla de ligadura de
   T& / const T& / T&&, y la repartición de sobrecargas por categoría).

   CONSIGNA:
   Un sistema de estados para una mesa de billar quiere saber si la
   recepción de cada paquete fue por LVALUE (hay que copiar: sigue
   vivo), por CONST& (viene const), o por RVALUE (se puede mover).

      1. Completá el trío de sobrecargas de `registrar` para que
         devuelvan la letra correcta:
           - registrar(Paquete&)       -> 'L'  (lvalue no const)
           - registrar(const Paquete&) -> 'C'  (const: lvalue const)
           - registrar(Paquete&&)      -> 'R'  (rvalue / temporal)
         Los PARÁMETROS ya están declarados: completá los cuerpos.
      2. Completá `clasificarTemporales()`: tiene que llamar a
         registrar con un TEMPORAL y devolver lo que respondió.

   PISTA: la tabla de la L02: un rvalue SOLO se liga a `T&&`; un
   lvalue no const elige `T&` si existe (gana la exacta).

   COMPILACIÓN:
     g++ -std=c++20 -Wall -Wextra -I../../include \
         ejercicio_01_sobrecargas_de_categoria.cpp -o ej01 && ./ej01
   =============================================================================
 */
#include <iostream>
#include <utility>
#include "../../include/test_framework.hpp"

struct Paquete {
    int valor = 0;
};

// ---------- Sobrecargas de `registrar` (elección por categoría) ----------

// TODO 1a: esta sobrecarga debe responder 'L' (lvalue modificable).
char registrar(Paquete&) {
    return 'x';                                    // PLACEHOLDER
}

// TODO 1b: esta sobrecarga debe responder 'C' (const: lvalue const).
char registrar(const Paquete&) {
    return 'x';                                    // PLACEHOLDER
}

// TODO 1c: esta sobrecarga debe responder 'R' (rvalue: movible).
char registrar(Paquete&&) {
    return 'x';                                    // PLACEHOLDER
}

// TODO 2: llamar a registrar pasando un TEMPORAL y devolver su respuesta.
char clasificarTemporal() {
    return 'x';                                    // PLACEHOLDER
}

// ============= TESTS =============
TEST_CASE(LvalueNoConstVaATi) {
    Paquete p{1};
    Paquete& alias = p;
    ASSERT_EQ(registrar(alias), 'L');
}

TEST_CASE(LvalueConstVaAConstRef) {
    const Paquete p{2};
    ASSERT_EQ(registrar(p), 'C');
}

TEST_CASE(TemporalVaARefRvalue) {
    ASSERT_EQ(registrar(Paquete{3}), 'R');
    ASSERT_EQ(clasificarTemporal(), 'R');
}

TEST_CASE(MoveDeLvalueVaARefRvalue) {
    Paquete p{4};
    ASSERT_EQ(registrar(std::move(p)), 'R');
}

TEST_CASE(PaqueteIntacto) {
    Paquete p{5};
    ASSERT_EQ(registrar(p), 'L');
    ASSERT_EQ(p.valor, 5);
}

int main() {
    std::cout << "Ejecutando tests de Ejercicio 01...\n";
    return RUN_ALL_TESTS();
}
