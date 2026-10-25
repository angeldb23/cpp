/* =============================================================================
   MÓDULO 24 - LECCIÓN 07: Matemática de gizmos (translate/rotate/scale)
   =============================================================================
   TEMA 4 de TEMAS.md (primera lección): "Herramientas de edición: gizmo
   (translate/rotate/scale), property grid, curve editor y color picker".

   QUÉ HACE UN GIZMO
   -----------------
   Un gizmo traduce el movimiento del mouse a una edición 3D. Para el eje X:

   1. Picking: ¿el mouse está sobre la flecha X? Se testea en pantalla, no en
      mundo. Proyectá los extremos del eje a pantalla y medí distancia al
      segmento (en píxeles). El eje más cercano dentro de un umbral gana.
   2. Arrastre: con el eje elegido, construí un rayo desde la cámara a través
      del mouse y calculá el punto de la recta del eje más cercano a ese rayo.
      Ese parámetro `s` (a lo largo del eje) es lo que cambia al mover.
   3. Delta: `s_actual - s_inicial`, opcionalmente snap, y se aplica al objeto
      en espacio local o mundo.

   RAYO vs PLANO
   -------------
   Para translate libre (plano perpendicular a la cámara) o para el gizmo de
   rotación, se intersecta el rayo con un plano:
       denom = dot(dir, n)
       t = dot(planePoint - origin, n) / denom
   Si |denom| ~ 0 el rayo es paralelo: no hay intersección (no dividas por
   cero). Si t < 0 el plano está detrás de la cámara.

   RAYO vs EJE (el cálculo importante)
   -----------------------------------
   Recta A(s) = origenEje + s * eje (eje unitario)
   Rayo  R(t) = origenRayo + t * dir (dir unitario)
   Minimizar |A(s) - R(t)|². Con w0 = origenEje - origenRayo:
       a = eje·eje = 1, b = eje·dir, c = dir·dir = 1
       d = eje·w0, e = dir·w0
       denom = a*c - b*b
       s = (b*e - c*d) / denom
   Si `denom` ~ 0, eje y rayo son paralelos: el punto no está bien definido.

   SNAP
   ----
   `round(valor / paso) * paso`. Cuidado con paso <= 0 (devolvé el valor) y
   con los negativos: `std::round` redondea medio hacia arriba, así que -0.5
   con paso 1 da -0? En realidad -0.5/1 = -0.5, round(-0.5) = -1 (half away
   from zero en C++). Verificalo con tests, no lo asumas.

   ESPACIO LOCAL vs MUNDO
   ----------------------
   El eje puede estar rotado con el objeto. Si la base local son las columnas
   (X, Y, Z) de su matriz de rotación, el delta local (lx, ly, lz) se lleva a
   mundo con:
       deltaMundo = X*lx + Y*ly + Z*lz
   Para rotación, el delta es un ángulo alrededor del eje elegido; para escala,
   es un factor (no una suma): `escala *= 1 + delta`.

   ROTACIÓN
   --------
   El ángulo se saca del ángulo polar del mouse alrededor del centro del gizmo
   en pantalla: `atan2(mouse.y - centro.y, mouse.x - centro.x)`. Hay que
   "unwrapear" el salto de 360 -> 0 para que girar varias vueltas acumule.

   Esta lección imprime los números de un arrastre sintético.
   ============================================================================= */
#include "include/gui_util.hpp"
#include "include/imgui_app.hpp"

#include <cmath>
#include <cstdio>
#include <optional>

namespace {

constexpr float kEps = 1e-6f;

struct Rayo {
    gui::Vec3 origen;
    gui::Vec3 dir;  // normalizado
};

std::optional<gui::Vec3> rayoPlano(const Rayo& r, gui::Vec3 puntoPlano, gui::Vec3 normal) {
    const float denom = gui::dot(r.dir, normal);
    if (std::fabs(denom) < kEps) return std::nullopt;
    const float t = gui::dot(puntoPlano - r.origen, normal) / denom;
    if (t < 0.0f) return std::nullopt;
    return r.origen + r.dir * t;
}

std::optional<float> parametroEnEje(const Rayo& r, gui::Vec3 origenEje, gui::Vec3 eje) {
    const gui::Vec3 w0 = origenEje - r.origen;
    const float a = gui::dot(eje, eje);
    const float b = gui::dot(eje, r.dir);
    const float c = gui::dot(r.dir, r.dir);
    const float d = gui::dot(eje, w0);
    const float e = gui::dot(r.dir, w0);
    const float denom = a * c - b * b;
    if (std::fabs(denom) < kEps) return std::nullopt;
    return (b * e - c * d) / denom;
}

float snap(float valor, float paso) {
    if (paso <= 0.0f) return valor;
    return std::round(valor / paso) * paso;
}

gui::Vec3 aplicarBase(const gui::Vec3& local, gui::Vec3 x, gui::Vec3 y, gui::Vec3 z) {
    return x * local.x + y * local.y + z * local.z;
}

// Distancia en píxeles de un punto a un segmento 2D: el test de picking.
float distanciaASegmento(gui::Vec2 p, gui::Vec2 a, gui::Vec2 b) {
    const gui::Vec2 ab = b - a;
    const float len2 = gui::dot(ab, ab);
    if (len2 < kEps) return std::sqrt(gui::dot(p - a, p - a));
    float t = gui::dot(p - a, ab) / len2;
    t = gui::clampf(t, 0.0f, 1.0f);
    const gui::Vec2 q = a + ab * t;
    const gui::Vec2 d = p - q;
    return std::sqrt(gui::dot(d, d));
}

void demoHeadless() {
    // Rayo desde (0.2, 1.5, 5) hacia -Z; el eje X vive en el origen.
    Rayo r{{0.2f, 1.5f, 5.0f}, {0.0f, 0.0f, -1.0f}};
    auto p = rayoPlano(r, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f});
    if (p) std::printf("rayo-plano: (%.2f, %.2f, %.2f)\n", p->x, p->y, p->z);

    auto s0 = parametroEnEje(r, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f});
    Rayo r2{{0.8f, 1.5f, 5.0f}, {0.0f, 0.0f, -1.0f}};
    auto s1 = parametroEnEje(r2, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f});
    if (s0 && s1) {
        std::printf("arrastre eje X: s0=%.2f s1=%.2f delta=%.2f\n", *s0, *s1, *s1 - *s0);
        std::printf("con snap 0.25: delta=%.2f\n", snap(*s1 - *s0, 0.25f));
    }

    // Base local rotada 45 grados alrededor de Z.
    const float c = std::cos(gui::kPi / 4.0f);
    const float s = std::sin(gui::kPi / 4.0f);
    const gui::Vec3 bx{c, s, 0.0f}, by{-s, c, 0.0f}, bz{0.0f, 0.0f, 1.0f};
    const gui::Vec3 mundo = aplicarBase({1.0f, 0.0f, 0.0f}, bx, by, bz);
    std::printf("delta local (1,0,0) con base 45deg -> mundo (%.3f, %.3f, %.3f)\n",
                mundo.x, mundo.y, mundo.z);

    std::printf("picking: distancia al eje X en pantalla = %.1f px\n",
                distanciaASegmento({50.0f, 52.0f}, {10.0f, 50.0f}, {90.0f, 50.0f}));
}

#if M24_HAS_IMGUI
void dibujarUI() {
    static float x = 0.0f, y = 0.0f, z = 0.0f;
    static float rot = 0.0f;
    static float escala = 1.0f;
    static int snapDiv = 4;

    ImGui::SetNextWindowPos(ImVec2(24.0f, 24.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(480.0f, 320.0f), ImGuiCond_FirstUseEver);
    ImGui::Begin("M24 / L07 - Gizmo");
    ImGui::TextUnformatted("Simulacion de transform (sin viewport 3D):");
    ImGui::DragFloat("Pos X", &x, 0.1f);
    ImGui::DragFloat("Pos Y", &y, 0.1f);
    ImGui::DragFloat("Pos Z", &z, 0.1f);
    ImGui::SliderFloat("Rot Z", &rot, -180.0f, 180.0f, "%.1f deg");
    ImGui::DragFloat("Escala", &escala, 0.01f, 0.1f, 10.0f);
    ImGui::SliderInt("Snap 1/N", &snapDiv, 1, 16);
    const float paso = 1.0f / static_cast<float>(snapDiv);
    ImGui::Text("snap(0.37, %.3f) = %.3f", paso, snap(0.37f, paso));
    ImGui::TextWrapped("En un viewport real, el mouse se convierte en rayo y el eje "
                       "elegido se intersecta con la recta del gizmo.");
    ImGui::End();
}
#endif

} // namespace

int main() {
    std::puts("M24 / L07 - Matematica de gizmos");
    std::puts("--------------------------------");

    demoHeadless();
#if M24_HAS_IMGUI
    std::puts("");
    return gui::runImGuiDemo("M24 L07 - gizmo", dibujarUI);
#else
    return 0;
#endif
}
