/* =============================================================================
   MÓDULO 04 - EJERCICIO 09: void*, bytes y alineación
   =============================================================================
   DIFICULTAD: Intermedio/Avanzado
   PRACTICA: lección 13 (void*, reinterpret_cast, uintptr_t, alignof/alignas,
             aritmética de bytes).

   CONSIGNA:
     1. estaAlineado(const void* p, std::size_t alineacion): true si la
        dirección de p es múltiplo de alineacion. Si p es nullptr o alineacion
        es 0, devolvé false. Usá reinterpret_cast<std::uintptr_t>.
     2. bytesHastaAlinear(const void* p, std::size_t alineacion): cuántos bytes
        hay que avanzar desde p para llegar a una dirección alineada. Si ya
        está alineada, 0. Si p es nullptr o alineacion es 0, devolvé 0.
     3. avanzarBytes(void* p, std::size_t n): devolvé p + n bytes. Si p es
        nullptr, devolvé nullptr.
     4. retrocederBytes(const void* p, std::size_t n): devolvé p - n bytes. Si
        p es nullptr, devolvé nullptr.
     5. alinearPuntero(void* p, std::size_t alineacion): devolvé el primer
        puntero alineado desde p (p + bytesHastaAlinear). Si p es nullptr o
        alineacion es 0, devolvé nullptr.

   PISTA: para mover bytes, convertí a unsigned char* (o std::byte*), no sumes
   directamente sobre void* porque no compila.

   COMPILACIÓN:
     g++ -std=c++20 -Wall -Wextra ejercicio_09_alineacion.cpp -o ej09 && ./ej09
   ============================================================================= */
#include <cstddef>
#include <cstdint>
#include <iostream>
#include "../../include/test_framework.hpp"

// TODO: implementá las cinco funciones.
bool estaAlineado(const void* p, std::size_t alineacion) {
    (void)p; (void)alineacion;
    return false;  // placeholder
}

std::size_t bytesHastaAlinear(const void* p, std::size_t alineacion) {
    (void)p; (void)alineacion;
    return 0;  // placeholder
}

void* avanzarBytes(void* p, std::size_t n) {
    (void)p; (void)n;
    return nullptr;  // placeholder
}

const void* retrocederBytes(const void* p, std::size_t n) {
    (void)p; (void)n;
    return nullptr;  // placeholder
}

void* alinearPuntero(void* p, std::size_t alineacion) {
    (void)p; (void)alineacion;
    return nullptr;  // placeholder
}

// ============= TESTS =============
TEST_CASE(TestEstaAlineado) {
    alignas(16) unsigned char buffer[64] = {};
    ASSERT_TRUE(estaAlineado(buffer, 16));
    ASSERT_TRUE(estaAlineado(buffer, 8));
    ASSERT_TRUE(estaAlineado(buffer, 1));
    ASSERT_FALSE(estaAlineado(buffer + 1, 16));
    ASSERT_FALSE(estaAlineado(nullptr, 16));
    ASSERT_FALSE(estaAlineado(buffer, 0));
}

TEST_CASE(TestBytesHastaAlinear) {
    alignas(16) unsigned char buffer[64] = {};
    ASSERT_EQ(bytesHastaAlinear(buffer, 16), 0u);
    ASSERT_EQ(bytesHastaAlinear(buffer + 1, 16), 15u);
    ASSERT_EQ(bytesHastaAlinear(buffer + 8, 16), 8u);
    ASSERT_EQ(bytesHastaAlinear(nullptr, 16), 0u);
    ASSERT_EQ(bytesHastaAlinear(buffer, 0), 0u);
}

TEST_CASE(TestAvanzarYRetroceder) {
    unsigned char buffer[16] = {};
    ASSERT_TRUE(avanzarBytes(buffer, 0) == buffer);
    ASSERT_TRUE(avanzarBytes(buffer, 8) == buffer + 8);
    ASSERT_TRUE(retrocederBytes(buffer + 8, 8) == buffer);
    ASSERT_TRUE(avanzarBytes(nullptr, 4) == nullptr);
    ASSERT_TRUE(retrocederBytes(nullptr, 4) == nullptr);
}

TEST_CASE(TestAlinearPuntero) {
    alignas(16) unsigned char buffer[64] = {};
    ASSERT_TRUE(alinearPuntero(buffer, 16) == buffer);
    ASSERT_TRUE(alinearPuntero(buffer + 1, 16) == buffer + 16);
    ASSERT_TRUE(alinearPuntero(nullptr, 16) == nullptr);
    ASSERT_TRUE(alinearPuntero(buffer, 0) == nullptr);
}

TEST_CASE(TestDireccionComoEntero) {
    int x = 0;
    auto direccion = reinterpret_cast<std::uintptr_t>(&x);
    ASSERT_TRUE(direccion != 0);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 09...\n";
    return RUN_ALL_TESTS();
}
