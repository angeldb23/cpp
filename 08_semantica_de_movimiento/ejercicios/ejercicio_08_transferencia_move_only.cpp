/* =============================================================================
   MÓDULO 08 - EJERCICIO 08: Transferir balas move-only entre dueños
   =============================================================================
   DIFICULTAD: Intermedio
   PRACTICA: lecciones 10 y 11 (el vector como dueño de la colección;
   push_back(std::move) al pasar de un dueño a otro; tipos move-only).

   CONSIGNA:
   Tenés un cargo de balas (vector de `std::unique_ptr<Bala>`, dueño)
   y una escena también dueña. Implementá:

      1. `descargar`: TRANSFIERE todas las balas del cargo hacia la
         escena (el cargo queda VACÍO). Sin new y sin copiar punteros
         a mano: un bucle con push_back(std::move).
      2. `robarPrimera`: saca la primera bala del cargo y la devuelve
         (el caller queda dueño). PISTA: std::move del elemento +
         erase (el deslize del erase usa asignación de movimiento:
         L10).

   La clase Bala es MOVE-ONLY (copia borrada, módulo 05): la única
   transferencia legal es el movimiento, y la propiedad de cada
   puntero se maneja a fuerza de std::move.

   COMPILACIÓN:
     g++ -std=c++20 -Wall -Wextra -I../../include \
         ejercicio_08_transferencia_move_only.cpp -o ej08 && ./ej08
   =============================================================================
 */
#include <cstddef>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>
#include "../../include/test_framework.hpp"

class Bala {
public:
    explicit Bala(float x, float vx) : x_(x), vx_(vx) {}
    void mover() { x_ += vx_; }
    float x() const { return x_; }
private:
    float x_;
    float vx_;
};

using Cargo = std::vector<std::unique_ptr<Bala>>;

// TODO 1: transferir TODAS las balas de `cargo` a `escena`.
//         Al final: escena.size() crece lo que cargaba y cargo queda vacío.
void descargar(Cargo& cargo, Cargo& escena) {
    (void)cargo;
    (void)escena;                                  // PLACEHOLDER
}

// TODO 2: devolver la primera bala (la propiedad pasa al caller),
//         sacándola del cargo. Si el cargo está vacío: nullptr.
std::unique_ptr<Bala> robarPrimera(Cargo& cargo) {
    (void)cargo;
    return nullptr;                                // PLACEHOLDER
}

// ============= TESTS =============
TEST_CASE(DescargarTransfiereBalaPorBala) {
    Cargo cargo;
    cargo.push_back(std::make_unique<Bala>(0.f, 1.f));
    cargo.push_back(std::make_unique<Bala>(3.f, 2.f));
    Cargo escena;
    descargar(cargo, escena);
    ASSERT_EQ(cargo.size(), std::size_t(0));
    ASSERT_EQ(escena.size(), std::size_t(2));
    ASSERT_NEAR(escena[0]->x(), 0.f, 0.0001f);
    ASSERT_NEAR(escena[1]->x(), 3.f, 0.0001f);
}

TEST_CASE(RobarPrimeraTransfiereLaPropiedad) {
    Cargo cargo;
    cargo.push_back(std::make_unique<Bala>(1.f, 0.5f));
    cargo.push_back(std::make_unique<Bala>(2.f, 0.5f));
    std::unique_ptr<Bala> roba = robarPrimera(cargo);
    ASSERT_EQ(roba != nullptr, true);
    ASSERT_NEAR(roba->x(), 1.f, 0.0001f);
    ASSERT_EQ(cargo.size(), std::size_t(1));
    ASSERT_NEAR(cargo[0]->x(), 2.f, 0.0001f);
}

TEST_CASE(RobarDeCargoVacio) {
    Cargo cargo;
    std::unique_ptr<Bala> nada = robarPrimera(cargo);
    ASSERT_EQ(nada == nullptr, true);
}

int main() {
    std::cout << "Ejecutando tests de Ejercicio 08...\n";
    return RUN_ALL_TESTS();
}
