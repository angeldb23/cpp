/* =============================================================================
   MÓDULO 10 - EJERCICIO 04: transform, copy_if, fill, generate, iota (L05 y L06)
   =============================================================================
   DIFICULTAD: Intermedio
   CONSIGNA (temas 3 de TEMAS.md: algoritmos modificantes): el cuartel.
     Todas las funciones MANIPULAN secuencias ya existentes (acá no hay
     push_back después de los algoritmos: los algoritmos escriben sobre
     espacio que ya está), usa los algoritmos de las lecciones 05 y 06:
       1. hpDe(niveles, salida): rellena `salida` (ya tiene espacio:
          salida.resize YA lo hiciste) con niveles * 12 usando transform
          unario (la lección 05).
       2. mayoresQue(datos, limite, salida): copia con copy_if los
          elementos > limite. `salida` llega con lugar de sobra ya hecho
          (resize a datos.size() lo hizo el caller): después de copiar,
          achicalo al tamaño real con std::distance y el iterador del
          nuevo fin (el patrón de la lección 05).
       3. resetear(barras): pone TODAS las barras en 0 con fill.
       4. proximasTanda(cola, base): con generate, escribe en cada lugar
          de `cola` los valores base, base*2, base*3... (un lambda
          mutable con contador, como el ejemplo de la lección 06).
       5. ids(c, inicio): pisa el contenido de `c` con consecutivos que
          empiezan en `inicio` usando iota.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_04_cuartel.cpp -o ej04 && ./ej04
   ============================================================================= */
#include <algorithm>
#include <numeric>
#include <vector>
#include "../../include/test_framework.hpp"

// TODO 1: hpDe: transform unario nivel * 12 sobre la salida ya con espacio.
// TU CODIGO AQUI:
void hpDe([[maybe_unused]] const std::vector<int>& niveles,
          [[maybe_unused]] std::vector<int>& salida) {
}

// TODO 2: mayoresQue: copy_if + resize (el patrón de copy_if con resize).
// TU CODIGO AQUI:
void mayoresQue([[maybe_unused]] const std::vector<int>& datos,
                [[maybe_unused]] int limite,
                [[maybe_unused]] std::vector<int>& salida) {
}

// TODO 3: resetear: fill 0 en toda la secuencia.
// TU CODIGO AQUI:
void resetear([[maybe_unused]] std::vector<int>& barras) {
}

// TODO 4: proximasTanda: generate escribiendo base, 2*base, 3*base...
// TU CODIGO AQUI:
void proximasTanda([[maybe_unused]] std::vector<int>& cola,
                   [[maybe_unused]] int base) {
}

// TODO 5: ids: iota desde `inicio`.
// TU CODIGO AQUI:
void ids([[maybe_unused]] std::vector<int>& c,
         [[maybe_unused]] int inicio) {
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
    salida.resize(datos.size());            // espacio de sobra
    mayoresQue(datos, 40, salida);
    ASSERT_EQ(salida.size(), 3u);           // 45 80 92
    ASSERT_TRUE(salida[0] == 45 && salida[1] == 80 && salida[2] == 92);
    ASSERT_TRUE((salida == std::vector<int>{45, 80, 92}));
}

TEST_CASE(ResetearConFill) {
    std::vector<int> barras{3, 7, 2, 9};
    resetear(barras);
    ASSERT_TRUE((barras == std::vector<int>{0, 0, 0, 0}));
}

TEST_CASE(ProximaTandaConGenerate) {
    std::vector<int> cola(3);
    proximasTanda(cola, 35);
    ASSERT_TRUE((cola == std::vector<int>{35, 70, 105}));
    std::vector<int> mini(2);
    proximasTanda(mini, 10);
    ASSERT_TRUE((mini == std::vector<int>{10, 20}));
    std::vector<int> vacio;
    proximasTanda(vacio, 10);               // no rompe: no conoce nada
    ASSERT_TRUE(vacio.empty());
}

TEST_CASE(IdsSecuenciadas) {
    std::vector<int> c(4);
    ids(c, 100);
    ASSERT_TRUE(c == std::vector<int>(std::vector<int>{100, 101, 102, 103}));
    std::vector<int> par{9, 9, 9, 9, 9};
    ids(par, -2);
    ASSERT_TRUE((par == std::vector<int>{-2, -1, 0, 1, 2}));
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 04 (cuartel)...\n";
    return RUN_ALL_TESTS();
}
