/* =============================================================================
   MÓDULO 11 - EJERCICIO 05: tablero hexagonal con hash propio (lección 06)
   =============================================================================
   DIFICULTAD: Medio
   CONSIGNA (tema 3 de TEMAS.md: clave = operator== + función de hash):
     Un mapa de tiles se guarda como std::unordered_map<Hex, String,
     HashHex>; el struct Hex { int q; int r; } es una COORDENADA
     HEXAGONAL y vos tenés que armar las DOS piezas que necesita la
     tabla (los placeholders que están abajo compilan pero FALLAN: son
     tu TODO):
       1. operator== de Hex: dos Hex son iguales si q y r coinciden
          (el placeholder devuelve false SIEMPRE: el test de igualdad
          explota). El patrón "bool operator==(const Hex&) const = 
          default;" de la L06 es una opción válida.
       2. HashHex::operator(): la combinación razonable de los dos
          campos (hash(q) ^ (hash(r) << 1), el patrón de la L06). El
          placeholder devuelve 0 para TODO: los tests chequean que
          iguales den IGUAL hash y distintos den DISTINCTO hash, así
          que "return 0" explota dos veces.
       3. Con las piezas listas, el almacenamiento:
            bool descubrir(mapa, hex, contenido): si el hex NO estaba
            lo inserta (true); si ya estaba (false) NO lo toca
            (try_emplace o insert de las L02/L06).
            std::string caminar(mapa, hex): el contenido del hex SIN
            crear la entrada; "desierto" si el hex no está (find).
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_05_hex.cpp -o ej05 && ./ej05
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

    // TODO 1: comparar los dos campos (el placeholder siempre false).
    bool operator==(const Hex&) const {
        return false;    // placeholder
    }
};

struct HashHex {
    // TODO 2: combinación de los campos (el placeholder devuelve 0).
    std::size_t operator()(const Hex&) const noexcept {
        return 0;    // placeholder
    }
};

using Celdas = std::unordered_map<Hex, std::string, HashHex>;

// TODO 3a: inserta si falta (true); no toca si ya estaba (false).
bool descubrir([[maybe_unused]] Celdas& mapa, [[maybe_unused]] const Hex& pos,
               [[maybe_unused]] const std::string& contenido) {
    return false;    // placeholder
}

// TODO 3b: el contenido del hex SIN crear entrada; "desierto" si falta.
std::string caminar([[maybe_unused]] const Celdas& mapa,
                    [[maybe_unused]] const Hex& pos) {
    return "desierto";    // placeholder
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
    ASSERT_TRUE(h1 == h2);                          // INVARIANTE (L06)
    ASSERT_FALSE(h1 == HashHex{}(Hex{5, 7}));       // no "0 para todo"
    ASSERT_FALSE(h1 == HashHex{}(Hex{4, 8}));
    ASSERT_FALSE(HashHex{}(Hex{2, 2}) == HashHex{}(Hex{2, 3}));
}

TEST_CASE(DescubrirSinDuplicarCeldas) {
    Celdas mapa;
    ASSERT_TRUE(descubrir(mapa, Hex{0, 0}, "bosque"));
    ASSERT_FALSE(descubrir(mapa, Hex{0, 0}, "ciudad"));   // ya descubierta
    ASSERT_EQ(mapa.at(Hex{0, 0}), "bosque");              // sin cambios
    ASSERT_EQ(mapa.size(), 1u);
}

TEST_CASE(CaminarNoCreaEntradas) {
    Celdas mapa{{Hex{3, -2}, "lago"}};
    ASSERT_EQ(caminar(mapa, Hex{3, -2}), "lago");
    ASSERT_EQ(caminar(mapa, Hex{9, 9}), "desierto");
    ASSERT_EQ(mapa.size(), 1u);                  // NO se creó el {9, 9}
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 05 (tablero hexagonal)...\n";
    return RUN_ALL_TESTS();
}
