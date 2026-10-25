/* =============================================================================
   MÓDULO 07 - EJERCICIO 01: Tu primer envoltorio RAII completo
   =============================================================================
   DIFICULTAD: Básico
   PRACTICA: lección 01 (recurso atado a la vida del objeto) y lección 02
   (orden de destrucción en reversa).

   CONSIGNA:
   Completá la clase `Buffer` para que sea RAII de verdad: administra un
   `int*` dinámico (reserva con `new int[n]` y libera con `delete[]`),
   incluye un CONTADOR ESTÁTICO de buffers vivos para que los tests
   verifiquen que todo recurso se libera.

      1. Constructor: adquiere `new int[tamanio]` (todo en 0 con `{}`),
         guarda el tamanio y suma 1 al contador `Buffer::vivos_`.
      2. Destructor: libera `delete[]` y decrementa el contador.
         Copias y asignaciones: prohibidas (= delete en ambas).
      3. `set(i, valor)` / `get(i)` con chequeo de límite (fuera = -1).
      4. Orden de destrucción: en `main`, creamos dos buffers y NADA más.
         El test pide que el contador vuelva a 0 después del bloque.

   COMPILACIÓN:
     g++ -std=c++20 -Wall -Wextra -I../../include \
         ejercicio_01_envoltorio_raii.cpp -o ej01 && ./ej01
   =============================================================================
 */
#include <cstddef>
#include <iostream>
#include <string>
#include "../../include/test_framework.hpp"

class Buffer {
private:
    int*    datos_ = nullptr;
    std::size_t tamanio_ = 0;

public:
    inline static int vivos_ = 0;   // contador de buffers con recurso vivo

    explicit Buffer(std::size_t tamanio) {
        // TODO 1: reservar `tamanio` ints inicializados en 0 (new int[n]{}),
        // guardar tamanio y sumar 1 a vivos_.
        (void)tamanio;
    }

    ~Buffer() {
        // TODO 2: delete[] datos_ y restar 1 a vivos_.
    }

    // TODO 3: prohibir copia y asignación (= delete ambas).

    void set(std::size_t i, int valor) {
        // TODO 4: solo si i < tamanio_.
        (void)i; (void)valor;
    }

    int get(std::size_t i) const {
        // TODO 5: retornar datos_[i] si i < tamanio_, sino -1.
        (void)i;
        return 0;                        // PLACEHOLDER
    }

    std::size_t tamanio() const { return tamanio_; }
};

// ============= TESTS =============
TEST_CASE(ContadorVuelveACero) {
    {
        Buffer b(8);
        ASSERT_EQ(Buffer::vivos_, 1);
    }   // al salir del bloque: dtor en reversa (L02)
    ASSERT_EQ(Buffer::vivos_, 0);
}

TEST_CASE(SetGetChequeaLimites) {
    Buffer b(4);
    b.set(0, 42);
    b.set(3, 7);
    b.set(4, 99);          // fuera de rango: se ignora (no UB)
    ASSERT_EQ(b.get(0), 42);
    ASSERT_EQ(b.get(3), 7);
    ASSERT_EQ(b.get(4), -1);
    ASSERT_EQ(b.tamanio(), std::size_t(4));
}

TEST_CASE(TodosFueronLiberados) {
    {
        Buffer b1(2);
        Buffer b2(2);
        Buffer b3(2);
        ASSERT_EQ(Buffer::vivos_, 3);
    }   // se destruyen en reversa: b3, b2, b1
    ASSERT_EQ(Buffer::vivos_, 0);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 01 (RAII)...\n";
    return RUN_ALL_TESTS();
}
