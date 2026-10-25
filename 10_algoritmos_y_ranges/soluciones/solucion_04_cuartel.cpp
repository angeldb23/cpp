/* =============================================================================
   MÓDULO 10 - SOLUCIÓN 04: transform, copy_if, fill, generate, iota (L05 y L06)
   =============================================================================
   Resolución punto por punto:
       - hpDe: transform unario in-place sobre `salida` (ya con espacio).
       - mayoresQue: el patrón de la lección 05 COMPLETO: copy_if devuelve
         el ITERADOR al nuevo fin; `resize(distance(...))` achica al
         tamaño real. NO hay que tocar `size()` a mano.
       - resetear: fill.
       - proximasTanda: generate con un lambda mutable que lleva su
         multiplicador interno (1, 2, 3...) como en el ejemplo 2 de la
         lección 06.
       - ids: iota desde `inicio`.
   ============================================================================= */
#include <algorithm>
#include <iterator>
#include <numeric>
#include <vector>
#include "../../include/test_framework.hpp"

// SOLUCIÓN 1: transform escribe sobre el espacio YA existente:
void hpDe(const std::vector<int>& niveles, std::vector<int>& salida) {
    std::transform(niveles.begin(), niveles.end(), salida.begin(),
                   [](int nivel) { return nivel * 12; });
}

// SOLUCIÓN 2: copy_if + resize al nuevo fin (el patrón del módulo):
void mayoresQue(const std::vector<int>& datos, int limite,
                std::vector<int>& salida) {
    const auto nuevo_fin =
        std::copy_if(datos.begin(), datos.end(), salida.begin(),
                     [limite](int valor) { return valor > limite; });
    salida.resize(static_cast<std::size_t>(
        std::distance(salida.begin(), nuevo_fin)));    // achicar al útil
}

// SOLUCIÓN 3: fill:
void resetear(std::vector<int>& barras) {
    std::fill(barras.begin(), barras.end(), 0);
}

// SOLUCIÓN 4: generate con estado mutable (el contador dentro del lambda):
void proximasTanda(std::vector<int>& cola, int base) {
    int multiplicador = 1;
    std::generate(cola.begin(), cola.end(),
                  [base, &multiplicador]() {
                      return base * multiplicador++;
                  });
}

// SOLUCIÓN 5: iota:
void ids(std::vector<int>& c, int inicio) {
    std::iota(c.begin(), c.end(), inicio);
}

// ============= TESTS =============
TEST_CASE(HpDeTransformaElNivel) {
    std::vector<int> salida(3);
    hpDe({2, 5, 7}, salida);
    ASSERT_EQ(salida[0], 24);
    ASSERT_EQ(salida[1], 60);
    ASSERT_EQ(salida[2], 84);
}

TEST_CASE(MayoresQueReservaYAchica) {
    const std::vector<int> datos{30, 45, 80, 5, 92};
    std::vector<int> salida;
    salida.resize(datos.size());
    mayoresQue(datos, 40, salida);
    ASSERT_EQ(salida.size(), 3u);
    ASSERT_TRUE((salida == std::vector<int>{45, 80, 92}));
}

TEST_CASE(ResetearConFill) {
    std::vector<int> barras{3, 7, 2, 9};
    resetear(barras);
    ASSERT_TRUE((barras == std::vector<int>{0, 0, 0, 0}));
}

TEST_CASE(ProximasTandaConGenerate) {
    std::vector<int> cola(3);
    proximasTanda(cola, 35);
    ASSERT_TRUE((cola == std::vector<int>{35, 70, 105}));
    std::vector<int> mini(2);
    proximasTanda(mini, 10);
    ASSERT_TRUE((mini == std::vector<int>{10, 20}));
    std::vector<int> vacio;
    proximasTanda(vacio, 10);
    ASSERT_TRUE(vacio.empty());
}

TEST_CASE(IdsSecuenciadas) {
    std::vector<int> c(4);
    ids(c, 100);
    ASSERT_TRUE((c == std::vector<int>{100, 101, 102, 103}));
    std::vector<int> par{9, 9, 9, 9, 9};
    ids(par, -2);
    ASSERT_TRUE((par == std::vector<int>{-2, -1, 0, 1, 2}));
}

int main() {
    std::cout << "Solución 04 (cuartel)...\n";
    return RUN_ALL_TESTS();
}
