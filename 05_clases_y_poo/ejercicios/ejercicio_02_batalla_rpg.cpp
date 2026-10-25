/* =============================================================================
   MÓDULO 05 - EJERCICIO 02: Clase Personaje y batalla RPG
   =============================================================================
   DIFICULTAD: Básico
   PRACTICA: lecciones 03-04 (struct->class, control de acceso, métodos,
   this; material migrado y rehecho desde el viejo ejercicio_personaje).

   CONSIGNA:
     Creá la clase `Personaje` con:
     1. Atributos PRIVADOS: `'std::string m_nombre'`, `'int m_vida'`,
        `'int m_ataque'`.
     2. Constructor: reciba (nombre, vida, ataque).
     3. Métodos públicos:
        - `bool estaVivo() const` → true si m_vida > 0.
        - `int getVida() const`, `std::string getNombre() const`.
        - `void recibirDanio(int cantidad)`: resta vida y no permite
          valores negativos (si baja de 0, queda en 0).
        - `void atacar(Personaje& objetivo)`:
          - si el atacante NO está vivo, no hace nada.
          - llama a `objetivo.recibirDanio(m_ataque)`.
          - si el objetivo muere, queremos que el TEST lo detecte (no
            imprimas: los tests no leen la pantalla).

   COMPILACIÓN:
     g++ -std=c++20 -Wall -Wextra ejercicio_02_batalla_rpg.cpp -o ej02 && ./ej02
   ============================================================================= */
#include <iostream>
#include <string>
#include "../../include/test_framework.hpp"

// TODO: implementá la clase Personaje. El esqueleto de abajo compile y
// falla todos los tests: reemplazalo por tu solución.
class Personaje {
public:
    Personaje(const std::string& nombre, int vida, int ataque)
        : m_nombre(nombre), m_vida(0), m_ataque(0) {
        (void)vida; (void)ataque;   // TODO: guardalos de verdad
    }

    // TODO: métodos const correctos (estaVivo, getVida, getNombre).
    bool estaVivo() const { return false; }
    int getVida() const { return -1; }
    std::string getNombre() const { return ""; }

    // TODO: recibirDanio con clamp a 0, atacar según la consigna.
    void recibirDanio(int cantidad) { (void)cantidad; }
    void atacar(Personaje& objetivo) { (void)objetivo; }

private:
    // TODO: atributos privados con los nombres de la consigna.
    std::string m_nombre;
    int m_vida;
    int m_ataque;
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
    ASSERT_EQ(d.getVida(), 0);      // clamp: nunca -80
    ASSERT_FALSE(d.estaVivo());
}

TEST_CASE(TestAtacarModificaObjetivo) {
    Personaje a("A", 50, 20);
    Personaje b("B", 45, 15);
    a.atacar(b);                     // 45 - 20
    ASSERT_EQ(b.getVida(), 25);
    ASSERT_TRUE(b.estaVivo());
    a.atacar(b);                     // 25 - 20
    ASSERT_EQ(b.getVida(), 5);
    ASSERT_TRUE(b.estaVivo());
    a.atacar(b);                     // 5 - 20 → 0 (clamp)
    ASSERT_EQ(b.getVida(), 0);
    ASSERT_FALSE(b.estaVivo());
}

TEST_CASE(TestMuertoNoAtaca) {
    Personaje fantasma("G", 0, 99);
    Personaje victima("V", 100, 5);
    fantasma.atacar(victima);
    ASSERT_EQ(victima.getVida(), 100);  // sin vida, no puede atacar
}

TEST_CASE(TestBatallaCompleta) {
    // heroe (100 vida, 25 ataque) vs dragon (80 vida, 20 ataque):
    // turnos alternados empezando por heroe hasta que uno muere.
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
    ASSERT_FALSE(dragon.estaVivo());   // heroe gana la batalla
    ASSERT_TRUE(heroe.getVida() > 0);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 02...\n";
    return RUN_ALL_TESTS();
}
