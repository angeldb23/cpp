/* =============================================================================
   MÓDULO 09 - EJERCICIO 11: Fábrica variádica con forwarding (lección 07)
   =============================================================================
   DIFICULTAD: Avanzado
   CONSIGNA (temas 7 y 8 de TEMAS.md: forwarding con packs y collapsing):
     La lección 07 fue del wrapper simple a la fábrica estilo
     std::make_unique. Ahora te toca a vos la versión genérica:

     1. fabricar<T>(Args&&... args): construye un T con ESOS argumentos y
        devuelve un std::unique_ptr<T>. Clave: cada argumento debe llegar
        al constructor de T con su categoría original (forward por cada uno).
     2. struct Carta: tipo rastreado que recibe (std::string, int). La
        struct ya está: mirá que construcción + copia/movimiento se
        cuentan en copias/movimientos estáticos.
     3. guardar(mazo, args...): agrega AL MAZO una carta construida en
        el lugar" (igual que emplace_back): fabrica la Carta con forwarding
        y la mete con std::move al push_back (la carta es nueva: se puede
        mover sin copia).
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_11_fabrica.cpp -o ej11 && ./ej11
   ============================================================================= */
#include <iostream>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>
#include "../../include/test_framework.hpp"

// Tipo rastreado: cuenta copias y movimientos para VER el forwarding.
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

// TODO 1: la fábrica variádica: cada Args llega al constructor de T con su
//         categoría original. Devolvé un std::unique_ptr<T>.
// TU CÓDIGO AQUÍ:
template <typename T, typename... Args>
std::unique_ptr<T> fabricar([[maybe_unused]] Args&&... args) {
    return nullptr;   // placeholder: reemplazalo por la construcción real
}

// TODO 2: construir en el lugar: la carta NUEVA entra al mazo por move.
//         (pista: fabricar + push_back(std::move(...)))
void guardar([[maybe_unused]] std::vector<Carta>& mazo, [[maybe_unused]] std::string nombre,
             [[maybe_unused]] int valor) {
    // TU CÓDIGO AQUÍ (reemplazá este comentario)
}

// ============= TESTS =============
TEST_CASE(TestFabricarRvalueMueve) {
    reiniciar();
    auto c = fabricar<Carta>(std::string("as"), 14);   // ambos rvalue: 0 copias
    ASSERT_TRUE(c != nullptr);
    ASSERT_EQ(c->nombre, std::string("as"));
    ASSERT_EQ(c->valor, 14);
    ASSERT_EQ(Carta::copias, 0);
    ASSERT_EQ(Carta::movimientos, 0);
}

TEST_CASE(TestFabricarLvalueCopiaSoloLoNecesario) {
    reiniciar();
    std::string nombre = "rey";
    auto c = fabricar<Carta>(nombre, 13);              // lvalue string: 1 copia
    ASSERT_TRUE(c != nullptr);
    ASSERT_EQ(c->nombre, std::string("rey"));
    ASSERT_EQ(Carta::copias, 0);                      // Carta en sí no se copia
    ASSERT_EQ(nombre, std::string("rey"));            // el dueño queda intacto
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
    ASSERT_EQ(Carta::copias, 0);                      // entró por move, no copia
    ASSERT_EQ(Carta::movimientos, 1);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 11 (fábrica con forwarding)...\n";
    return RUN_ALL_TESTS();
}
