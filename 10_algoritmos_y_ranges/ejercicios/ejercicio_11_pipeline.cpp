/* =============================================================================
   MÓDULO 10 - EJERCICIO 11: pipeline lazy con views (lecciones 14, 15 y 16)
   =============================================================================
   DIFICULTAD: Intermedio
   CONSIGNA (tema 8 de TEMAS.md: ranges y views): materializar barato.
     Sobre una lista de puntajes (vector<int>):
       1. seleccionar(puntajes, umbral): ARMA (no ejecuta) el pipeline
          filter (> umbral) | transform (x2) | take (primeros 3).
          Devolvé la receta con auto (la L14: un view, nada más).
       2. materializar(puntajes, umbral): el vector CON los datos del
          pipeline, sin espacios de más: ranges::copy +
          back_inserter (L18) sobre un vector vacío.
       3. costoDeRevisiones(puntajes, umbral): cuántas veces corre el
          predicado del filter en UNA iteración completa del pipeline
          de `seleccionar` con ese umbral, contadas con una lambda
          mutable (L12). Dos reglas de la L16 que definen el número:
            - el take pide un elemento DE MAS despues de llenar sus 3
              (ese "probe" se evalua y luego se descarta: esto en un
              GCC moderno; razonalo con la L16);
            - si ya no hay más elementos debajo, el next-check no
              invoca al filter (no se revisa lo que no existe).
       4. resumenEager(puntajes, umbral): el mismo resultado SIN views
          copy_if, el transform in-place sobre la copia y un resize al
          nuevo fin. Y CORTÁ en 3 (el take de la receta): la comparación
          tiene que dar exactamente lo mismo que materializar.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_11_pipeline.cpp -o ej11 && ./ej11
   ============================================================================= */
#include <algorithm>
#include <ranges>
#include <vector>
#include "../../include/test_framework.hpp"

// TODO 1: seleccionar: ARMAR el pipeline (filter | transform | take(3)).
// TU CODIGO AQUI (devolver la receta con auto):
auto seleccionar([[maybe_unused]] const std::vector<int>& puntajes,
                 [[maybe_unused]] int umbral) {
    return std::vector<int>{};    // placeholder
}

// TODO 2: materializar: ranges::copy + back_inserter a un vector VACIO.
// TU CODIGO AQUI:
std::vector<int> materializar([[maybe_unused]] const std::vector<int>& puntajes,
                              [[maybe_unused]] int umbral) {
    return {};    // placeholder
}

// TODO 3: costoDeRevisiones: cuántas veces corre el filtro por iteración.
// TU CODIGO AQUI:
int costoDeRevisiones([[maybe_unused]] const std::vector<int>& puntajes,
                      [[maybe_unused]] int umbral) {
    return 0;    // placeholder
}

// TODO 4: resumenEager: el mismo resultado SIN views (copy_if + transform).
// TU CODIGO AQUI:
std::vector<int> resumenEager([[maybe_unused]] const std::vector<int>& puntajes,
                              [[maybe_unused]] int umbral) {
    return {};    // placeholder
}

// ============= TESTS =============
TEST_CASE(MaterializarElPipeline) {
    const std::vector<int> puntajes{10, 20, 30, 40, 50, 60};
    // > 20: 30 40 50 60; x2: 60 80 100 120; take(3): 60 80 100
    ASSERT_TRUE((materializar(puntajes, 20) == std::vector<int>{60, 80, 100}));
    // > 30: 40 50 60; x2: 80 100 120; take(3): todos
    ASSERT_TRUE((materializar(puntajes, 30) == std::vector<int>{80, 100, 120}));
    // > 100: nada pasa
    ASSERT_TRUE(materializar(puntajes, 100).empty());
}

TEST_CASE(EagerYLazyDanLoMismo) {
    const std::vector<int> puntajes{10, 20, 30, 40, 50, 60};
    ASSERT_TRUE((materializar(puntajes, 20) == resumenEager(puntajes, 20)));
    ASSERT_TRUE((materializar(puntajes, 30) == resumenEager(puntajes, 30)));
    ASSERT_TRUE((materializar(puntajes, 100) == resumenEager(puntajes, 100)));
}

TEST_CASE(CostoDeRevisionesDelFilter) {
    // umbral 20: fallan 10 y 20, pasan 30 40 50 (take se llena en el 50)
    // y la próxima revisión es el 60 (el probe que se descarta):
    ASSERT_EQ(costoDeRevisiones({10, 20, 30, 40, 50, 60}, 20), 6);
    // umbral 10: falla solo el 10, pasan 35 40 50 (todos): no hay nada
    // de mas para comprobar: no hay probe:
    ASSERT_EQ(costoDeRevisiones({10, 35, 40, 50}, 10), 4);
    // umbral 20: fallan 1 y 20, pasan 30 99 99 (el 3ro es EL ULTIMO):
    // sin probe (no hay quien revisar):
    ASSERT_EQ(costoDeRevisiones({1, 20, 30, 99, 99}, 20), 5);
}

TEST_CASE(LaRecetaSeArmaSinEjecutar) {
    const std::vector<int> puntajes{10, 20, 30, 40, 50, 60};
    auto receta = seleccionar(puntajes, 20);
    (void)receta;                              // armala y no la uses
    ASSERT_EQ(materializar(puntajes, 20).size(), 3u);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 11 (pipeline)...\n";
    return RUN_ALL_TESTS();
}
