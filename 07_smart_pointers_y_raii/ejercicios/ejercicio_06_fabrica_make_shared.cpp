/* =============================================================================
   MÓDULO 07 - EJERCICIO 06: Fábrica con make_shared y traslado de dueños
   =============================================================================
   DIFICULTAD: Medio
   PRACTICA: lección 07 (make_shared: una sola allocación; objeto y bloque
   contiguos) y lección 06 (el operador= entre shared_ptr mueve el recurso
   entre dueños sin cambiar el bloque).

   CONSIGNA:
   Fábrica de `Vagon` (los vagones del tren cambian de dueño entre sistemas)
   y una función de traslado de propiedad.

      1. `crearVagon(const std::string&)` con make_shared: devuelve el
         shared_ptr listo (una sola allocación).
      2. `esMismoVagon(std::shared_ptr<Vagon>, std::shared_ptr<Vagon>)`:
         comparador puro: ¿apuntan al MISMO recurso? (== entre shared).
      3. `trasladar(std::shared_ptr<Vagon>& origen,`
         `std::shared_ptr<Vagon>& destino)`: pasa el recurso de una
         variable a la otra: al terminar, `origen` queda VACÍO (nullptr)
         y `destino` es el nuevo dueño del MISMO recurso (use_count no
         cambia: es una asignación, no una copia al firestore).
      4. La clase `Vagon` es DADA (ctor suma a `Vagon::vivos_`, dtor resta).

   COMPILACIÓN:
     g++ -std=c++20 -Wall -Wextra -I../../include \
         ejercicio_06_fabrica_make_shared.cpp -o ej06 && ./ej06
   =============================================================================
 */
#include <cstddef>
#include <iostream>
#include <memory>
#include <string>
#include "../../include/test_framework.hpp"

class Vagon {
private:
    std::string nombre_;
    inline static int vivos_ = 0;

public:
    explicit Vagon(std::string n) : nombre_(std::move(n)) { ++vivos_; }
    ~Vagon() { --vivos_; }

    const std::string& nombre() const { return nombre_; }
    static int vivos() { return vivos_; }
};

// TODO 1: fábrica con make_shared.
std::shared_ptr<Vagon> crearVagon(const std::string& nombre) {
    (void)nombre;
    return nullptr;                  // PLACEHOLDER
}

// TODO 2: ¿apuntan al mismo recurso?
bool esMismoVagon(std::shared_ptr<Vagon> a, std::shared_ptr<Vagon> b) {
    (void)a; (void)b;
    return false;                    // PLACEHOLDER
}

// TODO 3: traslado de propiedad entre variables dueñas.
void trasladar(std::shared_ptr<Vagon>& origen, std::shared_ptr<Vagon>& destino) {
    (void)origen; (void)destino;
}

// ============= TESTS =============
TEST_CASE(FactoryConUnaAllocacion) {
    auto v = crearVagon("carbonero");
    ASSERT_TRUE(v != nullptr);
    ASSERT_EQ(v.use_count(), 1);
    ASSERT_EQ(v->nombre(), std::string("carbonero"));
}

TEST_CASE(LosDiferentesSonDiferentes) {
    auto a = crearVagon("a");
    auto b = crearVagon("b");
    ASSERT_FALSE(esMismoVagon(a, b));
    auto c = b;                      // copia del MISMO recurso
    ASSERT_TRUE(esMismoVagon(b, c));
}

TEST_CASE(AsignacionCambiaElDuenoNoElRecurso) {
    auto origen = crearVagon("pasajero");
    std::shared_ptr<Vagon> destino;
    ASSERT_EQ(origen.use_count(), 1);

    trasladar(origen, destino);
    ASSERT_EQ(destino.use_count(), 1);       // sigue 1: solo cambió el dueño
    ASSERT_TRUE(origen == nullptr);          // origen quedó vacío
    ASSERT_EQ(destino->nombre(), std::string("pasajero"));
}

TEST_CASE(MuereConElUltimo) {
    {
        auto v = crearVagon("ultimo");
        std::shared_ptr<Vagon> masUno = v;
        ASSERT_EQ(v.use_count(), 2);
    }
    ASSERT_EQ(Vagon::vivos(), 0);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 06 (make_shared)...\n";
    return RUN_ALL_TESTS();
}
