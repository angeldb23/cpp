/* =============================================================================
   MÓDULO 07 - EJERCICIO 02: Stack unwinding: el error que limpia solo
   =============================================================================
   DIFICULTAD: Básico-medio
   PRACTICA: lección 03 (stack unwinding: los destructores corren aunque la
   función "retorne por excepción") y lección 01 (RAII básico).

   CONSIGNA:
   La clase `Recurso` ya trae su contador estático de instancias vivas
   (el constructor suma, el destructor resta): RAII de verdad.

      1. Completá `cargarConRecurso()`: adquirí un `Recurso` local con
         nombre "tiles" y DESPUÉS lanzá std::runtime_error("falló").
      2. Completá `instalar()` de la misma manera: recurso "schemas" y
         error "instalación rota".
      3. Los tests atrapan ambas excepciones: RAII garantiza que, al
         llegar al catch, todos los destructores del camino ya corrieron:
         `Recurso::vivos()` tiene que quedar en 0.

   IMPORTANTE: no "arregles nada" en el catch: el punto es que la
   limpieza NO dependa del catch. PERO ojo: con un `new` crudo en vez
   de un objeto RAII, este mismo código fuga la memoria.

   COMPILACIÓN:
     g++ -std=c++20 -Wall -Wextra -I../../include \
         ejercicio_02_stack_unwinding.cpp -o ej02 && ./ej02
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

// ============================================================================
// TODO 1 y 2: rellenar las dos funciones.
// ============================================================================
void cargarConRecurso() {
    // TODO: adquirí "tiles" con RAII y después lanzá la excepción.
    throw std::runtime_error("PLACEHOLDER sin_cleanup");
}

void instalar() {
    // TODO: adquirí "schemas" y después lanzá la excepción.
    throw std::runtime_error("PLACEHOLDER sin_cleanup");
}

// ============= TESTS =============
TEST_CASE(ExceptionLimpiaTodo) {
    ASSERT_EQ(Recurso::vivos(), 0);
    try {
        cargarConRecurso();                 // lanza a mitad del camino
    } catch (const std::exception& e) {
        ASSERT_EQ(std::string(e.what()), std::string("falló"));
    }
    // El RAII del recurso YA limpió SIN depender de este catch:
    ASSERT_EQ(Recurso::vivos(), 0);
}

TEST_CASE(OtraFuncionOtroCamino) {
    ASSERT_EQ(Recurso::vivos(), 0);
    try {
        instalar();
    } catch (const std::exception&) {
        // atrapada y afuera, sin líneas extra de limpieza
    }
    ASSERT_EQ(Recurso::vivos(), 0);    // sin dtor RAII: quedaría 1 fugado
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 02 (unwinding)...\n";
    return RUN_ALL_TESTS();
}
