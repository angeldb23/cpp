/* =============================================================================
   MÓDULO 04 - SOLUCIÓN 11: Excepciones y limpieza durante el unwind
   =============================================================================
   POR QUÉ FUNCIONA:
     - dividirSeguro valida antes de dividir y lanza invalid_argument.
     - validarRango usa el rango cerrado [minimo, maximo].
     - parsearEntero delega en std::stoi y deja propagar su excepción.
     - liberarAlLanzar crea un objeto local con destructor; al lanzar, el
       runtime desenrolla la pila y ejecuta ese destructor, que marca el flag.
     - Las pruebas atrapan por const& para no copiar ni hacer slicing.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra solucion_11_excepciones.cpp -o sol11 && ./sol11
   ============================================================================= */
#include <iostream>
#include <stdexcept>
#include <string>
#include "../../include/test_framework.hpp"

int dividirSeguro(int a, int b) {
    if (b == 0) throw std::invalid_argument("division por cero");
    return a / b;
}

int validarRango(int valor, int minimo, int maximo) {
    if (valor < minimo || valor > maximo) {
        throw std::out_of_range("valor fuera del rango permitido");
    }
    return valor;
}

int parsearEntero(const std::string& texto) {
    return std::stoi(texto);
}

void liberarAlLanzar(bool& liberado) {
    struct Guardia {
        bool* objetivo;
        ~Guardia() { *objetivo = true; }
    };
    [[maybe_unused]] Guardia g{&liberado};
    throw std::runtime_error("falla simulada");
}

// ============= TESTS =============
TEST_CASE(TestDividirSeguro) {
    ASSERT_EQ(dividirSeguro(10, 2), 5);
    ASSERT_EQ(dividirSeguro(-9, 3), -3);

    bool lanzo = false;
    try {
        dividirSeguro(1, 0);
    } catch (const std::invalid_argument& e) {
        lanzo = true;
        ASSERT_TRUE(std::string(e.what()).size() > 0);
    }
    ASSERT_TRUE(lanzo);
}

TEST_CASE(TestValidarRango) {
    ASSERT_EQ(validarRango(5, 0, 10), 5);
    ASSERT_EQ(validarRango(0, 0, 10), 0);
    ASSERT_EQ(validarRango(10, 0, 10), 10);

    bool lanzo = false;
    try {
        validarRango(11, 0, 10);
    } catch (const std::out_of_range&) {
        lanzo = true;
    }
    ASSERT_TRUE(lanzo);
}

TEST_CASE(TestParsearEntero) {
    ASSERT_EQ(parsearEntero("123"), 123);
    ASSERT_EQ(parsearEntero("-45"), -45);

    bool lanzo = false;
    try {
        parsearEntero("abc");
    } catch (const std::invalid_argument&) {
        lanzo = true;
    }
    ASSERT_TRUE(lanzo);
}

TEST_CASE(TestLimpiezaDuranteUnwind) {
    bool liberado = false;
    bool lanzo = false;
    try {
        liberarAlLanzar(liberado);
    } catch (const std::runtime_error&) {
        lanzo = true;
    }
    ASSERT_TRUE(lanzo);
    ASSERT_TRUE(liberado);
}

TEST_CASE(TestStdExceptionComoBase) {
    bool lanzo = false;
    try {
        dividirSeguro(3, 0);
    } catch (const std::exception& e) {
        lanzo = std::string(e.what()).size() > 0;
    }
    ASSERT_TRUE(lanzo);
}

int main() {
    std::cout << "Ejecutando pruebas para Solución 11...\n";
    return RUN_ALL_TESTS();
}
