/* =============================================================================
   MÓDULO 05 - SOLUCIÓN 09: friend, explicit y conversión controlada
   =============================================================================
   POR QUÉ FUNCIONA:
     - Los constructores con lista de inicialización y `explicit` en el
       de un parámetro: sin conversiones de int a Nave por accidente.
     - recibeGolpe conserva el invariante (escudo >= 0) y cuenta la baja
       una única vez: cuando el escudo pasó de >0 a 0 por SU golpe.
     - dueloCercano es amiga: toca m_escudo de dos objetos a la vez,
       algo que los getters por objeto no expresan tan directo.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra solucion_09... -o sol09 && ./sol09
   ============================================================================= */
#include <iostream>
#include "../../include/test_framework.hpp"

class Nave {
public:
    explicit Nave(int escudo) : Nave(escudo, 1) {}  // delega

    Nave(int escudo, int motores)
        : m_escudo(escudo), m_motores(motores) {}

    int escudo() const { return m_escudo; }
    int motores() const { return m_motores; }

    void recibeGolpe(int danio) {
        if (danio <= 0 || m_escudo == 0) {
            return;   // nada que dañar
        }
        m_escudo -= danio;
        if (m_escudo < 0) {
            m_escudo = 0;
        }
        if (m_escudo == 0) {
            ++s_destruidas;
        }
    }

    bool operativa() const { return m_escudo > 0; }

    friend bool dueloCercano(const Nave& a, const Nave& b);

    static int destruidas() { return s_destruidas; }

private:
    int m_escudo;      // INVARIANTE: 0 <= m_escudo
    int m_motores;
    static int s_destruidas;
};

int Nave::s_destruidas = 0;

bool dueloCercano(const Nave& a, const Nave& b) {
    int diferencia = a.m_escudo - b.m_escudo;
    if (diferencia < 0) {
        diferencia = -diferencia;
    }
    return diferencia <= 10;
}

// ============= TESTS =============
TEST_CASE(TestConstructorExplicitoYGetters) {
    Nave a(100);
    ASSERT_EQ(a.escudo(), 100);
    ASSERT_EQ(a.motores(), 1);
    Nave b(80, 3);
    ASSERT_EQ(b.escudo(), 80);
    ASSERT_EQ(b.motores(), 3);
}

TEST_CASE(TestGolpeYOperativa) {
    Nave n(50);
    ASSERT_TRUE(n.operativa());
    n.recibeGolpe(30);
    ASSERT_EQ(n.escudo(), 20);
    n.recibeGolpe(30);
    ASSERT_EQ(n.escudo(), 0);
    ASSERT_FALSE(n.operativa());
}

TEST_CASE(TestContadorDestruidas) {
    int antes = Nave::destruidas();
    Nave a(10);
    a.recibeGolpe(10);
    ASSERT_EQ(Nave::destruidas(), antes + 1);

    Nave b(10);
    b.recibeGolpe(50);
    ASSERT_EQ(Nave::destruidas(), antes + 2);

    Nave c(100, 2);
    c.recibeGolpe(1);
    ASSERT_EQ(Nave::destruidas(), antes + 2);
}

TEST_CASE(TestAmigaDueloCercano) {
    Nave a(60);
    Nave b(65);
    Nave c(80);
    ASSERT_TRUE(dueloCercano(a, b));
    ASSERT_TRUE(dueloCercano(b, a));
    ASSERT_FALSE(dueloCercano(a, c));
    ASSERT_FALSE(dueloCercano(c, a));
}

int main() {
    std::cout << "Ejecutando pruebas para Solución 09...\n";
    return RUN_ALL_TESTS();
}
