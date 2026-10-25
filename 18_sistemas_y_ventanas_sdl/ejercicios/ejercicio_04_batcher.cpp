/* =============================================================================
   MÓDULO 18 - EJERCICIO 04: Batcher manual de draw items
   =============================================================================
   DIFICULTAD: Media
   APOYO: Lección 07 (batching manual), Módulo 10 (sort estable), Módulo 16
          (separación update/render, command buffer)

   CONSIGNA:
   Cada llamado de render cambia (potencialmente) el estado asociado a una
   textura. El batching manual de la lección 07 reordena la LISTA de draws
   ANTES de dibujar: conserva el orden de CAPAS y reduce cambios de textura.
   Completá el Batcher (lógica pura, sin SDL: la textura es un int id):
     void submit(int capa, int textura, int serie)
       - capa: fondo < jugador < UI (menor = más abajo)
       - textura: id lógico (misma textura -> mismo batch)
       - serie: número de ingreso, tie-breaker del orden estable
     std::vector<Batch> ordenar()
       - reordena por (capa ASC, textura ASC, serie ASC) — sort ESTABLE
       - los items CONTIGUOS de la misma textura forman UN Batch
         {textura, begin, count}: begin/count son índices sobre la lista
         reordenada, como un draw call con N quads
       - dos items de la misma textura que no quedan contiguos en la lista
         reordenada (entre medio hay otra capa) son OTRO batch

   MEDIDAS (para comparar el ahorro del batching):
     int drawsNaive(): cuántos draws haría el orden de ingreso (1 por item)
     int drawsBateados(): cuántos Batch devuelve ordenar()
     bool ordenValido(const std::vector<Batch>&): los batches respetan el
       orden de capas (no hay batch de capa menor después de uno mayor)

   TESTS (no tocar). COMPILACIÓN:
     g++ -std=c++20 -Wall -Wextra ejercicio_04_batcher.cpp -o ej && ./ej
   ============================================================================= */
#include <vector>
#include <cstdint>
#include <algorithm>
#include "../../include/test_framework.hpp"

// ============================ MODELO (no tocar) ==============================
struct Batch {
    int textura = -1;   // un draw call por textura
    int begin = 0;      // índice del primer item en la lista reordenada
    int count = 0;      // cuántos items van en ese draw call
};

struct Item { int capa; int textura; int serie; };

// ============================ TU CÓDIGO AQUÍ ================================
class Batcher {
public:
    void submit(int capa, int textura, int serie) {
        (void)capa; (void)textura; (void)serie;
        // TODO 1: guardar el item (conservá el orden de ingreso con la serie)
    }

    // TODO 2: stable sort por (capa, textura, serie) y armar los batch
    std::vector<Batch> ordenar() {
        return {};
    }

    // TODO 3: como vino (1 draw por item)
    int drawsNaive() const { return 0; }

    // TODO 4: los que da el reorden (cantidad de batches)
    int drawsBateados() const { return 0; }

    // TODO 5: ninguna capa mayor aparece antes de una capa menor
    bool ordenValido(const std::vector<Batch>& bats) const {
        (void)bats;
        return false;
    }
};

// ============================ TESTS (no tocar) ==============================
TEST_CASE(BatchingJuntaContiguosTrasReordenar) {
    Batcher b;
    b.submit(0, 5, 1);
    b.submit(0, 7, 2);
    b.submit(0, 5, 3);
    b.submit(0, 7, 4);
    b.submit(0, 5, 5);
    b.submit(0, 7, 6);
    auto bats = b.ordenar();
    // reordenado: 5,5,5,7,7,7 -> 2 batches, no 6 draws
    ASSERT_EQ(bats.size(), std::size_t(2));
    ASSERT_EQ(bats[0].textura, 5);
    ASSERT_EQ(bats[0].count, 3);
    ASSERT_EQ(bats[0].begin, 0);
    ASSERT_EQ(bats[1].textura, 7);
    ASSERT_EQ(bats[1].count, 3);
    ASSERT_EQ(bats[1].begin, 3);
    ASSERT_EQ(b.drawsNaive(), 6);
    ASSERT_EQ(b.drawsBateados(), 2);
    ASSERT_TRUE(b.ordenValido(bats));
}

TEST_CASE(CapasAgrupanSinMezclarse) {
    Batcher b;
    b.submit(1, 3, 10);     // capa 1 (jugador)
    b.submit(0, 2, 20);     // capa 0 (fondo)
    b.submit(2, 4, 30);     // capa 2 (UI)
    b.submit(0, 2, 40);     // fondo otra vez (misma textura)
    auto bats = b.ordenar();
    ASSERT_EQ(bats.size(), std::size_t(3));
    // capa 0 primero (los dos fondos juntan: quedan contiguos), luego 1 y 2
    ASSERT_EQ(bats[0].textura, 2);
    ASSERT_EQ(bats[0].count, 2);
    ASSERT_EQ(bats[1].textura, 3);
    ASSERT_EQ(bats[2].textura, 4);
    ASSERT_TRUE(b.ordenValido(bats));
    ASSERT_EQ(b.drawsBateados(), 3);
    ASSERT_EQ(b.drawsNaive(), 4);
}

TEST_CASE(SerieHaceElOrdenEstableDentroDeLaMismaTextura) {
    Batcher b;
    b.submit(0, 1, 100);
    b.submit(0, 1, 50);  // serie menor pero ingresó después: queda PRIMERO
    auto bats = b.ordenar();
    ASSERT_EQ(bats.size(), std::size_t(1));
    ASSERT_EQ(bats[0].count, 2);
    ASSERT_TRUE(b.ordenValido(bats));
}

TEST_CASE(CeroDrawsSinItems) {
    Batcher b;
    ASSERT_TRUE(b.ordenar().empty());
    ASSERT_EQ(b.drawsNaive(), 0);
    ASSERT_EQ(b.drawsBateados(), 0);
    ASSERT_TRUE(b.ordenValido(b.ordenar()));
}

int main() { return RUN_ALL_TESTS(); }
