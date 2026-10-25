/* =============================================================================
   MÓDULO 07 - SOLUCIÓN 06: Fábrica con make_shared y traslado de dueños
   =============================================================================
   Práctica de la lección 07: la fábrica con make_shared (una allocación) y
   el traslado con asignación (=), que libera el anterior y adopta el del
   origen sin duplicar nada.
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

std::shared_ptr<Vagon> crearVagon(const std::string& nombre) {
    // make_shared: objeto + bloque de control en UNA allocación.
    return std::make_shared<Vagon>(nombre);
}

bool esMismoVagon(std::shared_ptr<Vagon> a, std::shared_ptr<Vagon> b) {
    // == entre shared_ptr compara el recurso, no los contadores.
    return a == b;
}

void trasladar(std::shared_ptr<Vagon>& origen, std::shared_ptr<Vagon>& destino) {
    // Asignación: destino adopta el recurso del origen (misma apuntada),
    // y si destino tenía algo, lo liberaba. El origen queda vacío.
    destino = origen;   // +1 fuerte, -1 fuerte: neto igual, cambia dueño
    origen = nullptr;   // origen suelta SU copia: resource count baja, pero
    // ...equivalente estándar idiomático es std::move(origen) (módulo 08);
    // sin mover, esta pareja de líneas logra el mismo resultado visible.
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
    auto c = b;
    ASSERT_TRUE(esMismoVagon(b, c));
}

TEST_CASE(AsignacionCambiaElDuenoNoElRecurso) {
    auto origen = crearVagon("pasajero");
    std::shared_ptr<Vagon> destino;
    ASSERT_EQ(origen.use_count(), 1);

    trasladar(origen, destino);
    ASSERT_EQ(destino.use_count(), 1);
    ASSERT_TRUE(origen == nullptr);
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
    std::cout << "Ejecutando SOLUCIÓN 06 (make_shared)...\n";
    return RUN_ALL_TESTS();
}
