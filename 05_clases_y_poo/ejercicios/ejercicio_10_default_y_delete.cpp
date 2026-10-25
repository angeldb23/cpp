/* =============================================================================
   MÓDULO 05 - EJERCICIO 10: = default y = delete
   =============================================================================
   DIFICULTAD: Intermedio
   PRACTICA: lección 15 (operaciones especiales pedidas o prohibidas
   al compilador explicitamente).

   CONSIGNA: clase `RelojArena` (dos cámaras de arena):
     1. Constructor por defecto con `= default` PROPIAMENTE escrito
        (necesita valor inicial de miembro: sin él, default deja
        basura; con `int m_arena = 5;` el default arranca con valor).
     2. Constructor parametrizado (arena, vidrio) con lista de
        inicialización.
     3. `explicit RelojArena(int arena)`: pone esa arena y el mismo
        valor en vidrio (simétrico).
     4. COPIA PROHIBIDA: ctor de copia y operator= marcados
        `= delete` (un reloj es único en la casa).
     5. `RelojArena(double) = delete` (prohibimos entra por double).
     6. Métodos:
        - `void girar()`: intercambia arena y vidrio; incrementa el
          contador de giradas.
        - `int arena() const`, `int vidrio() const`. `int giradas()
          const`.
     Comprobación manual: descomentá las líneas del test NoCopiable
     y comprobá que el compilador prohibe (con mensaje "deleted").
   ============================================================================= */
#include <iostream>
#include "../../include/test_framework.hpp"

class RelojArena {
public:
    RelojArena() = default;              // el valor viene de los miembros

    // TODO 2-3: los dos constructores parametrizados.
    explicit RelojArena(int arena) {
        (void)arena;
        return;  // placeholder
    }

    RelojArena(int arena, int vidrio) {
        (void)arena; (void)vidrio;
        return;  // placeholder
    }

    // TODO 4: copia prohibida a los gritos.

    // TODO 5: double prohibido.

    // TODO 6: girar() y getters.
    void girar() {
        // placeholder
    }

    int arena() const { return -1; }
    int vidrio() const { return -1; }
    int giradas() const { return -1; }

private:
    int m_arena = 5;    // default de miembro: el `= default` lo usa
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
    // RelojArena a;
    // RelojArena b = a;       // descomentar: ERROR "deleted function"
    // b = a;
    ASSERT_TRUE(true);   // (el chequeo es manual: el compilador decide)
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 10...\n";
    return RUN_ALL_TESTS();
}
