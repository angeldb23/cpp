/* =============================================================================
   MÓDULO 06 - EJERCICIO 09: Identificar tipo real: dynamic_cast y typeid
   =============================================================================
   DIFICULTAD: Medio
   PRACTICA: lecciones 11 y 12 (upcast/downcast, dynamic_cast con puntero
   y referencia, typeid dinámico vs estático).

   CONSIGNA:
   La jerarquía `Guardian` está DADA (no la toques): dos derivadas con
   métodos PROPIOS distintos (Knight tiene escudo; Mago tiene grimorio).

   1. `std::string identificar(const Guardian* g)`:
      - si el tipo real es Knight  -> "knight"
      - si el tipo real es Mago    -> "mago"
      - si es nullptr o desconocido-> "desconocido"
      USÁ dynamic_cast (con punteros: nullptr es la falla segura).
   2. `const Knight* comoKnight(const Guardian* g)`:
      devuelve el puntero ya convertido SOLO si de verdad es Knight;
      en otro caso, nullptr (el llamador chequea).
   3. `bool mismoTipoQueMago(const Guardian* g)`:
      flujo con typeid: devuelve true si typeid(*g) == typeid(Mago).
      (ojo al `*g`: typeid del PUNTERO es el tipo estático, trampa
      clásica; el test lo castiga).

   COMPILACIÓN:
     g++ -std=c++20 -Wall -Wextra -I../../include \
         ejercicio_09_dynamic_cast_y_typeid.cpp -o ej09 && ./ej09
   ============================================================================= */
#include <iostream>
#include <string>
#include <typeinfo>
#include "../../include/test_framework.hpp"

// ---------------------------------------------------------------------------
// Jerarquía DADA (no la toques).
// ---------------------------------------------------------------------------
class Guardian {
public:
    virtual ~Guardian() = default;
    virtual void actuar() const = 0;
};

class Knight : public Guardian {
public:
    void actuar() const override { std::cout << "  escudo al frente\n"; }
    int escudo() const { return 20; }
};

class Mago : public Guardian {
public:
    void actuar() const override { std::cout << "  lee el grimorio\n"; }
    int hechizos() const { return 3; }
};

// ---------------------------------------------------------------------------
// TODOs (todos PLACEHOLDER a la espera del trabajo real).
// ---------------------------------------------------------------------------
std::string identificar(const Guardian* g) {
    (void)g;
    return "placeholder";                       // TODO 1
}

const Knight* comoKnight(const Guardian* g) {
    (void)g;
    return nullptr;                             // TODO 2 (placeholder sutil)
}

bool mismoTipoQueMago(const Guardian* g) {
    (void)g;
    return false;                               // TODO 3
}

// ============= TESTS =============
TEST_CASE(TestIdentificarConDynamicCast) {
    Knight k;
    Mago   m;
    ASSERT_EQ(identificar(&k), std::string("knight"));
    ASSERT_EQ(identificar(&m), std::string("mago"));
    ASSERT_EQ(identificar(nullptr), std::string("desconocido"));
}

TEST_CASE(TestComoKnight) {
    Knight k;
    Mago   m;
    const Knight* kn = comoKnight(&k);
    ASSERT_TRUE(kn != nullptr);
    ASSERT_EQ(kn->escudo(), 20);
    ASSERT_TRUE(comoKnight(&m) == nullptr);     // el mago no es knight
}

TEST_CASE(TestTypeidDinamico) {
    Knight k;
    Mago   m;
    const Guardian* p = &m;
    ASSERT_TRUE(mismoTipoQueMago(p));
    ASSERT_FALSE(mismoTipoQueMago(&k));
    // typeid del puntero vs del objeto: la trampa de la lección 12:
    ASSERT_TRUE(typeid(p) != typeid(Mago));
    ASSERT_TRUE(typeid(*p) == typeid(Mago));
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 09...\n";
    return RUN_ALL_TESTS();
}
