/* =============================================================================
   MÓDULO 23 - SOLUCIÓN 03: Sweep-and-prune (1D)
   ============================================================================= */
#include "../include/fisica2d.hpp"
#include "../../include/test_framework.hpp"

#include <algorithm>
#include <set>
#include <vector>

using f2d::Pair;

struct Intervalo {
    int id = 0;
    float minX = 0.0f;
    float maxX = 0.0f;
};

std::vector<Pair> sweepAndPrune(std::vector<Intervalo> intervalos) {
    std::sort(intervalos.begin(), intervalos.end(),
              [](const Intervalo& a, const Intervalo& b) {
                  if (a.minX != b.minX) return a.minX < b.minX;
                  return a.id < b.id;
              });

    std::vector<size_t> activos;
    std::set<Pair> pares;
    for (size_t i = 0; i < intervalos.size(); ++i) {
        const Intervalo& actual = intervalos[i];
        activos.erase(
            std::remove_if(activos.begin(), activos.end(),
                           [&](size_t idx) {
                               return intervalos[idx].maxX < actual.minX;
                           }),
            activos.end());

        for (size_t idx : activos) {
            pares.insert({std::min(intervalos[idx].id, actual.id),
                          std::max(intervalos[idx].id, actual.id)});
        }
        activos.push_back(i);
    }
    return {pares.begin(), pares.end()};
}

TEST_CASE(DosQueSeSolapan) {
    const auto p = sweepAndPrune({{0, 0.0f, 2.0f}, {1, 1.0f, 3.0f}});
    ASSERT_EQ(p.size(), 1u);
    ASSERT_EQ(p[0].a, 0);
    ASSERT_EQ(p[0].b, 1);
}

TEST_CASE(DosSeparados) {
    const auto p = sweepAndPrune({{0, 0.0f, 1.0f}, {1, 2.0f, 3.0f}});
    ASSERT_EQ(p.size(), 0u);
}

TEST_CASE(TocarseCuenta) {
    const auto p = sweepAndPrune({{0, 0.0f, 1.0f}, {1, 1.0f, 2.0f}});
    ASSERT_EQ(p.size(), 1u);
}

TEST_CASE(TresSolapados) {
    const auto p = sweepAndPrune({{0, 0.0f, 3.0f}, {1, 1.0f, 4.0f}, {2, 2.0f, 5.0f}});
    ASSERT_EQ(p.size(), 3u);
    ASSERT_EQ(p[0].a, 0); ASSERT_EQ(p[0].b, 1);
    ASSERT_EQ(p[1].a, 0); ASSERT_EQ(p[1].b, 2);
    ASSERT_EQ(p[2].a, 1); ASSERT_EQ(p[2].b, 2);
}

TEST_CASE(EntradaDesordenada) {
    const auto p = sweepAndPrune({{2, 5.0f, 7.0f}, {0, 0.0f, 2.0f}, {1, 1.0f, 3.0f}});
    ASSERT_EQ(p.size(), 1u);
    ASSERT_EQ(p[0].a, 0);
    ASSERT_EQ(p[0].b, 1);
}

TEST_CASE(IntervalosQueSoloSeTocanEnCadena) {
    const auto p = sweepAndPrune({{0, 0.0f, 1.0f}, {1, 1.0f, 2.0f}, {2, 2.0f, 3.0f}});
    ASSERT_EQ(p.size(), 2u);
}

int main() {
    std::cout << "Solución 03 (sweep-and-prune)...\n";
    return RUN_ALL_TESTS();
}
