/* =============================================================================
   MÓDULO 08 - EJERCICIO 03: El constructor de movimiento a mano
   =============================================================================
   DIFICULTAD: Intermedio
   PRACTICA: lección 05 (el constructor de movimiento con buffer crudo:
   robar los internos y DEJAR AL OTRO VACÍO).

   CONSIGNA:
   La clase `Muestras` guarda un buffer crudo dinámico (float*). El
   constructor de copia y el destructor YA están escritos. Te toca:

      1. Completar el CONSTRUCTOR DE MOVIMIENTO:
         - tomar el puntero y el tamaño del otro,
         - dejarlo VACÍO (nullptr / 0) para que su destructor no
           haga doble delete (L05: es obligatorio).
      2. Que quede `noexcept` (promesa de no lanzar).

   PISTA: en el PLACEHOLDER el cuerpo está vacío a propósito:
   los miembros no robados quedan con valores "nuevos" (no UB), pero
   el contrato no se cumple y los tests lo detectan.

   COMPILACIÓN:
     g++ -std=c++20 -Wall -Wextra -I../../include \
         ejercicio_03_constructor_de_movimiento.cpp -o ej03 && ./ej03
   =============================================================================
 */
#include <cstddef>
#include <iostream>
#include <utility>
#include "../../include/test_framework.hpp"

class Muestras {
public:
    explicit Muestras(std::size_t n) : datos_(new float[n]{}), tamanio_(n) {}

    // Constructor de copia (ya escrito): O(n), recorre todo.
    Muestras(const Muestras& otro)
        : datos_(new float[otro.tamanio_]), tamanio_(otro.tamanio_) {
        for (std::size_t i = 0; i < tamanio_; ++i) datos_[i] = otro.datos_[i];
        ++copias_;
    }

    // TODO 1: completar el constructor de movimiento (y hacerlo noexcept):
    //   - robar los internos (datos_, tamanio_),
    //   - dejar al otro VACIO (nullptr / 0) para que su dtor no lo doble,
    //   - ++movimientos_.
    Muestras(Muestras&& otro)
        : datos_(nullptr), tamanio_(0) {          // PLACEHOLDER: falta robar
        (void)otro;
    }

    ~Muestras() { delete[] datos_; }

    std::size_t tamanio() const { return tamanio_; }
    float muestra(std::size_t i) const { return datos_[i]; }
    void guardo(std::size_t i, float v) { if (i < tamanio_) datos_[i] = v; }

    static int copias() { return copias_; }
    static int movimientos() { return movimientos_; }

private:
    float*      datos_;
    std::size_t tamanio_;
    inline static int copias_ = 0;
    inline static int movimientos_ = 0;
};

// ============= TESTS =============
TEST_CASE(MoverTransfiereLosDatos) {
    Muestras a(4);
    a.guardo(0, 0.5f); a.guardo(1, 1.5f);
    Muestras b(std::move(a));
    ASSERT_EQ(b.tamanio(), std::size_t(4));
    ASSERT_NEAR(b.muestra(0), 0.5f, 0.0001f);
    ASSERT_NEAR(b.muestra(1), 1.5f, 0.0001f);
    ASSERT_EQ(Muestras::movimientos(), 1);
}

TEST_CASE(EsNoexcept) {
    ASSERT_EQ(noexcept(Muestras(std::declval<Muestras&&>())), true);
}

TEST_CASE(OrigenQuedaVacio) {
    Muestras a(6);
    Muestras b(std::move(a));
    ASSERT_EQ(a.tamanio(), std::size_t(0));   // el robado: vacio, no muerto
}

TEST_CASE(NoHuboCopias) {
    const int copiasAntes = Muestras::copias();
    Muestras a(3);
    Muestras b(std::move(a));
    ASSERT_EQ(Muestras::copias(), copiasAntes);   // cero copias al mover
}

int main() {
    std::cout << "Ejecutando tests de Ejercicio 03...\n";
    return RUN_ALL_TESTS();
}
