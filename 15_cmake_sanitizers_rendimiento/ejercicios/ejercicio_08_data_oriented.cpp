/* =============================================================================
   MÓDULO 15 - EJERCICIO 08: Data-Oriented Design — SoA, SIMD, memory layout
   =============================================================================
   DIFICULTAD: Avanzado
   CONSIGNA (tema 9 de TEMAS.md: complejidad, localidad de caché, copias evitables):
     1. aos_to_soa_particle(): string con transformación de AoS a SoA
        para struct Particle { float x,y,z, vx,vy,vz, mass; }
     2. simd_add_avx2(arr1, arr2, arr3, n): string con loop SIMD AVX2
        que suma 8 floats por iteración (_mm256_add_ps)
     3. memory_layout_optimal(): string con struct optimizado
        (orden grande->pequeño, bitfields para flags)
     4. pmr_arena_config(size_mb): string con std::pmr::monotonic_buffer_resource
        de tamaño size_mb MB
     5. prefetch_next(ptr, stride): string con __builtin_prefetch
        para siguiente elemento a stride bytes

   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra -I../../include ejercicio_08_data_oriented.cpp -o ej08 && ./ej08
   ============================================================================= */
#include <iostream>
#include <string>
#include "../../include/test_framework.hpp"

// TODO 1: AoS a SoA para partículas
std::string aos_to_soa_particle() {
    // TU CÓDIGO: struct ParticleSoA con vectores separados x,y,z,vx,vy,vz,mass
    return "";
}

// TODO 2: SIMD AVX2 loop para suma
std::string simd_add_avx2([[maybe_unused]] const std::string& arr1,
                           [[maybe_unused]] const std::string& arr2,
                           [[maybe_unused]] const std::string& arr3,
                           [[maybe_unused]] const std::string& n) {
    // TU CÓDIGO: loop con _mm256_loadu_ps, _mm256_add_ps, _mm256_storeu_ps
    return "";
}

// TODO 3: Memory layout óptimo
std::string memory_layout_optimal() {
    // TU CÓDIGO: struct con double, int, char, char + bitfields
    return "";
}

// TODO 4: PMR arena configuración
std::string pmr_arena_config([[maybe_unused]] int size_mb) {
    // TU CÓDIGO: monotonic_buffer_resource con size_mb * 1024 * 1024
    return "";
}

// TODO 5: Prefetch siguiente elemento
std::string prefetch_next([[maybe_unused]] const std::string& ptr,
                           [[maybe_unused]] int stride) {
    // TU CÓDIGO: __builtin_prefetch(ptr + stride, 0, 3)
    return "";
}

// ============= TESTS =============
TEST_CASE(AoSSoAParticleContieneCampos) {
    std::string soa = aos_to_soa_particle();
    ASSERT_TRUE(soa.find("ParticleSoA") != std::string::npos);
    ASSERT_TRUE(soa.find("x") != std::string::npos);
    ASSERT_TRUE(soa.find("y") != std::string::npos);
    ASSERT_TRUE(soa.find("z") != std::string::npos);
    ASSERT_TRUE(soa.find("vx") != std::string::npos);
    ASSERT_TRUE(soa.find("vy") != std::string::npos);
    ASSERT_TRUE(soa.find("vz") != std::string::npos);
    ASSERT_TRUE(soa.find("mass") != std::string::npos);
    ASSERT_TRUE(soa.find("vector") != std::string::npos);
}

TEST_CASE(SIMDAVX2ContieneIntrinsics) {
    std::string simd = simd_add_avx2("a", "b", "c", "n");
    ASSERT_TRUE(simd.find("_mm256_loadu_ps") != std::string::npos);
    ASSERT_TRUE(simd.find("_mm256_add_ps") != std::string::npos);
    ASSERT_TRUE(simd.find("_mm256_storeu_ps") != std::string::npos);
    ASSERT_TRUE(simd.find("immintrin.h") != std::string::npos || simd.find("avx") != std::string::npos);
}

TEST_CASE(MemoryLayoutOptimalOrdenCorrecto) {
    std::string layout = memory_layout_optimal();
    ASSERT_TRUE(layout.find("double") != std::string::npos);
    ASSERT_TRUE(layout.find("int") != std::string::npos);
    ASSERT_TRUE(layout.find("bitfield") != std::string::npos || layout.find(":") != std::string::npos);
    // Verificar orden: grande primero
    size_t pos_double = layout.find("double");
    size_t pos_int = layout.find("int");
    if (pos_double != std::string::npos && pos_int != std::string::npos) {
        ASSERT_TRUE(pos_double < pos_int);
    }
}

TEST_CASE(PMRConfigContieneArena) {
    std::string pmr = pmr_arena_config(64);
    ASSERT_TRUE(pmr.find("monotonic_buffer_resource") != std::string::npos);
    ASSERT_TRUE(pmr.find("64") != std::string::npos);
    ASSERT_TRUE(pmr.find("1024") != std::string::npos);
    ASSERT_TRUE(pmr.find("pmr") != std::string::npos);
}

TEST_CASE(PrefetchNextContieneBuiltin) {
    std::string pf = prefetch_next("entities[i+1].hot", 64);
    ASSERT_TRUE(pf.find("__builtin_prefetch") != std::string::npos);
    ASSERT_TRUE(pf.find("entities") != std::string::npos);
    ASSERT_TRUE(pf.find("64") != std::string::npos);
}

int main() {
    return RUN_ALL_TESTS();
}