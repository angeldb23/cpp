/* =============================================================================
   MÓDULO 11 - SOLUCIÓN 06: registro con presupuesto de comparaciones (ej. 06)
   =============================================================================
   L07-L08 son las lecciones de respaldo: la tabla hash (unordered_map,
   L02/L06 para el hasher) da O(1) promedio; el map daría O(log n) y
   también pasaría el presupuesto; el vector con búsqueda lineal NO.
   ============================================================================= */
#include <cstddef>
#include <functional>
#include <iostream>
#include <unordered_map>
#include "../../include/test_framework.hpp"

std::size_t& comparaciones() {
    static std::size_t n = 0;
    return n;
}

struct Llave {
    int id;

    bool operator<(const Llave& otra) const {
        ++comparaciones();
        return id < otra.id;
    }
};

// Elección: unordered_map<int, int> (el "hash" del id es el propio id).
class Rotulo {
  public:
    void agrega(const Llave& k, int v) { data[k.id] = v; }
    bool esta(const Llave& k) const { return data.count(k.id) > 0; }
    int  valor(const Llave& k) const {
        const auto it = data.find(k.id);
        return it == data.end() ? 0 : it->second;
    }

  private:
    std::unordered_map<int, int> data;
};

void agrega(Rotulo& r, const Llave& k, int v) { r.agrega(k, v); }
bool esta(const Rotulo& r, const Llave& k) { return r.esta(k); }
int  valor(const Rotulo& r, const Llave& k) { return r.valor(k); }

// ============= TESTS =============
TEST_CASE(RegistraConsultaDevuelve) {
    Rotulo r;
    agrega(r, Llave{10}, 1);
    agrega(r, Llave{20}, 2);
    ASSERT_TRUE(esta(r, Llave{10}));
    ASSERT_FALSE(esta(r, Llave{30}));
    ASSERT_EQ(valor(r, Llave{20}), 2);
    ASSERT_EQ(valor(r, Llave{30}), 0);
}

TEST_CASE(ActualizaElValorSinDuplicar) {
    Rotulo r;
    agrega(r, Llave{1}, 5);
    agrega(r, Llave{1}, 7);
    ASSERT_EQ(valor(r, Llave{1}), 7);
    ASSERT_TRUE(esta(r, Llave{1}));
}

TEST_CASE(PresupuestoDeComparaciones_LogaritmoOHash) {
    Rotulo r;
    for (int i = 0; i < 200; ++i) {
        agrega(r, Llave{i * 3}, i);    // ids dispersos
    }
    comparaciones() = 0;               // la fase medida: SÓLO búsquedas
    for (int i = 0; i < 100; ++i) {
        (void)esta(r, Llave{i * 6});       // mitad existen / mitad no
    }
    const std::size_t gastadas = comparaciones();
    std::cout << "  (comparaciones al buscar: " << gastadas << ")\n";
    ASSERT_TRUE(gastadas < 4000);
}

int main() {
    std::cout << "Ejecutando pruebas para Solucion 06 (registro con presupuesto)...\n";
    return RUN_ALL_TESTS();
}
