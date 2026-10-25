/* =============================================================================
   MÓDULO 07 - SOLUCIÓN 02: Stack unwinding con RAII: dos funciones limpias
   =============================================================================
   Práctica de la lección 03: cuando una excepción cruza la función, los
   destructores de los locales corren IGUAL: la limpieza no depende del
   catch ni de un return perfecto.
   =============================================================================
 */
#include <iostream>
#include <stdexcept>
#include <string>
#include "../../include/test_framework.hpp"

class Recurso {
private:
    std::string nombre_;
    inline static int vivos_ = 0;

public:
    explicit Recurso(std::string n) : nombre_(std::move(n)) { ++vivos_; }
    ~Recurso() { --vivos_; }

    static int vivos() { return vivos_; }
    const std::string& nombre() const { return nombre_; }
};

void cargarConRecurso() {
    Recurso tiles("tiles");                            // RAII: nace
    throw std::runtime_error("falló");   // ¡escapa!: el dtor corre igual
}

void instalar() {
    Recurso schemas("schemas");
    throw std::runtime_error("instalación rota");
}

// ============= TESTS =============
TEST_CASE(ExceptionLimpiaTodo) {
    ASSERT_EQ(Recurso::vivos(), 0);
    try {
        cargarConRecurso();
    } catch (const std::exception& e) {
        ASSERT_EQ(std::string(e.what()), std::string("falló"));
    }
    ASSERT_EQ(Recurso::vivos(), 0);      // el unwinding ya pasó por acá
}

TEST_CASE(OtraFuncionOtroCamino) {
    ASSERT_EQ(Recurso::vivos(), 0);
    try {
        instalar();
    } catch (const std::exception&) {
        // atrapada y afuera, sin líneas extra de limpieza
    }
    ASSERT_EQ(Recurso::vivos(), 0);
}

int main() {
    std::cout << "Ejecutando SOLUCIÓN 02 (unwinding)...\n";
    return RUN_ALL_TESTS();
}
