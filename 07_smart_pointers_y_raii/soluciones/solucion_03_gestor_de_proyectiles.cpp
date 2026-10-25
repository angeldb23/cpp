/* =============================================================================
   MÓDULO 07 - SOLUCIÓN 03: El gestor de proyectiles con unique_ptr
   =============================================================================
   Práctica de la lección 04: el vector dueño con make_unique y erase
   destruyendo YA (determinismo RAII, no "limpieza al final").
   =============================================================================
 */
#include <cstddef>
#include <iostream>
#include <memory>
#include <vector>
#include "../../include/test_framework.hpp"

class Bala {
private:
    float x_, y_, vx_;
    inline static int vivas_ = 0;
    inline static int destruidas_ = 0;

public:
    Bala(float x, float y, float vx) : x_(x), y_(y), vx_(vx) { ++vivas_; }
    ~Bala() { --vivas_; ++destruidas_; }

    void mover() { x_ += vx_; }
    float x() const { return x_; }
    float y() const { return y_; }

    static int vivas() { return vivas_; }
    static int destruidas() { return destruidas_; }
};

std::vector<std::unique_ptr<Bala>> crearCargador() {
    std::vector<std::unique_ptr<Bala>> balas;
    balas.push_back(std::make_unique<Bala>(0.f, 1.f,  0.5f));
    balas.push_back(std::make_unique<Bala>(2.f, 1.f, -0.5f));
    balas.push_back(std::make_unique<Bala>(4.f, 1.f,  1.f));
    return balas;   // rose:_return del vector dueño: sin copia (módulo 08)
}

void dispararYBorrar(std::vector<std::unique_ptr<Bala>>& balas) {
    // mover todo el cargador:
    for (auto& b : balas) {
        b->mover();
    }
    // Y borrar la primera: el unique_ptr de ese slot se destruye YA y
    // con él la Bala (compara con delete a mano: misma funcionalidad, 0
    // líneas extra y cero chance de olvido).
    balas.erase(balas.begin());
}

// ============= TESTS =============
TEST_CASE(ConstructorCuentaVida) {
    {
        Bala b(1.f, 2.f, 0.f);
        ASSERT_EQ(Bala::vivas(), 1);
    }
    ASSERT_EQ(Bala::vivas(), 0);
    ASSERT_EQ(Bala::destruidas(), 1);
}

TEST_CASE(MoverModifica) {
    Bala b(0.f, 0.f, 1.5f);
    b.mover();
    ASSERT_EQ(b.x(), 1.5f);
}

TEST_CASE(ElVectorEsElDueno) {
    auto balas = crearCargador();
    ASSERT_EQ(balas.size(), std::size_t(3));
    ASSERT_EQ(Bala::vivas(), 3);
}

TEST_CASE(EraseDestruyeInmediatamente) {
    auto balas = crearCargador();
    const int base = Bala::destruidas();
    dispararYBorrar(balas);
    ASSERT_EQ(balas.size(), std::size_t(2));
    // erase destruye YA (no al salir del scope):
    ASSERT_EQ(Bala::destruidas(), base + 1);
    ASSERT_EQ(Bala::vivas(), 2);
}

int main() {
    std::cout << "Ejecutando SOLUCIÓN 03 (proyectiles)...\n";
    return RUN_ALL_TESTS();
}
