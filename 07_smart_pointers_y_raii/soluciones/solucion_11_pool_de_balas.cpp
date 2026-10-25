/* =============================================================================
   MÓDULO 07 - SOLUCIÓN 11: El pool de balas: reciclar en vez de crear
   =============================================================================
   Práctica de la lección 14: el POOL es el único dueño: spawn = tomar
   slot de la free-list y resetear; liberar = devolver el índice. Cero
   new/delete por bala y el agotamiento no fuga ni crashea.
   =============================================================================
 */
#include <cstddef>
#include <iostream>
#include <string>
#include <vector>
#include "../../include/test_framework.hpp"

struct Bala {
    float x = 0.f, y = 0.f, vx = 0.f;
    bool activo = false;
    std::string dueno;
};

class PoolBalas {
private:
    std::vector<Bala> bloques_;
    std::vector<std::size_t> libres_;

public:
    explicit PoolBalas(std::size_t capacidad) : bloques_(capacidad) {
        for (std::size_t i = capacidad; i > 0; --i) {
            libres_.push_back(i - 1);      // TODOS los slots libres
        }
    }

    Bala* spawn(const std::string& dueno, float x, float y, float vx) {
        if (libres_.empty()) return nullptr;     // agotado: ni crear ni fugar
        Bala& slot = bloques_[libres_.back()];
        libres_.pop_back();
        // RESET COMPLETO: el reciclado no puede traer el pasado:
        slot.x = x; slot.y = y; slot.vx = vx;
        slot.activo = true;
        slot.dueno = dueno;
        return &slot;
    }

    void liberar(Bala* b) {
        if (b == nullptr) return;                // defensivo (test lo pide)
        b->activo = false;
        b->dueno.clear();
        for (std::size_t i = 0; i < bloques_.size(); ++i) {
            if (&bloques_[i] == b) {
                libres_.push_back(i);            // el índice vuelve al pool
                return;
            }
        }
    }

    std::size_t activos() const {
        std::size_t n = 0;
        for (const auto& b : bloques_) {
            if (b.activo) ++n;
        }
        return n;
    }
};

// ============= TESTS =============
TEST_CASE(PrealocaCeroCeroCero) {
    PoolBalas pool(4);
    ASSERT_EQ(pool.activos(), std::size_t(0));
}

TEST_CASE(SpawnActivaConEstadoFresco) {
    PoolBalas pool(4);
    Bala* a = pool.spawn("player", 1.f, 2.f, 0.5f);
    ASSERT_TRUE(a != nullptr);
    ASSERT_TRUE(a->activo);
    ASSERT_EQ(a->x, 1.f);
    ASSERT_EQ(a->dueno, std::string("player"));
    ASSERT_EQ(pool.activos(), std::size_t(1));
}

TEST_CASE(LiberarReciclaSinCrear) {
    PoolBalas pool(4);
    Bala* a = pool.spawn("player", 1.f, 2.f, 0.5f);
    Bala* b = pool.spawn("player", 5.f, 2.f, -0.5f);
    pool.liberar(a);
    ASSERT_EQ(pool.activos(), std::size_t(1));
    ASSERT_FALSE(a->activo);

    Bala* c = pool.spawn("torre", 9.f, 1.f, 0.f);
    ASSERT_TRUE(c != nullptr);
    ASSERT_TRUE(c->activo);
    ASSERT_EQ(c->dueno, std::string("torre"));
    ASSERT_EQ(pool.activos(), std::size_t(2));
}

TEST_CASE(ElPoolNuncaCreaPorDebajo) {
    PoolBalas pool(2);
    ASSERT_TRUE(pool.spawn("a", 0.f, 0.f, 0.f) != nullptr);
    ASSERT_TRUE(pool.spawn("b", 0.f, 0.f, 0.f) != nullptr);
    ASSERT_TRUE(pool.spawn("c", 0.f, 0.f, 0.f) == nullptr);
    pool.liberar(nullptr);
    ASSERT_EQ(pool.activos(), std::size_t(2));
}

int main() {
    std::cout << "Ejecutando SOLUCIÓN 11 (pool)...\n";
    return RUN_ALL_TESTS();
}
