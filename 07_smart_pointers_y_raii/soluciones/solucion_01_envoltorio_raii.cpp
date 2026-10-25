/* =============================================================================
   MÓDULO 07 - SOLUCIÓN 01: Tu primer envoltorio RAII completo
   =============================================================================
   Práctica de las lecciones 01 y 02: el recurso vive DENTRO del objeto y
   su liberación es la destrucción. Nada de delete suelto en el main.
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

    explicit Buffer(std::size_t tamanio)
        : datos_(new int[tamanio]{}), tamanio_(tamanio) {   // {} = todo en 0
        ++vivos_;
    }

    ~Buffer() {
        delete[] datos_;        // el ÚNICO delete del módulo: RAII
        --vivos_;
    }

    // Sin copia: dos buffers con el mismo puntero serían doble delete.
    Buffer(const Buffer&)            = delete;
    Buffer& operator=(const Buffer&) = delete;

    void set(std::size_t i, int valor) {
        if (i < tamanio_) datos_[i] = valor;
    }

    int get(std::size_t i) const {
        return (i < tamanio_) ? datos_[i] : -1;
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
    std::cout << "Ejecutando SOLUCIÓN 01 (RAII)...\n";
    return RUN_ALL_TESTS();
}
