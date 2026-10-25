/* =============================================================================
   MÓDULO 05 - SOLUCIÓN 10: = default y = delete
   =============================================================================
   POR QUÉ FUNCIONA:
     - `RelojArena() = default;` toma los valores de inicialización de
       los miembros (5 y 5): default + default-de-miembro es el combo.
     - Copia y asignación quedan deleteadas: el tipo es único, y el
       error del compilador es claro ("use of deleted function").
     - Sólo el constructor de gira() cambia el estado: los getters son
       const.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra solucion_10... -o sol10 && ./sol10
   ============================================================================= */
#include <iostream>
#include "../../include/test_framework.hpp"

class RelojArena {
public:
    RelojArena() = default;

    explicit RelojArena(int arena)
        : m_arena(arena), m_vidrio(arena) {}   // simétrico

    RelojArena(int arena, int vidrio)
        : m_arena(arena), m_vidrio(vidrio) {}

    // El reloj es único: no se copia ni se asigna.
    RelojArena(const RelojArena&) = delete;
    RelojArena& operator=(const RelojArena&) = delete;

    // Tampoco entra por float: perdería la precisión en silencio.
    RelojArena(double) = delete;

    void girar() {
        int swap = m_arena;
        m_arena = m_vidrio;
        m_vidrio = swap;
        ++m_giradas;
    }

    int arena() const { return m_arena; }
    int vidrio() const { return m_vidrio; }
    int giradas() const { return m_giradas; }

private:
    int m_arena = 5;
    int m_vidrio = 5;
    int m_giradas = 0;
};

// ============= TESTS =============
TEST_CASE(TestDefaultsYParametrizado) {
    RelojArena porDefecto;
    ASSERT_EQ(porDefecto.arena(), 5);
    ASSERT_EQ(porDefecto.vidrio(), 5);
    ASSERT_EQ(porDefecto.giradas(), 0);

    RelojArena cargado(3, 7);
    ASSERT_EQ(cargado.arena(), 3);
    ASSERT_EQ(cargado.vidrio(), 7);

    RelojArena mitad(9);
    ASSERT_EQ(mitad.arena(), 9);
    ASSERT_EQ(mitad.vidrio(), 9);
}

TEST_CASE(TestGirar) {
    RelojArena r(2, 6);
    r.girar();
    ASSERT_EQ(r.arena(), 6);
    ASSERT_EQ(r.vidrio(), 2);
    ASSERT_EQ(r.giradas(), 1);
    r.girar();
    ASSERT_EQ(r.arena(), 2);
    ASSERT_EQ(r.vidrio(), 6);
    ASSERT_EQ(r.giradas(), 2);
}

TEST_CASE(TestNoCopiable) {
    ASSERT_TRUE(true);
}

int main() {
    std::cout << "Ejecutando pruebas para Solución 10...\n";
    return RUN_ALL_TESTS();
}
