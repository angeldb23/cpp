/* =============================================================================
   MÓDULO 04 - SOLUCIÓN 09: void*, bytes y alineación
   =============================================================================
   POR QUÉ FUNCIONA:
     - reinterpret_cast<std::uintptr_t> da un entero del tamaño de un puntero,
       que sí admite módulo y comparaciones.
     - (alineacion - direccion % alineacion) % alineacion da 0 si ya está
       alineada, y el faltante exacto si no.
     - Para avanzar bytes se convierte a unsigned char*, porque void* no admite
       aritmética.
     - nullptr se trata antes de cualquier operación.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra solucion_09_alineacion.cpp -o sol09 && ./sol09
   ============================================================================= */
#include <cstddef>
#include <cstdint>
#include <iostream>
#include "../../include/test_framework.hpp"

bool estaAlineado(const void* p, std::size_t alineacion) {
    if (p == nullptr || alineacion == 0) return false;
    auto direccion = reinterpret_cast<std::uintptr_t>(p);
    return direccion % alineacion == 0;
}

std::size_t bytesHastaAlinear(const void* p, std::size_t alineacion) {
    if (p == nullptr || alineacion == 0) return 0;
    auto direccion = reinterpret_cast<std::uintptr_t>(p);
    std::size_t resto = static_cast<std::size_t>(direccion % alineacion);
    return (resto == 0) ? 0 : alineacion - resto;
}

void* avanzarBytes(void* p, std::size_t n) {
    if (p == nullptr) return nullptr;
    auto* bytes = static_cast<unsigned char*>(p);
    return bytes + n;
}

const void* retrocederBytes(const void* p, std::size_t n) {
    if (p == nullptr) return nullptr;
    auto* bytes = static_cast<const unsigned char*>(p);
    return bytes - n;
}

void* alinearPuntero(void* p, std::size_t alineacion) {
    if (p == nullptr || alineacion == 0) return nullptr;
    return avanzarBytes(p, bytesHastaAlinear(p, alineacion));
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
    std::cout << "Ejecutando pruebas para Solución 09...\n";
    return RUN_ALL_TESTS();
}
