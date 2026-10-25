/* =============================================================================
   MÓDULO 24 - SOLUCIÓN 11: Mundo ECS-lite para el entity inspector
   =============================================================================
   DIFICULTAD: Alta
   APOYO: Lección 03 (inspector, máscara de componentes) y lección 04
   (jerarquía y reparent).

   CONSIGNA
   --------
   Completá `World`:

   - `create(nombre, padre=0)`: crea una entidad con ID incremental desde 1.
     Si `padre` existe, la agrega a sus hijos. Devuelve el ID nuevo.
   - `find(id)`: puntero a la entidad o nullptr.
   - `roots()`: IDs con padre == 0, en orden creciente.
   - `childrenOf(id)`: copia de los hijos, en orden creciente.
   - `reparent(id, nuevoPadre)`: mueve la entidad. Falla (false) si la entidad
     no existe, si el padre no existe (salvo 0), si es ella misma o si el
     nuevo padre es descendiente de ella (ciclo). Al mover, sacala de la lista
     del padre viejo y agregala al nuevo.
   - `addComponent(id, bit)`: OR de la máscara; no hace nada si no existe.
   - `hasComponent(id, bit)`: true si tiene el bit.
   - `query(mask)`: IDs que tienen TODOS los bits, en orden creciente.
   - `rename(id, nombre)`: true si existe.
   ============================================================================= */
#include "../include/gui_util.hpp"
#include "../../include/test_framework.hpp"

#include <algorithm>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

struct Entity {
    std::uint32_t id = 0;
    std::string nombre;
    std::uint32_t padre = 0;
    std::vector<std::uint32_t> hijos;
    std::uint32_t mascara = 0;
};

class World {
public:
    std::uint32_t create(std::string nombre, std::uint32_t padre = 0) {
        if (padre != 0 && !buscar(padre)) return 0;
        Entity e;
        e.id = siguiente_++;
        e.nombre = std::move(nombre);
        e.padre = padre;
        entidades_.push_back(std::move(e));
        if (padre != 0) buscar(padre)->hijos.push_back(entidades_.back().id);
        return entidades_.back().id;
    }

    const Entity* find(std::uint32_t id) const {
        return buscar(id);
    }

    std::vector<std::uint32_t> roots() const {
        std::vector<std::uint32_t> out;
        for (const auto& e : entidades_) if (e.padre == 0) out.push_back(e.id);
        std::sort(out.begin(), out.end());
        return out;
    }

    std::vector<std::uint32_t> childrenOf(std::uint32_t id) const {
        const Entity* e = buscar(id);
        if (!e) return {};
        auto out = e->hijos;
        std::sort(out.begin(), out.end());
        return out;
    }

    bool reparent(std::uint32_t id, std::uint32_t nuevoPadre) {
        Entity* e = buscar(id);
        if (!e) return false;
        if (nuevoPadre != 0 && !buscar(nuevoPadre)) return false;
        if (id == nuevoPadre) return false;

        // Rechazar ciclos: el nuevo padre no puede ser descendiente de `id`.
        std::uint32_t actual = nuevoPadre;
        while (actual != 0) {
            if (actual == id) return false;
            const Entity* p = buscar(actual);
            if (!p) break;
            actual = p->padre;
        }

        if (e->padre != 0) {
            Entity* viejo = buscar(e->padre);
            if (viejo) {
                viejo->hijos.erase(std::remove(viejo->hijos.begin(), viejo->hijos.end(), id),
                                   viejo->hijos.end());
            }
        }
        e->padre = nuevoPadre;
        if (nuevoPadre != 0) buscar(nuevoPadre)->hijos.push_back(id);
        return true;
    }

    void addComponent(std::uint32_t id, std::uint32_t bit) {
        if (Entity* e = buscar(id)) e->mascara |= bit;
    }

    bool hasComponent(std::uint32_t id, std::uint32_t bit) const {
        const Entity* e = buscar(id);
        return e && (e->mascara & bit) == bit;
    }

    std::vector<std::uint32_t> query(std::uint32_t mascara) const {
        std::vector<std::uint32_t> out;
        for (const auto& e : entidades_) {
            if ((e.mascara & mascara) == mascara) out.push_back(e.id);
        }
        std::sort(out.begin(), out.end());
        return out;
    }

    bool rename(std::uint32_t id, std::string nombre) {
        Entity* e = buscar(id);
        if (!e) return false;
        e->nombre = std::move(nombre);
        return true;
    }

private:
    Entity* buscar(std::uint32_t id) {
        for (auto& e : entidades_) if (e.id == id) return &e;
        return nullptr;
    }
    const Entity* buscar(std::uint32_t id) const {
        for (const auto& e : entidades_) if (e.id == id) return &e;
        return nullptr;
    }

    std::vector<Entity> entidades_;
    std::uint32_t siguiente_ = 1;
};

// ============================================================================
// TESTS (no tocar)
// ============================================================================
TEST_CASE(CreaIdsIncrementales) {
    World w;
    ASSERT_EQ(w.create("Nivel"), 1u);
    ASSERT_EQ(w.create("Jugador"), 2u);
    ASSERT_EQ(w.create("Slime"), 3u);
    const Entity* e = w.find(2);
    ASSERT_TRUE(e != nullptr);
    ASSERT_EQ(e->nombre, std::string("Jugador"));
}

TEST_CASE(JerarquiaYRaices) {
    World w;
    const auto raiz = w.create("Nivel");
    const auto enemigos = w.create("Enemigos", raiz);
    const auto slime = w.create("Slime", enemigos);
    ASSERT_EQ(w.roots().size(), 1u);
    ASSERT_EQ(w.roots()[0], raiz);
    const auto hijos = w.childrenOf(enemigos);
    ASSERT_EQ(hijos.size(), 1u);
    ASSERT_EQ(hijos[0], slime);
    ASSERT_EQ(w.childrenOf(raiz).size(), 1u);
}

TEST_CASE(QueryPorMascara) {
    World w;
    const auto a = w.create("A");
    const auto b = w.create("B");
    const auto c = w.create("C");
    w.addComponent(a, 0x1);
    w.addComponent(a, 0x2);
    w.addComponent(b, 0x1);
    w.addComponent(c, 0x4);
    ASSERT_TRUE(w.hasComponent(a, 0x2));
    ASSERT_FALSE(w.hasComponent(b, 0x2));
    const auto q = w.query(0x1);
    ASSERT_EQ(q.size(), 2u);
    ASSERT_EQ(q[0], a);
    ASSERT_EQ(q[1], b);
    const auto q2 = w.query(0x1 | 0x2);
    ASSERT_EQ(q2.size(), 1u);
    ASSERT_EQ(q2[0], a);
}

TEST_CASE(ReparentLegal) {
    World w;
    const auto raiz = w.create("Nivel");
    const auto a = w.create("A", raiz);
    const auto b = w.create("B", raiz);
    ASSERT_TRUE(w.reparent(a, b));
    const Entity* ea = w.find(a);
    ASSERT_TRUE(ea != nullptr);
    ASSERT_EQ(ea->padre, b);
    ASSERT_EQ(w.childrenOf(b).size(), 1u);
    ASSERT_EQ(w.childrenOf(raiz).size(), 1u);
    ASSERT_EQ(w.childrenOf(raiz)[0], b);
}

TEST_CASE(ReparentRechazaCiclos) {
    World w;
    const auto raiz = w.create("Nivel");
    const auto hijo = w.create("Hijo", raiz);
    const auto nieto = w.create("Nieto", hijo);
    ASSERT_FALSE(w.reparent(raiz, raiz));
    ASSERT_FALSE(w.reparent(raiz, nieto));
    ASSERT_FALSE(w.reparent(raiz, 999));
    ASSERT_FALSE(w.reparent(999, raiz));
    const Entity* r = w.find(raiz);
    ASSERT_TRUE(r != nullptr);
    if (r) ASSERT_EQ(r->padre, 0u);
}

TEST_CASE(Rename) {
    World w;
    const auto id = w.create("Viejo");
    ASSERT_TRUE(w.rename(id, "Nuevo"));
    const Entity* e = w.find(id);
    ASSERT_TRUE(e != nullptr);
    if (e) ASSERT_EQ(e->nombre, std::string("Nuevo"));
    ASSERT_FALSE(w.rename(999, "X"));
}

TEST_CASE(FindInexistenteYComponenteInexistente) {
    World w;
    ASSERT_TRUE(w.find(42) == nullptr);
    w.addComponent(42, 0x1);  // no debe crashear
    ASSERT_FALSE(w.hasComponent(42, 0x1));
}

int main() {
    std::cout << "Solución 11 (entity inspector)...\n";
    return RUN_ALL_TESTS();
}
