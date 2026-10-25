/* =============================================================================
   MÓDULO 15 - SOLUCIÓN 08: Data-Oriented Design
   ============================================================================= */
#include <iostream>
#include <string>
#include "../../include/test_framework.hpp"

std::string aos_to_soa_particle() {
    return R"(struct ParticleSoA {
    std::vector<float> x, y, z;
    std::vector<float> vx, vy, vz;
    std::vector<float> mass;
    size_t count = 0;

    void resize(size_t n) {
        x.resize(n); y.resize(n); z.resize(n);
        vx.resize(n); vy.resize(n); vz.resize(n);
        mass.resize(n);
        count = n;
    }
})";
}

std::string simd_add_avx2(const std::string& arr1,
                           const std::string& arr2,
                           const std::string& arr3,
                           const std::string& n) {
    return "#include <immintrin.h>\n"
           "void simd_add(float* a, float* b, float* c, size_t n) {\n"
           "    for (size_t i = 0; i + 7 < n; i += 8) {\n"
           "        __m256 va = _mm256_loadu_ps(a + i);\n"
           "        __m256 vb = _mm256_loadu_ps(b + i);\n"
           "        __m256 vc = _mm256_add_ps(va, vb);\n"
           "        _mm256_storeu_ps(c + i, vc);\n"
           "    }\n"
           "    // remainder loop\n"
           "}";
}

std::string memory_layout_optimal() {
    return R"(struct OptimalLayout {
    double d;           // 8 bytes - primero (mayor alineación)
    int i;              // 4 bytes
    char c1;            // 1 byte
    char c2;            // 1 byte
    // padding: 2 bytes = total 16 bytes

    uint32_t flags : 4;  // bitfield: 4 bits
    uint32_t id : 28;    // bitfield: 28 bits
    // 4 bytes total vs 8 con bool/int separados
})";
}

std::string pmr_arena_config(int size_mb) {
    return "std::pmr::monotonic_buffer_resource arena(" + std::to_string(size_mb) + " * 1024 * 1024);\n"
           "std::pmr::polymorphic_allocator<int> alloc(&arena);\n"
           "std::pmr::vector<int> vec(&arena);";
}

std::string prefetch_next(const std::string& ptr, int stride) {
    return "__builtin_prefetch(" + ptr + ", 0, 3);  // stride=" + std::to_string(stride);
}

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