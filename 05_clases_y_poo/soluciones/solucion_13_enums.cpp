/* =============================================================================
   MÓDULO 05 - SOLUCIÓN 13: enum y enum class
   =============================================================================
   POR QUÉ FUNCIONA:
     - EstadoPorta usa unsigned char: 1 byte, Listo para red/disco.
     - El switch sin default cubre a los 3 casos: si en el futuro se
       agrega un estado, -Wswitch avisa que el switch quedó corto.
     - Las conversiones son SIEMPRE static_cast: sin sorpresas de int
       que aparezca como estado.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra solucion_13... -o sol13 && ./sol13
   ============================================================================= */
#include <iostream>
#include <string>
#include "../../include/test_framework.hpp"

enum class EstadoPorta : unsigned char {
    Verde,
    Amarillo,
    Rojo
};

enum class Prioridad {
    Baja = 1,
    Media = 10,
    Alta = 100
};

const char* nombreEstado(EstadoPorta e) {
    switch (e) {
        case EstadoPorta::Verde:   return "verde";
        case EstadoPorta::Amarillo: return "amarillo";
        case EstadoPorta::Rojo:     return "rojo";
    }
    return "desconocido";  // inalcanzable (y por eso no hay default)
}

EstadoPorta siguiente(EstadoPorta e) {
    switch (e) {
        case EstadoPorta::Verde:    return EstadoPorta::Amarillo;
        case EstadoPorta::Amarillo: return EstadoPorta::Rojo;
        case EstadoPorta::Rojo:     return EstadoPorta::Verde;
    }
    return EstadoPorta::Verde;
}

EstadoPorta desdeInt(int n) {
    if (n < 0 || n > 2) {
        return EstadoPorta::Verde;
    }
    return static_cast<EstadoPorta>(n);
}

int valorPrioridad(Prioridad p) {
    return static_cast<int>(p);
}

// ============= TESTS =============
TEST_CASE(TestTipoSubyacente) {
    ASSERT_EQ(sizeof(EstadoPorta), 1);
}

TEST_CASE(TestNombres) {
    ASSERT_EQ(std::string(nombreEstado(EstadoPorta::Verde)), "verde");
    ASSERT_EQ(std::string(nombreEstado(EstadoPorta::Amarillo)), "amarillo");
    ASSERT_EQ(std::string(nombreEstado(EstadoPorta::Rojo)), "rojo");
}

TEST_CASE(TestRueda) {
    ASSERT_TRUE(siguiente(EstadoPorta::Verde) == EstadoPorta::Amarillo);
    ASSERT_TRUE(siguiente(EstadoPorta::Amarillo) == EstadoPorta::Rojo);
    ASSERT_TRUE(siguiente(EstadoPorta::Rojo) == EstadoPorta::Verde);
}

TEST_CASE(TestDesdeInt) {
    ASSERT_TRUE(desdeInt(0) == EstadoPorta::Verde);
    ASSERT_TRUE(desdeInt(1) == EstadoPorta::Amarillo);
    ASSERT_TRUE(desdeInt(2) == EstadoPorta::Rojo);
    ASSERT_TRUE(desdeInt(-1) == EstadoPorta::Verde);
    ASSERT_TRUE(desdeInt(99) == EstadoPorta::Verde);
}

TEST_CASE(TestPrioridadesNumeradas) {
    ASSERT_EQ(valorPrioridad(Prioridad::Baja), 1);
    ASSERT_EQ(valorPrioridad(Prioridad::Media), 10);
    ASSERT_EQ(valorPrioridad(Prioridad::Alta), 100);
}

int main() {
    std::cout << "Ejecutando pruebas para Solución 13...\n";
    return RUN_ALL_TESTS();
}
