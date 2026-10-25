/* =============================================================================
   MÓDULO 15 - LECCIÓN 05: Sanitizers — ASan, UBSan, TSan, MSan
   =============================================================================
   TEMA 5 de TEMAS.md (3 lecciones): Sanitizers en profundidad.

   SANITIZERS DISPONIBLES (GCC/Clang):

   1. ADDRESS SANITIZER (ASan) — -fsanitize=address
      Detecta: buffer overflow (stack/heap/global), use-after-free, use-after-scope,
      double-free, memory leaks (con LSAN).
      Overhead: ~2x memoria, ~2x tiempo.
      Flags útiles:
        -fsanitize=address -fno-omit-frame-pointer
        ASAN_OPTIONS=detect_leaks=1:halt_on_error=1:abort_on_error=1
      LSAN (LeakSanitizer): integrado en ASan, detecta leaks.

   2. UNDEFINED BEHAVIOR SANITIZER (UBSan) — -fsanitize=undefined
      Detecta: signed integer overflow, division by zero, null dereference,
      misaligned pointer, enum value out of range, bool load not 0/1,
      shift overflow, vla bound, pointer overflow, etc.
      Overhead: bajo (~1.2x).
      Flags útiles:
        -fsanitize=undefined -fno-sanitize-recover=undefined (abort on first)
      Sub-checks individuales: -fsanitize=signed-integer-overflow, etc.

   3. THREAD SANITIZER (TSan) — -fsanitize=thread
      Detecta: data races (race conditions), deadlocks, thread leaks.
      Overhead: ~5-10x memoria, ~2-20x tiempo.
      Requiere: -fno-omit-frame-pointer -fPIC
      TSan_OPTIONS=halt_on_error=1:report_destroy_locked=1
      OJO: TSan NO detecta races en atomic (eso está bien).
      TSan detecta: races en mutex, condition_variable mal usados, etc.

   4. MEMORY SANITIZER (MSan) — -fsanitize=memory
      Detecta: uninitialized memory reads (use of uninitialized values).
      Overhead: ~3x memoria, ~2-3x tiempo.
      Requiere: instrumentar TODO (incluyendo libc++) -> difícil en Linux.
      Útil para: codebases que pueden recompilar todo desde fuente.

   COMPILACIÓN TÍPICA (Debug):
     -fsanitize=address,undefined -fno-omit-frame-pointer -fno-optimize-sibling-calls

   COMBINACIONES:
   - ASan + UBSan: -fsanitize=address,undefined (común, compatible)
   - TSan SOLO: -fsanitize=thread (NO combinar con ASan/UBSan)
   - MSan SOLO: -fsanitize=memory (requiere rebuild world)

   LECTURA DE REPORTES:
   - ASan: "ERROR: AddressSanitizer: heap-buffer-overflow on address ..."
     -> READ/WRITE size N at 0x... thread T1
     -> #0 0x... in function file.cpp:123
     -> Shadow bytes around buggy address: ...
   - UBSan: "runtime error: signed integer overflow: 2147483647 + 1"
   - TSan: "WARNING: ThreadSanitizer: data race"
     -> Location 1: mutex.lock() file.cpp:10
     -> Location 2: mutex.lock() file.cpp:20

   SUPPRESSIONS (ignorar falsos positivos):
   - ASan: ASAN_OPTIONS=suppressions=file.supp
   - TSan: TSAN_OPTIONS=suppressions=file.supp
   - Formato: race:func1:func2 (TSan), leak:allocator (ASan)

   INTEGRACIÓN EN CMAKE:
     target_compile_options(mi_lib PRIVATE
       $<$<CXX_COMPILER_ID:GNU,Clang>:-fsanitize=address,undefined -fno-omit-frame-pointer>
     )
     # O vía CMAKE_CXX_FLAGS_DEBUG en CMakePresets.json

   ASAN_OPTIONS RECOMENDADOS (env):
     ASAN_OPTIONS=detect_leaks=1:halt_on_error=1:abort_on_error=1:print_stacktrace=1

   TSAN_OPTIONS:
     TSAN_OPTIONS=halt_on_error=1:report_destroy_locked=1:report_signal_unsafe=1

   LSAN (LeakSanitizer, parte de ASan):
     LSAN_OPTIONS=suppressions=leaks.supp:print_suppressions=0

   COMPILACIÓN PARA TSan:
     -fsanitize=thread -fPIC -fno-omit-frame-pointer -O1 -g
     (O1 recomendado para TSan, O0 muy lento) */

#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <mutex>
#include <chrono>

// Demos de bugs que detectan los sanitizers

// 1. ASan: heap buffer overflow
void demo_asan_overflow() {
    std::cout << "--- ASan: heap buffer overflow ---\n";
    // Descomentar para ver ASan en acción:
    // int* arr = new int[10];
    // arr[10] = 42;  // BOOM: heap-buffer-overflow
    // delete[] arr;
    std::cout << "int* arr = new int[10]; arr[10] = 42;  // heap-buffer-overflow\n\n";
}

// 2. ASan: use-after-free
void demo_asan_uaf() {
    std::cout << "--- ASan: use-after-free ---\n";
    // int* p = new int(42);
    // delete p;
    // *p = 10;  // BOOM: heap-use-after-free
    std::cout << "int* p = new int(42); delete p; *p = 10;  // heap-use-after-free\n\n";
}

// 3. ASan: memory leak (LSan)
void demo_asan_leak() {
    std::cout << "--- ASan/LSan: memory leak ---\n";
    // void leak() { new int[100]; }  // leak detectado al exit
    std::cout << "void leak() { new int[100]; }  // LeakSanitizer reporta al exit\n\n";
}

// 4. UBSan: signed integer overflow
void demo_ubsan_overflow() {
    std::cout << "--- UBSan: signed integer overflow ---\n";
    // int x = INT_MAX;
    // x += 1;  // BOOM: signed integer overflow
    std::cout << "int x = INT_MAX; x += 1;  // signed integer overflow\n\n";
}

// 5. UBSan: division by zero
void demo_ubsan_divzero() {
    std::cout << "--- UBSan: division by zero ---\n";
    // int x = 1 / 0;  // BOOM: division by zero
    std::cout << "int x = 1 / 0;  // division by zero\n\n";
}

// 6. UBSan: null pointer dereference
void demo_ubsan_null() {
    std::cout << "--- UBSan: null pointer dereference ---\n";
    // int* p = nullptr; *p = 42;  // BOOM: null pointer dereference
    std::cout << "int* p = nullptr; *p = 42;  // null pointer dereference\n\n";
}

// 7. TSan: data race
std::atomic<int> atomic_counter{0};
int plain_counter = 0;
std::mutex mtx;

void tsan_race_worker() {
    for (int i = 0; i < 10000; ++i) {
        atomic_counter++;      // OK: atomic
        plain_counter++;       // RACE: plain int sin protección
    }
}

void demo_tsan_race() {
    std::cout << "--- TSan: data race ---\n";
    // Compilar con -fsanitize=thread
    // std::thread t1(tsan_race_worker), t2(tsan_race_worker);
    // t1.join(); t2.join();
    // TSan reporta: data race en plain_counter
    std::cout << "atomic++ OK, plain_counter++ -> DATA RACE detectado por TSan\n\n";
}

// 8. TSan: deadlock
std::mutex m1, m2;

void deadlock_a() {
    std::lock_guard<std::mutex> l1(m1);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    std::lock_guard<std::mutex> l2(m2);
}

void deadlock_b() {
    std::lock_guard<std::mutex> l1(m2);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    std::lock_guard<std::mutex> l2(m1);
}

void demo_tsan_deadlock() {
    std::cout << "--- TSan: deadlock detection ---\n";
    // std::thread t1(deadlock_a), t2(deadlock_b);
    // TSan detecta deadlock potencial
    std::cout << "Thread A: lock m1 -> sleep -> lock m2\n";
    std::cout << "Thread B: lock m2 -> sleep -> lock m1\n";
    std::cout << "TSan detecta deadlock potencial (thread A waiting for m2, B for m1)\n\n";
}

// 8. MSan: uninitialized read
void demo_msan_uninit() {
    std::cout << "--- MSan: uninitialized read ---\n";
    // int x; int y = x + 1;  // BOOM: uninitialized value
    std::cout << "int x; int y = x + 1;  // uninitialized value read\n";
    std::cout << "MSan requiere recompilar TODO (incluyendo libc++) -> difícil en Linux\n\n";
}

// 9. ASan_OPTIONS / TSan_OPTIONS recomendados
void demo_options() {
    std::cout << "--- Variables de entorno recomendadas ---\n";
    std::cout << "ASAN_OPTIONS=detect_leaks=1:halt_on_error=1:abort_on_error=1:print_stacktrace=1\n";
    std::cout << "TSAN_OPTIONS=halt_on_error=1:report_destroy_locked=1:report_signal_unsafe=1\n";
    std::cout << "LSAN_OPTIONS=suppressions=leaks.supp:print_suppressions=0\n";
    std::cout << "UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1\n\n";
}

int main() {
    std::cout << "=== LECCIÓN 05: Sanitizers — ASan, UBSan, TSan, MSan ===\n\n";

    demo_asan_overflow();
    demo_asan_uaf();
    demo_asan_leak();
    demo_ubsan_overflow();
    demo_ubsan_divzero();
    demo_ubsan_null();
    demo_tsan_race();
    demo_tsan_deadlock();
    demo_msan_uninit();
    demo_options();

    std::cout << "=== FIN LECCIÓN 05 ===\n";
    return 0;
}