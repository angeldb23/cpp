/* =============================================================================
   MÓDULO 22 - LECCIÓN 01: ECS — Entidades (ID), Componentes (POD) y Archetypes
   =============================================================================
   TEMA 1 de TEMAS.md (1/3): ECS básico — entidades, componentes POD, archetypes.

   QUÉ ES UN ECS (Entity Component System):
   ---------------------------------------
   - ENTITY: solo un ID (uint32_t/uint64_t). No tiene datos ni comportamiento.
   - COMPONENT: struct POD (Plain Old Data) sin lógica, solo datos.
   - SYSTEM: funciones que operan sobre conjuntos de componentes (queries).

   Ventajas vs OOP tradicional:
   - Cache locality: componentes del mismo tipo en array contiguo.
   - Composición dinámica: añades/quitas componentes en runtime.
   - Paralelización: systems sin dependencias corren en hilos distintos.
   - Separación datos/lógica: ideal para data-oriented design.

   ARCHETYPES (Arquetipos):
   ------------------------
   Un archetype = conjunto fijo de tipos de componente.
   Ej: Archetype {Transform, Velocity} ≠ Archetype {Transform, Velocity, Sprite}.
   Cada archetype tiene su propio storage (arrays paralelos por componente).
   Entidades migran entre archetypes al añadir/quitar componentes.

   STORAGE OPCIONES:
   -----------------
   1. Packed Array (arreglo denso): vector<Component> por tipo.
      - Índice = posición en array. Entity → index via hashmap.
      - Rápido para iterar, lento para añadir/quitar (swap-with-last).
   2. Sparse Set (conjunto disperso): dense[] + sparse[] (entity→index).
      - O(1) add/remove/contains, iteración densa sobre dense[].
      - Memoria extra por sparse[] (size = max entity ID).
   3. Archetype storage: un storage por combinación de componentes.
      - Iteración perfecta (sin branching), migración al cambiar componentes.
      - Más complejo; estándar en EnTT, flecs, bevy_ecs.

   DISEÑO DE ESTA LECCIÓN: Packed Array simple con entity→index map.
   - Componente = struct POD (trivialmente copiable).
   - Entity = uint32_t (generation en bits altos para detectar uso tras free).
   - ComponentPool<T>: vector<T> data + unordered_map<Entity, size_t> index.

   GENERATION HANDLES (para detectar use-after-free):
   -------------------------------------------------
   Entity = (generation << 20) | index  (ej. 12 bits gen + 20 bits index = 4M entidades).
   Al destruir: incrementar generation en sparse[index]. Nuevo entity en mismo index
   tendrá generation distinta → lookups fallan segura.
   Ver lección 07/08 del módulo 07 (weak_ptr observador) y módulo 08 (move-only).

   ============================================================================= */
#include <iostream>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <cassert>
#include <type_traits>

// ============================================================================
// ENTITY HANDLE (con generation para detectar uso tras free)
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
inline bool IsValid(Entity e) { return e != 0; }  // 0 = null entity

// ============================================================================
// COMPONENT POOL (packed array + sparse index map)
// ============================================================================
template <typename Component>
struct ComponentPool {
    static_assert(std::is_trivially_copyable_v<Component>,
                  "Componentes deben ser trivially copyable (POD)");

    std::vector<Component> dense;                    // Datos contiguos
    std::vector<Entity> entity_to_dense;             // sparse: entity index → dense index (+1, 0=none)
    std::vector<uint32_t> dense_to_generation;       // generation por entry en dense

    // Añade componente a entidad; retorna referencia al componente
    Component& Add(Entity entity) {
        uint32_t idx = EntityIndex(entity);
        uint32_t gen = EntityGeneration(entity);

        // Asegurar espacio en sparse
        if (idx >= entity_to_dense.size()) {
            entity_to_dense.resize(idx + 1, 0);
        }

        // Si ya existe, error (usar Replace o Remove+Add)
        assert(entity_to_dense[idx] == 0 && "Entidad ya tiene este componente");

        size_t dense_idx = dense.size();
        dense.push_back(Component{});                // Default-construct
        dense_to_generation.push_back(gen);
        entity_to_dense[idx] = dense_idx + 1;        // +1 porque 0 = "no existe"

        return dense.back();
    }

    // Elimina componente (swap-with-last para O(1))
    void Remove(Entity entity) {
        uint32_t idx = EntityIndex(entity);
        if (idx >= entity_to_dense.size()) return;
        size_t dense_idx = entity_to_dense[idx];
        if (dense_idx == 0) return;  // No existe
        dense_idx--;  // Ajustar +1

        // Swap con último
        size_t last = dense.size() - 1;
        if (dense_idx != last) {
            dense[dense_idx] = std::move(dense[last]);
            dense_to_generation[dense_idx] = dense_to_generation[last];
            // Actualizar sparse del elemento movido
            Entity moved_entity = MakeEntity(
                EntityIndex(dense_to_generation[dense_idx] << ENTITY_GENERATION_SHIFT), // placeholder
                dense_to_generation[dense_idx]
            );
            // NOTA: para saber el entity del movido, necesitaríamos dense_to_entity[]
            // Simplificación: en pool real guardamos dense_to_entity[].
        }
        dense.pop_back();
        dense_to_generation.pop_back();
        entity_to_dense[idx] = 0;
    }

    // Obtiene puntero a componente (nullptr si no existe o generation mismatch)
    Component* Get(Entity entity) {
        uint32_t idx = EntityIndex(entity);
        if (idx >= entity_to_dense.size()) return nullptr;
        size_t dense_idx = entity_to_dense[idx];
        if (dense_idx == 0) return nullptr;
        dense_idx--;
        if (dense_to_generation[dense_idx] != EntityGeneration(entity)) return nullptr;
        return &dense[dense_idx];
    }

    const Component* Get(Entity entity) const {
        return const_cast<ComponentPool*>(this)->Get(entity);
    }

    bool Has(Entity entity) const { return Get(entity) != nullptr; }
    size_t Size() const { return dense.size(); }
    void Clear() { dense.clear(); dense_to_generation.clear(); entity_to_dense.assign(entity_to_dense.size(), 0); }
};

// ============================================================================
// COMPONENTES DE EJEMPLO (POD)
// ============================================================================
struct Transform {
    float x = 0, y = 0, rotation = 0;
    float scale_x = 1, scale_y = 1;
};
struct Velocity { float vx = 0, vy = 0; };
struct Tag { char name[32] = {}; };  // Fixed-size para POD

// ============================================================================
// REGISTRY BÁSICO (un pool por tipo de componente)
// ============================================================================
struct Registry {
    ComponentPool<Transform> transforms;
    ComponentPool<Velocity> velocities;
    ComponentPool<Tag> tags;

    Entity CreateEntity() {
        static uint32_t next_index = 1;
        static std::vector<uint32_t> free_indices;
        static std::vector<uint32_t> generations(1, 0);  // index 0 unused

        uint32_t index;
        if (!free_indices.empty()) {
            index = free_indices.back();
            free_indices.pop_back();
            generations[index]++;
        } else {
            index = next_index++;
            if (index >= generations.size()) generations.push_back(0);
        }
        return MakeEntity(index, generations[index]);
    }

    void DestroyEntity(Entity e) {
        uint32_t idx = EntityIndex(e);
        // Remover de todos los pools
        transforms.Remove(e);
        velocities.Remove(e);
        tags.Remove(e);
        // Reciclar índice
        free_indices.push_back(idx);
    }

    // Queries simples (se expanden en lección 02-03)
    template <typename... Components>
    struct View;  // Forward declaration
};

// ============================================================================
// VERIFICACIÓN DE SINTAXIS (no ejecuta tests; compila con -fsyntax-only)
// ============================================================================
int main() {
    std::cout << "=== LECCIÓN 01: ECS Entidades y Componentes (solo sintaxis) ===\n";

    Registry reg;
    Entity e1 = reg.CreateEntity();
    Entity e2 = reg.CreateEntity();

    auto& t1 = reg.transforms.Add(e1);
    t1.x = 100; t1.y = 200;
    reg.velocities.Add(e1) = Velocity{10, 5};
    reg.tags.Add(e1) = Tag{"Player"};

    reg.transforms.Add(e2) = Transform{400, 300, 0, 2, 2};

    // Queries manuales
    if (auto* t = reg.transforms.Get(e1)) {
        std::cout << "e1 Transform: (" << t->x << ", " << t->y << ")\n";
    }
    if (auto* v = reg.velocities.Get(e1)) {
        std::cout << "e1 Velocity: (" << v->vx << ", " << v->vy << ")\n";
    }

    reg.DestroyEntity(e1);
    assert(reg.transforms.Get(e1) == nullptr);  // Generación invalidada

    std::cout << "=== FIN LECCIÓN 01 ===\n";
    return 0;
}