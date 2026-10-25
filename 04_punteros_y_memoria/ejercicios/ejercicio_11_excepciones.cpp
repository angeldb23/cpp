/* =============================================================================
   MÓDULO 04 - EJERCICIO 11: Excepciones y limpieza durante el unwind
   =============================================================================
   DIFICULTAD: Intermedio
   PRACTICA: lección 16 (throw, try/catch, std::exception, captura por const&,
             desenrollado de la pila).

   CONSIGNA:
     1. dividirSeguro(int a, int b): si b == 0, lanzá std::invalid_argument
        con el mensaje "division por cero". Si no, devolvé a / b.
     2. validarRango(int valor, int minimo, int maximo): si valor está fuera
        del rango cerrado [minimo, maximo], lanzá std::out_of_range con un
        mensaje descriptivo. Si no, devolvé valor.
     3. parsearEntero(const std::string& texto): usá std::stoi y devolvé el
        entero. Si el texto no es un número, dejá propagar la excepción
        std::invalid_argument de stoi (no la atrapes).
     4. liberarAlLanzar(bool& liberado): creá un objeto local cuyo destructor
        ponga liberado = true, y después lanzá std::runtime_error. La idea es
        comprobar que el destructor corre durante el desenrollado.

   COMPILACIÓN:
     g++ -std=c++20 -Wall -Wextra ejercicio_11_excepciones.cpp -o ej11 && ./ej11
   ============================================================================= */
#include <iostream>
#include <stdexcept>
#include <string>
#include "../../include/test_framework.hpp"

// TODO: implementá las cuatro funciones.
int dividirSeguro(int a, int b) {
    (void)b;
    return a;  // placeholder
}

int validarRango(int valor, int minimo, int maximo) {
    (void)minimo; (void)maximo;
    return valor;  // placeholder
}

int parsearEntero(const std::string& texto) {
    (void)texto;
    return 0;  // placeholder
}

void liberarAlLanzar(bool& liberado) {
    (void)liberado;  // placeholder
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
    std::cout << "Ejecutando pruebas para Ejercicio 11...\n";
    return RUN_ALL_TESTS();
}
