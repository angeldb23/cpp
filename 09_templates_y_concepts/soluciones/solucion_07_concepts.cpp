/* =============================================================================
   MÓDULO 09 - SOLUCIÓN 07: Concepts de C++20
   =============================================================================
   POR QUÉ FUNCIONA:
     - Un concept es un nombre para una condición booleana evaluada en compilación.
     - requires { expr } pregunta si la expresión compila; '-> C' además exige que su tipo cumpla C.
     - Si dos sobrecargas aplican, el compilador elige la más restringida: por eso 'otro' va último.
     - PuedeDuplicar<std::string> es false (y no un error) porque dentro de un requires una falla es solo 'no cumple'.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra solucion_07_concepts.cpp -o sol07 && ./sol07
   ============================================================================= */
#include <iostream>
#include <string>
#include <vector>
#include <concepts>
#include <cstddef>
#include <utility>
#include "../../include/test_framework.hpp"

template <typename T>
concept Numerico = std::integral<T> || std::floating_point<T>;

// Una requires-expression pregunta "¿compila esta expresión?". El '-> concepto'
// exige además que el RESULTADO cumpla otra condición.
template <typename T>
concept ConSize = requires(const T& t) {
    { t.size() } -> std::convertible_to<std::size_t>;
};

template <typename T>
concept Imprimible = requires(std::ostream& os, const T& t) {
    os << t;
};

// Restricción en la lista de parámetros del template
template <Numerico T>
T duplicar(T x) {
    return x * 2;
}

// Misma idea con una cláusula requires
template <typename T>
    requires ConSize<T>
std::size_t cantidad(const T& c) {
    return c.size();
}

// Sobrecarga por concepts: gana la versión MÁS restringida que aplique.
template <Numerico T>
std::string describir(const T&) { return "numero"; }

template <ConSize T>
std::string describir(const T&) { return "coleccion"; }

template <typename T>
std::string describir(const T&) { return "otro"; }

// ============= TESTS (los mismos del ejercicio) =============
struct SinSalida {};   // un tipo que no sabe imprimirse con <<

// Concept auxiliar de los tests: ¿se puede llamar duplicar con un T?
template <typename T>
concept PuedeDuplicar = requires(T t) { duplicar(t); };

TEST_CASE(TestConceptNumerico) {
    ASSERT_TRUE(Numerico<int>);
    ASSERT_TRUE(Numerico<double>);
    ASSERT_TRUE(Numerico<long>);
    ASSERT_FALSE(Numerico<std::string>);
    ASSERT_FALSE(Numerico<std::vector<int>>);
}

TEST_CASE(TestConceptConSize) {
    ASSERT_TRUE(ConSize<std::vector<int>>);
    ASSERT_TRUE(ConSize<std::string>);
    ASSERT_FALSE(ConSize<int>);
    ASSERT_FALSE(ConSize<SinSalida>);
}

TEST_CASE(TestConceptImprimible) {
    ASSERT_TRUE(Imprimible<int>);
    ASSERT_TRUE(Imprimible<std::string>);
    ASSERT_FALSE(Imprimible<SinSalida>);
    ASSERT_FALSE(Imprimible<std::vector<int>>);
}

TEST_CASE(TestDuplicar) {
    ASSERT_EQ(duplicar(21), 42);
    ASSERT_NEAR(duplicar(1.5), 3.0, 1e-9);
    ASSERT_TRUE(PuedeDuplicar<int>);
    ASSERT_FALSE(PuedeDuplicar<std::string>);   // el concepto la rechaza
}

TEST_CASE(TestCantidad) {
    std::vector<int> v = {1, 2, 3};
    ASSERT_EQ(cantidad(v), 3u);
    ASSERT_EQ(cantidad(std::string("hola")), 4u);
}

TEST_CASE(TestDescribirPorConcepts) {
    std::vector<int> v = {1};
    ASSERT_EQ(describir(5), std::string("numero"));
    ASSERT_EQ(describir(2.5), std::string("numero"));
    ASSERT_EQ(describir(v), std::string("coleccion"));
    ASSERT_EQ(describir(std::string("x")), std::string("coleccion"));
    ASSERT_EQ(describir(SinSalida{}), std::string("otro"));
}

int main() {
    std::cout << "Ejecutando pruebas para Solución 07 (Concepts de C++20)...\n";
    return RUN_ALL_TESTS();
}
