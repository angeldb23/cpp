/* =============================================================================
   MÓDULO 10 - SOLUCIÓN 11: pipeline lazy con views (lecciones 14, 15 y 16)
   =============================================================================
   Resolución:
       - seleccionar: la receta se ARMA con el pipe: filter | transform |
         take(3). Un view no recorre nada hasta que alguien lo itera (L14):
         devolverlo con auto es devolver "la promesa", no los datos.
       - materializar: ranges::copy + back_inserter (L18) drena la receta a
         un vector vacío: el back_inserter crece justo lo justo.
       - costoDeRevisiones: el contador vive AFUERA y la lambda mutable lo
         captura por referencia (L12): las copias internas del predicado
         dentro del view siguen apuntando al MISMO contador. GCC: el take
         pide un elemento DE MÁS después de llenar sus 3 (el probe se
         evalúa y se descarta), pero si el fuente se agotó no hay probe.
       - resumenEager: el mismo dato SIN vistas: copy_if + transform
         in-place sobre la copia + cortar a 3 con resize (solo si sobran).
   ============================================================================= */
#include <algorithm>
#include <ranges>
#include <vector>
#include "../../include/test_framework.hpp"

// SOLUCIÓN 1: armar la receta (filter | transform | take), sin ejecutarla:
auto seleccionar(const std::vector<int>& puntajes, int umbral) {
    return puntajes |
           std::views::filter([umbral](int v) { return v > umbral; }) |
           std::views::transform([](int v) { return v * 2; }) |
           std::views::take(3);
}

// SOLUCIÓN 2: materializar la receta en un vector vacío:
std::vector<int> materializar(const std::vector<int>& puntajes, int umbral) {
    std::vector<int> salida;
    std::ranges::copy(seleccionar(puntajes, umbral),
                      std::back_inserter(salida));
    return salida;
}

// SOLUCIÓN 3: cuántas veces corre el predicado del filter en UNA pasada:
int costoDeRevisiones(const std::vector<int>& puntajes, int umbral) {
    int revisiones = 0;
    auto receta = puntajes |
                  std::views::filter([umbral, &revisiones](int v) mutable {
                      ++revisiones;
                      return v > umbral;
                  }) |
                  std::views::transform([](int v) { return v * 2; }) |
                  std::views::take(3);
    // UNA iteración completa (el drenaje completa el conteo):
    std::ranges::for_each(receta, [](int) {});
    return revisiones;
}

// SOLUCIÓN 4: el mismo resultado SIN views (eager):
std::vector<int> resumenEager(const std::vector<int>& puntajes, int umbral) {
    std::vector<int> salida;
    std::copy_if(puntajes.begin(), puntajes.end(),
                 std::back_inserter(salida),
                 [umbral](int v) { return v > umbral; });
    std::transform(salida.begin(), salida.end(), salida.begin(),
                   [](int v) { return v * 2; });
    const std::size_t corte = std::min<std::size_t>(salida.size(), 3);
    salida.resize(corte);    // el take: solo corta, nunca rellena
    return salida;
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
    std::cout << "Solución 11 (pipeline)...\n";
    return RUN_ALL_TESTS();
}
