/* =============================================================================
   MÓDULO 05 - EJERCICIO 09: friend, explicit y conversión controlada
   =============================================================================
   DIFICULTAD: Intermedio
   PRACTICA: lección 14 (friend para funciones que miran el estado
   interno de dos objetos a la vez, explicit para cortar conversiones
   sorpresa, statics para el conteo del tipo).

   CONSIGNA: clase `Nave`:
     1. Constructores (explícitos el de un parámetro):
        - `explicit Nave(int escudo)`: motores en 1.
        - `Nave(int escudo, int motores)`.
     2. Getters const: `escudo()`, `motores()`.
     3. `void recibeGolpe(int danio)`: resta escudo sin bajar de 0.
        Si después del golpe el escudo quedó en 0: incrementa el
        static de destruidas (una sola vez por nave).
     4. `bool operativa() const`: escudo > 0.
     5. Función AMIGA `dueloCercano(const Nave& a, const Nave& b)`:
        devuelve |escudo(a) - escudo(b)| <= 10. Debe ser amiga porque
        toca atributos privados de DOS objetos distintos a la vez.
        (También podés hacerla libre con getters: igual es válida; el
        ejercicio pide la versión friend para practicar la mecánica.)
     6. Static `destruidas()` para leer el contador.
   ============================================================================= */
#include <iostream>
#include "../../include/test_framework.hpp"

class Nave {
public:
    // TODO 1: constructores explicit con lista de inicialización.
    explicit Nave(int escudo) {
        (void)escudo;
        return;  // placeholder
    }

    Nave(int escudo, int motores) {
        (void)escudo; (void)motores;
        return;  // placeholder
    }

    // TODO 2: getters const.
    int escudo() const { return -1; }
    int motores() const { return -1; }

    // TODO 3-4: golpe con invariante escudo >= 0 y el conteo de bajas.
    void recibeGolpe(int danio) { (void)danio; }
    bool operativa() const { return false; }

    // TODO 5: amiga que compara dos naves a la vez.
    // friend bool dueloCercano(const Nave& a, const Nave& b);

    // TODO 6: static destruidas() y su contador.
    static int destruidas() { return 0; }

private:
    // TODO: atributos privados + contador static.
    int m_escudo = -1;
    int m_motores = -1;
};

// TODO: la versión FRIEND reemplaza este placeholder (tocar m_escudo):
bool dueloCercano(const Nave& a, const Nave& b) {
    (void)a; (void)b;
    return false;  // placeholder
}

// ============= TESTS =============
TEST_CASE(TestConstructorExplicitoYGetters) {
    Nave a(100);
    ASSERT_EQ(a.escudo(), 100);
    ASSERT_EQ(a.motores(), 1);        // el de un parámetro: 1 motor
    Nave b(80, 3);
    ASSERT_EQ(b.escudo(), 80);
    ASSERT_EQ(b.motores(), 3);
}

TEST_CASE(TestGolpeYOperativa) {
    Nave n(50);
    ASSERT_TRUE(n.operativa());
    n.recibeGolpe(30);
    ASSERT_EQ(n.escudo(), 20);
    n.recibeGolpe(30);                // 20 - 30 → 0 (clamp, no -10)
    ASSERT_EQ(n.escudo(), 0);
    ASSERT_FALSE(n.operativa());
}

TEST_CASE(TestContadorDestruidas) {
    int antes = Nave::destruidas();
    Nave a(10);
    a.recibeGolpe(10);                // llega exactamente a 0: cuenta
    ASSERT_EQ(Nave::destruidas(), antes + 1);

    Nave b(10);
    b.recibeGolpe(50);                // clamp a 0: TAMBIÉN cuenta
    ASSERT_EQ(Nave::destruidas(), antes + 2);

    Nave c(100, 2);
    c.recibeGolpe(1);                 // sigue operativa: no cuenta
    ASSERT_EQ(Nave::destruidas(), antes + 2);
}

TEST_CASE(TestAmigaDueloCercano) {
    Nave a(60);
    Nave b(65);         // de a: diferencia 5
    Nave c(80);         // de a: 20; de b: 15
    ASSERT_TRUE(dueloCercano(a, b));
    ASSERT_TRUE(dueloCercano(b, a));  // simétrica
    ASSERT_FALSE(dueloCercano(a, c));
    ASSERT_FALSE(dueloCercano(c, a));
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 09...\n";
    return RUN_ALL_TESTS();
}
