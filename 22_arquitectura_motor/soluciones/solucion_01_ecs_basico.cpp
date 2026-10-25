/* =============================================================================
   MÓDULO 22 - SOLUCIÓN 01: ECS Básico — Entidades, Componentes, Pool
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
// COMPONENT POOL
// ============================================================================
template <typename Component>
struct ComponentPool {
    static_assert(std::is_trivially_copyable_v<Component>, "Component debe ser POD");

    std::vector<Component> dense;
    std::vector<uint32_t> sparse;           // entity_index -> dense_index + 1
    std::vector<uint32_t> generations;      // generation por entry en dense
    std::vector<Entity> dense_to_entity;    // para swap-with-last

    Component& Add(Entity entity) {
        uint32_t idx = EntityIndex(entity);
        uint32_t gen = EntityGeneration(entity);

        if (idx >= sparse.size()) sparse.resize(idx + 1, 0);
        assert(sparse[idx] == 0 && "Entidad ya tiene este componente");

        size_t dense_idx = dense.size();
        dense.push_back(Component{});
        generations.push_back(gen);
        dense_to_entity.push_back(entity);
        sparse[idx] = dense_idx + 1;

        return dense.back();
    }

    void Remove(Entity entity) {
        uint32_t idx = EntityIndex(entity);
        if (idx >= sparse.size()) return;
        size_t dense_idx = sparse[idx];
        if (dense_idx == 0) return;
        dense_idx--;

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
        return const_cast<ComponentPool*>(this)->Get(entity);
    }
    bool Has(Entity entity) const { return Get(entity) != nullptr; }
    size_t Size() const { return dense.size(); }
    void Clear() { dense.clear(); generations.clear(); dense_to_entity.clear(); sparse.assign(sparse.size(), 0); }
};

// ============================================================================
// COMPONENTES
// ============================================================================
struct Transform { float x=0, y=0, rot=0, sx=1, sy=1; };
struct Velocity { float vx=0, vy=0; };
struct Tag { char name[32]={}; };

// ============================================================================
// REGISTRY
// ============================================================================
struct Registry {
    ComponentPool<Transform> transforms;
    ComponentPool<Velocity> velocities;
    ComponentPool<Tag> tags;

    std::vector<uint32_t> generations{1};
    std::vector<uint32_t> free_indices;

    Entity CreateEntity() {
        uint32_t index;
        if (!free_indices.empty()) {
            index = free_indices.back();
            free_indices.pop_back();
            generations[index]++;
        } else {
            index = generations.size();
            generations.push_back(0);
        }
        return MakeEntity(index, generations[index]);
    }

    void DestroyEntity(Entity e) {
        uint32_t idx = EntityIndex(e);
        transforms.Remove(e);
        velocities.Remove(e);
        tags.Remove(e);
        free_indices.push_back(idx);
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
// TESTS (mismos que ejercicio)
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
    ASSERT_EQ(reg.transforms.Size(), 0);
}

TEST_CASE(MultipleEntitiesComponentIsolation) {
    Registry reg;
    Entity e1 = reg.CreateEntity();
    Entity e2 = reg.CreateEntity();
    reg.Add<Transform>(e1) = {1,2};
    reg.Add<Transform>(e2) = {3,4};
    ASSERT_NEAR(reg.Get<Transform>(e1)->x, 1.0f, 0.001f);
    ASSERT_NEAR(reg.Get<Transform>(e2)->x, 3.0f, 0.001f);
}

TEST_CASE(GenerationMismatchReturnsNull) {
    Registry reg;
    Entity e = reg.CreateEntity();
    reg.Add<Transform>(e);
    reg.DestroyEntity(e);
    ASSERT_FALSE(reg.Get<Transform>(e));
    Entity e2 = reg.CreateEntity();
    ASSERT_FALSE(reg.Get<Transform>(e2));
}

TEST_CASE(ComponentPoolSwapRemoveMaintainsDense) {
    Registry reg;
    Entity e1 = reg.CreateEntity();
    Entity e2 = reg.CreateEntity();
    Entity e3 = reg.CreateEntity();
    reg.Add<Velocity>(e1) = {1,0};
    reg.Add<Velocity>(e2) = {2,0};
    reg.Add<Velocity>(e3) = {3,0};
    reg.Remove<Velocity>(e2);
    ASSERT_EQ(reg.velocities.Size(), 2);
    ASSERT_TRUE(reg.Get<Velocity>(e1));
    ASSERT_TRUE(reg.Get<Velocity>(e3));
    ASSERT_FALSE(reg.Get<Velocity>(e2));
}

int main() {
    std::cout << "Solución 01 (ECS Básico)...\n";
    return RUN_ALL_TESTS();
}