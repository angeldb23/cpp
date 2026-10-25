/* =============================================================================
   MÓDULO 11 - SOLUCIÓN 05: tablero hexagonal con hash propio (ejerc. 05)
   =============================================================================
   L06 es la lección de respaldo: operator== (con "default") +
   HashHex con el patrón de combinación (shift + XOR), y find/try_emplace
   para respetar "no tocar lo que ya está".
   ============================================================================= */
#include <cstddef>
#include <functional>
#include <iostream>
#include <string>
#include <unordered_map>
#include "../../include/test_framework.hpp"

struct Hex {
    int q;
    int r;

    bool operator==(const Hex&) const = default;   // C++20 (L06)
};

// La combinación clásica de campo con desplazamiento (L06):
struct HashHex {
    std::size_t operator()(const Hex& h) const noexcept {
        return std::hash<int>{}(h.q) ^ (std::hash<int>{}(h.r) << 1);
    }
};

using Celdas = std::unordered_map<Hex, std::string, HashHex>;

bool descubrir(Celdas& mapa, const Hex& pos, const std::string& contenido) {
    const auto [posicion, nueva] = mapa.try_emplace(pos, contenido);
    (void)posicion;
    return nueva;
}

std::string caminar(const Celdas& mapa, const Hex& pos) {
    const auto it = mapa.find(pos);         // find: no crea, no lanza
    return it == mapa.end() ? std::string{"desierto"} : it->second;
}

// ============= TESTS =============
TEST_CASE(HexIgualdad) {
    const Hex a{1, -3};
    const Hex b{1, -3};
    ASSERT_TRUE(a == b);
    ASSERT_FALSE((Hex{0, 0} == Hex{0, 1}));
    ASSERT_FALSE((Hex{1, 0} == Hex{0, 0}));
}

TEST_CASE(HashIgualParaIgualesDistintoParaDiferentes) {
    const std::size_t h1 = HashHex{}(Hex{4, 7});
    const std::size_t h2 = HashHex{}(Hex{4, 7});
    ASSERT_TRUE(h1 == h2);
    ASSERT_FALSE(h1 == HashHex{}(Hex{5, 7}));
    ASSERT_FALSE(h1 == HashHex{}(Hex{4, 8}));
    ASSERT_FALSE(HashHex{}(Hex{2, 2}) == HashHex{}(Hex{2, 3}));
}

TEST_CASE(DescubrirSinDuplicarCeldas) {
    Celdas mapa;
    ASSERT_TRUE(descubrir(mapa, Hex{0, 0}, "bosque"));
    ASSERT_FALSE(descubrir(mapa, Hex{0, 0}, "ciudad"));
    ASSERT_EQ(mapa.at(Hex{0, 0}), "bosque");
    ASSERT_EQ(mapa.size(), 1u);
}

TEST_CASE(CaminarNoCreaEntradas) {
    Celdas mapa{{Hex{3, -2}, "lago"}};
    ASSERT_EQ(caminar(mapa, Hex{3, -2}), "lago");
    ASSERT_EQ(caminar(mapa, Hex{9, 9}), "desierto");
    ASSERT_EQ(mapa.size(), 1u);
}

int main() {
    std::cout << "Ejecutando pruebas para Solucion 05 (tablero hexagonal)...\n";
    return RUN_ALL_TESTS();
}
