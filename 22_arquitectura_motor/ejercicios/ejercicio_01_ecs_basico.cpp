/* =============================================================================
   MÓDULO 22 - EJERCICIO 01: ECS Básico — Entidades, Componentes, Pool
   =============================================================================
   DIFICULTAD: Básico
   APOYO: Lección 01 (ECS entidades y componentes), Lección 02 (Archetypes)

   CONSIGNA:
   Implementa un ECS mínimo con:
   - Entity (uint32_t con generation)
   - ComponentPool<T> (packed array + sparse index)
   - Registry con CreateEntity, DestroyEntity, Add/Get/Remove component

   TESTS (no tocar):
   - Crear entidades, añadir componentes, verificar Get
   - DestroyEntity invalida handles (generation mismatch)
   - Remove component funciona y compacta storage
   - Iterar sobre entidades con componente específico

   NOTA: Este ejercicio es LÓGICA PURA (sin gráficos, sin ventana).
   ============================================================================= */
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <cassert>
#include <type_traits>
#include "../../include/test_framework.hpp"

using Entity = uint32_t;
constexpr uint32_t ENTITY_INDEX_BITS = 20;
constexpr uint32_t ENTITY_INDEX_MASK = (1u << ENTITY_INDEX_BITS) - 1u;
constexpr uint32_t ENTITY_GENERATION_SHIFT = ENTITY_INDEX_BITS;

inline Entity MakeEntity(uint32_t index, uint32_t generation) {
    return (generation << ENTITY_GENERATION_SHIFT) | index;
}
inline uint32_t EntityIndex(Entity e) { return e & ENTITY_INDEX_MASK; }
inline uint32_t EntityGeneration(Entity e) { return e >> ENTITY_GENERATION_SHIFT; }
inline bool IsValid(Entity e) { return e != 0; }

// ============================================================================
// TODO 1: Implementa ComponentPool<T>
// ============================================================================
template <typename Component>
struct ComponentPool {
    static_assert(std::is_trivially_copyable_v<Component>, "Component debe ser POD");

    std::vector<Component> dense;
    std::vector<uint32_t> sparse;           // entity_index -> dense_index + 1
    std::vector<uint32_t> generations;      // generation por entry en dense
    std::vector<Entity> dense_to_entity;    // para swap-with-last

    // Añade componente a entidad; retorna referencia
    Component& Add(Entity entity) {
        // TU CÓDIGO AQUÍ
        (void)entity;
        return dense.emplace_back();
    }

    // Elimina componente (swap-with-last)
    void Remove(Entity entity) {
        // TU CÓDIGO AQUÍ
        (void)entity;
    }

    // Obtiene puntero (nullptr si no existe o generation mismatch)
    Component* Get(Entity entity) {
        // TU CÓDIGO AQUÍ
        (void)entity;
        return nullptr;
    }
    const Component* Get(Entity entity) const {
        return const_cast<ComponentPool*>(this)->Get(entity);
    }
    bool Has(Entity entity) const { return Get(entity) != nullptr; }
    size_t Size() const { return dense.size(); }
    void Clear() { dense.clear(); generations.clear(); dense_to_entity.clear(); sparse.assign(sparse.size(), 0); }
};

// ============================================================================
// COMPONENTES DE TEST
// ============================================================================
struct Transform { float x=0, y=0, rot=0, sx=1, sy=1; };
struct Velocity { float vx=0, vy=0; };
struct Tag { char name[32]={}; };

// ============================================================================
// TODO 2: Implementa Registry
// ============================================================================
struct Registry {
    ComponentPool<Transform> transforms;
    ComponentPool<Velocity> velocities;
    ComponentPool<Tag> tags;

    std::vector<uint32_t> generations{1};
    std::vector<uint32_t> free_indices;

    Entity CreateEntity() {
        // TU CÓDIGO AQUÍ
        return 0;
    }

    void DestroyEntity(Entity e) {
        // TU CÓDIGO AQUÍ
        (void)e;
    }

    template<typename T> T* Get(Entity e) {
        if constexpr(std::is_same_v<T, Transform>) return transforms.Get(e);
        else if constexpr(std::is_same_v<T, Velocity>) return velocities.Get(e);
        else if constexpr(std::is_same_v<T, Tag>) return tags.Get(e);
        return nullptr;
    }
    template<typename T> T& Add(Entity e) {
        if constexpr(std::is_same_v<T, Transform>) return transforms.Add(e);
        else if constexpr(std::is_same_v<T, Velocity>) return velocities.Add(e);
        else if constexpr(std::is_same_v<T, Tag>) return tags.Add(e);
        throw std::runtime_error("Unsupported");
    }
    template<typename T> void Remove(Entity e) {
        if constexpr(std::is_same_v<T, Transform>) transforms.Remove(e);
        else if constexpr(std::is_same_v<T, Velocity>) velocities.Remove(e);
        else if constexpr(std::is_same_v<T, Tag>) tags.Remove(e);
    }
};

// ============================================================================
// TESTS
// ============================================================================
TEST_CASE(CreateEntityReturnsValidHandle) {
    Registry reg;
    Entity e = reg.CreateEntity();
    ASSERT_TRUE(IsValid(e));
    ASSERT_NE(e, 0);
}

TEST_CASE(DestroyEntityInvalidatesHandle) {
    Registry reg;
    Entity e = reg.CreateEntity();
    reg.DestroyEntity(e);
    ASSERT_FALSE(reg.Get<Transform>(e));
    // Reutilizar índice con nueva generation
    Entity e2 = reg.CreateEntity();
    ASSERT_NE(EntityGeneration(e), EntityGeneration(e2));
    ASSERT_EQ(EntityIndex(e), EntityIndex(e2));
}

TEST_CASE(AddAndGetComponent) {
    Registry reg;
    Entity e = reg.CreateEntity();
    auto& t = reg.Add<Transform>(e);
    t.x = 10; t.y = 20;
    auto* got = reg.Get<Transform>(e);
    ASSERT_TRUE(got);
    ASSERT_NEAR(got->x, 10.0f, 0.001f);
    ASSERT_NEAR(got->y, 20.0f, 0.001f);
}

TEST_CASE(RemoveComponent) {
    Registry reg;
    Entity e = reg.CreateEntity();
    reg.Add<Transform>(e) = {5,5};
    reg.Remove<Transform>(e);
    ASSERT_FALSE(reg.Get<Transform>(e));
    // Storage compactado
    ASSERT_EQ(reg.transforms.Size(), 0);
}

TEST_CASE(MultipleEntitiesComponentIsolation) {
    Registry reg;
    Entity e1 = reg.CreateEntity();
    Entity e2 = reg.CreateEntity();
    reg.Add<Transform>(e1) = {1,2};
    reg.Add<Transform>(e2) = {3,4};
    auto* t1 = reg.Get<Transform>(e1);
    auto* t2 = reg.Get<Transform>(e2);
    // Guardas: con los stubs sin resolver Get() devuelve nullptr. Se falla
    // limpio en la aserción, sin desreferenciar (no crash).
    ASSERT_TRUE(t1 != nullptr);
    ASSERT_TRUE(t2 != nullptr);
    ASSERT_NEAR(t1->x, 1.0f, 0.001f);
    ASSERT_NEAR(t2->x, 3.0f, 0.001f);
}

TEST_CASE(GenerationMismatchReturnsNull) {
    Registry reg;
    Entity e = reg.CreateEntity();
    reg.Add<Transform>(e);
    reg.DestroyEntity(e);
    ASSERT_FALSE(reg.Get<Transform>(e));
    Entity e2 = reg.CreateEntity();  // Mismo índice, distinta generation
    ASSERT_FALSE(reg.Get<Transform>(e2));  // No tiene componente
}

TEST_CASE(ComponentPoolSwapRemoveMaintainsDense) {
    Registry reg;
    Entity e1 = reg.CreateEntity();
    Entity e2 = reg.CreateEntity();
    Entity e3 = reg.CreateEntity();
    reg.Add<Velocity>(e1) = {1,0};
    reg.Add<Velocity>(e2) = {2,0};
    reg.Add<Velocity>(e3) = {3,0};
    reg.Remove<Velocity>(e2);  // Remove middle
    ASSERT_EQ(reg.velocities.Size(), 2);
    // e1 y e3 deben seguir accesibles
    ASSERT_TRUE(reg.Get<Velocity>(e1));
    ASSERT_TRUE(reg.Get<Velocity>(e3));
    ASSERT_FALSE(reg.Get<Velocity>(e2));
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 01 (ECS Básico)...\n";
    return RUN_ALL_TESTS();
}