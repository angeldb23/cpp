/* =============================================================================
   MÓDULO 18 - SOLUCIÓN 04: Batcher manual de draw items
   =============================================================================
   MISMO ENUNCIADO que ejercicios/ejercicio_04_batcher.cpp.
   SOLUCIÓN COMENTADA: qué hace cada pieza y por qué.

   PUNTOS CLAVE:
   - std::stable_sort con lambda-miembro comparando (capa, textura, serie):
     estable = ties quedan en orden de ingreso (la serie ES el tie-breaker
     y, al compararla, el sort estable nunca reordena dos items idénticos).
   - Batches: run-length de la lista ordenada; un cambio de textura cierra
     el batch ACTUAL y abre otro; misma textura pero la capa cambia ->
     otro batch también (pasa por el update: capa es otra pasada).
   - begin apunta al índice EN LA LISTA REORDENADA: es el equivalente al
     "offset de quads" que un draw call necesita.
   - El sort estable es la herramienta del módulo 10: al comparar la serie
     explícitamente, garantizamos el orden dentro de cada batch.
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

// ============================ SOLUCIÓN ======================================
class Batcher {
    std::vector<Item> items_;

public:
    void submit(int capa, int textura, int serie) {
        items_.push_back(Item{capa, textura, serie});
    }

    std::vector<Batch> ordenar() {
        std::vector<Batch> out;
        if (items_.empty()) return out;

        // reordena una COPIA: los items_ originales quedan como llegaron
        std::vector<Item> aux = items_;
        std::stable_sort(aux.begin(), aux.end(), [](const Item& a, const Item& b) {
            if (a.capa != b.capa) return a.capa < b.capa;         // capas primero
            if (a.textura != b.textura) return a.textura < b.textura; // por batch
            return a.serie < b.serie;                              // tie-breaker
        });

        // run-length: contiguos de la MISMA textura = un batch
        int i = 0;
        const int n = int(aux.size());
        while (i < n) {
            int j = i;
            while (j < n && aux[j].textura == aux[i].textura) ++j;
            out.push_back(Batch{aux[i].textura, i, j - i});
            i = j;   // cambio de textura (o fin): cierra y abre batch
        }
        return out;
    }

    int drawsNaive() const {
        return int(items_.size());  // como vino: 1 draw por item
    }

    // const-correctness: ordenar() construye el resultado; para contar los
    // batches sin mutar, drawsBateados() repite el mismo criterio de sort
    // local (duplica ~6 líneas: la alternativa es no-const o cache)
    int drawsBateados() const {
        if (items_.empty()) return 0;
        std::vector<Item> aux = items_;
        std::stable_sort(aux.begin(), aux.end(), [](const Item& a, const Item& b) {
            if (a.capa != b.capa) return a.capa < b.capa;
            if (a.textura != b.textura) return a.textura < b.textura;
            return a.serie < b.serie;
        });
        int count = 1;
        for (std::size_t i = 1; i < aux.size(); ++i) {
            if (aux[i].textura != aux[i-1].textura) ++count;
        }
        return count;
    }

    bool ordenValido(const std::vector<Batch>& bats) const {
        // los batches no tienen una capa MÁS ALTA antes de una más baja:
        // toma por textura... én los items ordenados y el batch begin/count,
        // recorremos en orden y verificamos que la capa no decresca
        std::vector<Item> aux = items_;
        if (aux.empty()) return true;
        std::stable_sort(aux.begin(), aux.end(), [](const Item& a, const Item& b) {
            if (a.capa != b.capa) return a.capa < b.capa;
            if (a.textura != b.textura) return a.textura < b.textura;
            return a.serie < b.serie;
        });
        int capaPrev = -1;                 // ninguna capa es < 0 en nuestro juego
        for (const Batch& b : bats) {
            if (b.begin < 0 || b.count <= 0 ||
                b.begin + b.count > int(aux.size())) return false;
            const int capa = aux[b.begin].capa;
            if (capa < capaPrev) return false;  // capa menor después de mayor
            capaPrev = capa;
        }
        return true;
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
