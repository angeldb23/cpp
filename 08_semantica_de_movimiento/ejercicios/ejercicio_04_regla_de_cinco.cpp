/* =============================================================================
   MÓDULO 08 - EJERCICIO 04: Regla de 5: la asignación de movimiento + noexcept
   =============================================================================
   DIFICULTAD: Intermedio
   PRACTICA: lecciones 05 y 06 (constructor de movimiento, asignación de
   movimiento con guard contra autoasignación, y noexcept como promesa
   usada por los contenedores).

   CONSIGNA:
   La clase `Cola` guarda un texto ATENIDO en un `std::string`. Copia
   y destructor ya están. Falta:

      1. El CONSTRUCTOR de movimiento ya está escrito: miralo como
         guía (roba el texto con std::move).
      2. Completar la ASIGNACIÓN de movimiento `operator=(Cola&&)`:
         - guard contra autoasignación (L06: this != &otro),
         - robar el contenido del otro,
         - dejar al otro vacío,
         - devolver *this,
         - declararla noexcept.

   PISTA: acá no hay delete a mano (el string se autogestiona): el
   robo es simplemente `texto_ = std::move(otro.texto_);` con el
   guard y el return *this alrededor.

   COMPILACIÓN:
     g++ -std=c++20 -Wall -Wextra -I../../include \
         ejercicio_04_regla_de_cinco.cpp -o ej04 && ./ej04
   =============================================================================
 */
#include <cstddef>
#include <iostream>
#include <utility>
#include "../../include/test_framework.hpp"

class Cola {
public:
    explicit Cola(std::string texto) : texto_(std::move(texto)) {}

    // (guía) constructor de movimiento: roba el string interno.
    Cola(Cola&& otro) noexcept : texto_(std::move(otro.texto_)) { ++movimientos_; }

    // TODO: la ASIGNACIÓN de movimiento (guard, robo, vacío, *this, noexcept).
    Cola& operator=(Cola&& otro) {
        (void)otro;
        return *this;                            // PLACEHOLDER sin robar
    }

    ~Cola() {}

    // (escrito) asignación de copia, para completar la regla de 5.
    Cola& operator=(const Cola& otro) {
        texto_ = otro.texto_;
        ++copias_;
        return *this;
    }

    Cola(const Cola& otro) : texto_(otro.texto_) { ++copias_; }

    const std::string& texto() const { return texto_; }
    std::size_t tamanio() const { return texto_.size(); }
    static int copias() { return copias_; }
    static int movimientos() { return movimientos_; }

private:
    std::string texto_;
    inline static int copias_ = 0;
    inline static int movimientos_ = 0;
};

// ============= TESTS =============
TEST_CASE(AsignacionMueveElContenido) {
    Cola destino("vacio ");
    Cola origen("mensaje largo y pesado del nivel");
    destino = std::move(origen);
    ASSERT_EQ(destino.texto(), "mensaje largo y pesado del nivel");
    ASSERT_EQ(origen.tamanio(), std::size_t(0));   // garantia para std::string movido
    ASSERT_EQ(Cola::movimientos(), 1);
}

TEST_CASE(EsNoexcept) {
    ASSERT_EQ(noexcept(std::declval<Cola&>() = std::declval<Cola&&>()), true);
}

TEST_CASE(AutoasignacionPorAliasEsSegura) {
    Cola a("contenido importante");
    Cola& alias = a;
    a = std::move(alias);                          // this == &otro: no debe romper
    ASSERT_EQ(a.texto(), "contenigo importante");
}

TEST_CASE(NoSeCopio) {
    const int copiasAntes = Cola::copias();
    Cola destino("vacio ");
    Cola origen("otro mensaje");
    destino = std::move(origen);
    ASSERT_EQ(Cola::copias(), copiasAntes);
}

TEST_CASE(CadenaDeAsignaciones) {
    Cola a("x"), b("y"), c("mensaje final");
    a = b = std::move(c);                          // T& devuelto: habilita cadenas
    ASSERT_EQ(a.texto(), "mensaje final");
    ASSERT_EQ(b.texto(), "mensaje final");
}

int main() {
    std::cout << "Ejecutando tests de Ejercicio 04...\n";
    return RUN_ALL_TESTS();
}
