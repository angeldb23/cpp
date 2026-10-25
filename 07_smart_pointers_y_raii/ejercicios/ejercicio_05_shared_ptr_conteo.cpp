/* =============================================================================
   MÓDULO 07 - EJERCICIO 05: shared_ptr: varios dueños, un solo recurso
   =============================================================================
   DIFICULTAD: Medio
   PRACTICA: lección 06 (control block, use_count, dueños iguales y el
   detalle de que el bloque es único SOLO si el origen es el mismo).

   CONSIGNA:
   El `Inventario` guarda un recurso y COMPARTIRLO con otros dueños.

      1. `agregarOtroDueno(std::shared_ptr<Item> item)`: guarda una COPIA
         del shared en el vector interno (las copias son dueños: todos
         usan el mismo recurso y cada uno sostiene su copia).
      2. `usar()`: llama `usar()` sobre el PRIMER slot guardado (si hay).
         El estado se mantiene en el propio Item: quien lo copia lo ve.
      3. La clase `Item` es la DADA (ctor suma a `Item::vivos_`, dtor
         resta): es la llave de los tests: nada más al contador.

   El flujo que piden los tests (lección 06 completa):
     a) crea Item con make_shared (única fuente de verdad);
     b) agrega el MISMO shared a DOS inventarios distintos;
     c) use_count debe llegar a 3 (origen + 2 inventarios);
     d) al destruir los inventarios, main sigue siendo dueño: el Item
        NO muere todavía;
     e) al morir la última copia, quedan 0 vivos.

   COMPILACIÓN:
     g++ -std=c++20 -Wall -Wextra -I../../include \
         ejercicio_05_shared_ptr_conteo.cpp -o ej05 && ./ej05
   =============================================================================
 */
#include <cstddef>
#include <iostream>
#include <memory>
#include <vector>
#include "../../include/test_framework.hpp"

class Item {
private:
    int usos_ = 0;
    inline static int vivos_ = 0;

public:
    Item() { ++vivos_; }
    ~Item() { --vivos_; }

    void usar() { ++usos_; }
    int usos() const { return usos_; }
    static int vivos() { return vivos_; }
};

// ----------------------------------------------------------------------------
class Inventario {
private:
    std::vector<std::shared_ptr<Item>> slots_;

public:
    // TODO 1: copiar el shared (todos comparten el mismo recurso).
    void agregarOtroDueno(std::shared_ptr<Item> item) {
        (void)item;
    }

    std::size_t duenos() const { return slots_.size(); }

    // TODO 2: usar() el primer slot guardado, si hay.
    void usar() {}
};

// ============= TESTS =============
TEST_CASE(UnRecursoVariosDuenos) {
    {
        auto item = std::make_shared<Item>();
        ASSERT_EQ(item.use_count(), 1);
        {
            Inventario a, b;
            a.agregarOtroDueno(item);
            b.agregarOtroDueno(item);
            ASSERT_EQ(item.use_count(), 3);   // main + a + b
        }   // a y b mueren: el conteo baja
        ASSERT_EQ(item.use_count(), 1);
        ASSERT_EQ(Item::vivos(), 1);
    }
    ASSERT_EQ(Item::vivos(), 0);              // última referencia: dtor
}

TEST_CASE(EsElMismoRecurso) {
    auto original = std::make_shared<Item>();
    Inventario inv;
    inv.agregarOtroDueno(original);
    inv.usar();           // cambia el estado del recurso COMPARTIDO
    ASSERT_EQ(original->usos(), 1);           // visible desde el otro dueño
}

TEST_CASE(CadaMakeSharedEsUnRecursoPropio) {
    // Dos make_shared: DOS recursos y DOS bloques (lección 06). Eso es
    // CORRECTO así: lo incorrecto sería un shared "nuevo" sobre un ptr
    // crudo ya dueño (doble bloque, doble delete).
    auto a = std::make_shared<Item>();
    auto b = std::make_shared<Item>();
    ASSERT_TRUE(a.get() != b.get());          // recursos distintos
    ASSERT_EQ(a.use_count(), 1);
    ASSERT_EQ(b.use_count(), 1);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 05 (shared_ptr)...\n";
    return RUN_ALL_TESTS();
}
