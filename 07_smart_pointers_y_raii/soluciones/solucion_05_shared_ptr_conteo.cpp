/* =============================================================================
   MÓDULO 07 - SOLUCIÓN 05: shared_ptr: varios dueños, un solo recurso
   =============================================================================
   Práctica de la lección 06: copiar un shared_ptr es INVITAR a un dueño;
   el recurso vive mientras quede uno; y cada make_shared es un bloque
   nuevo (nunca se "re-crea" un shared sobre un crudo ya dueño).
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

class Inventario {
private:
    std::vector<std::shared_ptr<Item>> slots_;

public:
    // Copiar = invitar a otro dueño del MISMO bloque de control.
    void agregarOtroDueno(std::shared_ptr<Item> item) {
        slots_.push_back(std::move(item));   // el vector adopta la copia
    }

    std::size_t duenos() const { return slots_.size(); }

    void usar() {
        if (!slots_.empty()) slots_[0]->usar();
    }
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
        }
        ASSERT_EQ(item.use_count(), 1);
        ASSERT_EQ(Item::vivos(), 1);
    }
    ASSERT_EQ(Item::vivos(), 0);
}

TEST_CASE(EsElMismoRecurso) {
    auto original = std::make_shared<Item>();
    Inventario inv;
    inv.agregarOtroDueno(original);
    inv.usar();
    ASSERT_EQ(original->usos(), 1);           // el estado es DEL recurso
}

TEST_CASE(CadaMakeSharedEsUnRecursoPropio) {
    auto a = std::make_shared<Item>();
    auto b = std::make_shared<Item>();
    ASSERT_TRUE(a.get() != b.get());
    ASSERT_EQ(a.use_count(), 1);
    ASSERT_EQ(b.use_count(), 1);
}

int main() {
    std::cout << "Ejecutando SOLUCIÓN 05 (shared_ptr)...\n";
    return RUN_ALL_TESTS();
}
