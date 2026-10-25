/* =============================================================================
   MÓDULO 08 - SOLUCIÓN 07: El vector crece: noexcept manda
   =============================================================================
   EXPLICACIÓN:
     La regla de la STL (L10): cuando std::vector realoca, transporta
     con movimiento SI Y SOLO SI `noexcept(move-ctor)` es true; si
     no, copia. ¿Por qué? Si un movimiento lanzara a mitad de la
     realocación no habría estado coherente al que volver; con copia,
     ante una falla la reserva vieja queda intacta.

     La solución ES AGREGAR LA PALABRA. Con noexcept:
       - `noexcept(Grano(declval<Grano&&>()))` pasa a true,
       - el vector mueve en cada realocación: los 32 push_back con
         crecimiento natural => CERO copias, todos los transportes
         con el constructor que roba.
       - la asignación de movimiento también es noexcept: así el
         erase/insert del vector puede deslizar elementos robando.

     El que hace esa pregunta adentro de la STL recae en
     std::move_if_noexcept; con los contadores la respuesta queda a la
     vista de todos.
   ============================================================================= */

#include <cstddef>
#include <iostream>
#include <utility>
#include <vector>
#include "../../include/test_framework.hpp"

class Grano {
public:
    explicit Grano(int id) : id_(id) {}

    Grano(const Grano& otro) : id_(otro.id_) { ++copias_; }

    // CON noexcept: la STL SÍ lo usa para realocar.
    Grano(Grano&& otro) noexcept : id_(otro.id_) { ++movimientos_; }

    Grano& operator=(Grano&& otro) noexcept {
        id_ = otro.id_;
        ++movimientos_;
        return *this;
    }

    static int copias() { return copias_; }
    static int movimientos() { return movimientos_; }

private:
    int id_;
    inline static int copias_ = 0;
    inline static int movimientos_ = 0;
};

TEST_CASE(MovimientosSonNoexcept) {
    ASSERT_EQ(noexcept(Grano(std::declval<Grano&&>())), true);
    ASSERT_EQ(noexcept(std::declval<Grano&>() = std::declval<Grano&&>()), true);
}

TEST_CASE(ElVectorMueveAlCrecer) {
    const int copiasAntes = Grano::copias();
    const int movesAntes = Grano::movimientos();
    std::vector<Grano> silo;                          // SIN reserve: que crezca solo
    for (int i = 0; i < 32; ++i) silo.push_back(Grano{i});
    ASSERT_EQ(silo.size(), std::size_t(32));
    ASSERT_EQ(Grano::copias(), copiasAntes);          // cero copias: todo movido
    ASSERT_TRUE(Grano::movimientos() > movesAntes);
}

TEST_CASE(PushBackConMoveExplicito) {
    const int copiasAntes = Grano::copias();
    std::vector<Grano> silo;
    Grano g(42);
    silo.push_back(std::move(g));
    ASSERT_EQ(Grano::copias(), copiasAntes);
    ASSERT_EQ(silo.size(), std::size_t(1));
}

int main() {
    std::cout << "[SOLUCIÓN] Ejecutando tests de Solución 07...\n";
    return RUN_ALL_TESTS();
}
