/* =============================================================================
   MÓDULO 15 - SOLUCIÓN 06: Cache y velocidad
   ============================================================================= */
#include <iostream>
#include <string>
#include <vector>
#include "../../include/test_framework.hpp"

std::string ccache_cmake_flags() {
    return "set(CMAKE_CXX_COMPILER_LAUNCHER ccache)\n"
           "set(CMAKE_C_COMPILER_LAUNCHER ccache)";
}

std::string pch_target(const std::string& lib_name,
                        const std::vector<std::string>& public_headers,
                        const std::vector<std::string>& private_headers) {
    std::string result = "target_precompile_headers(" + lib_name + "\n";
    result += "  PRIVATE\n";
    for (const auto& h : private_headers) {
        result += "    \"" + h + "\"\n";
    }
    result += "  INTERFACE\n";
    for (const auto& h : public_headers) {
        result += "    <" + h + ">\n";
    }
    result += ")";
    return result;
}

std::string unity_build_config(int batch_size, const std::string& unique_id) {
    return "set_target_properties(mi_lib PROPERTIES\n"
           "  UNITY_BUILD ON\n"
           "  UNITY_BUILD_BATCH_SIZE " + std::to_string(batch_size) + "\n"
           "  UNITY_BUILD_UNIQUE_ID " + unique_id + "\n"
           ")";
}

std::string mold_linker_flags() {
    return "set(CMAKE_EXE_LINKER_FLAGS \"-fuse-ld=mold\")\n"
           "set(CMAKE_SHARED_LINKER_FLAGS \"-fuse-ld=mold\")\n"
           "set(CMAKE_MODULE_LINKER_FLAGS \"-fuse-ld=mold\")";
}

TEST_CASE(CcacheFlagsContieneLauncher) {
    std::string flags = ccache_cmake_flags();
    ASSERT_TRUE(flags.find("CMAKE_CXX_COMPILER_LAUNCHER") != std::string::npos);
    ASSERT_TRUE(flags.find("ccache") != std::string::npos);
}

TEST_CASE(PchTargetContienePublicPrivate) {
    std::string pch = pch_target("mi_lib", {"fmt/format.h"}, {"internal.h"});
    ASSERT_TRUE(pch.find("target_precompile_headers") != std::string::npos);
    ASSERT_TRUE(pch.find("PRIVATE") != std::string::npos);
    ASSERT_TRUE(pch.find("INTERFACE") != std::string::npos);
    ASSERT_TRUE(pch.find("fmt/format.h") != std::string::npos);
}

TEST_CASE(UnityBuildConfigContieneProps) {
    std::string ub = unity_build_config(16, "mi_lib_");
    ASSERT_TRUE(ub.find("UNITY_BUILD") != std::string::npos);
    ASSERT_TRUE(ub.find("ON") != std::string::npos);
    ASSERT_TRUE(ub.find("UNITY_BUILD_BATCH_SIZE") != std::string::npos);
    ASSERT_TRUE(ub.find("16") != std::string::npos);
    ASSERT_TRUE(ub.find("UNITY_BUILD_UNIQUE_ID") != std::string::npos);
    ASSERT_TRUE(ub.find("mi_lib_") != std::string::npos);
}

TEST_CASE(MoldLinkerFlagsContieneFuseLd) {
    std::string flags = mold_linker_flags();
    ASSERT_TRUE(flags.find("fuse-ld=mold") != std::string::npos);
    ASSERT_TRUE(flags.find("CMAKE_EXE_LINKER_FLAGS") != std::string::npos);
    ASSERT_TRUE(flags.find("CMAKE_SHARED_LINKER_FLAGS") != std::string::npos);
    ASSERT_TRUE(flags.find("CMAKE_MODULE_LINKER_FLAGS") != std::string::npos);
}

int main() {
    return RUN_ALL_TESTS();
}