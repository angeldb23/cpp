/* =============================================================================
   MÓDULO 13 - EJERCICIO 08: Destructor no lanza (noexcept implícito)
   =============================================================================
   DIFICULTAD: Intermedio
   CONSIGNA:
     En C++ los destructores son noexcept POR DEFECTO (implícitamente).
     Si un destructor lanza durante stack unwinding (otra excepción activa),
     std::terminate() es llamado.
     
     Implementá:
     1. Una clase `RecursoPeligroso` cuyo destructor INTENTA hacer algo que
        puede lanzar (ej. escribir a disco, cerrar conexión de red).
     2. Una función `limpiarSeguro(RecursoPeligroso&)` que hace la limpieza
        REAL (puede lanzar) ANTES de que el destructor corra.
     3. El destructor de `RecursoPeligroso` debe ser noexcept y NO lanzar:
        - Llamar a limpiarSeguro() dentro de un try/catch que traga la excepción
        - O mejor: mover la lógica que lanza a un método explícito `cerrar()`
          que el usuario DEBE llamar, y el destructor solo hace cleanup básico
          que no falla (ej. close fd, free mem).
     
     Demostrá el problema: creá un scope donde se lanza una excepción,
     y dentro hay un `RecursoPeligroso` cuyo destructor también lanzaría.
     Sin la protección, el programa termina. Con la protección, la excepción
     original se propaga.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_08_dtor_no_lanza.cpp -o ej08 && ./ej08
   ============================================================================= */
#include <iostream>
#include <stdexcept>
#include <string>
#include "../../include/test_framework.hpp"

// Variable global para simular fallo en "escritura a disco"
static bool g_fallar_escritura = false;

// TODO: void escribirLog(const std::string& msg); // lanza si g_fallar_escritura

// TODO: class RecursoPeligroso { ... };

// TODO: void limpiarSeguro(RecursoPeligroso& r); // puede lanzar

// ============= TESTS =============
TEST_CASE(TestDestructorNoLanzaEnNormal) {
    // RecursoPeligroso r;
    // // Destructor no debe lanzar en condiciones normales
    // ASSERT_TRUE(true); // si llegamos aquí, OK
    ASSERT_TRUE(false);
}

TEST_CASE(TestDestructorNoLanzaDuranteUnwind) {
    // g_fallar_escritura = true;
    // bool termino = false;
    // try {
    //     RecursoPeligroso r;
    //     throw std::runtime_error("excepcion original");
    // } catch (const std::runtime_error&) {
    //     termino = true; // la excepción original se propagó, no terminate
    // }
    // g_fallar_escritura = false;
    // ASSERT_TRUE(termino);
    ASSERT_TRUE(false);
}

TEST_CASE(TestCerrarExplicitoLanza) {
    // g_fallar_escritura = true;
    // RecursoPeligroso r;
    // bool lanzo = false;
    // try {
    //     r.cerrar(); // método explícito que SÍ puede lanzar
    // } catch (const std::runtime_error&) {
    //     lanzo = true;
    // }
    // g_fallar_escritura = false;
    // ASSERT_TRUE(lanzo);
    // // Después de cerrar, destructor no debe hacer nada (idempotente)
    ASSERT_TRUE(false);
}

TEST_CASE(TestLimpiarSeguroFunciona) {
    // RecursoPeligroso r;
    // g_fallar_escritura = true;
    // bool lanzo = false;
    // try {
    //     limpiarSeguro(r);
    // } catch (const std::runtime_error&) {
    //     lanzo = true;
    // }
    // g_fallar_escritura = false;
    // ASSERT_TRUE(lanzo);
    ASSERT_TRUE(false);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 08 (Destructor no lanza)...\n";
    return RUN_ALL_TESTS();
}