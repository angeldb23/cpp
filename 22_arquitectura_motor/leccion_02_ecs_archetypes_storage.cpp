/* =============================================================================
   MÓDULO 22 - LECCIÓN 02: ECS — Archetypes, Sparse Set y Packed Array Storage
   =============================================================================
   TEMA 1 de TEMAS.md (2/3): Archetypes y storage (sparse set / packed array).

   ARCHETYPES EN DETALLE:
   ----------------------
   Un archetype = combinación única de tipos de componente.
   Ejemplos:
   - A1: {Transform}
   - A2: {Transform, Velocity}
   - A3: {Transform, Velocity, Sprite}
   - A4: {Transform, Health, AI}

   Cada entidad pertenece a EXACTAMENTE UN archetype en un momento dado.
   Al añadir/quitar componente → entidad migra a otro archetype.

   VENTAJAS DE ARCHETYPES:
   - Iteración sin branching: todos los componentes existen garantizados.
   - Cache locality perfecta: arrays contiguos por componente dentro del archetype.
   - Filtros with/without triviales: archetype contiene X ≡ with(X), no contiene Y ≡ without(Y).

   SPARSE SET (Conjunto Disperso) — Estructura base por componente en archetype:
   ------------------------------------------------------------------------
   dense[]:   [Comp0, Comp1, Comp2, ...]  — datos contiguos, iteración rápida
   sparse[]:  [index+1 o 0] por entity_id — O(1) lookup entity→dense_index
   generations[]: generation por entry en dense — detecta use-after-free

   Operaciones:
   - Add(entity, comp): dense.push_back(comp), sparse[entity]=dense.size(), gen[entity]=gen
   - Remove(entity): swap-with-last en dense, actualizar sparse del movido, sparse[entity]=0
   - Get(entity): if sparse[entity]!=0 && gen matches → &dense[sparse[entity]-1]
   - Iterar: for (auto& c : dense) ...

   PACKED ARRAY SIMPLE (sin sparse, solo para iteration):
   -----------------------------------------------------
   Solo dense[] + entity_to_index map (unordered_map o vector si ID denso).
   Más simple, pero Remove es O(N) o swap-with-last + actualizar map.
   Adecuado para componentes que rara vez se añaden/quitan (ej. Transform siempre presente).

   DISEÑO HÍBRIDO DE ESTA LECCIÓN:
   - ArchetypeStorage: un struct por archetype con SparseSet por componente.
   - ArchetypeGraph: edges entre archetypes por Add/Remove componente.
   - Registry: map<Signature, ArchetypeStorage> + entity→(archetype*, index_in_archetype).

   SIGNATURE (firma de archetype):
   -------------------------------
   bitset<MAX_COMPONENTS> o uint64_t (si ≤64 componentes).
   Cada bit = tipo de componente registrado.
   Signature permite lookup O(1) de archetype.

   ============================================================================= */
#include <iostream>
#include <vector>
#include <unordered_map>
#include <bitset>
#include <cstdint>
#include <cassert>
#include <type_traits>
#include <array>
#include <algorithm>

// ============================================================================
// CONFIGURACIÓN
// ============================================================================
constexpr size_t MAX_COMPONENT_TYPES = 64;
using ComponentMask = uint64_t;  // bitset manual para velocidad
using Entity = uint32_t;
constexpr uint32_t ENTITY_INDEX_BITS = 20;
constexpr uint32_t ENTITY_INDEX_MASK = (1u << ENTITY_INDEX_BITS) - 1u;
constexpr uint32_t ENTITY_GENERATION_SHIFT = ENTITY_INDEX_BITS;

inline Entity MakeEntity(uint32_t index, uint32_t generation) {
    return (generation << ENTITY_GENERATION_SHIFT) | index;
}
inline uint32_t EntityIndex(Entity e) { return e & ENTITY_INDEX_MASK; }
inline uint32_t EntityGeneration(Entity e) { return e >> ENTITY_GENERATION_SHIFT; }

// ============================================================================
// COMPONENT TYPE REGISTRY (asigna bit ID a cada tipo)
// ============================================================================
struct ComponentTypeRegistry {
    static ComponentTypeRegistry& Instance() {
        static ComponentTypeRegistry reg;
        return reg;
    }

    template <typename T>
    uint8_t Register() {
        static uint8_t id = [this]() {
            uint8_t new_id = next_id++;
            assert(new_id < MAX_COMPONENT_TYPES);
            return new_id;
        }();
        return id;
    }

    template <typename T>
    ComponentMask Mask() const { return ComponentMask(1) << Register<T>(); }

    size_t next_id = 0;
};

// Helper macro para registrar componentes
#define REGISTER_COMPONENT(T) \
    template <> uint8_t ComponentTypeRegistry::Register<T>() { \
        static uint8_t id = ComponentTypeRegistry::Instance().next_id++; \
        return id; \
    }

// ============================================================================
// SPARSE SET POR COMPONENTE (dentro de un archetype)
// ============================================================================
template <typename Component>
struct SparseSet {
    static_assert(std::is_trivially_copyable_v<Component>, "Component debe ser POD");

    std::vector<Component> dense;
    std::vector<uint32_t> sparse;      // entity_index → dense_index + 1 (0 = none)
    std::vector<uint32_t> generations; // generation por entry en dense
    std::vector<Entity> dense_to_entity; // Para swap-with-last: entity por dense index

    Component* Add(Entity entity, Component comp = Component{}) {
        uint32_t idx = EntityIndex(entity);
        uint32_t gen = EntityGeneration(entity);

        if (idx >= sparse.size()) sparse.resize(idx + 1, 0);
        assert(sparse[idx] == 0 && "Entity already has component");

        size_t dense_idx = dense.size();
        dense.push_back(comp);
        generations.push_back(gen);
        dense_to_entity.push_back(entity);
        sparse[idx] = dense_idx + 1;
        return &dense.back();
    }

    void Remove(Entity entity) {
        uint32_t idx = EntityIndex(entity);
        if (idx >= sparse.size()) return;
        size_t dense_idx = sparse[idx];
        if (dense_idx == 0) return;
        dense_idx--;

        // Swap with last
        size_t last = dense.size() - 1;
        if (dense_idx != last) {
            dense[dense_idx] = std::move(dense[last]);
            generations[dense_idx] = generations[last];
            Entity moved_e = dense_to_entity[last];
            dense_to_entity[dense_idx] = moved_e;
            sparse[EntityIndex(moved_e)] = dense_idx + 1;
        }
        dense.pop_back();
        generations.pop_back();
        dense_to_entity.pop_back();
        sparse[idx] = 0;
    }

    Component* Get(Entity entity) {
        uint32_t idx = EntityIndex(entity);
        if (idx >= sparse.size()) return nullptr;
        size_t dense_idx = sparse[idx];
        if (dense_idx == 0) return nullptr;
        dense_idx--;
        if (generations[dense_idx] != EntityGeneration(entity)) return nullptr;
        return &dense[dense_idx];
    }

    const Component* Get(Entity entity) const {
        return const_cast<SparseSet*>(this)->Get(entity);
    }

    bool Has(Entity entity) const { return Get(entity) != nullptr; }
    size_t Size() const { return dense.size(); }
    void Clear() { dense.clear(); generations.clear(); dense_to_entity.clear(); sparse.assign(sparse.size(), 0); }
};

// ============================================================================
// ARCHETYPE STORAGE (un storage por combinación de componentes)
// ============================================================================
struct ArchetypeStorage {
    ComponentMask mask = 0;
    std::vector<Entity> entities;  // Entidades en este archetype (orden = dense index)

    // Un SparseSet por tipo de componente (indexado por component type ID)
    // Usamos void* + type-erased callbacks porque cada archetype tiene componentes distintos
    // En implementación real: tuple<SparseSet<T>...> o vector<unique_ptr<IComponentArray>>
    // Aquí simplificamos: almacenamos punteros a arrays genéricos por componente.
    // Para lección: demostramos con Transform + Velocity hardcodeados.

    // Versión simplificada para demo: solo Transform y Velocity
    SparseSet<Transform> transforms;
    SparseSet<Velocity> velocities;

    // En implementación completa: std::array<void*, MAX_COMPONENT_TYPES> component_arrays;
    // con vtable de operaciones (add, remove, get, move).

    bool Matches(ComponentMask query_mask, ComponentMask query_without) const {
        return (mask & query_mask) == query_mask && (mask & query_without) == 0;
    }
};

// ============================================================================
// COMPONENTES POD
// ============================================================================
struct Transform { float x=0, y=0, rot=0, sx=1, sy=1; };
struct Velocity { float vx=0, vy=0; };
struct Sprite { int tex_id=0; float u=0, v=0, w=1, h=1; };
struct Health { int current=100, max=100; };

REGISTER_COMPONENT(Transform)
REGISTER_COMPONENT(Velocity)
REGISTER_COMPONENT(Sprite)
REGISTER_COMPONENT(Health)

// ============================================================================
// REGISTRY CON ARCHETYPES
// ============================================================================
struct Registry {
    std::unordered_map<ComponentMask, ArchetypeStorage> archetypes;
    std::vector<uint32_t> entity_generations;  // index → generation
    std::vector<ComponentMask> entity_masks;   // index → archetype mask
    std::vector<ArchetypeStorage*> entity_archetype;  // index → ptr a archetype
    std::vector<uint32_t> entity_dense_index; // index → index en archetype.entities
    std::vector<uint32_t> free_indices;

    Registry() { entity_generations.push_back(0); entity_masks.push_back(0); entity_archetype.push_back(nullptr); entity_dense_index.push_back(0); }

    Entity CreateEntity() {
        uint32_t index;
        if (!free_indices.empty()) {
            index = free_indices.back(); free_indices.pop_back();
            entity_generations[index]++;
        } else {
            index = entity_generations.size();
            entity_generations.push_back(0);
            entity_masks.push_back(0);
            entity_archetype.push_back(nullptr);
            entity_dense_index.push_back(0);
        }
        return MakeEntity(index, entity_generations[index]);
    }

    ArchetypeStorage* GetOrCreateArchetype(ComponentMask mask) {
        auto it = archetypes.find(mask);
        if (it != archetypes.end()) return &it->second;
        ArchetypeStorage& arch = archetypes[mask];
        arch.mask = mask;
        return &arch;
    }

    void MoveEntityToArchetype(Entity entity, ArchetypeStorage* new_arch) {
        uint32_t idx = EntityIndex(entity);
        ArchetypeStorage* old_arch = entity_archetype[idx];
        if (old_arch == new_arch) return;

        // Remove from old archetype
        if (old_arch) {
            size_t dense_idx = entity_dense_index[idx];
            size_t last = old_arch->entities.size() - 1;
            if (dense_idx != last) {
                Entity moved_e = old_arch->entities[last];
                old_arch->entities[dense_idx] = moved_e;
                uint32_t moved_idx = EntityIndex(moved_e);
                entity_dense_index[moved_idx] = dense_idx;
            }
            old_arch->entities.pop_back();
        }

        // Add to new archetype
        entity_archetype[idx] = new_arch;
        entity_masks[idx] = new_arch->mask;
        entity_dense_index[idx] = new_arch->entities.size();
        new_arch->entities.push_back(entity);
    }

    // Add component: migra entidad a archetype con componente añadido
    template <typename Component>
    Component& Add(Entity entity) {
        uint32_t idx = EntityIndex(entity);
        ComponentMask comp_mask = ComponentTypeRegistry::Instance().Mask<Component>();
        ComponentMask new_mask = entity_masks[idx] | comp_mask;
        ArchetypeStorage* new_arch = GetOrCreateArchetype(new_mask);

        // Migrar datos de componentes existentes
        ArchetypeStorage* old_arch = entity_archetype[idx];
        if (old_arch) {
            // Copiar componentes existentes al nuevo archetype
            // En implementación real: iterar bits de old_mask y mover cada SparseSet
            // Aquí demo: solo Transform y Velocity
            if (old_arch->mask & ComponentTypeRegistry::Instance().Mask<Transform>()) {
                if (auto* t = old_arch->transforms.Get(entity)) {
                    new_arch->transforms.Add(entity, *t);
                }
            }
            if (old_arch->mask & ComponentTypeRegistry::Instance().Mask<Velocity>()) {
                if (auto* v = old_arch->velocities.Get(entity)) {
                    new_arch->velocities.Add(entity, *v);
                }
            }
        }

        MoveEntityToArchetype(entity, new_arch);
        return *new_arch->transforms.Add(entity);  // Demo: retorna Transform
    }

    template <typename Component>
    void Remove(Entity entity) {
        uint32_t idx = EntityIndex(entity);
        ComponentMask comp_mask = ComponentTypeRegistry::Instance().Mask<Component>();
        ComponentMask new_mask = entity_masks[idx] & ~comp_mask;
        ArchetypeStorage* new_arch = GetOrCreateArchetype(new_mask);

        ArchetypeStorage* old_arch = entity_archetype[idx];
        if (old_arch) {
            // Mover componentes que se quedan
            if ((new_mask & ComponentTypeRegistry::Instance().Mask<Transform>()) &&
                (old_arch->mask & ComponentTypeRegistry::Instance().Mask<Transform>())) {
                if (auto* t = old_arch->transforms.Get(entity)) {
                    new_arch->transforms.Add(entity, *t);
                }
            }
            if ((new_mask & ComponentTypeRegistry::Instance().Mask<Velocity>()) &&
                (old_arch->mask & ComponentTypeRegistry::Instance().Mask<Velocity>())) {
                if (auto* v = old_arch->velocities.Get(entity)) {
                    new_arch->velocities.Add(entity, *v);
                }
            }
        }

        MoveEntityToArchetype(entity, new_arch);
    }

    template <typename Component>
    Component* Get(Entity entity) {
        uint32_t idx = EntityIndex(entity);
        ArchetypeStorage* arch = entity_archetype[idx];
        if (!arch) return nullptr;
        if (std::is_same_v<Component, Transform>) return arch->transforms.Get(entity);
        if (std::is_same_v<Component, Velocity>) return arch->velocities.Get(entity);
        return nullptr;
    }

    void DestroyEntity(Entity entity) {
        uint32_t idx = EntityIndex(entity);
        ArchetypeStorage* arch = entity_archetype[idx];
        if (arch) {
            // Remove from archetype entities array
            size_t dense_idx = entity_dense_index[idx];
            size_t last = arch->entities.size() - 1;
            if (dense_idx != last) {
                Entity moved_e = arch->entities[last];
                arch->entities[dense_idx] = moved_e;
                entity_dense_index[EntityIndex(moved_e)] = dense_idx;
            }
            arch->entities.pop_back();
        }
        entity_archetype[idx] = nullptr;
        entity_masks[idx] = 0;
        free_indices.push_back(idx);
    }
};

// ============================================================================
// VERIFICACIÓN DE SINTAXIS
// ============================================================================
int main() {
    std::cout << "=== LECCIÓN 02: ECS Archetypes y Storage (solo sintaxis) ===\n";

    Registry reg;
    Entity e1 = reg.CreateEntity();
    Entity e2 = reg.CreateEntity();

    // Añadir componentes (migra archetypes)
    auto& t1 = reg.Add<Transform>(e1);
    t1.x = 10; t1.y = 20;
    auto& v1 = reg.Add<Velocity>(e1);
    v1.vx = 5; v1.vy = 3;

    reg.Add<Transform>(e2) = Transform{100, 100, 0, 1, 1};

    // Query
    if (auto* t = reg.Get<Transform>(e1)) std::cout << "e1 pos: " << t->x << "," << t->y << "\n";
    if (auto* v = reg.Get<Velocity>(e1)) std::cout << "e1 vel: " << v->vx << "," << v->vy << "\n";

    // Quitar componente
    reg.Remove<Velocity>(e1);
    assert(reg.Get<Velocity>(e1) == nullptr);
    assert(reg.Get<Transform>(e1) != nullptr);

    reg.DestroyEntity(e1);
    assert(reg.Get<Transform>(e1) == nullptr);

    std::cout << "Archetypes creados: " << reg.archetypes.size() << "\n";
    for (auto& [mask, arch] : reg.archetypes) {
        std::cout << "  Mask: " << mask << " | Entities: " << arch.entities.size() << "\n";
    }

    std::cout << "=== FIN LECCIÓN 02 ===\n";
    return 0;
}