/* =============================================================================
   MÓDULO 07 - EJERCICIO 03: El gestor de proyectiles con unique_ptr
   =============================================================================
   DIFICULTAD: Básico
   PRACTICA: lección 04 (unique_ptr: propiedad exclusiva, make_unique, el
   vector como dueño de la colección).

   CONSIGNA:
   Un casillero de balas completamente libre de fugas de memoria.

      1. Completá la clase `Bala`:
         - constructor `Bala(float x, float y, float vx)`.
         - destructor que cuando muere hace ++Bala::destruidas_ (static).
         - `mover()`: suma vx a x.
         - `x()` para leer la posición.
      2. Completá `crearCargador()`: un `vector<std::unique_ptr<Bala>>`
         con 3 balas creadas con make_unique (posiciones distintas).
      3. Completá `dispararYBorrar()`: recorre el vector llamando `mover()`
         a cada bala, y DESPUÉS borra la primera con erase. El test pide
         que la DESTRUCCIÓN de esa bala sea INMEDIATA (contador destruidas_
         en 1 tras el erase: no al final del frame).

   NOTA: `push_back(make_unique<...>)` funciona directo: no necesitás
   std::move (eso llega en el módulo 08).

   COMPILACIÓN:
     g++ -std=c++20 -Wall -Wextra -I../../include \
         ejercicio_03_gestor_de_proyectiles.cpp -o ej03 && ./ej03
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
    // TODO 1: constructor: guarda x, y, vx y suma 1 a vivas_.
    Bala(float, float, float) {
        // tu código acá
    }

    // TODO 2: destructor: --vivas_, ++destruidas_.
    ~Bala() {
        // tu código acá
    }

    // TODO 3: mover() (x += vx) y x() (leer posición).
    void mover() {}
    float x() const { return -1.f; }

    static int vivas() { return vivas_; }
    static int destruidas() { return destruidas_; }
};

// TODO 4: crearCargador: 3 balas con make_unique en un vector dueño.
std::vector<std::unique_ptr<Bala>> crearCargador() {
    return {};                    // PLACEHOLDER
}

// TODO 5: recorrer moviendo y borrar la primera con erase.
void dispararYBorrar(std::vector<std::unique_ptr<Bala>>& balas) {
    (void)balas;
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
    const int destruidasCancelar = Bala::destruidas();
    dispararYBorrar(balas);
    ASSERT_EQ(balas.size(), std::size_t(2));
    // erase destruye YA (no al salir del scope):
    ASSERT_EQ(Bala::destruidas(), destruidasCancelar + 1);
    ASSERT_EQ(Bala::vivas(), 2);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 03 (proyectiles)...\n";
    return RUN_ALL_TESTS();
}
