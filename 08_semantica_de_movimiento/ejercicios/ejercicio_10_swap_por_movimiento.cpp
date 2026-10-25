/* =============================================================================
   MÓDULO 08 - EJERCICIO 10: Swap escrito a puro movimiento
   =============================================================================
   DIFICULTAD: Intermedio
   PRACTICA: lecciones 04, 06 y 07 (std::move en asignaciones; la
   receta de asignación por movimiento; el origen moved-from como
   "casi apagado").

   CONSIGNA:
   Programá `intercambiar(Cofre& a, Cofre& b)` que swappee el
   contenido de los dos cofres SIN COPIAR contenido:

      1. Un Cofre temp con std::move(a)   (constructor de movimiento)
      2. a ← std::move(b)                 (asignación de movimiento)
      3. b ← std::move(temp)              (asignación de movimiento)

   NO uses std::swap ni copies strings: el conteo `copias_` de la
   clase muestra SIEMPRE la verdad: los tests piden que NO crezca
   durante el swap, y que el contenido efectivamente intercambie.

   PISTA: son exactamente TRES movimientos; el conteo de
   `asignaciones_` + `construcciones_` movidas queda evidenciado en el
   test (20-3-3).

   COMPILACIÓN:
     g++ -std=c++20 -Wall -Wextra -I../../include \
         ejercicio_10_swap_por_movimiento.cpp -o ej10 && ./ej10
   =============================================================================
 */
#include <cstddef>
#include <iostream>
#include <string>
#include <utility>
#include "../../include/test_framework.hpp"

class Cofre {
public:
    explicit Cofre(std::string tesoro) : tesoro_(std::move(tesoro)) {}

    Cofre(const Cofre& otro) : tesoro_(otro.tesoro_) { ++copias_; }
    Cofre(Cofre&& otro) noexcept : tesoro_(std::move(otro.tesoro_)) { ++construcciones_; }

    Cofre& operator=(const Cofre& otro) {
        tesoro_ = otro.tesoro_;
        ++copias_;
        return *this;
    }
    Cofre& operator=(Cofre&& otro) noexcept {
        tesoro_ = std::move(otro.tesoro_);
        ++asignaciones_;
        return *this;
    }

    const std::string& tesoro() const { return tesoro_; }
    std::size_t tamanio() const { return tesoro_.size(); }
    static int copias() { return copias_; }
    static int asignacionesDeMove() { return asignaciones_; }
    static int construccionesDeMove() { return construcciones_; }

private:
    std::string tesoro_;
    inline static int copias_ = 0;
    inline static int asignaciones_ = 0;
    inline static int construcciones_ = 0;
};

// TODO: intercambiar a y b en TRES movimientos (temp + dos asignaciones),
//       SIN copiar contenido (el conteo de copias no debe crecer).
void intercambiar(Cofre& a, Cofre& b) {
    (void)a;
    (void)b;                                       // PLACEHOLDER
}

// ============= TESTS =============
TEST_CASE(ElContenidoCambiaDeDuenio) {
    Cofre a("gema de valor 1");
    Cofre b("estuche de valor 2");
    intercambiar(a, b);
    ASSERT_EQ(a.tesoro(), "estuche de valor 2");
    ASSERT_EQ(b.tesoro(), "gema de valor 1");
}

TEST_CASE(SinCopiasEnElSwap) {
    const int copiasAntes = Cofre::copias();
    Cofre a("oro");
    Cofre b("plata");
    intercambiar(a, b);
    ASSERT_EQ(Cofre::copias(), copiasAntes);
}

TEST_CASE(ElRecuentoDeMovimientos) {
    const int consAntes = Cofre::construccionesDeMove();
    const int asigAntes = Cofre::asignacionesDeMove();
    Cofre a("maquillaje");
    Cofre b("camisa");
    intercambiar(a, b);
    ASSERT_EQ(Cofre::construccionesDeMove(), consAntes + 1);   // el temp
    ASSERT_EQ(Cofre::asignacionesDeMove(), asigAntes + 2);     // los dos swaps
}

TEST_CASE(AmbosSiguenValidos) {
    Cofre a("x");
    Cofre b("y");
    intercambiar(a, b);
    ASSERT_EQ(a.tamanio(), std::size_t(1));        // a y b siguen siendo objetos validos
    ASSERT_EQ(b.tamanio(), std::size_t(1));
    a.tesoro();                                    // leibles, sin estados raros
}

int main() {
    std::cout << "Ejecutando tests de Ejercicio 10...\n";
    return RUN_ALL_TESTS();
}
