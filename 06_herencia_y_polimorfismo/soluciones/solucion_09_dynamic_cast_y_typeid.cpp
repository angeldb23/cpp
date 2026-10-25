/* =============================================================================
   MÓDULO 06 - SOLUCIÓN 09: Identificar tipo real: dynamic_cast y typeid
   =============================================================================
   Claves de las lecciones 11 y 12:
   - dynamic_cast<Derivada*>(base)：nullptr si falla. Es la forma SEGURA.
   - typeid(*p) mira el tipo DINÁMICO (el vptr); typeid(p) el estático
     del puntero: cuidado con ellos.
   - dynamic_cast void*: dirección del objeto completo.
   ============================================================================= */
#include <iostream>
#include <string>
#include <typeinfo>
#include "../../include/test_framework.hpp"

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

std::string identificar(const Guardian* g) {
    if (dynamic_cast<const Knight*>(g) != nullptr) return "knight";
    if (dynamic_cast<const Mago*>(g) != nullptr) return "mago";
    return "desconocido";
}

const Knight* comoKnight(const Guardian* g) {
    return dynamic_cast<const Knight*>(g);   // nullptr si no coy
}

bool mismoTipoQueMago(const Guardian* g) {
    return g != nullptr && typeid(*g) == typeid(Mago);  // *g: objeto real
}

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
    ASSERT_TRUE(comoKnight(&m) == nullptr);
}

TEST_CASE(TestTypeidDinamico) {
    Knight k;
    Mago   m;
    const Guardian* p = &m;
    ASSERT_TRUE(mismoTipoQueMago(p));
    ASSERT_FALSE(mismoTipoQueMago(&k));
    ASSERT_TRUE(typeid(p) != typeid(Mago));   // p es Guardian const*
    ASSERT_TRUE(typeid(*p) == typeid(Mago));  // *p mira el vptr
}

int main() {
    std::cout << "Solucion Ejercicio 09: dynamic_cast y typeid.\n";
    return RUN_ALL_TESTS();
}
