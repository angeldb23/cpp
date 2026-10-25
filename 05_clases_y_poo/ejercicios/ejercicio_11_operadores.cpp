/* =============================================================================
   MÓDULO 05 - EJERCICIO 11: Sobrecarga de operadores (+, +=, ==, <<)
   =============================================================================
   DIFICULTAD: Intermedio
   PRACTICA: lecciones 16-17 (+ como función libre, += miembro y
   encadenable, == con default, << con return del stream).

   CONSIGNA: clase `Puntaje` (dos int: m_puntos, m_fracasos):
     1. Constructor (puntos, fracasos) y getters const.
     2. `Puntaje& operator+=(const Puntaje& otra)`: suma puntos Y
        fracasos, devuelve *this (encadenable).
     3. Función LIBRE `operator+` en términos de += (recibe una copia
        por valor, la muta con += y la devuelve por valor).
     4. `bool operator==(const Puntaje& otra) const` con `= default`
        (C++20: compara ambos campos).
     5. Función libre `operator<<(std::ostream&, const Puntaje&)`
        que imprima "p:X/f:Y" y devuelva el stream (encadenable).

   NOTA: para testear << sin mirar pantalla, el test cambia `os` por un
   std::ostringstream: mismos operadores, buffer en memoria.
   ============================================================================= */
#include <iostream>
#include <sstream>
#include <string>
#include "../../include/test_framework.hpp"

class Puntaje {
public:
    Puntaje(int puntos = 0, int fracasos = 0) {
        (void)puntos; (void)fracasos;
        return;  // placeholder
    }

    // TODO 2: getters const.
    int puntos() const { return -1; }
    int fracasos() const { return -1; }

    // TODO 3: operator+= como método mutable y encadenable.
    Puntaje& operator+=(const Puntaje& otra) {
        (void)otra;
        return *this;  // placeholder
    }

    // TODO 5: operator== con = default (C++20).
    bool operator==(const Puntaje& otra) const {
        (void)otra;
        return false;  // placeholder
    }

private:
    int m_puntos;
    int m_fracasos;
};

// TODO 4: operator+ libre (usa +=). Definilo para que compile:
Puntaje operator+(Puntaje copia, const Puntaje& otra) {
    (void)otra;
    return copia;  // placeholder
}

// TODO 6: operator<< libre (imprime "p:X/f:Y" y devuelve el stream).
std::ostream& operator<<(std::ostream& os, const Puntaje& p) {
    (void)p;
    os << "?";
    return os;  // placeholder
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
    (c += b) += b;             // encadenado (+= devuelve *this)
    ASSERT_EQ(c.puntos(), 21);
    ASSERT_EQ(c.fracasos(), 5);
}

TEST_CASE(TestSumaLibre) {
    Puntaje a(30, 1), b(60, 4);
    Puntaje c = a + b;
    ASSERT_EQ(c.puntos(), 90);
    ASSERT_EQ(c.fracasos(), 5);
    ASSERT_EQ(a.puntos(), 30);   // + no muta
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
    os2 << q << " " << r;      // encadenado gracias al return del stream
    ASSERT_TRUE(os2.str() == "p:3/f:0 p:1/f:1");
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 11...\n";
    return RUN_ALL_TESTS();
}
