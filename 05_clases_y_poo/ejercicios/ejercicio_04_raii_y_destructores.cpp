/* =============================================================================
   MÓDULO 05 - EJERCICIO 04: Destructores, orden de vida y RAII
   =============================================================================
   DIFICULTAD: Intermedio
   PRACTICA: lección 07 (destructor, orden nacimiento/muerte, RAII) con
   un static que se cuenta (lección 18).

   CONSIGNA: clase `TorreGuardia`:
     1. Constructor: recibe el nombre, lo guarda, y REGISTRA el
        nacimiento: incrementa el static de torres VIVAS y el de
        construidas.
     2. Destructor: decrementa SOLO el de vivas (los difuntos no
        descuentan lo construido).
     3. Getter static int vivas(): cuántas torres están vivas ahora.
     4. Getter static int construidas(): cuántas nacieron desde el
        inicio del programa (no baja al morir).
     5. Getter std::string nombre() const.

   Nota: la definición fuera de la clase de los dos contadores static
   se deja preparada (lección 18) para que solo escribas la lógica.
   ============================================================================= */
#include <iostream>
#include <string>
#include "../../include/test_framework.hpp"

class TorreGuardia {
public:
    // TODO 1: constructor con lista de inicialización + ++m_vivas,
    // ++m_construidas.
    explicit TorreGuardia(std::string nombre) {
        (void)nombre;
        m_nombre = "";
        // TODO: ++m_vivas; ++m_construidas;
        return;  // placeholder
    }

    // TODO 2: --m_vivas; (y nada más)
    ~TorreGuardia() {
        // TODO
    }

    // TODO 3 y 4: dos getters static.
    static int vivas() { return 0; }        // placeholder
    static int construidas() { return 0; }  // placeholder

    // TODO 5: getter const.
    std::string nombre() const { return ""; }

private:
    std::string m_nombre;
    // TODO: declarar acá: static int m_vivas; static int m_construidas;
};

// Definiciones de los contadores (la clase solo DECLARA los static).
// int TorreGuardia::m_vivas = 0;
// int TorreGuardia::m_construidas = 0;

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
        }   // sur muere acá
        ASSERT_EQ(TorreGuardia::vivas(), 1);
        ASSERT_EQ(TorreGuardia::construidas(), 2);
    }   // norte muere acá
    ASSERT_EQ(TorreGuardia::vivas(), 0);
    ASSERT_EQ(TorreGuardia::construidas(), 2);
}

TEST_CASE(TestConstruidasNoBaja) {
    // continúa el estado del test anterior (los statics viven hasta el fin).
    TorreGuardia oeste("oeste");
    ASSERT_EQ(TorreGuardia::vivas(), 1);
    ASSERT_EQ(TorreGuardia::construidas(), 3);
}   // oeste muere al salir

TEST_CASE(TestCincoTorres) {
    for (int i = 0; i < 5; ++i) {
        TorreGuardia t("t");
        ASSERT_EQ(TorreGuardia::vivas(), 1);    // solo esta vive en el ciclo
        ASSERT_EQ(TorreGuardia::construidas(), 4 + i);
    }
    ASSERT_EQ(TorreGuardia::vivas(), 0);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 04...\n";
    return RUN_ALL_TESTS();
}
