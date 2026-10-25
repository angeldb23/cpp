/* =============================================================================
   SOLUCION 09 (L13-L14): decisiones_features
   ============================================================================ */

#include "../../include/test_framework.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <algorithm>

namespace {

struct Version { int major, minor, patch; };

Version parseGLVersion(const std::string& str) {
    // "3.3.0 Mesa 23.1.0" -> 3, 3, 0
    std::istringstream iss(str);
    std::string versionStr;
    iss >> versionStr;
    int major = 0, minor = 0, patch = 0;
    char dot;
    std::istringstream vss(versionStr);
    vss >> major >> dot >> minor >> dot >> patch;
    return {major, minor, patch};
}

std::vector<std::string> checkExtensions(const std::vector<std::string>& all, const std::vector<std::string>& req) {
    std::vector<std::string> missing;
    for (auto const& r : req) {
        bool found = false;
        for (auto const& a : all) {
            if (a == r) { found = true; break; }
        }
        if (!found) missing.push_back(r);
    }
    return missing;
}

std::unordered_map<std::string, bool> featuresFromVersion(int major, int minor) {
    std::unordered_map<std::string, bool> f;
    // GL 3.0+
    f["GLSL 330 core"] = (major >= 3);
    f["UBO std140"] = (major >= 3);
    f["VAO core"] = (major >= 3);
    f["FBO core"] = (major >= 3);
    f["sampler2DShadow"] = (major >= 3);
    f["glTexImage3D"] = (major >= 3);
    f["glDrawElementsBaseVertex"] = (major >= 3);
    // GL 4.3+
    f["compute shader"] = (major > 4 || (major == 4 && minor >= 3));
    f["SSBO"] = (major > 4 || (major == 4 && minor >= 3));
    f["texture arrays"] = (major > 4 || (major == 4 && minor >= 3));
    f["tessellation"] = (major > 4 || (major == 4 && minor >= 3));
    return f;
}

} // namespace

TEST_CASE(test_parse_version_basico) {
    Version v = parseGLVersion("3.3.0 Mesa 23.1.0");
    ASSERT_EQ(v.major, 3);
    ASSERT_EQ(v.minor, 3);
    ASSERT_EQ(v.patch, 0);
}

TEST_CASE(test_parse_version_nvidia) {
    Version v = parseGLVersion("4.6.0 NVIDIA 535.0");
    ASSERT_EQ(v.major, 4);
    ASSERT_EQ(v.minor, 6);
}

TEST_CASE(test_extensions_faltantes) {
    std::vector<std::string> all = {"GL_ARB_texture_float", "GL_EXT_framebuffer_object"};
    std::vector<std::string> req = {"GL_ARB_texture_float", "GL_ARB_compute_shader"};
    auto missing = checkExtensions(all, req);
    ASSERT_EQ(missing.size(), 1);
    ASSERT_EQ(missing[0], "GL_ARB_compute_shader");
}

TEST_CASE(test_features_gl33) {
    auto f = featuresFromVersion(3, 3);
    ASSERT_TRUE(f["GLSL 330 core"]);
    ASSERT_TRUE(f["UBO std140"]);
    ASSERT_TRUE(f["VAO core"]);
    ASSERT_TRUE(f["FBO core"]);
    ASSERT_TRUE(f["sampler2DShadow"]);
    ASSERT_TRUE(f["glTexImage3D"]);
    ASSERT_TRUE(f["glDrawElementsBaseVertex"]);
    ASSERT_FALSE(f["compute shader"]);
    ASSERT_FALSE(f["SSBO"]);
    ASSERT_FALSE(f["tessellation"]);
}

TEST_CASE(test_features_gl43) {
    auto f = featuresFromVersion(4, 3);
    ASSERT_TRUE(f["compute shader"]);
    ASSERT_TRUE(f["SSBO"]);
    ASSERT_TRUE(f["texture arrays"]);
}

int main() {
    return RUN_ALL_TESTS();
}