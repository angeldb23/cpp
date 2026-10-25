/* =============================================================================
   MÓDULO 09 - SOLUCIÓN 11: Fábrica variádica con forwarding
   =============================================================================
   POR QUÉ FUNCIONA:
     - Args&&... es un pack de referencias universales: cada argumento deduce
       su propio U (referencia si era lvalue, el tipo pelado si era rvalue).
     - std::forward<Args>(args)... reenvía CADA argumento a su categoría
       original: los rvalue strings no se copian, los lvalue se copian UNA
       vez (el dueño queda intacto) y Carta nunca se copia entera.
     - La Carta recién construida es temporaria: por eso guardar la mete
       al mazo con std::move: un único movimiento, nada de copias.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra solucion_11_fabrica.cpp -o sol11 && ./sol11
   ============================================================================= */
#include <iostream>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>
#include "../../include/test_framework.hpp"

class Carta {
public:
    static inline int copias = 0;
    static inline int movimientos = 0;

    std::string nombre;
    int valor = 0;

    Carta(std::string n, int v) : nombre(std::move(n)), valor(v) {}
    Carta(const Carta& o) : nombre(o.nombre), valor(o.valor) { ++copias; }
    Carta(Carta&& o) noexcept : nombre(std::move(o.nombre)), valor(o.valor) {
        ++movimientos;
    }
};

static void reiniciar() {
    Carta::copias = 0;
    Carta::movimientos = 0;
}

template <typename T, typename... Args>
std::unique_ptr<T> fabricar(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

void guardar(std::vector<Carta>& mazo, std::string nombre, int valor) {
    mazo.push_back(std::move(*fabricar<Carta>(std::move(nombre), valor)));
}

// ============= TESTS (los mismos del ejercicio) =============
TEST_CASE(TestFabricarRvalueMueve) {
    reiniciar();
    auto c = fabricar<Carta>(std::string("as"), 14);
    ASSERT_TRUE(c != nullptr);
    ASSERT_EQ(c->nombre, std::string("as"));
    ASSERT_EQ(c->valor, 14);
    ASSERT_EQ(Carta::copias, 0);
    ASSERT_EQ(Carta::movimientos, 0);
}

TEST_CASE(TestFabricarLvalueCopiaSoloLoNecesario) {
    reiniciar();
    std::string nombre = "rey";
    auto c = fabricar<Carta>(nombre, 13);
    ASSERT_TRUE(c != nullptr);
    ASSERT_EQ(c->nombre, std::string("rey"));
    ASSERT_EQ(Carta::copias, 0);
    ASSERT_EQ(nombre, std::string("rey"));
}

TEST_CASE(TestFabricarSigueSiendoUniquePtr) {
    auto c = fabricar<Carta>(std::string("sota"), 10);
    ASSERT_TRUE(c != nullptr);
    static_assert(std::is_same_v<decltype(c), std::unique_ptr<Carta>>);
}

TEST_CASE(TestGuardarmeConstruidaEnElLugar) {
    reiniciar();
    std::vector<Carta> mazo;
    guardar(mazo, std::string("caballero"), 11);
    ASSERT_EQ(mazo.size(), 1u);
    ASSERT_EQ(mazo[0].nombre, std::string("caballero"));
    ASSERT_EQ(mazo[0].valor, 11);
    ASSERT_EQ(Carta::copias, 0);
    ASSERT_EQ(Carta::movimientos, 1);
}

int main() {
    std::cout << "Ejecutando pruebas para Solución 11 (fábrica con forwarding)...\n";
    return RUN_ALL_TESTS();
}
