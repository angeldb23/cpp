/* =============================================================================
   MÓDULO 10 - SOLUCIÓN 10: lambdas con estado (lecciones 12 y 13)
   =============================================================================
   Resolución:
       - fabricaContador: la lambda con init-capture `[n = 0]` y
         `mutable`: el estado vive DENTRO del objeto lambda; devolverla
         con auto no la "functionifica" (su tipo es único).
       - acumularHistoria: el parámetro es UNA REFERENCIA RVALUE (&&) y
         el init-capture `[g = std::move(historia)]` mueve el ORIGINAL
         del caller a la lambda (si aceptara por valor, moveríamos la
         copia y el del caller seguiría lleno: el por-valor pierde el
         traspaso de propiedad).
       - ordenarPorHp: UNA plantilla que funciona con la misma lambda
         genérica (L13) para cualquier tipo con .hp comparable.
   ============================================================================= */
#include <algorithm>
#include <string>
#include <utility>
#include <vector>
#include "../../include/test_framework.hpp"

struct Crew {
    std::string nombre;
    int hp;
};

struct Torreta {
    std::string nombre;
    int hp;
};

// SOLUCIÓN 1: init-capture + mutable = contador personal:
auto fabricaContador() {
    return [n = 0]() mutable { return ++n; };
}

// SOLUCIÓN 2: el vector viaja en move: el parámetro es && (se liga al
// original del caller) y el init-capture lo mueve a la lambda:
auto acumularHistoria(std::vector<std::string>&& historia) {
    return [guardadas = std::move(historia)]() {
        return guardadas.size();     // cuántos elementos guarda
    };
}

// SOLUCIÓN 3: la lambda genérica (L13) fuera del sort:
template <typename Secuencia>
void ordenarPorHp(Secuencia& secuencia) {
    const auto compararPorHp = [](const auto& a, const auto& b) {
        return a.hp < b.hp;          // generico: Crew, Torreta, lo que venga
    };
    std::sort(secuencia.begin(), secuencia.end(), compararPorHp);
}

// ============= TESTS =============
TEST_CASE(CadaContadorEsIndependiente) {
    auto a = fabricaContador();
    auto b = fabricaContador();
    ASSERT_EQ(a(), 1);
    ASSERT_EQ(a(), 2);
    ASSERT_EQ(b(), 1);
    ASSERT_EQ(a(), 3);
}

TEST_CASE(ContadorNoColapsoState) {
    auto c = fabricaContador();
    (void)c(); (void)c();
    ASSERT_EQ(c(), 3);
}

TEST_CASE(HistoriaGuardadaPorMove) {
    std::vector<std::string> historia{"tanda1", "tanda2", "tanda3"};
    // el std::move en la llamada es el traspaso de propiedad (08):
    auto guardia = acumularHistoria(std::move(historia));
    ASSERT_EQ(guardia(), 3);
    ASSERT_TRUE(historia.empty());
    auto copia = guardia;
    ASSERT_EQ(copia(), 3);
}

TEST_CASE(OrdenarGenericoPorHp) {
    std::vector<Crew> equipo{
        {"Quesito", 90}, {"Mordel", 60}, {"Turbo", 120},
    };
    std::vector<Torreta> defensas{
        {"Pistola", 70}, {"Cañon", 30},
    };
    ordenarPorHp(equipo);
    ordenarPorHp(defensas);
    ASSERT_EQ(equipo[0].hp, 60);
    ASSERT_EQ(equipo[2].hp, 120);
    ASSERT_EQ(defensas[0].hp, 30);
    ASSERT_EQ(defensas[1].hp, 70);
}

int main() {
    std::cout << "Solución 10 (lambdas)...\n";
    return RUN_ALL_TESTS();
}
