/* =============================================================================
   MÓDULO 13 - EJERCICIO 05: Garantías de excepción (básica, fuerte, nothrow)
   =============================================================================
   DIFICULTAD: Intermedio
   CONSIGNA:
     Implementá una clase `Buffer` que gestiona un array dinámico de ints.
     Debe ofrecer tres operaciones con distintas garantías:
     
     1. `void agregarBasico(int v)` - Garantía BÁSICA:
        - Si falla la reallocación, el objeto queda en estado VÁLIDO
          (invariantes mantenidos), pero el contenido puede haber cambiado.
        - Pista: asigná el nuevo puntero ANTES de borrar el viejo.
     
     2. `void agregarFuerte(int v)` - Garantía FUERTE (copy-and-swap):
        - Si falla, el objeto queda EXACTAMENTE como antes (rollback total).
        - Pista: creá un temporal, hacé todo el trabajo ahí, y swap al final.
     
     3. `void agregarNothrow(int v)` - Garantía NOTHROW:
        - Nunca lanza (noexcept). Si no hay espacio, no agrega y devuelve bool.
        - Usá `std::nothrow` en la asignación y chequeá puntero nulo.
     
     La clase debe tener: size(), capacity(), data() (para tests).
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_05_garantias.cpp -o ej05 && ./ej05
   ============================================================================= */
#include <iostream>
#include <memory>
#include <algorithm>
#include <cstddef>
#include <new>
#include "../../include/test_framework.hpp"

// TODO: class Buffer { ... };

// ============= TESTS =============
TEST_CASE(TestGarantiaBasicaEstadoValido) {
    // Buffer b;
    // b.agregarBasico(1);
    // b.agregarBasico(2);
    // ASSERT_EQ(b.size(), 2u);
    ASSERT_TRUE(false);
}

TEST_CASE(TestGarantiaFuerteRollback) {
    // Buffer b;
    // b.agregarFuerte(1);
    // b.agregarFuerte(2);
    // // Simular fallo forzando capacidad limitada es difícil sin inyección,
    // // pero verificamos que la operación normal funciona:
    // ASSERT_EQ(b.size(), 2u);
    ASSERT_TRUE(false);
}

TEST_CASE(TestGarantiaNothrowNoLanza) {
    // Buffer b;
    // // Esta no debe lanzar nunca
    // bool ok = b.agregarNothrow(42);
    // ASSERT_TRUE(ok);
    // ASSERT_EQ(b.size(), 1u);
    ASSERT_TRUE(false);
}

TEST_CASE(TestNothrowSinEspacio) {
    // Buffer b(0); // capacidad 0
    // bool ok = b.agregarNothrow(1);
    // ASSERT_FALSE(ok); // falla sin lanzar
    // ASSERT_EQ(b.size(), 0u);
    ASSERT_TRUE(false);
}

TEST_CASE(TestDataAcceso) {
    // Buffer b;
    // b.agregarFuerte(10);
    // b.agregarFuerte(20);
    // ASSERT_EQ(b.data()[0], 10);
    // ASSERT_EQ(b.data()[1], 20);
    ASSERT_TRUE(false);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 05 (Garantías de excepción)...\n";
    return RUN_ALL_TESTS();
}