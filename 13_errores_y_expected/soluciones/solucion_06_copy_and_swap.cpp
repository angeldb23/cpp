/* =============================================================================
   MÓDULO 13 - SOLUCIÓN 06: Copy-and-swap idiom
   =============================================================================
   POR QUÉ FUNCIONA:
     - Copy-and-swap: operator=(Matriz other) toma por VALOR (invoca copy ctor).
       Si la copia falla, *this no se toca. Si OK, swap() (nothrow) intercambia
       el estado viejo por el nuevo. Garantía FUERTE automática.
     - Move ctor/assign: roban punteros, ponen origen en estado válido vacío.
       Marcados noexcept para que std::vector use move en realloc.
     - swap() es la operación atómica nothrow que hace el "commit" atómico.
     - Regla de 5/0 (M07/M08): si definís uno, definí los 5 (o usá =default).
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra solucion_06_copy_and_swap.cpp -o sol06 && ./sol06
   ============================================================================= */
#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <utility>
#include <cstddef>
#include "../../include/test_framework.hpp"

class Matriz {
public:
    Matriz(std::size_t filas, std::size_t cols) : filas_(filas), cols_(cols) {
        if (filas_ > 0 && cols_ > 0) {
            data_ = new int*[filas_];
            data_[0] = new int[filas_ * cols_]();  // () = value-init (ceros)
            for (std::size_t i = 1; i < filas_; ++i) {
                data_[i] = data_[0] + i * cols_;
            }
        } else {
            data_ = nullptr;
        }
    }

    // Copy constructor: copia profunda
    Matriz(const Matriz& other) : filas_(other.filas_), cols_(other.cols_) {
        if (filas_ > 0 && cols_ > 0) {
            data_ = new int*[filas_];
            data_[0] = new int[filas_ * cols_];
            std::copy(other.data_[0], other.data_[0] + filas_ * cols_, data_[0]);
            for (std::size_t i = 1; i < filas_; ++i) {
                data_[i] = data_[0] + i * cols_;
            }
        } else {
            data_ = nullptr;
        }
    }

    // Move constructor: robo, noexcept
    Matriz(Matriz&& other) noexcept 
        : data_(other.data_), filas_(other.filas_), cols_(other.cols_) {
        other.data_ = nullptr;
        other.filas_ = 0;
        other.cols_ = 0;
    }

    // Copy assignment: COPY-AND-SWAP (toma por valor -> copy ctor o move ctor)
    // Esto maneja TANTO copy como move assignment sin ambigüedad.
    Matriz& operator=(Matriz other) noexcept {
        swap(other);
        return *this;
    }

    ~Matriz() { liberar(); }

    void swap(Matriz& other) noexcept {
        std::swap(data_, other.data_);
        std::swap(filas_, other.filas_);
        std::swap(cols_, other.cols_);
    }

    int& operator()(std::size_t f, std::size_t c) {
        if (f >= filas_ || c >= cols_) throw std::out_of_range("indice fuera de rango");
        return data_[f][c];
    }

    const int& operator()(std::size_t f, std::size_t c) const {
        if (f >= filas_ || c >= cols_) throw std::out_of_range("indice fuera de rango");
        return data_[f][c];
    }

    constexpr std::size_t filas() const noexcept { return filas_; }
    constexpr std::size_t cols() const noexcept { return cols_; }

private:
    void liberar() {
        if (data_) {
            delete[] data_[0];
            delete[] data_;
            data_ = nullptr;
        }
        filas_ = 0;
        cols_ = 0;
    }

    int** data_ = nullptr;
    std::size_t filas_ = 0;
    std::size_t cols_ = 0;
};

// ============= TESTS =============
TEST_CASE(TestConstructorYAcceso) {
    Matriz m(2, 3);
    ASSERT_EQ(m.filas(), 2u);
    ASSERT_EQ(m.cols(), 3u);
    m(0, 0) = 5;
    m(1, 2) = 10;
    ASSERT_EQ(m(0, 0), 5);
    ASSERT_EQ(m(1, 2), 10);
}

TEST_CASE(TestBoundsCheck) {
    Matriz m(2, 2);
    bool lanzo = false;
    try { m(2, 0) = 1; } catch (const std::out_of_range&) { lanzo = true; }
    ASSERT_TRUE(lanzo);
    lanzo = false;
    try { m(0, 2) = 1; } catch (const std::out_of_range&) { lanzo = true; }
    ASSERT_TRUE(lanzo);
}

TEST_CASE(TestCopyConstructor) {
    Matriz a(2, 2);
    a(0, 0) = 1; a(0, 1) = 2;
    a(1, 0) = 3; a(1, 1) = 4;
    Matriz b(a);
    ASSERT_EQ(b(0, 0), 1);
    ASSERT_EQ(b(1, 1), 4);
    b(0, 0) = 99;
    ASSERT_EQ(a(0, 0), 1);
}

TEST_CASE(TestMoveConstructor) {
    Matriz a(2, 2);
    a(0, 0) = 7;
    Matriz b(std::move(a));
    ASSERT_EQ(b(0, 0), 7);
    ASSERT_EQ(a.filas(), 0u);
}

TEST_CASE(TestCopyAssignmentCopyAndSwap) {
    Matriz a(2, 2);
    a(0, 0) = 1; a(1, 1) = 2;
    Matriz b(3, 3);
    b = a;
    ASSERT_EQ(b.filas(), 2u);
    ASSERT_EQ(b.cols(), 2u);
    ASSERT_EQ(b(0, 0), 1);
    ASSERT_EQ(b(1, 1), 2);
    ASSERT_EQ(a(0, 0), 1);
}

TEST_CASE(TestMoveAssignment) {
    Matriz a(2, 2);
    a(0, 0) = 42;
    Matriz b(1, 1);
    b = std::move(a);
    ASSERT_EQ(b(0, 0), 42);
    ASSERT_EQ(a.filas(), 0u);
}

TEST_CASE(TestSelfAssignment) {
    Matriz a(2, 2);
    a(0, 0) = 5;
    a = a;
    ASSERT_EQ(a(0, 0), 5);
}

int main() {
    std::cout << "Ejecutando pruebas para Solución 06 (Copy-and-swap)...\n";
    return RUN_ALL_TESTS();
}