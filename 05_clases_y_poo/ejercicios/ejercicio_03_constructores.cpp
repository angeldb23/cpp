/* =============================================================================
   MÓDULO 05 - EJERCICIO 03: Constructores: por defecto, parametrizado y
   delegante
   =============================================================================
   DIFICULTAD: Básico-Intermedio
   PRACTICA: lecciones 05-06 (constructores lista de inicialización,
   delegación, constructor por defecto que YA NO existe solo).

   CONSIGNA: clase `Monstruo` con m_nivel (int) y m_nombre (string):
     1. Constructor parametrizado (nombre, nivel): lista de
        inicialización; si nivel no está en 1..100, se clampa al rango.
     2. Constructor delegante Monstruo(const std::string& nombre)
        que delega en el parametrizado con nivel = 1.
     3. Constructor por defecto: nivel 1, nombre "?" (¡el compilador
        NO te lo da porque declaraste otros constructores!).
     4. método nivel() y nombre() const (getters).
     5. Método static bool nivelValido(int n): static en [1, 100].

   El esqueleto deja TODO el comportamiento placeholder: todos los
   tests fallan hasta que lo implementes.
   ============================================================================= */
#include <iostream>
#include <string>
#include "../../include/test_framework.hpp"

class Monstruo {
public:
    // TODO 1: parametrizado con lista de inicialización + clamp 1..100.
    Monstruo(const std::string& nombre, int nivel) {
        (void)nombre; (void)nivel;
        return;  // placeholder
    }

    // TODO 2: delegante (delega en el parametrizado con nivel 1).
    explicit Monstruo(const std::string& nombre) {
        (void)nombre;
        return;  // placeholder (aún NO delega)
    }

    // TODO 3: por defecto con nivel 1 y nombre "?".
    Monstruo() {
        return;  // placeholder
    }

    // TODO 4: getters const.
    int nivel() const { return -1; }
    std::string nombre() const { return ""; }

    // TODO 5: static nivelValido: true si n está en 1..100 inclusive.
    static bool nivelValido(int n) {
        (void)n;
        return false;  // placeholder
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
    ASSERT_EQ(Monstruo("Boss", 150).nivel(), 100);   // clamp arriba
    ASSERT_EQ(Monstruo("Larva", 0).nivel(), 1);      // clamp abajo
    ASSERT_EQ(Monstruo("Larva2", -9).nivel(), 1);
}

TEST_CASE(TestDelegante) {
    Monstruo m("Caballero");
    ASSERT_TRUE(m.nombre() == "Caballero");
    ASSERT_EQ(m.nivel(), 1);             // delegó: nivel 1
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
    std::cout << "Ejecutando pruebas para Ejercicio 03...\n";
    return RUN_ALL_TESTS();
}
