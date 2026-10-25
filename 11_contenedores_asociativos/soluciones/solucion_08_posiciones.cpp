/* =============================================================================
   MÓDULO 11 - SOLUCIÓN 08: geometría con tuples (ejercicio 08)
   =============================================================================
   L11-L12 son las lecciones de respaldo: structured bindings sobre el
   tuple, std::apply para la suma en una sola llamada, y la reutilización
   de la función de distancia para el chequeo de rango.
   ============================================================================= */
#include <cmath>
#include <iostream>
#include <tuple>
#include "../../include/test_framework.hpp"

using Posicion = std::tuple<float, float>;

// 1) structured bindings sobre el tuple (L12):
float distanciaAlOrigen(const Posicion& p) {
    const auto [x, y] = p;
    return x * x + y * y;
}

// 2) std::apply: el tuple se convierte en la lista de argumentos (L12):
Posicion sumar(const Posicion& a, const Posicion& b) {
    return std::apply(
        [](float ax, float ay, float bx, float by) {
            return std::make_tuple(ax + bx, ay + by);
        },
        std::tuple_cat(a, b));
}

bool enRango(const Posicion& t, float radio) {
    return distanciaAlOrigen(t) <= radio * radio;   // reutiliza TODO 1
}

// 4) otro binding: la otra cara es "impido" al espejo (L12):
Posicion reflejar(const Posicion& t) {
    const auto [x, y] = t;
    return {-x, -y};
}

// ============= TESTS =============
TEST_CASE(DistanciaAlOrigenAlCuadrado) {
    ASSERT_NEAR(distanciaAlOrigen(Posicion{3.f, 4.f}), 25.f, 0.001f);
    ASSERT_NEAR(distanciaAlOrigen(Posicion{0.f, 0.f}), 0.f, 0.001f);
    ASSERT_NEAR(distanciaAlOrigen(Posicion{-3.f, -4.f}), 25.f, 0.001f);
}

TEST_CASE(SumaComponenteAComponente) {
    const Posicion s = sumar(Posicion{1.f, 2.f}, Posicion{10.f, 20.f});
    ASSERT_NEAR(std::get<0>(s), 11.f, 0.001f);
    ASSERT_NEAR(std::get<1>(s), 22.f, 0.001f);
}

TEST_CASE(EnRangoUsaLaMismaFuncion) {
    ASSERT_TRUE(enRango(Posicion{3.f, 3.f}, 5.f));
    ASSERT_FALSE(enRango(Posicion{3.f, 3.f}, 4.f));
    ASSERT_TRUE(enRango(Posicion{0.f, 0.f}, 0.f));
}

TEST_CASE(ReflejarInvierteAmbosComponentes) {
    const Posicion r = reflejar(Posicion{2.f, -8.f});
    ASSERT_NEAR(std::get<0>(r), -2.f, 0.001f);
    ASSERT_NEAR(std::get<1>(r), 8.f, 0.001f);
}

int main() {
    std::cout << "Ejecutando pruebas para Solucion 08 (geometria con tuples)...\n";
    return RUN_ALL_TESTS();
}
