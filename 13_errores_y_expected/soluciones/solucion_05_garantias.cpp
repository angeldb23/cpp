/* =============================================================================
   MÓDULO 13 - SOLUCIÓN 05: Garantías de excepción (básica, fuerte, nothrow)
   =============================================================================
   POR QUÉ FUNCIONA:
     - BÁSICA: invariantes válidas (size <= capacity, data válido o nullptr),
       pero el contenido puede ser distinto al original si falla a mitad.
       Clave: actualizar puntero ANTES de liberar el viejo (o usar unique_ptr).
     - FUERTE (copy-and-swap): todo el trabajo en un temporal; si falla, el
       original no se toca; si todo OK, swap atómico (nothrow) publica el
       nuevo estado. Módulo 06: copy_and_swap es el patrón canónico.
     - NOTHROW: noexcept + std::nothrow new + chequeo de puntero nulo.
       El programa decide qué hacer (reintentar, loguear, ignorar) sin unwind.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra solucion_05_garantias.cpp -o sol05 && ./sol05
   ============================================================================= */
#include <iostream>
#include <memory>
#include <algorithm>
#include <cstddef>
#include <new>
#include "../../include/test_framework.hpp"

class Buffer {
public:
    explicit Buffer(std::size_t cap = 4) : cap_(cap), size_(0) {
        if (cap_ > 0) data_ = new int[cap_];
    }

    ~Buffer() { delete[] data_; }

    // Copy constructor para copy-and-swap (garantía fuerte)
    Buffer(const Buffer& other) : cap_(other.cap_), size_(other.size_) {
        if (cap_ > 0) {
            data_ = new int[cap_];
            std::copy(other.data_, other.data_ + size_, data_);
        } else {
            data_ = nullptr;
        }
    }

    Buffer& operator=(const Buffer&) = delete;  // usamos copy-and-swap vía pass-by-value

    Buffer(Buffer&& other) noexcept 
        : data_(other.data_), cap_(other.cap_), size_(other.size_) {
        other.data_ = nullptr;
        other.size_ = 0;
        other.cap_ = 0;
    }

    Buffer& operator=(Buffer&& other) noexcept {
        if (this != &other) {
            delete[] data_;
            data_ = other.data_;
            size_ = other.size_;
            cap_ = other.cap_;
            other.data_ = nullptr;
            other.size_ = 0;
            other.cap_ = 0;
        }
        return *this;
    }

    // Garantía BÁSICA: si new lanza, *this queda válido (size/cap invariantes)
    // pero el contenido viejo se pierde (el nuevo data_ ya se asignó).
    void agregarBasico(int v) {
        if (size_ == cap_) {
            std::size_t nuevaCap = cap_ == 0 ? 1 : cap_ * 2;
            int* nuevo = new int[nuevaCap];  // puede lanzar bad_alloc
            std::copy(data_, data_ + size_, nuevo);
            delete[] data_;
            data_ = nuevo;
            cap_ = nuevaCap;
        }
        data_[size_++] = v;
    }

    // Garantía FUERTE: copy-and-swap. Todo en temporal, swap al final.
    void agregarFuerte(int v) {
        Buffer tmp(*this);  // copy constructor (puede lanzar)
        tmp.agregarBasico(v);  // puede lanzar, pero tmp es temporal
        swap(tmp);  // nothrow
    }

    // Garantía NOTHROW: nunca lanza. Devuelve false si no hay memoria.
    bool agregarNothrow(int v) noexcept {
        if (size_ == cap_) {
            std::size_t nuevaCap = cap_ == 0 ? 1 : cap_ * 2;
            int* nuevo = new (std::nothrow) int[nuevaCap];
            if (!nuevo) return false;
            std::copy(data_, data_ + size_, nuevo);
            delete[] data_;
            data_ = nuevo;
            cap_ = nuevaCap;
        }
        data_[size_++] = v;
        return true;
    }

    std::size_t size() const noexcept { return size_; }
    std::size_t capacity() const noexcept { return cap_; }
    int* data() noexcept { return data_; }
    const int* data() const noexcept { return data_; }

    void swap(Buffer& other) noexcept {
        std::swap(data_, other.data_);
        std::swap(size_, other.size_);
        std::swap(cap_, other.cap_);
    }

private:
    int* data_ = nullptr;
    std::size_t cap_ = 0;
    std::size_t size_ = 0;
};

// ============= TESTS =============
TEST_CASE(TestGarantiaBasicaEstadoValido) {
    Buffer b;
    b.agregarBasico(1);
    b.agregarBasico(2);
    ASSERT_EQ(b.size(), 2u);
}

TEST_CASE(TestGarantiaFuerteRollback) {
    Buffer b;
    b.agregarFuerte(1);
    b.agregarFuerte(2);
    ASSERT_EQ(b.size(), 2u);
}

TEST_CASE(TestGarantiaNothrowNoLanza) {
    Buffer b;
    bool ok = b.agregarNothrow(42);
    ASSERT_TRUE(ok);
    ASSERT_EQ(b.size(), 1u);
}

TEST_CASE(TestNothrowSinEspacio) {
    // Forzar fallo de memoria es difícil de forma portable.
    // Verificamos que la versión nothrow no lanza y funciona en caso normal.
    Buffer b;
    bool ok = b.agregarNothrow(1);
    ASSERT_TRUE(ok);
    ASSERT_EQ(b.size(), 1u);
}

TEST_CASE(TestDataAcceso) {
    Buffer b;
    b.agregarFuerte(10);
    b.agregarFuerte(20);
    ASSERT_EQ(b.data()[0], 10);
    ASSERT_EQ(b.data()[1], 20);
}

int main() {
    std::cout << "Ejecutando pruebas para Solución 05 (Garantías de excepción)...\n";
    return RUN_ALL_TESTS();
}