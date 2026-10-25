/* =============================================================================
   MÓDULO 15 - SOLUCIÓN 03: Sanitizers
   ============================================================================= */
#include <iostream>
#include <string>
#include "../../include/test_framework.hpp"

std::string sanitizer_para_bug(const std::string& bug) {
    if (bug == "buffer overflow") return "ASan";
    if (bug == "use after free") return "ASan";
    if (bug == "memory leak") return "ASan";
    if (bug == "signed integer overflow") return "UBSan";
    if (bug == "division by zero") return "UBSan";
    if (bug == "null pointer dereference") return "UBSan";
    if (bug == "data race") return "TSan";
    if (bug == "deadlock") return "TSan";
    if (bug == "uninitialized read") return "MSan";
    return "Ninguno";
}

TEST_CASE(SanitizerBufferOverflow) {
    ASSERT_EQ(sanitizer_para_bug("buffer overflow"), "ASan");
}

TEST_CASE(SanitizerUseAfterFree) {
    ASSERT_EQ(sanitizer_para_bug("use after free"), "ASan");
}

TEST_CASE(SanitizerMemoryLeak) {
    ASSERT_EQ(sanitizer_para_bug("memory leak"), "ASan");
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