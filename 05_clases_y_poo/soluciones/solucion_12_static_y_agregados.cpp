/* =============================================================================
   MÓDULO 05 - SOLUCIÓN 12: static, constantes de clase y agregados
   =============================================================================
   POR QUÉ FUNCIONA:
     - CAPACIDAD_MAX es static constexpr: constante del TIPO, de compile
       time y sin costo por objeto.
     - s_construidas pertenece a Sala (no baja al destruir): se define
       una única vez fuera de la clase.
     - cabe() valida el rango y entra() reutiliza cabe() para decidir.
     - SalaConfig sigue siendo agregado: initializer list y designated
       initializers nombrando solo los campos deseados.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra solucion_12... -o sol12 && ./sol12
   ============================================================================= */
#include <iostream>
#include <string>
#include "../../include/test_framework.hpp"

class Sala {
public:
    static constexpr int CAPACIDAD_MAX = 4;

    explicit Sala(const char* nombre) : m_nombre(nombre) {
        ++s_construidas;
    }

    static int construidas() { return s_construidas; }

    static bool cabe(int personas) {
        return personas >= 1 && personas <= CAPACIDAD_MAX;
    }

    void entra(int personas) {
        if (cabe(m_personas + personas)) {
            m_personas += personas;
        }
    }

    int enSala() const { return m_personas; }

private:
    std::string m_nombre;
    int m_personas = 0;      // INVARIANTE: 0 <= m_personas <= CAPACIDAD_MAX
    static int s_construidas;
};

int Sala::s_construidas = 0;

struct SalaConfig {
    int aforoTeorico = 8;
    bool conHumidificador = false;
    int volumen = 50;
};

// ============= TESTS =============
TEST_CASE(TestConstanteYContador) {
    ASSERT_EQ(Sala::CAPACIDAD_MAX, 4);
    int antes = Sala::construidas();
    {
        Sala a("a");
        Sala b("b");
        ASSERT_EQ(Sala::construidas(), antes + 2);
    }
    ASSERT_EQ(Sala::construidas(), antes + 2);
}

TEST_CASE(TestCabe) {
    ASSERT_TRUE(Sala::cabe(1));
    ASSERT_TRUE(Sala::cabe(Sala::CAPACIDAD_MAX));
    ASSERT_FALSE(Sala::cabe(0));
    ASSERT_FALSE(Sala::cabe(Sala::CAPACIDAD_MAX + 1));
    ASSERT_FALSE(Sala::cabe(-3));
}

TEST_CASE(TestEntraValidada) {
    Sala laboratorio("lab");
    ASSERT_EQ(laboratorio.enSala(), 0);
    laboratorio.entra(3);
    ASSERT_EQ(laboratorio.enSala(), 3);
    laboratorio.entra(2);
    ASSERT_EQ(laboratorio.enSala(), 3);
    laboratorio.entra(1);
    ASSERT_EQ(laboratorio.enSala(), 4);
}

TEST_CASE(TestAgregadoDesignated) {
    SalaConfig config{ .aforoTeorico = 4, .volumen = 90 };
    ASSERT_EQ(config.aforoTeorico, 4);
    ASSERT_FALSE(config.conHumidificador);
    ASSERT_EQ(config.volumen, 90);

    SalaConfig base;
    ASSERT_EQ(base.aforoTeorico, 8);
    ASSERT_EQ(base.volumen, 50);
}

int main() {
    std::cout << "Ejecutando pruebas para Solución 12...\n";
    return RUN_ALL_TESTS();
}
