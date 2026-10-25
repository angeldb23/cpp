/* =============================================================================
   MÓDULO 15 - LECCIÓN 06: Sanitizers — MSan, HWAsan, sanitizers combinados,
   fuzzing, hardening, casos reales
   =============================================================================
   TEMA 5 de TEMAS.md (3ra lección): Sanitizers casos avanzados.

   MEMORY SANITIZER (MSan):
   - Detecta: reads of uninitialized memory
   - -fsanitize=memory -fPIE -fno-omit-frame-pointer
   - Requiere: TODAS las librerías instrumentadas (incluyendo libc++)
   - En Linux: difícil (requiere recompilar libc++/libc)
   - En Fuchsii/Chrome: estándar
   - Overhead: ~3x memoria, ~2-3x tiempo
   - msan_options=halt_on_error=1:exitcode=86

   HWADDRESS SANITIZER (HWAsan):
   - ARM64: usa TAGs en punteros (Top Byte Ignore)
   - -fsanitize=hwaddress -fPIE
   - Overhead: ~2x memoria, ~1.5x tiempo (menos que ASan)
   - Detecta: buffer overflow, use-after-free, stack-use-after-return
   - No detecta: memory leaks (sin LSAn)
   - Android NDK: estándar, soporta HWAsan nativo

   SANITIZERS COMBINADOS:
   - ASan + UBSan: -fsanitize=address,undefined (compatibles, común)
   - TSan: SOLO -fsanitize=thread (NO combinar con ASan/UBSan)
   - MSan: SOLO -fsanitize=memory (NO combinar)
   - HWAsan: SOLO -fsanitize=hwaddress (ARM64)
   - Fuzzer: -fsanitize=fuzzer,address,undefined (libFuzzer)

   FUZZING CON SANITIZERS:
   - libFuzzer: -fsanitize=fuzzer,address,undefined
   - LLVM libFuzzer integrado en Clang
   - Corpus: directorio con inputs de prueba
   - Minimización: -minimize_crash=1
   - Diccionario: -dict=dict.txt
   - AFL++ / Honggfuzz: alternativas, más features

   LIBFUZZER EJEMPLO:
     extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
         if (size > 0 && data[0] == 'F') { ... }
         return 0;
     }
     # Compilar: clang++ -fsanitize=fuzzer,address,undefined fuzz.cpp -o fuzz
     # Ejecutar: ./fuzz corpus/

   HARDENING (sin sanitizers, para producción):
   - -fstack-protector-strong (stack canaries)
   - -D_FORTIFY_SOURCE=2 (bounds checking en glibc)
   - -fcf-protection=full (CET/IBT/shstk)
   - -Wl,-z,relro,-z,now (RELRO + BIND_NOW)
   - -fcf-protection=full (Intel CET)
   - -mindirect-branch=thunk -mfunction-return=thunk (Spectre mitigations)

   CASOS REALES DE BUGS ENCONTRADOS:
   1. Heartbleed (OpenSSL): ASan detectó buffer over-read
   2. Stagefright (Android): ASan/TSan en media framework
   3. BlueBorne (Bluetooth): TSan/ASan en stack BT
   4. Spectre/Meltdown: MSan/HWAsan en especulación
   5. Chrome: ClusterFuzz (libFuzzer + ASan) encuentra 1000s bugs/año

   SANITIZERS EN EMBEDDED:
   - ASan: difícil (memoria limitada)
   - HWAsan: ARM64 Cortex-A (Android Things, Zephyr)
   - UBSan: ligero, usable en embedded
   - Compilación: -Os -fsanitize=undefined

   SANITIZERS EN KERNEL:
   - KASAN (Kernel ASan): CONFIG_KASAN=y
   - KTSAN (Kernel TSan): CONFIG_KTSAN=y
   - KMSAN: CONFIG_KMSAN=y
   - KFENCE: CONFIG_KFENCE=y (baja overhead)

   FUTURO:
   - Memory Tagging Extension (MTE) ARMv8.5+ (hardware ASan)
   - CHERI (capability hardware)
   - eBPF sanitizers
   - Sanitizers en WebAssembly (WASI) */

#include <iostream>
#include <vector>
#include <string>

// Demos de conceptos avanzados

// 1. MSan concept
void demo_msan_concept() {
    std::cout << "--- MSan concept ---\n";
    std::cout << R"(
# Compilar con MSan (requiere libc++ instrumentado):
# clang++ -fsanitize=memory -fPIE -fno-omit-frame-pointer -g main.cpp

# main.cpp:
int main() {
    int x;          // uninitialized
    int y = x + 1;  // MSan: uninitialized value read
    return y;
}

# MSan output:
# WARNING: MemorySanitizer: use-of-uninitialized-value
#    #0 0x... in main main.cpp:3
#    #1 0x... in __libc_start_main
# Uninitialized value was created by:
#    #0 0x... in main main.cpp:2
#
# Requiere: recompilar TODO (libc++, libc, etc.)
)" << "\n\n";
}

// 2. HWAsan concept
void demo_hwasan() {
    std::cout << "--- HWAsan (ARM64) ---\n";
    std::cout << R"(
# Compilar con HWAsan (ARM64):
# clang++ -fsanitize=hwaddress -fPIE -fno-omit-frame-pointer -g main.cpp

# HWAsan usa Top Byte Ignore (TBI) en ARM64:
# - Punteros tienen tag en top byte (bits 56-63)
# - Memoria etiquetada con mismo tag
# - Acceso con tag distinto -> trap

# Ventajas vs ASan:
# - Menos overhead memoria (~2x vs 2-3x)
# - Menos overhead CPU (~1.5x vs 2x)
# - Detecta: buffer overflow, use-after-free, stack-use-after-return
# - NO detecta: memory leaks (sin LSAn)

# Android NDK: ndk-build con SANITIZE_HWADDRESS
)" << "\n\n";
}

// 3. Sanitizers combinados
void demo_combinados() {
    std::cout << "--- Sanitizers compatibles ---\n";
    std::cout << R"(
# Compatibles:
# -fsanitize=address,undefined      # ASan + UBSan (OK, común)
# -fsanitize=fuzzer,address,undefined  # libFuzzer + ASan + UBSan (OK)

# INCOMPATIBLES (NO combinar):
# -fsanitize=thread,address         # TSan + ASan = NO
# -fsanitize=thread,undefined       # TSan + UBSan = NO
# -fsanitize=memory,address         # MSan + ASan = NO
# -fsanitize=hwaddress,address      # HWAsan + ASan = NO

# TSan SOLO:
# -fsanitize=thread -fPIC -fno-omit-frame-pointer -O1 -g

# MSan SOLO:
# -fsanitize=memory -fPIE -fno-omit-frame-pointer

# HWAsan SOLO:
# -fsanitize=hwaddress -fPIE
)" << "\n\n";
}

// 4. libFuzzer ejemplo
void demo_libfuzzer() {
    std::cout << "--- libFuzzer ejemplo ---\n";
    std::cout << R"(
// fuzz_target.cpp
#include <cstdint>
#include <cstring>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    if (size >= 3 && data[0] == 'F' && data[1] == 'U' && data[2] == 'Z') {
        // Trigger bug
        if (size > 100) {
            volatile int* p = nullptr;
            *p = 42;  // Crash intencional
        }
    }
    return 0;
}

# Compilar:
# clang++ -fsanitize=fuzzer,address,undefined -g -O1 fuzz.cpp -o fuzz

# Ejecutar:
# mkdir corpus
# echo "FUZZ" > corpus/seed1
# echo "FUZZZZZZ" > corpus/seed2
# ./fuzz corpus/ -max_len=1000 -runs=1000

# Opciones útiles:
# -max_len=1000          # Max input size
# -runs=1000             # Número de runs
# -max_total_time=300    # Tiempo límite (seg)
# -dict=dict.txt         # Diccionario
# -minimize_crash=1      # Minimiza crash
# -focus_function=func   # Focus en función
)" << "\n\n";
}

// 5. Hardening
void demo_hardening() {
    std::cout << "--- Hardening (producción) ---\n";
    std::cout << R"(
# Flags de hardening (producción, sin sanitizers):

# Stack protection
-fstack-protector-strong    # Canary en funciones con arrays/locals

# Fortify source (glibc)
-D_FORTIFY_SOURCE=2         # Bounds checking en memcpy/strcpy/sprintf

# Control Flow Integrity (CET/IBT)
-fcf-protection=full        # IBT + SHSTK (Intel CET)

# RELRO / BIND_NOW
-Wl,-z,relro,-z,now         # RELRO + BIND_NOW

# Spectre/Meltdown mitigations
-mindirect-branch=thunk     # Retpoline / IBRS
-mfunction-return=thunk     # RSB filling

# PIE / ASLR
-fPIE -pie                  # Position Independent Executable

# CMake integration:
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-strong -D_FORTIFY_SOURCE=2")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-z,relro,-z,now")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcf-protection=full")

# MSVC:
# /GS /guard:cf /sdl /DYNAMICBASE /NXCOMPAT /HIGHENTROPYVA
)" << "\n\n";
}

// 6. Sanitizers en kernel/embedded
void demo_embedded_kernel() {
    std::cout << "--- Sanitizers en kernel/embedded ---\n";
    std::cout << R"(
# Kernel (Linux):
# CONFIG_KASAN=y           # Kernel ASan
# CONFIG_KASAN_INLINE=y    # Inline (más rápido)
# CONFIG_KTSAN=y           # Kernel TSan
# CONFIG_KMSAN=y           # Kernel MSan
# CONFIG_KFENCE=y          # Low-overhead fence (sampling)

# Embedded:
# - UBSan: ligero, usable en Cortex-M
#   -fsanitize=undefined -fno-sanitize-recover=undefined
# - HWAsan: ARM64 Cortex-A (Android Things, Zephyr RTOS)
# - KFENCE: low-overhead sampling fence (random sampling)

# WebAssembly (WASI):
# - wasi-sdk con -fsanitize=address
# - wasm-sanitizers (experimental)
)" << "\n\n";
}

// 7. Futuro
void demo_futuro() {
    std::cout << "--- Futuro: MTE, CHERI, eBPF ---\n";
    std::cout << R"(
# Memory Tagging Extension (MTE) - ARMv8.5+
# - Hardware tags en memoria (4 bits por 16 bytes)
# - stg/ldg instructions para tag check
# - ASan en hardware (casi zero overhead)
# Android 13+: MTE habilitado por defecto en apps

# CHERI (Capability Hardware Enhanced RISC Instructions)
# - Capabilities = punteros con bounds + permissions
# - Spatial + temporal memory safety en hardware
# - CHERI C/C++: recompilar con clang +cheri

# eBPF sanitizers:
# - bpf_sanitize: sanitizers en kernel via eBPF
# - Uprobe/uretprobe con ASan

# WebAssembly:
# - wasi-sdk con -fsanitize=address
# - wasm-sanitizers (experimental)
# - Component model + sanitizers
)" << "\n\n";
}

int main() {
    std::cout << "=== LECCIÓN 06: Sanitizers casos avanzados (parte 3) ===\n\n";

    demo_msan_concept();
    demo_hwasan();
    demo_combinados();
    demo_libfuzzer();
    demo_hardening();
    demo_embedded_kernel();
    demo_futuro();

    std::cout << "=== FIN LECCIÓN 06 (Sanitizers parte 3) ===\n";
    return 0;
}