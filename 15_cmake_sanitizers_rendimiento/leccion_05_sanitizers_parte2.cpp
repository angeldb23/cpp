/* =============================================================================
   MÓDULO 15 - LECCIÓN 05: Sanitizers avanzados — ASan/OOM, TSan/race patterns,
   UBSan/checks, suppressions, integración CI
   =============================================================================
   TEMA 5 de TEMAS.md (2da lección): Sanitizers avanzados.

   ASAN AVANZADO:
   - detect_leaks=1 (LSan integrado)
   - detect_stack_use_after_return=1 (use-after-return)
   - detect_invalid_pointer_pairs=1/2/3 (free/delete mismatch)
   - strict_string_checks=1 (memcpy/strcpy overlap)
   - allocator_may_return_null=1 (simula OOM)
   - quarantine_size_mb=256 (tamaño quarantine para use-after-free)
   - malloc_context_size=30 (stack depth en reportes)
   - symbolize=1 (requires llvm-symbolizer)

   TSan PATTERNS:
   - Data races: acceso concurrente sin sincronización
   - Lock inversion: deadlock potencial
   - Thread leaks: hilos no joined/detached
   - Signal handlers unsafe
   - TSan no detecta races en atomics (correcto)
   - TSan OJO: falsos positivos en lock-free algorithms

   UBSAN CHECKS:
   - signed-integer-overflow
   - unsigned-integer-overflow
   - division-by-zero
   - null-pointer-dereference
   - misaligned-pointer
   - enum-value-out-of-range
   - shift-overflow
   - vla-bound
   - pointer-overflow
   - bool-load (bool != 0/1)
   - enum-conversion
   - floating-point-exception

   SUPPRESSIONS:
   - ASan: suppressions=file.supp
     race:func1:func2 (TSan)
     leak:allocator (ASan)
   - TSan: suppressions=file.supp
   - Formato: tipo:función1:función2

   INTEGRACIÓN CI:
   - ASan/UBSan en Debug build (siempre)
   - TSan en CI separado (lento, 5-10x)
   - MSan: requiere rebuild world (difícil en CI)
   - ASAN_OPTIONS=halt_on_error=1:abort_on_error=1
   - TSAN_OPTIONS=halt_on_error=1:report_destroy_locked=1
   - UBSAN_OPTIONS=halt_on_error=1

   FALSOS POSITIVOS COMUNES:
   - ASan: false positives en custom allocators
   - TSan: atomics lock-free, signal handlers
   - UBSan: signed overflow en código legacy (intencional)
   - Suppressions para falsos positivos conocidos

   SANITIZERS EN PRODUCCIÓN:
   - ASan/LSan: solo staging/pre-prod (overhead 2x)
   - TSan: solo staging (overhead 5-10x)
   - MSan: casi nunca en prod (requiere rebuild world)
   - Hardening: -fstack-protector-strong -D_FORTIFY_SOURCE=2

   SANITIZERS + FUZZING:
   - libFuzzer + ASan/UBSan: -fsanitize=fuzzer,address,undefined
   - Honggfuzz + ASan
   - AFL++ + ASan
   - Corpus minimization: -minimize_crash=1 */

#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>
#include <vector>
#include <chrono>

// Demos de patterns que detectan sanitizers

// 1. ASan: stack-use-after-return
int* retorna_puntero_local() {
    int x = 42;
    return &x;  // ASan con detect_stack_use_after_return=1 detecta esto
}

void demo_asan_stack_uaf() {
    std::cout << "--- ASan: stack-use-after-return ---\n";
    std::cout << "int* p = retorna_puntero_local(); *p = 10;  // stack-use-after-return\n\n";
}

// 2. ASan: global buffer overflow
int global_arr[10];
void demo_asan_global() {
    std::cout << "--- ASan: global buffer overflow ---\n";
    std::cout << "global_arr[10] = 42;  // global-buffer-overflow\n\n";
}

// 3. UBSan: enum out of range
enum class Color { Rojo = 0, Verde = 1, Azul = 2 };
void demo_ubsan_enum() {
    std::cout << "--- UBSan: enum out of range ---\n";
    // static_cast<Color>(42);  // BOOM: enum value out of range
    std::cout << "static_cast<Color>(42);  // enum value out of range\n\n";
}

// 4. UBSan: bool load
void demo_ubsan_bool() {
    std::cout << "--- UBSan: bool load ---\n";
    // unsigned char x = 2; bool b = x;  // BOOM: load of value 2 not 0/1
    std::cout << "unsigned char x = 2; bool b = x;  // load of value 2\n\n";
}

// 5. TSan: lock inversion (deadlock potential)
std::mutex m_a, m_b;

void lock_a_then_b() {
    std::lock_guard<std::mutex> l1(m_a);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    std::lock_guard<std::mutex> l2(m_b);
}

void lock_b_then_a() {
    std::lock_guard<std::mutex> l1(m_b);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    std::lock_guard<std::mutex> l2(m_a);
}

void demo_tsan_deadlock() {
    std::cout << "--- TSan: lock inversion / deadlock ---\n";
    std::cout << "Thread A: lock m_a -> sleep -> lock m_b\n";
    std::cout << "Thread B: lock m_b -> sleep -> lock m_a\n";
    std::cout << "TSan detecta: Lock inversion (potential deadlock)\n\n";
}

// 4. TSan: signal handler unsafe
void demo_tsan_signal() {
    std::cout << "--- TSan: signal handler unsafe ---\n";
    std::cout << "TSan reporta: Signal handler llamado con mutex.lock()\n";
    std::cout << "Solución: async-signal-safe functions only en handlers\n\n";
}

// 5. UBSan: shift overflow
void demo_ubsan_shift() {
    std::cout << "--- UBSan: shift overflow ---\n";
    std::cout << "int x = 1 << 31;  // 1<<31 en int 32-bit = UB\n";
    std::cout << "uint32_t x = 1u << 32;  // shift >= width = UB\n\n";
}

// 6. Suppressions ejemplos
void demo_suppressions() {
    std::cout << "--- Suppressions examples ---\n";
    std::cout << R"(
# ASan suppressions (leaks.supp)
leak:mi_allocador_personalizado

# TSan suppressions (races.supp)
race:mi_lock_free_queue::push:mi_lock_free_queue::pop

# Formato general:
# tipo:función1:función2 (TSan race)
# tipo:función (ASan/UBSan leak/error)
# Uso:
# ASAN_OPTIONS=suppressions=leaks.supp
# TSAN_OPTIONS=suppressions=races.supp
)" << "\n\n";
}

// 6. CI Integration patterns
void demo_ci() {
    std::cout << "--- CI Integration patterns ---\n";
    std::cout << R"(
# GitHub Actions
jobs:
  asan:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - run: cmake --preset=debug
      - run: cmake --build --preset=debug
      - run: ASAN_OPTIONS=halt_on_error=1 ctest --preset=debug

  tsan:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - run: cmake --preset=tsan
      - run: cmake --build --preset=tsan
      - run: TSAN_OPTIONS=halt_on_error=1 ctest --preset=tsan

# CMakePresets.json para TSan
# "tsan": {
#   "inherits": "debug",
#   "cacheVariables": {
#     "CMAKE_CXX_FLAGS": "-fsanitize=thread -fPIC -O1 -g",
#     "BUILD_TESTS": "ON"
#   }
# }
)" << "\n\n";
}

int main() {
    std::cout << "=== LECCIÓN 05: Sanitizers avanzados (parte 2) ===\n\n";

    demo_asan_stack_uaf();
    demo_asan_global();
    demo_ubsan_enum();
    demo_ubsan_bool();
    demo_tsan_deadlock();
    demo_tsan_signal();
    demo_ubsan_shift();
    demo_suppressions();
    demo_ci();

    std::cout << "=== FIN LECCIÓN 05 (parte 2) ===\n";
    return 0;
}