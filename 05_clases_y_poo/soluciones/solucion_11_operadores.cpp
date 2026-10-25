/* =============================================================================
   MÓDULO 05 - SOLUCIÓN 11: Sobrecarga de operadores (+, +=, ==, <<)
   =============================================================================
   POR QUÉ FUNCIONA:
     - += como método muta al receptor y devuelve *this; + es libre y
       recibe su copia por valor: usa += una sola vez (una sola lógica).
     - == con = default compara los miembros en orden (C++20).
     - << devuelto el stream hace que os2 << q << " " << r funcione.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra solucion_11... -o sol11 && ./sol11
   ============================================================================= */
#include <iostream>
#include <sstream>
#include <string>
#include "../../include/test_framework.hpp"

class Puntaje {
public:
    Puntaje(int puntos = 0, int fracasos = 0)
        : m_puntos(puntos), m_fracasos(fracasos) {}

    int puntos() const { return m_puntos; }
    int fracasos() const { return m_fracasos; }

    Puntaje& operator+=(const Puntaje& otra) {
        m_puntos += otra.m_puntos;
        m_fracasos += otra.m_fracasos;
        return *this;
    }

    // C++20: comparación generada miembro por miembro (== y !=).
    bool operator==(const Puntaje&) const = default;

private:
    int m_puntos;
    int m_fracasos;
};

// + libre en términos de +=: una sola lógica de suma.
Puntaje operator+(Puntaje copia, const Puntaje& otra) {
    copia += otra;
    return copia;
}

// << libre: el stream va a la izquierda.
std::ostream& operator<<(std::ostream& os, const Puntaje& p) {
    os << "p:" << p.puntos() << "/f:" << p.fracasos();
    return os;
}

// ============= TESTS =============
TEST_CASE(TestConstructorYGetters) {
    Puntaje p(90, 10);
    ASSERT_EQ(p.puntos(), 90);
    ASSERT_EQ(p.fracasos(), 10);
}

TEST_CASE(TestSumarAsignar) {
    Puntaje a(80, 5), b(10, 2);
    a += b;
    ASSERT_EQ(a.puntos(), 90);
    ASSERT_EQ(a.fracasos(), 7);
    Puntaje c(1, 1);
    (c += b) += b;
    ASSERT_EQ(c.puntos(), 21);
    ASSERT_EQ(c.fracasos(), 5);
}

TEST_CASE(TestSumaLibre) {
    Puntaje a(30, 1), b(60, 4);
    Puntaje c = a + b;
    ASSERT_EQ(c.puntos(), 90);
    ASSERT_EQ(c.fracasos(), 5);
    ASSERT_EQ(a.puntos(), 30);
    ASSERT_EQ(b.puntos(), 60);
}

TEST_CASE(TestIgualdad) {
    Puntaje a(90, 10), b(90, 10), c(90, 11);
    ASSERT_TRUE(a == b);
    ASSERT_FALSE(a == c);
}

TEST_CASE(TestInsercionEnStream) {
    std::ostringstream os;
    Puntaje p(90, 10);
    os << p;
    ASSERT_TRUE(os.str() == "p:90/f:10");

    std::ostringstream os2;
    Puntaje q(3, 0), r(1, 1);
    os2 << q << " " << r;
    ASSERT_TRUE(os2.str() == "p:3/f:0 p:1/f:1");
}

int main() {
    std::cout << "Ejecutando pruebas para Solución 11...\n";
    return RUN_ALL_TESTS();
}
