/* =============================================================================
   MÓDULO 11 - EJERCICIO 08: geometría con tuples (lecciones 11 y 12)
   =============================================================================
   DIFICULTAD: Medio
   CONSIGNA (tema 6 de TEMAS.md: tuple, structured bindings y std::apply):
     Un sistema de posiciones trabaja con std::tuple<float, float>
     (x, y) para no crear structs. Completá:
       1. distanciaAlOrigen(p): la distancia AL CUADRADO entre el origen
          (0,0) y la posición (x y): get<0>/get<1> o structured
          bindings de la L11/L12.
       2. sumar(a, b): el tuple componente a componente: con std::apply
          (L12) queda en UNA línea; con get, dos: como vos prefieras.
       3. enRango(t, radio): true si distanciaAlOrigen(t) <= radio*radio.
          (reutilizá TU función 1: no dupliques el cálculo.)
       4. reflejar(t): devuelve el tuple (-x, -y): prontísimo "mira por
          espejo": si no hay nada "que reflejar": (los dos valores).
   (REFERENCIA: "tie con std::ignore" y "structured bindings" están en
   la L11-L12; usá lo que sea natural).
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_08_posiciones.cpp -o ej08 && ./ej08
   ============================================================================= */
#include <iostream>
#include <tuple>
#include "../../include/test_framework.hpp"

using Posicion = std::tuple<float, float>;

// TODO 1: distancia al origen ELEVADA AL CUADRADO (sin raíz: los tests
//         van con floats y ASSERT_NEAR).
float distanciaAlOrigen([[maybe_unused]] const Posicion& p) {
    return 0.0f;    // placeholder
}

// TODO 2: suma componente a componente.
Posicion sumar([[maybe_unused]] const Posicion& a,
               [[maybe_unused]] const Posicion& b) {
    return {0.0f, 0.0f};    // placeholder
}

// TODO 3: true si distanciaAlOrigen(t) <= radio al cuadrado.
bool enRango([[maybe_unused]] const Posicion& t,
             [[maybe_unused]] float radio) {
    return false;    // placeholder
}

// TODO 4: reflejar: devuelve el tuple (-x, -y).
Posicion reflejar([[maybe_unused]] const Posicion& t) {
    return {0.0f, 0.0f};    // placeholder
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
    std::cout << "Ejecutando pruebas para Ejercicio 08 (geometria con tuples)...\n";
    return RUN_ALL_TESTS();
}
