/* =============================================================================
   MÓDULO 07 - EJERCICIO 11: El pool de balas: reciclar en vez de crear
   =============================================================================
   DIFICULTAD: Medio-alto
   PRACTICA: lección 14 (pool con free-list: pre-alocación, spawn/liberar,
   reset del slot y el pool como ÚNICO dueño).

   CONSIGNA:
   Un pool de 4 balas (como en la demo de la L14, pero SIN el dueño crudo):

      1. `PoolBalas(int capacidad)`: pre-aloca `capacidad` balas en un
         vector y arma la free-list (todos índices libres, de mayor a
         menor o como quieras: el test no fija el orden).
      2. `spawn(dueño, x, y, vx)`: saca un slot de la free-list, lo RESETA
         completito (activo=true, dueño, coords, velocidad) y devuelve
         `Bala*` (el test lo necesita para liberar). Si no hay slots,
         devuelve nullptr SIN crear nada crudo.
      3. `liberar(Bala*)`: marcar el slot como libre (activo=false, dueño
         vacío) y devolver su índice a la free-list. Nunca borra memoria.
      4. `activos()`: cuenta balas activas.

   REGLA del pool: nadie hace new/delete de balas: la memoria ES del pool:
   un unique_ptr ahí sería doble dueño (L14). El test también falla
   si el slot se devuelve al pool SIN resetear (la "bala del pasado").

   COMPILACIÓN:
     g++ -std=c++20 -Wall -Wextra -I../../include \
         ejercicio_11_pool_de_balas.cpp -o ej11 && ./ej11
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
    // TODO 1: pre-alocar y armar la free-list.
    explicit PoolBalas(std::size_t capacidad) {
        (void)capacidad;
    }

    // TODO 2: tomar slot, resetear y devolver el puntero (o nullptr).
    Bala* spawn(const std::string& dueno, float x, float y, float vx) {
        (void)dueno; (void)x; (void)y; (void)vx;
        return nullptr;                   // PLACEHOLDER
    }

    // TODO 3: liberar slot (sin delete: el índice vuelve a la lista).
    void liberar(Bala* b) {
        (void)b;
    }

    // TODO 4: contar activos.
    std::size_t activos() const {
        return 0;                         // PLACEHOLDER
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
    (void)b;                          // (se usa cuando TODOs resueltos)
    pool.liberar(a);
    ASSERT_EQ(pool.activos(), std::size_t(1));   // solo b sigue activa
    ASSERT_FALSE(a->activo);

    Bala* c = pool.spawn("torre", 9.f, 1.f, 0.f);
    ASSERT_TRUE(c != nullptr);
    // el slot reciclado trae estado FRESCO (reset del TODO 2):
    ASSERT_TRUE(c->activo);
    ASSERT_EQ(c->dueno, std::string("torre"));
    ASSERT_EQ(pool.activos(), std::size_t(2));
}

TEST_CASE(ElPoolNuncaCreaPorDebajo) {
    PoolBalas pool(2);
    ASSERT_TRUE(pool.spawn("a", 0.f, 0.f, 0.f) != nullptr);
    ASSERT_TRUE(pool.spawn("b", 0.f, 0.f, 0.f) != nullptr);
    ASSERT_TRUE(pool.spawn("c", 0.f, 0.f, 0.f) == nullptr);   // agotado
    // y liberar por nullptr no explota (defensivo):
    pool.liberar(nullptr);
    ASSERT_EQ(pool.activos(), std::size_t(2));
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 11 (pool)...\n";
    return RUN_ALL_TESTS();
}
