/* =============================================================================
   EJERCICIO 09 (L13-L14): decisiones_features
   Parse "3.3.0 Mesa..." y tabla de features
   ============================================================================ */

#include "../../include/test_framework.hpp"
#include <string>
#include <vector>
#include <unordered_map>

namespace {

/* --- EJERCICIO ---
   1) parseGLVersion(str) -> (major, minor, patch)
   2) checkExtensions(list, required) -> missing
   3) featuresFromVersion(major, minor) -> map<feature, bool>
*/

struct Version { int major, minor, patch; };

Version parseGLVersion(const std::string& str) { (void)str; return {0,0,0}; }
std::vector<std::string> checkExtensions(const std::vector<std::string>& all, const std::vector<std::string>& req) { (void)all; (void)req; return {}; }
std::unordered_map<std::string, bool> featuresFromVersion(int major, int minor) { (void)major; (void)minor; return {}; }

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