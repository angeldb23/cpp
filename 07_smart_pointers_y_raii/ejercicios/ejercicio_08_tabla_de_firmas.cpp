/* =============================================================================
   MÓDULO 07 - EJERCICIO 08: La tabla de decisiones en firmas reales
   =============================================================================
   DIFICULTAD: Medio
   PRACTICA: lección 10 (dueño/observador/transitorio y "¿quién libera?"")
   y lección 11 (crudos y referencias no dueños).

   CONSIGNA:
   Cuatro firmas diferentes para cuatro relaciones distintas con el MISMO
   recurso. La clase `Pixol` y la fábrica `crearPixol` son DADAS (ya están
   hechas con make_unique): lo tuyo son los tres contratos de función:

      1. `pintar(const Pixol&)`: TRANSITORIA SEGURA: usa el recurso en la
         llamada: garantizada a no ser null (referencia).
      2. `pintarOpcional(const Pixol*)`: TRANSITORIA OPCIONAL: el crudo
         permite "sin dato" (nullptr es estado, no bug); devuelve true si
         pintó, false si no había.
      3. `avisar(std::weak_ptr<Pixol>)`: OBSERVADOR de vida dudosa: solo
         SI lock() da un dueño, informa "vivo: nombre"; si no, "(nadie)".

   Los tests usan la tabla entera y fallan si una firma no hace lo suyo.

   COMPILACIÓN:
     g++ -std=c++20 -Wall -Wextra -I../../include \
         ejercicio_08_tabla_de_firmas.cpp -o ej08 && ./ej08
   =============================================================================
 */
#include <cstddef>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
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

// (DADO): la fábrica: crea y ENTREGA el dueño (nada por acá).
std::unique_ptr<Pixol> crearPixol(const std::string& nombre) {
    return std::make_unique<Pixol>(nombre);
}

// TODO 1: transitoria segura: referencia: no puede ser nullptr.
void pintar(const Pixol& p) {
    (void)p;
}

// TODO 2: transitoria opcional: nullptr = sin dato (true si sí pintó).
bool pintarOpcional(const Pixol* p) {
    (void)p;
    return false;                         // PLACEHOLDER
}

// TODO 3: observador débil: solo si lock() da dueño.
std::string avisar(std::weak_ptr<Pixol> w) {
    (void)w;
    return "(nadie)";                     // PLACEHOLDER
}

// ============= TESTS =============
TEST_CASE(LaFabricaEntregaUnDueno) {
    {
        auto p = crearPixol("protagonista");
        ASSERT_TRUE(p != nullptr);        // dueño por fin
        ASSERT_EQ(Pixol::vivos(), 1);
    }
    ASSERT_EQ(Pixol::vivos(), 0);         // y muere con su dueño
}

TEST_CASE(TransitoriaGarantiza) {
    auto p = crearPixol("hud");
    pintar(*p);       // se le entrega desreferenciado: sin nullptr posible
    ASSERT_EQ(p->sellos(), std::string("pintado"));
}

TEST_CASE(OpcionalManejaNull) {
    auto p = crearPixol("mini_mapa");
    ASSERT_TRUE(pintarOpcional(p.get()));
    ASSERT_TRUE(pintarOpcional(nullptr) == false);   // "sin dato" == no pintar
}

TEST_CASE(ObservadorDiceLaVerdad) {
    std::weak_ptr<Pixol> w;
    ASSERT_EQ(avisar(w), std::string("(nadie)"));    // nunca compartido
    {
        auto compartida = std::make_shared<Pixol>("sobrante");
        w = compartida;
        ASSERT_EQ(avisar(w), std::string("vivo: sobrante"));
    }   // la compartida muere sola (el weak no la sostiene)
    ASSERT_EQ(avisar(w), std::string("(nadie)"));
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 08 (tabla)...\n";
    return RUN_ALL_TESTS();
}
