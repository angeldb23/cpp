/* =============================================================================
   MÓDULO 05 - SOLUCIÓN 03: Constructores: por defecto, parametrizado y
   delegante
   =============================================================================
   POR QUÉ FUNCIONA:
     - El clamp de nivel vive en un static y en el helper del cuerpo:
       una sola lógica de validación (invariante del nivel 1..100).
     - El delegante no repite la lógica: `Monstruo(nombre, 1)` y listo
       (lección 06: delegar no usa lista propia).
     - El por defecto se declara a mano porque al declarar parametrizados
       el compilador dejó de generarlo.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra solucion_03... -o sol03 && ./sol03
   ============================================================================= */
#include <iostream>
#include <string>
#include "../../include/test_framework.hpp"

class Monstruo {
public:
    Monstruo(const std::string& nombre, int nivel)
        : m_nombre(nombre), m_nivel(0) {
        if (nivel < 1) {
            m_nivel = 1;
        } else if (nivel > 100) {
            m_nivel = 100;
        } else {
            m_nivel = nivel;
        }
    }

    // Delegante: su cuerpo corre después del constructor destino.
    explicit Monstruo(const std::string& nombre) : Monstruo(nombre, 1) {}

    Monstruo() : m_nombre("?"), m_nivel(1) {}

    int nivel() const { return m_nivel; }
    std::string nombre() const { return m_nombre; }

    static bool nivelValido(int n) {
        return n >= 1 && n <= 100;
    }

private:
    std::string m_nombre;
    int m_nivel;
};

// ============= TESTS =============
TEST_CASE(TestParametrizadoYClamp) {
    Monstruo m("Goblin", 42);
    ASSERT_EQ(m.nivel(), 42);
    ASSERT_TRUE(m.nombre() == "Goblin");
    ASSERT_EQ(Monstruo("Boss", 150).nivel(), 100);
    ASSERT_EQ(Monstruo("Larva", 0).nivel(), 1);
    ASSERT_EQ(Monstruo("Larva2", -9).nivel(), 1);
}

TEST_CASE(TestDelegante) {
    Monstruo m("Caballero");
    ASSERT_TRUE(m.nombre() == "Caballero");
    ASSERT_EQ(m.nivel(), 1);
}

TEST_CASE(TestPorDefecto) {
    Monstruo m;
    ASSERT_TRUE(m.nombre() == "?");
    ASSERT_EQ(m.nivel(), 1);
}

TEST_CASE(TestStaticValido) {
    ASSERT_TRUE(Monstruo::nivelValido(1));
    ASSERT_TRUE(Monstruo::nivelValido(100));
    ASSERT_FALSE(Monstruo::nivelValido(0));
    ASSERT_FALSE(Monstruo::nivelValido(101));
}

int main() {
    std::cout << "Ejecutando pruebas para Solución 03...\n";
    return RUN_ALL_TESTS();
}
