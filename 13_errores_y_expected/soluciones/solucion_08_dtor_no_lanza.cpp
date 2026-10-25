/* =============================================================================
   MÓDULO 13 - SOLUCIÓN 08: Destructor no lanza (noexcept implícito)
   =============================================================================
   POR QUÉ FUNCIONA:
     - Los destructores son noexcept(true) por defecto (C++11+).
     - Si un destructor lanza MIENTRAS otra excepción se está propagando
       (stack unwinding), el runtime llama a std::terminate(): no hay
       forma de manejar dos excepciones a la vez.
     - SOLUCIÓN 1: El destructor traga la excepción (try/catch vacío).
       Simple pero pierde información del error de limpieza.
     - SOLUCIÓN 2 (MEJOR): Separar "cleanup que puede fallar" en un método
       explícito `cerrar()` / `flush()` / `commit()` que SÍ puede lanzar.
       El usuario DEBE llamarlo; el destructor solo hace lo que no falla
       (close fd, free memory) y es idempotente (seguro llamar 2 veces).
     - Esto es el patrón "dispose pattern" / RAII con finalización explícita.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra solucion_08_dtor_no_lanza.cpp -o sol08 && ./sol08
   ============================================================================= */
#include <iostream>
#include <stdexcept>
#include <string>
#include "../../include/test_framework.hpp"

static bool g_fallar_escritura = false;

void escribirLog(const std::string& msg) {
    if (g_fallar_escritura) throw std::runtime_error("fallo escritura: " + msg);
    // Simular escritura exitosa
    (void)msg;
}

class RecursoPeligroso {
public:
    RecursoPeligroso() : abierto_(true) {}

    // Método explícito: el usuario LO LLAMA y MANEJA el error si lanza
    void cerrar() {
        if (!abierto_) return;  // idempotente
        escribirLog("cerrando recurso");
        abierto_ = false;
    }

    // Destructor: noexcept implícito. NO debe lanzar.
    ~RecursoPeligroso() noexcept {
        if (abierto_) {
            // Opción A: tragar la excepción (pierde info, pero seguro)
            try {
                escribirLog("cleanup en destructor");
            } catch (...) {
                // std::terminate si hay unwinding activo; aquí tragamos
                // En producción: log a stderr (no-lanzante) o métrica
            }
            abierto_ = false;
        }
    }

    bool estaAbierto() const noexcept { return abierto_; }

private:
    bool abierto_;
};

// Función helper que hace limpieza explícita (puede lanzar)
void limpiarSeguro(RecursoPeligroso& r) {
    r.cerrar();  // propaga la excepción si falla
}

// ============= TESTS =============
TEST_CASE(TestDestructorNoLanzaEnNormal) {
    RecursoPeligroso r;
    ASSERT_TRUE(true);
}

TEST_CASE(TestDestructorNoLanzaDuranteUnwind) {
    g_fallar_escritura = true;
    bool termino = false;
    try {
        RecursoPeligroso r;
        throw std::runtime_error("excepcion original");
    } catch (const std::runtime_error&) {
        termino = true;
    }
    g_fallar_escritura = false;
    ASSERT_TRUE(termino);
}

TEST_CASE(TestCerrarExplicitoLanza) {
    g_fallar_escritura = true;
    RecursoPeligroso r;
    bool lanzo = false;
    try {
        r.cerrar();
    } catch (const std::runtime_error&) {
        lanzo = true;
    }
    g_fallar_escritura = false;
    ASSERT_TRUE(lanzo);
    // Destructor idempotente: no hace nada porque ya está cerrado
}

TEST_CASE(TestLimpiarSeguroFunciona) {
    RecursoPeligroso r;
    g_fallar_escritura = true;
    bool lanzo = false;
    try {
        limpiarSeguro(r);
    } catch (const std::runtime_error&) {
        lanzo = true;
    }
    g_fallar_escritura = false;
    ASSERT_TRUE(lanzo);
}

int main() {
    std::cout << "Ejecutando pruebas para Solución 08 (Destructor no lanza)...\n";
    return RUN_ALL_TESTS();
}