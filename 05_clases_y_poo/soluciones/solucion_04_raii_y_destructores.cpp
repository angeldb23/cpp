/* =============================================================================
   MÓDULO 05 - SOLUCIÓN 04: Destructores, orden de vida y RAII
   =============================================================================
   POR QUÉ FUNCIONA:
     - El constructor hace `++m_vivas; ++m_construidas;` con lista de
       inicialización para el nombre.
     - El destructor SOLO decrece lo vivo (RAII: la "vida" se cierra
       exactamente al salir del scope, en orden inverso al nacimiento).
     - Los statics se definen fuera de la clase; son del TIPO.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra solucion_04... -o sol04 && ./sol04
   ============================================================================= */
#include <iostream>
#include <string>
#include "../../include/test_framework.hpp"

class TorreGuardia {
public:
    explicit TorreGuardia(std::string nombre) : m_nombre(nombre) {
        ++m_vivas;
        ++m_construidas;
    }

    ~TorreGuardia() {
        --m_vivas;
    }

    static int vivas() { return m_vivas; }
    static int construidas() { return m_construidas; }

    std::string nombre() const { return m_nombre; }

private:
    std::string m_nombre;
    static int m_vivas;
    static int m_construidas;
};

// Definiciones de los contadores (una sola vez, fuera de la clase).
int TorreGuardia::m_vivas = 0;
int TorreGuardia::m_construidas = 0;

// ============= TESTS =============
TEST_CASE(TestCicloDeVida) {
    ASSERT_EQ(TorreGuardia::vivas(), 0);
    ASSERT_EQ(TorreGuardia::construidas(), 0);
    {
        TorreGuardia norte("norte");
        {
            TorreGuardia sur("sur");
            ASSERT_EQ(TorreGuardia::vivas(), 2);
            ASSERT_EQ(TorreGuardia::construidas(), 2);
            ASSERT_TRUE(norte.nombre() == "norte");
            ASSERT_TRUE(sur.nombre() == "sur");
        }
        ASSERT_EQ(TorreGuardia::vivas(), 1);
        ASSERT_EQ(TorreGuardia::construidas(), 2);
    }
    ASSERT_EQ(TorreGuardia::vivas(), 0);
    ASSERT_EQ(TorreGuardia::construidas(), 2);
}

TEST_CASE(TestConstruidasNoBaja) {
    TorreGuardia oeste("oeste");
    ASSERT_EQ(TorreGuardia::vivas(), 1);
    ASSERT_EQ(TorreGuardia::construidas(), 3);
}

TEST_CASE(TestCincoTorres) {
    for (int i = 0; i < 5; ++i) {
        TorreGuardia t("t");
        ASSERT_EQ(TorreGuardia::vivas(), 1);
        ASSERT_EQ(TorreGuardia::construidas(), 4 + i);
    }
    ASSERT_EQ(TorreGuardia::vivas(), 0);
}

int main() {
    std::cout << "Ejecutando pruebas para Solución 04...\n";
    return RUN_ALL_TESTS();
}
