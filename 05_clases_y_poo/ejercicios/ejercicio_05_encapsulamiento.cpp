/* =============================================================================
   MÓDULO 05 - EJERCICIO 05: Encapsulamiento, invariantes y métodos const
   =============================================================================
   DIFICULTAD: Intermedio
   PRACTICA: lecciones 08-09 (invariantes, getters/setters con validación,
   métodos const, mutable y parámetro const&).

   CONSIGNA: clase `ManejoDeMapa`:
     1. INVARIANTE DE CLASE: 0 <= m_temperatura <= 45 (grados).
     2. Constructor con (temperaturaInicial) que respete el invariante
        (si viene fuera de rango, se clampa).
     3. Getter `int temperatura() const`.
     4. `void setTemperatura(int nueva)`: valida y clampa al rango.
     5. `void aplicarModificador(int delta)`: suma delta, respeta rango.
     6. `std::string zonaDeCalor() const`: devuelve "tundra" si
        temperatura <= 5, "templada" si <= 25, "desierto" si > 25.
     7. Un static int `tanquesAlcanzados` que cuente las llamadas a
        aplicarModificador (método mutable no: un atributo private con
        la parte de administración: SIEMPRE válido).
     NO HAY setters sin validación: los tests lo chequean.

   COMPILACIÓN:
     g++ -std=c++20 -Wall -Wextra ejercicio_05_encapsulamiento.cpp -o ej05 && ./ej05
   ============================================================================= */
#include <iostream>
#include <string>
#include "../../include/test_framework.hpp"

namespace clima {
// Límites de la invariante: constantes de clase (lección 18).
// TODO: static constexpr int MIN = 0; static constexpr int MAX = 45;

class ManejoDeMapa {
public:
    // TODO 1-2: constructor que clampa al rango 0..45.
    explicit ManejoDeMapa(int temperaturaInicial) {
        (void)temperaturaInicial;
        // placeholder: no clampa ni guarda
    }

    // TODO 3-6: getters const y la lógica de zona.
    int temperatura() const { return -1; }         // placeholder
    std::string zonaDeCalor() const { return "?"; }// placeholder

    // TODO 4-5: setters con validación.
    void setTemperatura(int nueva) { (void)nueva; }
    void aplicarModificador(int delta) { (void)delta; }

    // TODO 7: static del contador de modificaciones.
    static int modificaciones() { return 0; }      // placeholder

private:
    // TODO: int m_temperatura; y los statics que quieras
};
}  // namespace clima

// ============= TESTS =============
TEST_CASE(TestConstructorClampa) {
    ASSERT_EQ(clima::ManejoDeMapa(20).temperatura(), 20);
    ASSERT_EQ(clima::ManejoDeMapa(-5).temperatura(), 0);    // clamp abajo
    ASSERT_EQ(clima::ManejoDeMapa(80).temperatura(), 45);   // clamp arriba
}

TEST_CASE(TestSetYDeltaRespetaRango) {
    clima::ManejoDeMapa m(25);
    m.setTemperatura(10);
    ASSERT_EQ(m.temperatura(), 10);
    m.aplicarModificador(50);
    ASSERT_EQ(m.temperatura(), 45);      // clamp, no 60
    m.aplicarModificador(-100);
    ASSERT_EQ(m.temperatura(), 0);       // clamp, no -55
}

TEST_CASE(TestSetInvalidoNoRompe) {
    clima::ManejoDeMapa m(30);
    m.setTemperatura(-1);
    ASSERT_EQ(m.temperatura(), 0);       // el setter clampa (no salta, no fuga)
    m.setTemperatura(100);
    ASSERT_EQ(m.temperatura(), 45);
}

TEST_CASE(TestZonas) {
    clima::ManejoDeMapa frio(0);
    clima::ManejoDeMapa templada(12);
    clima::ManejoDeMapa caliente(33);
    clima::ManejoDeMapa bordeCinco(5);
    clima::ManejoDeMapa bordeVeinticinco(25);
    clima::ManejoDeMapa bordeVeinteseis(26);
    ASSERT_TRUE(frio.zonaDeCalor() == "tundra");
    ASSERT_TRUE(bordeCinco.zonaDeCalor() == "tundra");       // <= 5
    ASSERT_TRUE(templada.zonaDeCalor() == "templada");
    ASSERT_TRUE(bordeVeinticinco.zonaDeCalor() == "templada"); // <= 25
    ASSERT_TRUE(caliente.zonaDeCalor() == "desierto");
    ASSERT_TRUE(bordeVeinteseis.zonaDeCalor() == "desierto");
}

TEST_CASE(TestContadorDeModificaciones) {
    clima::ManejoDeMapa m(10);
    int antes = clima::ManejoDeMapa::modificaciones();
    m.aplicarModificador(2);
    ASSERT_EQ(clima::ManejoDeMapa::modificaciones(), antes + 1);
    m.setTemperatura(12);              // el setter NO cuenta
    ASSERT_EQ(clima::ManejoDeMapa::modificaciones(), antes + 1);
    m.aplicarModificador(1);
    ASSERT_EQ(clima::ManejoDeMapa::modificaciones(), antes + 2);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 05...\n";
    return RUN_ALL_TESTS();
}
