/* =============================================================================
   MÓDULO 09 - EJERCICIO 07: Concepts de C++20
   =============================================================================
   DIFICULTAD: Avanzado
   CONSIGNA:
     Reemplazá el 'template aceptá-todo' por restricciones claras con concepts:
     1. concept Numerico: tipos enteros o de coma flotante (usá std::integral y std::floating_point)
     2. concept ConSize: tipos con un método .size() convertible a std::size_t (requires-expression)
     3. concept Imprimible: tipos que se pueden enviar a un std::ostream con <<
     4. duplicar(x): solo para Numerico, devuelve x * 2
     5. cantidad(c): solo para ConSize, devuelve c.size()
     6. describir(x): tres sobrecargas -> "numero", "coleccion" u "otro"
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_07_concepts.cpp -o ej07 && ./ej07
   ============================================================================= */
#include <iostream>
#include <string>
#include <vector>
#include <concepts>
#include <cstddef>
#include <utility>
#include "../../include/test_framework.hpp"

// Los concepts de abajo valen 'false' para todo tipo: reescribilos.
// TODO 1: Numerico = std::integral<T> || std::floating_point<T>
// TODO 2: ConSize = requires(const T& t) { { t.size() } -> std::convertible_to<std::size_t>; }
// TODO 3: Imprimible = requires(std::ostream& os, const T& t) { os << t; }
// TU CÓDIGO AQUÍ:
template <typename T> concept Numerico   = false;
template <typename T> concept ConSize    = false;
template <typename T> concept Imprimible = false;

// TODO 4: restringí duplicar con Numerico y devolvé x * 2
// TU CÓDIGO AQUÍ:
template <typename T>
T duplicar(T x) {
    return x;   // placeholder
}

// TODO 5: restringí cantidad con ConSize y devolvé c.size()
// TU CÓDIGO AQUÍ:
template <typename T>
std::size_t cantidad([[maybe_unused]] const T& c) {
    return 0;   // placeholder
}

// TODO 6: agregá dos sobrecargas restringidas (Numerico -> "numero", ConSize -> "coleccion").
//         La genérica de abajo se queda como último recurso.
// TU CÓDIGO AQUÍ:
template <typename T>
std::string describir([[maybe_unused]] const T& x) {
    return "otro";
}

// ============= TESTS =============
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
    std::cout << "Ejecutando pruebas para Ejercicio 07 (Concepts de C++20)...\n";
    return RUN_ALL_TESTS();
}
