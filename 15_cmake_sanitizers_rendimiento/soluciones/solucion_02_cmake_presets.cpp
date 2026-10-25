/* =============================================================================
   MÓDULO 15 - SOLUCIÓN 02: CMakePresets.json
   ============================================================================= */
#include <iostream>
#include <string>
#include "../../include/test_framework.hpp"

std::string presets_json = R"({
  "version": 6,
  "cmakeMinimumRequired": { "major": 3, "minor": 20, "patch": 0 },
  "configurePresets": [
    {
      "name": "debug",
      "displayName": "Debug Build",
      "generator": "Ninja",
      "binaryDir": "${sourceDir}/build/debug",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Debug",
        "CMAKE_CXX_FLAGS": "-Wall -Wextra -fsanitize=address,undefined -fno-omit-frame-pointer",
        "BUILD_TESTS": "ON"
      }
    },
    {
      "name": "release",
      "inherits": "debug",
      "displayName": "Release Build",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Release",
        "CMAKE_CXX_FLAGS": "-O3 -DNDEBUG -flto=auto",
        "BUILD_TESTS": "OFF"
      }
    },
    {
      "name": "ci",
      "inherits": "debug",
      "cacheVariables": {
        "BUILD_TESTS": "ON"
      }
    }
  ],
  "buildPresets": [
    { "name": "debug", "configurePreset": "debug" },
    { "name": "release", "configurePreset": "release" },
    { "name": "ci", "configurePreset": "ci", "jobs": 4 }
  ],
  "testPresets": [
    { "name": "debug", "configurePreset": "debug", "output": { "outputOnFailure": true } }
  ]
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