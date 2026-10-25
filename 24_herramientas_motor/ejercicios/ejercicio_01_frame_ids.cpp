/* =============================================================================
   MÓDULO 24 - EJERCICIO 01: IDs y estado de interacción (immediate mode)
   =============================================================================
   DIFICULTAD: Fácil
   APOYO: Lecciones 01 y 02 (ciclo de frames, IDs, widgets).

   CONSIGNA
   --------
   Completá `UiFrame` para simular el comportamiento de un widget de ImGui:

   1. `id(label)` = FNV-1a de `label` usando como semilla el tope del stack.
      El stack arranca con la semilla FNV (2166136261u).
   2. `pushId(texto)` apila `id(texto)`; `popId()` desapila (nunca deja el
      stack vacío).
   3. `beginFrame(mouseAbajo)`:
        - detecta el flanco de liberación (`released_ = mouse_ && !mouseAbajo`),
        - actualiza `mouse_`,
        - limpia `hovered_`.
   4. `setHovered(id)` marca el widget bajo el mouse en este frame.
   5. `buttonBehavior(id)`:
        - si el mouse está abajo y `hovered_ == id`, `active_ = id`;
        - devuelve true SOLO en el frame de liberación si `active_ == id` y
          `hovered_ == id`, y en ese caso limpia `active_`;
        - si no, false.
   6. `endFrame()` limpia `released_` y `active_` si el mouse está arriba.

   PISTA: el click se dispara al SOLTAR sobre el mismo widget, no al apretar.
   Es lo que permite cancelar arrastrando el mouse afuera.
   ============================================================================= */
#include "../include/gui_util.hpp"
#include "../../include/test_framework.hpp"

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

class UiFrame {
public:
    void beginFrame(bool mouseAbajo) {
        // TODO
        (void)mouseAbajo;
    }

    void endFrame() {
        // TODO
    }

    void pushId(std::string_view texto) {
        // TODO
        (void)texto;
    }

    void popId() {
        // TODO
    }

    std::uint32_t id(std::string_view label) const {
        // TODO
        (void)label;
        return 0;
    }

    void setHovered(std::uint32_t widget) {
        // TODO
        (void)widget;
    }

    bool buttonBehavior(std::uint32_t widget) {
        // TODO
        (void)widget;
        return false;
    }

    std::uint32_t active() const { return active_; }
    bool mouseDown() const { return mouse_; }

private:
    std::vector<std::uint32_t> stack_{2166136261u};
    std::uint32_t hovered_ = 0;
    std::uint32_t active_ = 0;
    bool mouse_ = false;
    bool released_ = false;
};

// ============================================================================
// TESTS (no tocar)
// ============================================================================
TEST_CASE(IdsDistintosPorLabel) {
    UiFrame f;
    ASSERT_NE(f.id("A"), f.id("B"));
}

TEST_CASE(IdEstableEntreFrames) {
    UiFrame f;
    const auto a = f.id("Boton");
    f.beginFrame(false);
    f.endFrame();
    ASSERT_EQ(f.id("Boton"), a);
}

TEST_CASE(PushIdSeparaScopes) {
    UiFrame f;
    const auto raiz = f.id("Vida");
    f.pushId("enemigo");
    const auto dentro = f.id("Vida");
    ASSERT_NE(dentro, raiz);
    f.popId();
    ASSERT_EQ(f.id("Vida"), raiz);
}

TEST_CASE(PopIdNuncaVaciaElStack) {
    UiFrame f;
    f.popId();
    f.popId();
    ASSERT_EQ(f.id("X"), gui::hashId("X", 2166136261u));
}

TEST_CASE(ClickAlSoltarSobreElMismoWidget) {
    UiFrame f;
    const auto id = f.id("Guardar");
    f.beginFrame(true);
    f.setHovered(id);
    ASSERT_FALSE(f.buttonBehavior(id));
    ASSERT_EQ(f.active(), id);
    f.endFrame();

    f.beginFrame(false);
    f.setHovered(id);
    ASSERT_TRUE(f.buttonBehavior(id));
    ASSERT_EQ(f.active(), 0u);
    f.endFrame();
}

TEST_CASE(SoltarAfueraNoDisparaClick) {
    UiFrame f;
    const auto id = f.id("Guardar");
    f.beginFrame(true);
    f.setHovered(id);
    ASSERT_FALSE(f.buttonBehavior(id));
    f.endFrame();

    f.beginFrame(false);
    f.setHovered(f.id("Otro"));
    ASSERT_FALSE(f.buttonBehavior(id));
}

TEST_CASE(SinApretarNoHayClick) {
    UiFrame f;
    const auto id = f.id("Guardar");
    f.beginFrame(false);
    f.setHovered(id);
    ASSERT_FALSE(f.buttonBehavior(id));
}

TEST_CASE(DosWidgetsMismoLabelEnScopesDistintos) {
    UiFrame f;
    f.pushId("fila0");
    const auto a = f.id("Eliminar");
    f.popId();
    f.pushId("fila1");
    const auto b = f.id("Eliminar");
    f.popId();
    ASSERT_NE(a, b);
}

int main() {
    std::cout << "Ejercicio 01 (IDs y estado de interacción)...\n";
    return RUN_ALL_TESTS();
}
