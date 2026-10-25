/* =============================================================================
   MÓDULO 15 - SOLUCIÓN 05: FetchContent y dependencias
   ============================================================================= */
#include <iostream>
#include <string>
#include "../../include/test_framework.hpp"

std::string fetchcontent_basic(const std::string& dep_name,
                                const std::string& git_repo,
                                const std::string& git_tag) {
    return R"(FetchContent_Declare(
  )" + dep_name + R"(
  GIT_REPOSITORY )" + git_repo + R"(
  GIT_TAG        )" + git_tag + R"(
)
FetchContent_MakeAvailable(}" + dep_name + R"() )";
}

std::string fetchcontent_shallow(const std::string& dep_name,
                                  const std::string& git_repo,
                                  const std::string& git_tag) {
    return R"(FetchContent_Declare(
  )" + dep_name + R"(
  GIT_REPOSITORY )" + git_repo + R"(
  GIT_TAG        )" + git_tag + R"(
  GIT_SHALLOW ON
  GIT_PROGRESS ON
)
FetchContent_MakeAvailable(}" + dep_name + R"() )";
}

std::string fetchcontent_url(const std::string& dep_name,
                              const std::string& url,
                              const std::string& sha256) {
    return R"(FetchContent_Declare(
  )" + dep_name + R"(
  URL )" + url + R"(
  URL_HASH )" + sha256 + R"(
)
FetchContent_MakeAvailable(}" + dep_name + R"() )";
}

std::string find_package_moderno(const std::string& pkg,
                                  const std::string& version) {
    return "find_package(" + pkg + " " + version + " REQUIRED)\n"
           "target_link_libraries(mi_target PRIVATE " + pkg + "::" + pkg + ")";
}

TEST_CASE(FetchContentBasicoContieneClaves) {
    std::string fc = fetchcontent_basic("Catch2",
        "https://github.com/catchorg/Catch2.git", "v3.5.3");
    ASSERT_TRUE(fc.find("FetchContent_Declare") != std::string::npos);
    ASSERT_TRUE(fc.find("FetchContent_MakeAvailable") != std::string::npos);
    ASSERT_TRUE(fc.find("Catch2") != std::string::npos);
    ASSERT_TRUE(fc.find("GIT_REPOSITORY") != std::string::npos);
    ASSERT_TRUE(fc.find("GIT_TAG") != std::string::npos);
}

TEST_CASE(FetchContentShallowContieneOpciones) {
    std::string fc = fetchcontent_shallow("fmt",
        "https://github.com/fmtlib/fmt.git", "10.2.1");
    ASSERT_TRUE(fc.find("FetchContent_Declare") != std::string::npos);
    ASSERT_TRUE(fc.find("GIT_SHALLOW") != std::string::npos);
    ASSERT_TRUE(fc.find("GIT_PROGRESS") != std::string::npos);
}

TEST_CASE(FetchContentURLContieneHash) {
    std::string fc = fetchcontent_url("lyra",
        "https://github.com/bfgroup/lyra/archive/refs/tags/v1.6.0.zip",
        "SHA256=abc123...");
    ASSERT_TRUE(fc.find("URL") != std::string::npos);
    ASSERT_TRUE(fc.find("URL_HASH") != std::string::npos);
    ASSERT_TRUE(fc.find("SHA256") != std::string::npos);
}

TEST_CASE(FindPackageModernoUsaTargets) {
    std::string fp = find_package_moderno("fmt", "10.2.1");
    ASSERT_TRUE(fp.find("find_package") != std::string::npos);
    ASSERT_TRUE(fp.find("fmt") != std::string::npos);
    ASSERT_TRUE(fp.find("REQUIRED") != std::string::npos);
    ASSERT_TRUE(fp.find("fmt::fmt") != std::string::npos);
    ASSERT_TRUE(fp.find("target_link_libraries") != std::string::npos);
}

int main() {
    return RUN_ALL_TESTS();
}