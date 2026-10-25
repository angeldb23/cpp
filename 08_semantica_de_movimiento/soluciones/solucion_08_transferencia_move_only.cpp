/* =============================================================================
   MÓDULO 08 - SOLUCIÓN 08: Transferir balas move-only entre dueños
   =============================================================================
   EXPLICACIÓN:
     Con unique_ptr (propiedad exclusiva, módulo 07) no hay copias:
     la transferencia de dueño es SIEMPRE movimiento.

     `descargar`: por cada elemento,
     `escena.push_back(std::move(bala))` cede la propiedad del
     unique_ptr al vector de la escena; el cargo queda con casillas
     vacías que clear() descarta al final (sin mirar los moved-from:
     L07).

     `robarPrimera`: se roba la propiedad AL VECTOR
     (`std::move(cargo.front())`) y después erase() aparta la
     casilla (su deslize usa asignación de movimiento; el unique_ptr
     nulo que queda se destruye sin dolor). Devolvemos el unique_ptr
     por valor: L09, return simple.

     La transferencia de propiedad se lee en el call site con
     std::move: cede una vez, en el punto exacto donde cambia de
     dueño.
   ============================================================================= */

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

// Cada bala viaja por movimiento: cero copias posibles (move-only).
void descargar(Cargo& cargo, Cargo& escena) {
    for (auto& bala : cargo) {
        escena.push_back(std::move(bala));     // la propiedad cambia de dueño
    }
    cargo.clear();   // todos los casilleros quedaron con unique_ptr vacío
}

// La propiedad del primer tightrope del vector sale del vector (caller dueño).
std::unique_ptr<Bala> robarPrimera(Cargo& cargo) {
    if (cargo.empty()) return nullptr;
    std::unique_ptr<Bala> salida = std::move(cargo.front());
    cargo.erase(cargo.begin());    // desliza el resto con asignaci�move
    return salida;                 // L09: el return simple ya es rvalue
}

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
    std::cout << "[SOLUCIÓN] Ejecutando tests de Solución 08...\n";
    return RUN_ALL_TESTS();
}
