/* =============================================================================
   MÓDULO 15 - EJERCICIO 03: Sanitizers — ASan, UBSan, TSan
   =============================================================================
   DIFICULTAD: Intermedio
   CONSIGNA:
     Identificar qué sanitizer detecta cada bug. Completar la función
     que devuelve el nombre del sanitizer apropiado.

   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra -I../../include ejercicio_03_sanitizers.cpp -o ej03 && ./ej03
   ============================================================================= */
#include <iostream>
#include <string>
#include "../../include/test_framework.hpp"

std::string sanitizer_para_bug(const std::string& bug) {
    // TODO: retornar "ASan", "UBSan", "TSan", "MSan" o "Ninguno"
    if (bug == "buffer overflow") return "";           // TODO
    if (bug == "use after free") return "";            // TODO
    if (bug == "memory leak") return "";               // TODO
    if (bug == "signed integer overflow") return "";   // TODO
    if (bug == "division by zero") return "";          // TODO
    if (bug == "null pointer dereference") return "";  // TODO
    if (bug == "data race") return "";                 // TODO
    if (bug == "deadlock") return "";                  // TODO
    if (bug == "uninitialized read") return "";        // TODO
    return "Ninguno";
}

TEST_CASE(SanitizerBufferOverflow) {
    ASSERT_EQ(sanitizer_para_bug("buffer overflow"), "ASan");
}

TEST_CASE(SanitizerUseAfterFree) {
    ASSERT_EQ(sanitizer_para_bug("use after free"), "ASan");
}

TEST_CASE(SanitizerMemoryLeak) {
    ASSERT_EQ(sanitizer_para_bug("memory leak"), "ASan");  // LSAn
}

TEST_CASE(SanitizerSignedOverflow) {
    ASSERT_EQ(sanitizer_para_bug("signed integer overflow"), "UBSan");
}

TEST_CASE(SanitizerDivZero) {
    ASSERT_EQ(sanitizer_para_bug("division by zero"), "UBSan");
}

TEST_CASE(SanitizerNullDeref) {
    ASSERT_EQ(sanitizer_para_bug("null pointer dereference"), "UBSan");
}

TEST_CASE(SanitizerDataRace) {
    ASSERT_EQ(sanitizer_para_bug("data race"), "TSan");
}

TEST_CASE(SanitizerDeadlock) {
    ASSERT_EQ(sanitizer_para_bug("deadlock"), "TSan");
}

TEST_CASE(SanitizerUninitRead) {
    ASSERT_EQ(sanitizer_para_bug("uninitialized read"), "MSan");
}

int main() {
    return RUN_ALL_TESTS();
}