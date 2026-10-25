/* =============================================================================
   MÓDULO 04 - EJERCICIO 12: Reemplazar el arreglo crudo por std::vector
   =============================================================================
   DIFICULTAD: Intermedio
   PRACTICA: lección 17 (diagnóstico, std::vector como reemplazo seguro,
             at() vs [], algoritmos de la STL).

   CONSIGNA:
     1. copiarRango(const int* datos, std::size_t n): devolvé un vector con
        los n valores. Si datos es nullptr o n es 0, devolvé un vector vacío.
        No uses new ni delete.
     2. sumaSegura(const std::vector<int>& v): suma los elementos; 0 si está
        vacío.
     3. recortar(const std::vector<int>& v, std::size_t desde, std::size_t
        hasta): devolvé el subrango [desde, hasta). Si desde > hasta o
        hasta > v.size(), lanzá std::out_of_range. Usá at() para validar.
     4. sinDuplicados(std::vector<int> v): devolvé una copia ordenada y sin
        repetidos (sort + unique + erase). No modifiques el vector del
        llamador (se recibe por valor).
     5. contarEnRango(const std::vector<int>& v, int minimo, int maximo):
        contá los valores dentro del rango cerrado [minimo, maximo].

   COMPILACIÓN:
     g++ -std=c++20 -Wall -Wextra ejercicio_12_vector_seguro.cpp -o ej12 && ./ej12
   ============================================================================= */
#include <algorithm>
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <vector>
#include "../../include/test_framework.hpp"

// TODO: implementá las cinco funciones.
std::vector<int> copiarRango(const int* datos, std::size_t n) {
    (void)datos; (void)n;
    return {};  // placeholder
}

int sumaSegura(const std::vector<int>& v) {
    (void)v;
    return 0;  // placeholder
}

std::vector<int> recortar(const std::vector<int>& v, std::size_t desde,
                          std::size_t hasta) {
    (void)v; (void)desde; (void)hasta;
    return {};  // placeholder
}

std::vector<int> sinDuplicados(std::vector<int> v) {
    (void)v;
    return {};  // placeholder
}

std::size_t contarEnRango(const std::vector<int>& v, int minimo, int maximo) {
    (void)v; (void)minimo; (void)maximo;
    return 0;  // placeholder
}

// ============= TESTS =============
TEST_CASE(TestCopiarRango) {
    int datos[] = {4, 8, 15};
    std::vector<int> v = copiarRango(datos, 3);
    ASSERT_EQ(v.size(), 3u);
    ASSERT_EQ(v[0], 4);
    ASSERT_EQ(v[1], 8);
    ASSERT_EQ(v[2], 15);

    ASSERT_TRUE(copiarRango(nullptr, 3).empty());
    ASSERT_TRUE(copiarRango(datos, 0).empty());
}

TEST_CASE(TestSumaSegura) {
    ASSERT_EQ(sumaSegura({1, 2, 3, 4}), 10);
    ASSERT_EQ(sumaSegura({}), 0);
}

TEST_CASE(TestRecortar) {
    std::vector<int> v = {0, 1, 2, 3, 4};
    std::vector<int> r = recortar(v, 1, 4);
    ASSERT_EQ(r.size(), 3u);
    ASSERT_EQ(r[0], 1);
    ASSERT_EQ(r[1], 2);
    ASSERT_EQ(r[2], 3);

    ASSERT_TRUE(recortar(v, 2, 2).empty());
    ASSERT_EQ(v.size(), 5u);  // el original no se tocó
}

TEST_CASE(TestRecortarFueraDeRango) {
    std::vector<int> v = {0, 1, 2};
    bool lanzo = false;
    try {
        recortar(v, 1, 9);
    } catch (const std::out_of_range&) {
        lanzo = true;
    }
    ASSERT_TRUE(lanzo);

    lanzo = false;
    try {
        recortar(v, 2, 1);
    } catch (const std::out_of_range&) {
        lanzo = true;
    }
    ASSERT_TRUE(lanzo);
}

TEST_CASE(TestSinDuplicados) {
    std::vector<int> original = {3, 1, 3, 2, 1};
    std::vector<int> r = sinDuplicados(original);
    ASSERT_EQ(r.size(), 3u);
    ASSERT_EQ(r[0], 1);
    ASSERT_EQ(r[1], 2);
    ASSERT_EQ(r[2], 3);
    ASSERT_EQ(original.size(), 5u);  // no se modificó el llamador
}

TEST_CASE(TestContarEnRango) {
    std::vector<int> v = {-2, 0, 3, 5, 9};
    ASSERT_EQ(contarEnRango(v, 0, 5), 3u);
    ASSERT_EQ(contarEnRango(v, 10, 20), 0u);
    ASSERT_EQ(contarEnRango({}, 0, 10), 0u);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 12...\n";
    return RUN_ALL_TESTS();
}
