/* =============================================================================
   MÓDULO 08 - SOLUCIÓN 01: Sobrecargas por valor-categoría
   =============================================================================
   EXPLICACIÓN:
     El compilador elige la sobrecarga por la CATEGORÍA de la
     expresión argumento (L01), no por el tipo (que es `Paquete` en
     los tres casos). La tabla de ligadura de la L02:

                                     T&      const T&    T&&
       lvalue modificable            SÍ      SÍ          no
       lvalue const                  no      SÍ          no
       rvalue (temporal/move)        no      SÍ          SÍ

     Con las tres sobrecargas activas se reparte SIN ambigüedad:
     - lvalue no const: gana `Paquete&` (la exacta).
     - lvalue const: solo `const Paquete&` puede ligarse.
     - rvalue: gana `Paquete&&`.
     Y la que más confunde del 01: `registrar(std::move(p))` con `p`
     lvalue: el resultado del cast es rvalue, así que entra por
     `Paquete&&`. El NOMBRE sigue siendo lvalue, la EXPRESSIÓN
     movida ya no: std::move cambia la categoría, no el objeto.
   ============================================================================= */

#include <iostream>
#include <utility>
#include "../../include/test_framework.hpp"

struct Paquete {
    int valor = 0;
};

char registrar(Paquete& p) {
    (void)p;
    return 'L';                // lvalue modificable: sigue vivo: copiaría
}

char registrar(const Paquete& p) {
    (void)p;
    return 'C';                // const: no se puede robar nada
}

char registrar(Paquete&& p) {
    (void)p;
    return 'R';                // rvalue: muriendo: perfecto para mover
}

char clasificarTemporal() {
    return registrar(Paquete{99});   // temporal directo: cae en la T&&
}

TEST_CASE(LvalueNoConstVaAReferenciaComun) {
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
    std::cout << "[SOLUCIÓN] Ejecutando tests de Solución 01...\n";
    return RUN_ALL_TESTS();
}
