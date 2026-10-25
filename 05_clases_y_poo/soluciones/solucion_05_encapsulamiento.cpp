/* =============================================================================
   MÓDULO 05 - SOLUCIÓN 05: Encapsulamiento, invariantes y métodos const
   =============================================================================
   POR QUÉ FUNCIONA:
     - TODAS las rutas de escritura pasan por el helper clampa() (una
       lógica, cero copias del invariante).
     - Los statics de la clase viven en el .cpp: un solo contador de
       modificaciones por programa.
     - temperatura() y zonaDeCalor() son const: lectura garantizada.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra solucion_05... -o sol05 && ./sol05
   ============================================================================= */
#include <iostream>
#include <string>
#include "../../include/test_framework.hpp"

class ManejoDeMapa {
public:
    explicit ManejoDeMapa(int temperaturaInicial)
        : m_temperatura(clampa(temperaturaInicial)) {}

    int temperatura() const { return m_temperatura; }

    void setTemperatura(int nueva) { m_temperatura = clampa(nueva); }

    void aplicarModificador(int delta) {
        m_temperatura = clampa(m_temperatura + delta);
        ++s_modificaciones;
    }

    std::string zonaDeCalor() const {
        if (m_temperatura <= 5) return "tundra";
        if (m_temperatura <= 25) return "templada";
        return "desierto";
    }

    static int modificaciones() { return s_modificaciones; }

private:
    static int clampa(int t) {
        constexpr int MIN = 0;
        constexpr int MAX = 45;
        if (t < MIN) return MIN;
        if (t > MAX) return MAX;
        return t;
    }

    int m_temperatura;               // INVARIANTE: MIN <= temperatura <= MAX
    static int s_modificaciones;
};

int ManejoDeMapa::s_modificaciones = 0;

// ============= TESTS =============
TEST_CASE(TestConstructorClampa) {
    ASSERT_EQ(ManejoDeMapa(20).temperatura(), 20);
    ASSERT_EQ(ManejoDeMapa(-5).temperatura(), 0);
    ASSERT_EQ(ManejoDeMapa(80).temperatura(), 45);
}

TEST_CASE(TestSetYDeltaRespetaRango) {
    ManejoDeMapa m(25);
    m.setTemperatura(10);
    ASSERT_EQ(m.temperatura(), 10);
    m.aplicarModificador(50);
    ASSERT_EQ(m.temperatura(), 45);
    m.aplicarModificador(-100);
    ASSERT_EQ(m.temperatura(), 0);
}

TEST_CASE(TestSetInvalidoNoRompe) {
    ManejoDeMapa m(30);
    m.setTemperatura(-1);
    ASSERT_EQ(m.temperatura(), 0);
    m.setTemperatura(100);
    ASSERT_EQ(m.temperatura(), 45);
}

TEST_CASE(TestZonas) {
    ASSERT_TRUE(ManejoDeMapa(0).zonaDeCalor() == "tundra");
    ASSERT_TRUE(ManejoDeMapa(5).zonaDeCalor() == "tundra");
    ASSERT_TRUE(ManejoDeMapa(12).zonaDeCalor() == "templada");
    ASSERT_TRUE(ManejoDeMapa(25).zonaDeCalor() == "templada");
    ASSERT_TRUE(ManejoDeMapa(33).zonaDeCalor() == "desierto");
    ASSERT_TRUE(ManejoDeMapa(26).zonaDeCalor() == "desierto");
}

TEST_CASE(TestContadorDeModificaciones) {
    ManejoDeMapa m(10);
    int antes = ManejoDeMapa::modificaciones();
    m.aplicarModificador(2);
    ASSERT_EQ(ManejoDeMapa::modificaciones(), antes + 1);
    m.setTemperatura(12);
    ASSERT_EQ(ManejoDeMapa::modificaciones(), antes + 1);
    m.aplicarModificador(1);
    ASSERT_EQ(ManejoDeMapa::modificaciones(), antes + 2);
}

int main() {
    std::cout << "Ejecutando pruebas para Solución 05...\n";
    return RUN_ALL_TESTS();
}
