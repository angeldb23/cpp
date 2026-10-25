/* =============================================================================
   MÓDULO 07 - SOLUCIÓN 08: La tabla de decisiones en firmas reales
   =============================================================================
   Práctica de las lecciones 10 y 11: una firma por RELACIÓN: fábrica que
   entrega dueño, transitoria por referencia, transitoria opcional por
   crudo, y observador débil con lock().
   =============================================================================
 */
#include <cstddef>
#include <iostream>
#include <memory>
#include <string>
#include "../../include/test_framework.hpp"

class Pixol {
private:
    std::string nombre_;
    mutable std::string sellos_;   // registro de usos (módulo 05: mutable)
    inline static int vivos_ = 0;
    inline static int destruidos_ = 0;

public:
    explicit Pixol(std::string nombre) : nombre_(std::move(nombre)) {
        ++vivos_;
    }
    ~Pixol() { ++destruidos_; --vivos_; }

    const std::string& nombre() const { return nombre_; }
    void pintarCon(const std::string& sello) const { sellos_ += sello; }
    const std::string& sellos() const { return sellos_; }

    static int vivos() { return vivos_; }
    static int destruidos() { return destruidos_; }
};

// FÁBRICA: crea y entrega el DUEÑO (transferencia de propiedad natural).
std::unique_ptr<Pixol> crearPixol(const std::string& nombre) {
    return std::make_unique<Pixol>(nombre);
}

// TRANSITORIA SEGURA: la referencia garantiza que hay objeto.
void pintar(const Pixol& p) {
    p.pintarCon("pintado");
}

// TRANSITORIA OPCIONAL: nullptr = "sin dato", no error.
bool pintarOpcional(const Pixol* p) {
    if (p == nullptr) return false;
    p->pintarCon("pintado");        // mutable: const sea lo que fuere
    return true;
}

// OBSERVADOR DÉBIL: decide y sostiene con lock() (el ritual de la L09).
std::string avisar(std::weak_ptr<Pixol> w) {
    if (auto vivo = w.lock()) {
        return "vivo: " + vivo->nombre();
    }
    return "(nadie)";
}

// ============= TESTS =============
TEST_CASE(LaFabricaEntregaUnDueno) {
    {
        auto p = crearPixol("protagonista");
        ASSERT_TRUE(p != nullptr);
        ASSERT_EQ(Pixol::vivos(), 1);
    }
    ASSERT_EQ(Pixol::vivos(), 0);
}

TEST_CASE(TransitoriaGarantiza) {
    auto p = crearPixol("hud");
    pintar(*p);
    ASSERT_EQ(p->sellos(), std::string("pintado"));
}

TEST_CASE(OpcionalManejaNull) {
    auto p = crearPixol("mini_mapa");
    ASSERT_TRUE(pintarOpcional(p.get()));
    ASSERT_TRUE(!pintarOpcional(nullptr));
}

TEST_CASE(ObservadorDiceLaVerdad) {
    std::weak_ptr<Pixol> w;
    ASSERT_EQ(avisar(w), std::string("(nadie)"));
    {
        auto compartida = std::make_shared<Pixol>("sobrante");
        w = compartida;
        ASSERT_EQ(avisar(w), std::string("vivo: sobrante"));
    }
    ASSERT_EQ(avisar(w), std::string("(nadie)"));
}

int main() {
    std::cout << "Ejecutando SOLUCIÓN 08 (tabla)...\n";
    return RUN_ALL_TESTS();
}
