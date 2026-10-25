/* =============================================================================
   MÓDULO 25 - SOLUCIÓN 03: streamer con páginas y starvation
   =============================================================================
   Igual al ejercicio, con la lógica del cursor completa:
   - `quedan` = lo que de verdad queda del archivo;
   - copiar min(quedan, framesPorPagina), rellenar el resto con 0.0f;
   - EOF limpio = cursor en el final => false + starved++.
   Es el patrón del refill de páginas de ma_resource_manager_data_stream
   (pageFrameCount[2] atómico): la página corta es un dato chico que
   recarga el cursor, no un error.
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
        if (destino.size() != framesPorPagina_) {
            return false; /* contrato de tamaño del enunciado */
        }
        const std::size_t quedan = fuente_->size() - cursor_;
        if (cursor_ >= fuente_->size()) {
            ++starved_; /* EOF limpio: underrun contado */
            return false;
        }
        const std::size_t n = framesPorPagina_ < quedan ? framesPorPagina_ : quedan;
        for (std::size_t i = 0; i < n; ++i) {
            destino[i] = (*fuente_)[cursor_ + i];
        }
        if (n < framesPorPagina_) {
            std::fill(destino.begin() + static_cast<long>(n), destino.end(), 0.0f);
            llegandoAlFinal_ = true; /* página corta: EOF a mitad */
        }
        cursor_ += n;
        ++paginasServidas_;
        return true;
    }

    std::size_t paginasServidas() const { return paginasServidas_; }
    std::size_t starved() const { return starved_; }
    bool llegoAlFinal() const { return llegandoAlFinal_; }

private:
    const std::vector<float>* fuente_ = nullptr;
    std::size_t framesPorPagina_ = 0;
    std::size_t cursor_ = 0;
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
        ASSERT_NEAR(pagina[i], 0.0f, 0.000001f); /* el flujo relleno */
    }
    ASSERT_NEAR(pagina[0], 1920.0f, 0.0001f); /* el dato real al frente */
    const std::size_t antes = s.starved();
    ASSERT_FALSE(s.leerPagina(pagina));
    ASSERT_TRUE(s.starved() == antes + 1);
}

int main() {
    return TestFramework::TestRunner::instance().run();
}
