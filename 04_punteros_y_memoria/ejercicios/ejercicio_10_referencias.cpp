/* =============================================================================
   MÓDULO 04 - EJERCICIO 10: Referencias vs punteros en APIs
   =============================================================================
   DIFICULTAD: Intermedio
   PRACTICA: lección 14 (referencias, alias, const&, referencia a elemento,
             cuándo usar referencia y cuándo puntero).

   CONSIGNA:
     1. escalarVector(std::vector<int>& v, int factor): multiplicá cada
        elemento por factor. Debe modificar el vector del llamador.
     2. intercambiarRef(int& a, int& b): intercambiá los valores usando
        referencias.
     3. masLargo(const std::string& a, const std::string& b): devolvé una
        referencia const al string más largo. Si tienen igual longitud,
        devolvé a. No copies los strings.
     4. elementoEn(std::vector<int>& v, std::size_t i): devolvé una referencia
        al elemento i. Si i está fuera de rango, lanzá std::out_of_range.
     5. duplicarPorPuntero(int* p): si p no es nullptr, duplicalo. Es la
        versión con puntero para comparar con la referencia.

   COMPILACIÓN:
     g++ -std=c++20 -Wall -Wextra ejercicio_10_referencias.cpp -o ej10 && ./ej10
   ============================================================================= */
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include "../../include/test_framework.hpp"

// TODO: implementá las cinco funciones.
void escalarVector(std::vector<int>& v, int factor) {
    (void)v; (void)factor;  // placeholder
}

void intercambiarRef(int& a, int& b) {
    (void)a; (void)b;  // placeholder
}

const std::string& masLargo(const std::string& a, const std::string& b) {
    (void)b;
    return a;  // placeholder
}

int& elementoEn(std::vector<int>& v, std::size_t i) {
    (void)v; (void)i;
    static int dummy = 0;
    return dummy;  // placeholder
}

void duplicarPorPuntero(int* p) {
    (void)p;  // placeholder
}

// ============= TESTS =============
TEST_CASE(TestEscalarVector) {
    std::vector<int> v = {1, 2, 3};
    escalarVector(v, 3);
    ASSERT_EQ(v[0], 3);
    ASSERT_EQ(v[1], 6);
    ASSERT_EQ(v[2], 9);
}

TEST_CASE(TestIntercambiarRef) {
    int a = 1;
    int b = 2;
    intercambiarRef(a, b);
    ASSERT_EQ(a, 2);
    ASSERT_EQ(b, 1);
}

TEST_CASE(TestMasLargo) {
    std::string corto = "corto";
    std::string largo = "mucho mas largo";
    const std::string& r = masLargo(corto, largo);
    ASSERT_TRUE(&r == &largo);
    ASSERT_EQ(r, largo);
}

TEST_CASE(TestMasLargoEmpate) {
    std::string a = "abc";
    std::string b = "xyz";
    ASSERT_TRUE(&masLargo(a, b) == &a);
}

TEST_CASE(TestElementoEn) {
    std::vector<int> v = {10, 20, 30};
    ASSERT_EQ(elementoEn(v, 1), 20);

    elementoEn(v, 1) = 42;  // modifica el vector a través de la referencia
    ASSERT_EQ(v[1], 42);
}

TEST_CASE(TestElementoFueraDeRango) {
    std::vector<int> v = {1, 2, 3};
    bool lanzo = false;
    try {
        elementoEn(v, 10);
    } catch (const std::out_of_range&) {
        lanzo = true;
    }
    ASSERT_TRUE(lanzo);
}

TEST_CASE(TestDuplicarPorPuntero) {
    int x = 7;
    duplicarPorPuntero(&x);
    ASSERT_EQ(x, 14);
    duplicarPorPuntero(nullptr);  // no debe romper
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 10...\n";
    return RUN_ALL_TESTS();
}
