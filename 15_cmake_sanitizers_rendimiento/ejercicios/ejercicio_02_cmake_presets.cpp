/* =============================================================================
   MÓDULO 15 - EJERCICIO 02: CMakePresets.json
   =============================================================================
   DIFICULTAD: Básico
   CONSIGNA:
     Escribir un CMakePresets.json con:
     - preset "debug": Ninja, build/debug, Debug, ASan+UBSan, tests ON
     - preset "release": hereda de debug, Release, -O3 -flto=auto, tests OFF
     - preset "ci": hereda de debug, jobs=4
     - buildPresets para cada uno
     - testPreset para debug

   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra -I../../include ejercicio_02_cmake_presets.cpp -o ej02 && ./ej02
   ============================================================================= */
#include <iostream>
#include <string>
#include "../../include/test_framework.hpp"

// TODO: Escribir el contenido del CMakePresets.json como string
std::string presets_json = R"(
{
  "version": 6,
  "cmakeMinimumRequired": { "major": 3, "minor": 20, "patch": 0 },
  "configurePresets": [
    {
      // TODO 1: preset "debug" con Ninja, build/debug, Debug, ASan+UBSan, tests ON
    },
    {
      // TODO 2: preset "release" que hereda de debug, Release, -O3 -flto=auto, tests OFF
    },
    {
      // TODO 3: preset "ci" que hereda de debug, jobs=4
    }
  ],
  // TODO 4: buildPresets para debug, release, ci
  // TODO 5: testPreset para debug
}
)";

TEST_CASE(PresetsContieneDebug) {
    ASSERT_TRUE(presets_json.find("\"name\": \"debug\"") != std::string::npos);
    ASSERT_TRUE(presets_json.find("\"generator\": \"Ninja\"") != std::string::npos);
    ASSERT_TRUE(presets_json.find("build/debug") != std::string::npos);
    ASSERT_TRUE(presets_json.find("CMAKE_BUILD_TYPE") != std::string::npos);
    ASSERT_TRUE(presets_json.find("Debug") != std::string::npos);
    ASSERT_TRUE(presets_json.find("fsanitize=address,undefined") != std::string::npos);
    ASSERT_TRUE(presets_json.find("BUILD_TESTS") != std::string::npos);
}

TEST_CASE(PresetsContieneRelease) {
    ASSERT_TRUE(presets_json.find("\"name\": \"release\"") != std::string::npos);
    ASSERT_TRUE(presets_json.find("\"inherits\": \"debug\"") != std::string::npos);
    ASSERT_TRUE(presets_json.find("Release") != std::string::npos);
    ASSERT_TRUE(presets_json.find("O3") != std::string::npos);
    ASSERT_TRUE(presets_json.find("flto=auto") != std::string::npos);
    ASSERT_TRUE(presets_json.find("\"BUILD_TESTS\": \"OFF\"") != std::string::npos);
}

TEST_CASE(PresetsContieneCI) {
    ASSERT_TRUE(presets_json.find("\"name\": \"ci\"") != std::string::npos);
    ASSERT_TRUE(presets_json.find("\"jobs\": 4") != std::string::npos);
}

TEST_CASE(PresetsContieneBuildPresets) {
    ASSERT_TRUE(presets_json.find("\"buildPresets\"") != std::string::npos);
    ASSERT_TRUE(presets_json.find("\"name\": \"debug\"") != std::string::npos);
    ASSERT_TRUE(presets_json.find("\"name\": \"release\"") != std::string::npos);
    ASSERT_TRUE(presets_json.find("\"name\": \"ci\"") != std::string::npos);
}

TEST_CASE(PresetsContieneTestPresets) {
    ASSERT_TRUE(presets_json.find("\"testPresets\"") != std::string::npos);
    ASSERT_TRUE(presets_json.find("outputOnFailure") != std::string::npos);
}

int main() {
    return RUN_ALL_TESTS();
}