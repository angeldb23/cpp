/* =============================================================================
   MÓDULO 24 - EJERCICIO 03: Property grid con dirty/revert
   =============================================================================
   DIFICULTAD: Media
   APOYO: Lección 08 (property grid) y lección 04 (component editor).

   CONSIGNA
   --------
   Completá `PropertyGrid`:

   - `add(nombre, valor, readOnly)` agrega una propiedad. Si el nombre ya
     existe, la reemplaza y resetea su valor original.
   - `set(nombre, valor)` edita: devuelve false si no existe o es read-only.
   - `get(nombre)` devuelve puntero al valor actual o nullptr.
   - `dirty(nombre)` = el valor actual difiere del original.
   - `revert(nombre)` vuelve al original; `revertAll()` revierte todas.
   - `filtrar(texto)` devuelve los NOMBRES que contienen el texto (sin
     distinguir mayúsculas), en orden de inserción.
   - `toIni()` devuelve una línea por propiedad: `nombre=valor\n` con
     int decimal, float con 3 decimales fijos, bool `true`/`false`, string
     tal cual.
   ============================================================================= */
#include "../include/gui_util.hpp"
#include "../../include/test_framework.hpp"

#include <iomanip>
#include <sstream>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

using Valor = std::variant<int, float, bool, std::string>;

struct Propiedad {
    std::string nombre;
    Valor valor;
    Valor original;
    bool readOnly = false;
};

class PropertyGrid {
public:
    void add(std::string nombre, Valor valor, bool readOnly = false) {
        // TODO
        (void)nombre;
        (void)valor;
        (void)readOnly;
    }

    bool set(std::string_view nombre, Valor valor) {
        // TODO
        (void)nombre;
        (void)valor;
        return false;
    }

    const Valor* get(std::string_view nombre) const {
        // TODO
        (void)nombre;
        return nullptr;
    }

    bool dirty(std::string_view nombre) const {
        // TODO
        (void)nombre;
        return false;
    }

    void revert(std::string_view nombre) {
        // TODO
        (void)nombre;
    }

    void revertAll() {
        // TODO
    }

    std::vector<std::string> filtrar(std::string_view texto) const {
        // TODO
        (void)texto;
        return {};
    }

    std::string toIni() const {
        // TODO
        return {};
    }

private:
    Propiedad* buscar(std::string_view nombre) {
        for (auto& p : props_) if (p.nombre == nombre) return &p;
        return nullptr;
    }
    const Propiedad* buscar(std::string_view nombre) const {
        for (const auto& p : props_) if (p.nombre == nombre) return &p;
        return nullptr;
    }

    std::vector<Propiedad> props_;
};

// ============================================================================
// TESTS (no tocar)
// ============================================================================
TEST_CASE(SetYGet) {
    PropertyGrid g;
    g.add("vida", 100);
    ASSERT_TRUE(g.set("vida", 75));
    const Valor* v = g.get("vida");
    ASSERT_TRUE(v != nullptr);
    ASSERT_EQ(std::get<int>(*v), 75);
}

TEST_CASE(ReadOnlyNoSeEdita) {
    PropertyGrid g;
    g.add("id", 7, true);
    ASSERT_FALSE(g.set("id", 8));
    const Valor* v = g.get("id");
    ASSERT_TRUE(v != nullptr);
    if (v) ASSERT_EQ(std::get<int>(*v), 7);
}

TEST_CASE(DirtyYRevert) {
    PropertyGrid g;
    g.add("speed", 3.5f);
    g.add("activo", true);
    ASSERT_FALSE(g.dirty("speed"));
    ASSERT_TRUE(g.set("speed", 4.0f));
    ASSERT_TRUE(g.dirty("speed"));
    g.revert("speed");
    ASSERT_FALSE(g.dirty("speed"));
    const Valor* speed = g.get("speed");
    ASSERT_TRUE(speed != nullptr);
    if (speed) ASSERT_NEAR(std::get<float>(*speed), 3.5f, 1e-6f);

    ASSERT_TRUE(g.set("activo", false));
    ASSERT_TRUE(g.set("speed", 9.0f));
    g.revertAll();
    const Valor* activo = g.get("activo");
    const Valor* speed2 = g.get("speed");
    ASSERT_TRUE(activo != nullptr);
    ASSERT_TRUE(speed2 != nullptr);
    if (activo) ASSERT_TRUE(std::get<bool>(*activo));
    if (speed2) ASSERT_NEAR(std::get<float>(*speed2), 3.5f, 1e-6f);
}

TEST_CASE(FiltroSinMayusculas) {
    PropertyGrid g;
    g.add("VidaMax", 100);
    g.add("velocidad", 3.0f);
    g.add("nombre", std::string("Slime"));
    const auto f = g.filtrar("VIDA");
    ASSERT_EQ(f.size(), 1u);
    ASSERT_EQ(f[0], std::string("VidaMax"));
}

TEST_CASE(ToIniFormato) {
    PropertyGrid g;
    g.add("vida", 100);
    g.add("speed", 3.5f);
    g.add("visible", true);
    g.add("nombre", std::string("Slime"));
    ASSERT_EQ(g.toIni(),
              std::string("vida=100\nspeed=3.500\nvisible=true\nnombre=Slime\n"));
}

TEST_CASE(SetDeNombreInexistente) {
    PropertyGrid g;
    ASSERT_FALSE(g.set("nada", 1));
    ASSERT_TRUE(g.get("nada") == nullptr);
}

TEST_CASE(AddReemplazaYReseteaOriginal) {
    PropertyGrid g;
    g.add("x", 1);
    g.set("x", 2);
    g.add("x", 5);
    ASSERT_FALSE(g.dirty("x"));
    const Valor* v = g.get("x");
    ASSERT_TRUE(v != nullptr);
    if (v) ASSERT_EQ(std::get<int>(*v), 5);
}

int main() {
    std::cout << "Ejercicio 03 (property grid)...\n";
    return RUN_ALL_TESTS();
}
