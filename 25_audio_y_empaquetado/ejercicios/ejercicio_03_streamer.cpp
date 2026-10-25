/* =============================================================================
   MÓDULO 25 - EJERCICIO 03: streamer con páginas y starvation
   =============================================================================
   DIFICULTAD: Media
   APOYO: Lección 03 (paginar el stream, EOF, el refill del resource manager).

   CONSIGNA
   --------
   Completá `Streamer`:
   1. `leerPagina(destino)`: toma la PRÓXIMA página del archivo fuente
      (`framesPorPagina` muestras). Copia los datos que quedan en el
      archivo; si la página queda incompleta (EOF a mitad), el RESTO del
      `destino` se llena con 0.0f y se marca `llegandoAlFinal_ = true`.
   2. Devuelve false cuando ya no queda NADA (EOF limpio) o no pediste
      nada: en ese caso NO toca el `destino`.
   3. En false también incrementa `starved_` (el contador de "intenté leer
      con el archivo ya vacío": en el motor real esto es un underrun).
   4. La página corta SÍ cuenta en `paginasServidas_` (fue un dato real).
   El `destino` es un vector cuya talla DEBE ser `framesPorPagina`
   (re-resize por llam es OUT del ejercicio).
   ============================================================================= */
#include "../../include/test_framework.hpp"

#include <algorithm>
#include <cstddef>
#include <vector>

class Streamer {
public:
    Streamer(const std::vector<float>& fuente, std::size_t framesPorPagina)
        : fuente_(&fuente), framesPorPagina_(framesPorPagina) {}

    bool leerPagina(std::vector<float>& destino) {
        // TODO
        (void)destino;
        return false;
    }

    std::size_t paginasServidas() const {
        // TODO
        return 0;
    }

    std::size_t starved() const {
        // TODO
        return 0;
    }

bool llegoAlFinal() const {
        // TODO
        return false;
    }

private:
    const std::vector<float>* fuente_ = nullptr;
    std::size_t framesPorPagina_ = 0;
    std::size_t cursor_ = 0;            /* próxima muestra a leer */
    std::size_t paginasServidas_ = 0;
    std::size_t starved_ = 0;
    bool llegandoAlFinal_ = false;
};

/* ------------------------------ TESTS (no tocar)-------------------------- */
TEST_CASE(PaginasCompletas) {
    std::vector<float> fuente(2400, 0.5f); /* 5 páginas de 480 */
    Streamer s(fuente, 480);
    std::vector<float> pagina(480, 0.0f);
    for (int i = 0; i < 5; ++i) {
        ASSERT_TRUE(s.leerPagina(pagina));
    }
    ASSERT_TRUE(s.paginasServidas() == 5);
    ASSERT_FALSE(s.llegoAlFinal());
    /* la 6a: EOF limpio, starvation contada, destino sin tocar. */
    std::fill(pagina.begin(), pagina.end(), 1.0f);
    ASSERT_FALSE(s.leerPagina(pagina));
    ASSERT_TRUE(s.starved() == 1);
    ASSERT_NEAR(pagina[0], 1.0f, 0.000001f); /* no la tocó */
}

TEST_CASE(PaginaCortaAlFinal) {
    std::vector<float> fuente(2000);
    for (std::size_t i = 0; i < fuente.size(); ++i) {
        fuente[i] = static_cast<float>(i);
    }
    Streamer s(fuente, 480);
    std::vector<float> pagina(480, 0.0f);
    /* 480 x 4 = 1920, quedan 80 reales: la 5a es "corta". */
    for (int i = 0; i < 4; ++i) {
        ASSERT_TRUE(s.leerPagina(pagina));
        ASSERT_NEAR(pagina[0], static_cast<float>(480 * i), 0.0001f);
    }
    ASSERT_FALSE(s.llegoAlFinal());
    ASSERT_TRUE(s.leerPagina(pagina)); /* la corta: TRUE (dato real) */
    ASSERT_TRUE(s.llegoAlFinal());
    ASSERT_TRUE(s.paginasServidas() == 5);
    const std::size_t esperado = 80;
    ASSERT_TRUE(pagina.size() == 480);
    for (std::size_t i = esperado; i < pagina.size(); ++i) {
        ASSERT_NEAR(pagina[i], 0.0f, 0.000001f); /* el flour relleno */
    }
    ASSERT_NEAR(pagina[0], 1920.0f, 0.0001f); /* el dato real al frente */
    /* y ahora: bool = false (ya se gastó) y starvation. */
    const std::size_t antes = s.starved();
    ASSERT_FALSE(s.leerPagina(pagina));
    ASSERT_TRUE(s.starved() == antes + 1);
}

int main() {
    return TestFramework::TestRunner::instance().run();
}
