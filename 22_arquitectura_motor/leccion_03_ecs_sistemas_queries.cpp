/* =============================================================================
   MÓDULO 22 - LECCIÓN 03: ECS — Sistemas (Funciones), Queries y Iteración Cache-Friendly
   =============================================================================
   TEMA 1 de TEMAS.md (3/3): Sistemas (funciones), queries y paralelización.

   SISTEMAS EN ECS:
   ----------------
   Un sistema = función que opera sobre un conjunto de entidades (query).
   No tiene estado propio (o estado mínimo: config, recursos compartidos).
   Recibe: Registry&, dt, y opcionalmente recursos globales.

   QUERIES / VIEWS:
   ----------------
   Query = filtro de componentes: With<A, B>, Without<C>, WithOneOf<D, E>.
   En archetype ECS: iterar SOLO archetypes que match el filtro.
   Ventaja: cero branching dentro del loop (todos los componentes garantizados).

   ITERACIÓN CACHE-FRIENDLY:
   -------------------------
   for (Entity e : view<Transform, Velocity>()) {
       auto& t = get<Transform>(e);
       auto& v = get<Velocity>(e);
       t.x += v.vx * dt;
       t.y += v.vy * dt;
   }
   En archetype storage: Transform[] y Velocity[] son arrays paralelos contiguos.
   Loop itera índices 0..N-1: acceso secuencial perfecto, prefetch automático.

   PARALELIZACIÓN:
   ---------------
   Sistemas SIN dependencias de datos → corren en paralelo.
   - Read-only components: múltiples sistemas pueden leer simultáneamente.
   - Write components: un solo sistema escribe por componente (exclusividad).
   - Dependency graph: sistema A escribe Transform, sistema B lee Transform → A antes que B.
   - Job system (lección 14-15): DAG de sistemas, fiber/thread pool.

   QUERY BUILDER PATTERN:
   ----------------------
   view<Transform, Velocity>().without<Static>().each([](Entity, Transform&, Velocity&){...});
   - Compile-time: template parameter pack para With.
   - Runtime: archetype mask matching para Without/WithOneOf.

   SYSTEM ORDERING:
   ----------------
   Fases típicas por frame:
   1. Input → 2. AI/Decision → 3. Physics (fixed step) → 4. Transform update
   5. Animation → 6. Culling → 7. Render command generation → 8. Render submit

   ESTA LECCIÓN: Query view con archetype matching + system scheduler simple.
   ============================================================================= */
#include <iostream>
#include <vector>
#include <unordered_map>
#include <bitset>
#include <cstdint>
#include <cassert>
#include <type_traits>
#include <functional>
#include <tuple>

// ============================================================================
// TIPOS BASE (reutiliza de lecciones 01-02)
// ============================================================================
using Entity = uint32_t;
constexpr uint32_t ENTITY_INDEX_BITS = 20;
constexpr uint32_t ENTITY_INDEX_MASK = (1u << ENTITY_INDEX_BITS) - 1u;
constexpr uint32_t ENTITY_GENERATION_SHIFT = ENTITY_INDEX_BITS;

inline Entity MakeEntity(uint32_t index, uint32_t generation) {
    return (generation << ENTITY_GENERATION_SHIFT) | index;
}
inline uint32_t EntityIndex(Entity e) { return e & ENTITY_INDEX_MASK; }
inline uint32_t EntityGeneration(Entity e) { return e >> ENTITY_GENERATION_SHIFT; }

using ComponentMask = uint64_t;
constexpr size_t MAX_COMPONENT_TYPES = 64;

struct ComponentTypeRegistry {
    static ComponentTypeRegistry& Instance() {
        static ComponentTypeRegistry reg;
        return reg;
    }
    template <typename T> uint8_t Register() {
        static uint8_t id = next_id++; return id;
    }
    template <typename T> ComponentMask Mask() const { return ComponentMask(1) << Register<T>(); }
    size_t next_id = 0;
};
#define REGISTER_COMPONENT(T) \
    template <> uint8_t ComponentTypeRegistry::Register<T>() { \
        static uint8_t id = ComponentTypeRegistry::Instance().next_id++; return id; \
    }

// ============================================================================
// SPARSE SET
// ============================================================================
template <typename Component>
struct SparseSet {
    static_assert(std::is_trivially_copyable_v<Component>, "POD required");
    std::vector<Component> dense;
    std::vector<uint32_t> sparse;
    std::vector<uint32_t> generations;
    std::vector<Entity> dense_to_entity;

    Component* Add(Entity e, Component c = {}) {
        uint32_t idx = EntityIndex(e), gen = EntityGeneration(e);
        if (idx >= sparse.size()) sparse.resize(idx + 1, 0);
        assert(sparse[idx] == 0);
        size_t di = dense.size();
        dense.push_back(c);
        generations.push_back(gen);
        dense_to_entity.push_back(e);
        sparse[idx] = di + 1;
        return &dense.back();
    }
    void Remove(Entity e) {
        uint32_t idx = EntityIndex(e);
        if (idx >= sparse.size()) return;
        size_t di = sparse[idx]; if (di == 0) return; di--;
        size_t last = dense.size() - 1;
        if (di != last) {
            dense[di] = std::move(dense[last]);
            generations[di] = generations[last];
            Entity me = dense_to_entity[last];
            dense_to_entity[di] = me;
            sparse[EntityIndex(me)] = di + 1;
        }
        dense.pop_back(); generations.pop_back(); dense_to_entity.pop_back();
        sparse[idx] = 0;
    }
    Component* Get(Entity e) {
        uint32_t idx = EntityIndex(e);
        if (idx >= sparse.size()) return nullptr;
        size_t di = sparse[idx]; if (di == 0) return nullptr; di--;
        if (generations[di] != EntityGeneration(e)) return nullptr;
        return &dense[di];
    }
    const Component* Get(Entity e) const { return const_cast<SparseSet*>(this)->Get(e); }
    bool Has(Entity e) const { return Get(e) != nullptr; }
    size_t Size() const { return dense.size(); }
};

// ============================================================================
// COMPONENTES
// ============================================================================
struct Transform { float x=0, y=0, rot=0, sx=1, sy=1; };
struct Velocity { float vx=0, vy=0; };
struct Acceleration { float ax=0, ay=0; };
struct Sprite { int tex=0; float u=0,v=0,w=1,h=1; };
struct Tag { char name[32]={}; };
struct StaticTag {};  // Zero-size marker component

REGISTER_COMPONENT(Transform)
REGISTER_COMPONENT(Velocity)
REGISTER_COMPONENT(Acceleration)
REGISTER_COMPONENT(Sprite)
REGISTER_COMPONENT(Tag)
REGISTER_COMPONENT(StaticTag)

// ============================================================================
// ARCHETYPE STORAGE
// ============================================================================
struct ArchetypeStorage {
    ComponentMask mask = 0;
    std::vector<Entity> entities;

    // Component arrays (simplificado: solo los que usamos en queries)
    SparseSet<Transform> transforms;
    SparseSet<Velocity> velocities;
    SparseSet<Acceleration> accelerations;
    SparseSet<Sprite> sprites;
    SparseSet<Tag> tags;
    SparseSet<StaticTag> statics;

    // Helper para Get genérico (en real: type-erased o tuple)
    template <typename T> T* Get(Entity e) {
        if constexpr (std::is_same_v<T, Transform>) return transforms.Get(e);
        else if constexpr (std::is_same_v<T, Velocity>) return velocities.Get(e);
        else if constexpr (std::is_same_v<T, Acceleration>) return accelerations.Get(e);
        else if constexpr (std::is_same_v<T, Sprite>) return sprites.Get(e);
        else if constexpr (std::is_same_v<T, Tag>) return tags.Get(e);
        else if constexpr (std::is_same_v<T, StaticTag>) return statics.Get(e);
        return nullptr;
    }
    template <typename T> const T* Get(Entity e) const {
        return const_cast<ArchetypeStorage*>(this)->Get<T>(e);
    }
    template <typename T> T* Add(Entity e, T c = {}) {
        if constexpr (std::is_same_v<T, Transform>) return transforms.Add(e, c);
        else if constexpr (std::is_same_v<T, Velocity>) return velocities.Add(e, c);
        else if constexpr (std::is_same_v<T, Acceleration>) return accelerations.Add(e, c);
        else if constexpr (std::is_same_v<T, Sprite>) return sprites.Add(e, c);
        else if constexpr (std::is_same_v<T, Tag>) return tags.Add(e, c);
        else if constexpr (std::is_same_v<T, StaticTag>) return statics.Add(e, c);
        return nullptr;
    }
    template <typename T> void Remove(Entity e) {
        if constexpr (std::is_same_v<T, Transform>) transforms.Remove(e);
        else if constexpr (std::is_same_v<T, Velocity>) velocities.Remove(e);
        else if constexpr (std::is_same_v<T, Acceleration>) accelerations.Remove(e);
        else if constexpr (std::is_same_v<T, Sprite>) sprites.Remove(e);
        else if constexpr (std::is_same_v<T, Tag>) tags.Remove(e);
        else if constexpr (std::is_same_v<T, StaticTag>) statics.Remove(e);
    }
};

// ============================================================================
// REGISTRY
// ============================================================================
struct Registry {
    std::unordered_map<ComponentMask, ArchetypeStorage> archetypes;
    std::vector<uint32_t> generations{1};
    std::vector<ComponentMask> masks{0};
    std::vector<ArchetypeStorage*> arch_ptrs{nullptr};
    std::vector<uint32_t> dense_indices{0};
    std::vector<uint32_t> free_indices;

    Entity Create() {
        uint32_t idx;
        if (!free_indices.empty()) { idx = free_indices.back(); free_indices.pop_back(); generations[idx]++; }
        else { idx = generations.size(); generations.push_back(0); masks.push_back(0); arch_ptrs.push_back(nullptr); dense_indices.push_back(0); }
        return MakeEntity(idx, generations[idx]);
    }

    ArchetypeStorage* GetOrCreateArch(ComponentMask mask) {
        auto it = archetypes.find(mask);
        if (it != archetypes.end()) return &it->second;
        return &archetypes[mask];
    }

    void Migrate(Entity e, ComponentMask new_mask) {
        uint32_t idx = EntityIndex(e);
        ArchetypeStorage* old_arch = arch_ptrs[idx];
        ArchetypeStorage* new_arch = GetOrCreateArch(new_mask);
        new_arch->mask = new_mask;

        if (old_arch) {
            // Copiar componentes que persisten
            size_t old_di = dense_indices[idx];
            // Copia manual para demo (en real: iterar bits)
            if ((old_arch->mask & ComponentTypeRegistry::Instance().Mask<Transform>()) &&
                (new_mask & ComponentTypeRegistry::Instance().Mask<Transform>())) {
                if (auto* t = old_arch->Get<Transform>(e)) new_arch->Add<Transform>(e, *t);
            }
            if ((old_arch->mask & ComponentTypeRegistry::Instance().Mask<Velocity>()) &&
                (new_mask & ComponentTypeRegistry::Instance().Mask<Velocity>())) {
                if (auto* v = old_arch->Get<Velocity>(e)) new_arch->Add<Velocity>(e, *v);
            }
            if ((old_arch->mask & ComponentTypeRegistry::Instance().Mask<Acceleration>()) &&
                (new_mask & ComponentTypeRegistry::Instance().Mask<Acceleration>())) {
                if (auto* a = old_arch->Get<Acceleration>(e)) new_arch->Add<Acceleration>(e, *a);
            }
            if ((old_arch->mask & ComponentTypeRegistry::Instance().Mask<Sprite>()) &&
                (new_mask & ComponentTypeRegistry::Instance().Mask<Sprite>())) {
                if (auto* s = old_arch->Get<Sprite>(e)) new_arch->Add<Sprite>(e, *s);
            }
            if ((old_arch->mask & ComponentTypeRegistry::Instance().Mask<Tag>()) &&
                (new_mask & ComponentTypeRegistry::Instance().Mask<Tag>())) {
                if (auto* t = old_arch->Get<Tag>(e)) new_arch->Add<Tag>(e, *t);
            }
            if ((old_arch->mask & ComponentTypeRegistry::Instance().Mask<StaticTag>()) &&
                (new_mask & ComponentTypeRegistry::Instance().Mask<StaticTag>())) {
                new_arch->Add<StaticTag>(e);
            }

            // Remove from old arch entities
            size_t last = old_arch->entities.size() - 1;
            if (old_di != last) {
                Entity me = old_arch->entities[last];
                old_arch->entities[old_di] = me;
                dense_indices[EntityIndex(me)] = old_di;
            }
            old_arch->entities.pop_back();
        }

        arch_ptrs[idx] = new_arch;
        masks[idx] = new_mask;
        dense_indices[idx] = new_arch->entities.size();
        new_arch->entities.push_back(e);
    }

    template <typename T> T& Add(Entity e) {
        ComponentMask cm = ComponentTypeRegistry::Instance().Mask<T>();
        Migrate(e, masks[EntityIndex(e)] | cm);
        ArchetypeStorage* arch = arch_ptrs[EntityIndex(e)];
        return *arch->Add<T>(e);
    }
    template <typename T> void Remove(Entity e) {
        ComponentMask cm = ComponentTypeRegistry::Instance().Mask<T>();
        Migrate(e, masks[EntityIndex(e)] & ~cm);
    }
    template <typename T> T* Get(Entity e) {
        ArchetypeStorage* arch = arch_ptrs[EntityIndex(e)];
        return arch ? arch->Get<T>(e) : nullptr;
    }
    void Destroy(Entity e) {
        uint32_t idx = EntityIndex(e);
        ArchetypeStorage* arch = arch_ptrs[idx];
        if (arch) {
            size_t di = dense_indices[idx];
            size_t last = arch->entities.size() - 1;
            if (di != last) {
                Entity me = arch->entities[last];
                arch->entities[di] = me;
                dense_indices[EntityIndex(me)] = di;
            }
            arch->entities.pop_back();
        }
        arch_ptrs[idx] = nullptr; masks[idx] = 0; free_indices.push_back(idx);
    }

    // ========================================================================
    // QUERY VIEW (iteración sobre archetypes que match)
    // ========================================================================
    template <typename... Components>
    struct View {
        Registry* reg;
        ComponentMask with_mask = 0;
        ComponentMask without_mask = 0;

        View(Registry* r) : reg(r) {
            (void)std::initializer_list<int>{(with_mask |= ComponentTypeRegistry::Instance().Mask<Components>(), 0)...};
        }

        View& Without() { return *this; }  // Para sintaxis .Without<T>(), implementar abajo

        template <typename T>
        View& Without() {
            without_mask |= ComponentTypeRegistry::Instance().Mask<T>();
            return *this;
        }

        // Iterador estilo range-for
        struct Iterator {
            Registry* reg;
            ComponentMask with_mask, without_mask;
            std::vector<ArchetypeStorage*>::iterator arch_it, arch_end;
            size_t entity_idx = 0;
            Entity current_entity = 0;

            Iterator(Registry* r, ComponentMask wm, ComponentMask wom, bool end)
                : reg(r), with_mask(wm), without_mask(wom) {
                if (!end) {
                    // Recopilar archetypes que match
                    matching_archs.clear();
                    for (auto& [mask, arch] : reg->archetypes) {
                        if ((mask & with_mask) == with_mask && (mask & without_mask) == 0) {
                            matching_archs.push_back(&arch);
                        }
                    }
                    arch_it = matching_archs.begin();
                    arch_end = matching_archs.end();
                    AdvanceToValid();
                }
            }

            void AdvanceToValid() {
                while (arch_it != arch_end) {
                    ArchetypeStorage* arch = *arch_it;
                    if (entity_idx < arch->entities.size()) {
                        current_entity = arch->entities[entity_idx];
                        return;
                    }
                    ++arch_it;
                    entity_idx = 0;
                }
                current_entity = 0;
            }

            Entity operator*() const { return current_entity; }
            Iterator& operator++() { entity_idx++; AdvanceToValid(); return *this; }
            bool operator!=(const Iterator& other) const { return current_entity != other.current_entity; }

            std::vector<ArchetypeStorage*> matching_archs;
        };

        Iterator begin() { return Iterator(reg, with_mask, without_mask, false); }
        Iterator end() { return Iterator(reg, with_mask, without_mask, true); }

        // .each(lambda) helper
        template <typename Func>
        void each(Func&& f) {
            for (Entity e : *this) {
                // Obtener componentes via Get
                std::apply([&](auto&... comps) { f(e, comps...); },
                    std::tuple<Components*...>{reg->Get<Components>(e)...});
            }
        }
    };

    template <typename... Components>
    View<Components...> View() { return View<Components...>(this); }
};

// ============================================================================
// SISTEMAS (funciones que usan Views)
// ============================================================================
void PhysicsSystem(Registry& reg, float dt) {
    // Entidades con Transform + Velocity (+ opcional Acceleration)
    // Sin StaticTag
    auto view = reg.View<Transform, Velocity>().Without<StaticTag>();
    for (Entity e : view) {
        auto* t = reg.Get<Transform>(e);
        auto* v = reg.Get<Velocity>(e);
        auto* a = reg.Get<Acceleration>(e);
        if (a) { v->vx += a->ax * dt; v->vy += a->ay * dt; }
        t->x += v->vx * dt;
        t->y += v->vy * dt;
    }
}

void SpriteCullingSystem(Registry& reg, float view_x, float view_y, float view_w, float view_h) {
    auto view = reg.View<Transform, Sprite>();
    for (Entity e : view) {
        auto* t = reg.Get<Transform>(e);
        auto* s = reg.Get<Sprite>(e);
        // Simple AABB culling
        bool visible = t->x + t->sx > view_x && t->x - t->sx < view_x + view_w &&
                       t->y + t->sy > view_y && t->y - t->sy < view_y + view_h;
        // En real: marcar visible/invisible para render system
        (void)visible; (void)s;
    }
}

// ============================================================================
// VERIFICACIÓN DE SINTAXIS
// ============================================================================
int main() {
    std::cout << "=== LECCIÓN 03: ECS Sistemas y Queries (solo sintaxis) ===\n";

    Registry reg;

    // Crear entidades
    Entity player = reg.Create();
    reg.Add<Transform>(player) = Transform{100, 100, 0, 1, 1};
    reg.Add<Velocity>(player) = Velocity{50, 0};
    reg.Add<Tag>(player); strcpy(reg.Get<Tag>(player)->name, "Player");

    Entity enemy = reg.Create();
    reg.Add<Transform>(enemy) = Transform{400, 300, 0, 1, 1};
    reg.Add<Velocity>(enemy) = Velocity{-30, 20};
    reg.Add<StaticTag>(enemy);  // Enemigo estático (no se mueve por physics)

    Entity prop = reg.Create();
    reg.Add<Transform>(prop) = Transform{200, 200, 0, 2, 2};
    reg.Add<Sprite>(prop) = Sprite{1, 0, 0, 32, 32};

    // Ejecutar sistemas
    PhysicsSystem(reg, 0.016f);

    // Verificar: player se movió, enemy no (tiene StaticTag)
    std::cout << "Player pos: " << reg.Get<Transform>(player)->x << ", "
              << reg.Get<Transform>(player)->y << "\n";
    std::cout << "Enemy pos: " << reg.Get<Transform>(enemy)->x << ", "
              << reg.Get<Transform>(enemy)->y << " (StaticTag -> no physics)\n";

    // Query con .each()
    reg.View<Transform, Velocity>().each([](Entity e, Transform& t, Velocity& v) {
        std::cout << "Entity " << e << ": pos=(" << t.x << "," << t.y
                  << ") vel=(" << v.vx << "," << v.vy << ")\n";
    });

    std::cout << "=== FIN LECCIÓN 03 ===\n";
    return 0;
}