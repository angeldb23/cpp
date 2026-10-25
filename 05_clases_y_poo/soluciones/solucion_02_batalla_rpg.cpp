/* =============================================================================
   MÓDULO 05 - SOLUCIÓN 02: Clase Personaje y batalla RPG
   =============================================================================
   POR QUÉ FUNCIONA:
     - Estado privado + validación en recibirDanio: la vida nunca es
       negativa (invariante: 0 <= m_vida).
     - atacar() valida el estado del ATACANTE y delega el daño en el
       objetivo, pasándolo por Personaje& (no copiamos personajes).
     - Los getters son const: los tests usan objetos por valor y
       a ref, pero la const-correctness hace que cualquiera pueda leer.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra solucion_02... -o sol02 && ./sol02
   ============================================================================= */
#include <iostream>
#include <string>
#include "../../include/test_framework.hpp"

class Personaje {
public:
    Personaje(const std::string& nombre, int vida, int ataque)
        : m_nombre(nombre), m_vida(0), m_ataque(0) {
        setVida(vida);          // reuso la validación (una sola lógica)
        m_ataque = ataque;
    }

    bool estaVivo() const { return m_vida > 0; }
    int getVida() const { return m_vida; }
    std::string getNombre() const { return m_nombre; }

    void recibirDanio(int cantidad) {
        if (cantidad <= 0) {   // nada de "daño negativo" (curandero tramposo)
            return;
        }
        m_vida -= cantidad;
        if (m_vida < 0) {
            m_vida = 0;        // clamp
        }
    }

    void atacar(Personaje& objetivo) {
        if (!estaVivo()) {
            return;            // el muerto no ataca
        }
        objetivo.recibirDanio(m_ataque);
    }

private:
    void setVida(int v) {
        if (v < 0) {
            m_vida = 0;
        } else {
            m_vida = v;
        }
    }

    std::string m_nombre;
    int m_vida;      // INVARIANTE: 0 <= m_vida
    int m_ataque;    // INVARIANTE: m_ataque >= 0 (el ctor no lo valida: KISS)
};

// ============= TESTS =============
TEST_CASE(TestConstructorYGetters) {
    Personaje heroe("Guerrero", 100, 25);
    ASSERT_EQ(heroe.getVida(), 100);
    ASSERT_TRUE(heroe.getNombre() == "Guerrero");
    ASSERT_TRUE(heroe.estaVivo());
}

TEST_CASE(TestRecibirDanioSinNegativos) {
    Personaje d("x", 30, 10);
    d.recibirDanio(10);
    ASSERT_EQ(d.getVida(), 20);
    d.recibirDanio(100);
    ASSERT_EQ(d.getVida(), 0);
    ASSERT_FALSE(d.estaVivo());
}

TEST_CASE(TestAtacarModificaObjetivo) {
    Personaje a("A", 50, 20);
    Personaje b("B", 45, 15);
    a.atacar(b);
    ASSERT_EQ(b.getVida(), 25);
    ASSERT_TRUE(b.estaVivo());
    a.atacar(b);
    ASSERT_EQ(b.getVida(), 5);
    ASSERT_TRUE(b.estaVivo());
    a.atacar(b);
    ASSERT_EQ(b.getVida(), 0);
    ASSERT_FALSE(b.estaVivo());
}

TEST_CASE(TestMuertoNoAtaca) {
    Personaje fantasma("G", 0, 99);
    Personaje victima("V", 100, 5);
    fantasma.atacar(victima);
    ASSERT_EQ(victima.getVida(), 100);
}

TEST_CASE(TestBatallaCompleta) {
    Personaje heroe("Guerrero", 100, 25);
    Personaje dragon("Dragon", 80, 20);
    int turno = 0;
    while (heroe.estaVivo() && dragon.estaVivo()) {
        if (turno % 2 == 0) {
            heroe.atacar(dragon);
        } else {
            dragon.atacar(heroe);
        }
        ++turno;
    }
    ASSERT_FALSE(dragon.estaVivo());
    ASSERT_TRUE(heroe.getVida() > 0);
}

int main() {
    std::cout << "Ejecutando pruebas para Solución 02...\n";
    return RUN_ALL_TESTS();
}
