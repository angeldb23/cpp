/* =============================================================================
   MÓDULO 22 - EJERCICIO 02: ECS Archetypes — Migración entre Archetypes
   =============================================================================
   DIFICULTAD: Medio
   APOYO: Lección 02 (Archetypes y Storage)

   CONSIGNA:
   Implementa Registry con Archetypes:
   - ComponentMask (uint64_t) por entidad
   - ArchetypeStorage por combinación única de componentes
   - Add/Remove componente migra entidad entre archetypes
   - Query: iterar entidades que match With<Components...>

   TESTS:
   - Add componente migra a archetype con más componentes
   - Remove componente migra a archetype con menos
   - Query With<A,B> solo retorna entidades con A y B
   - Entidades en archetype correcto tras múltiples migraciones
   ============================================================================= */
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <cassert>
#include <type_traits>
#include <bitset>
#include "../../include/test_framework.hpp"

using Entity = uint32_t;
constexpr uint32_t ENTITY_INDEX_BITS = 20;
constexpr uint32_t ENTITY_INDEX_MASK = (1u << ENTITY_INDEX_BITS) - 1u;
constexpr uint32_t ENTITY_GENERATION_SHIFT = ENTITY_INDEX_BITS;
inline Entity MakeEntity(uint32_t i, uint32_t g) { return (g << ENTITY_GENERATION_SHIFT) | i; }
inline uint32_t EntityIndex(Entity e) { return e & ENTITY_INDEX_MASK; }
inline uint32_t EntityGeneration(Entity e) { return e >> ENTITY_GENERATION_SHIFT; }
using ComponentMask = uint64_t;

// ============================================================================
// COMPONENT REGISTRY
// ============================================================================
struct CompReg {
    static CompReg& I() { static CompReg r; return r; }
    template<typename T> uint8_t Reg() { static uint8_t id = next++; return id; }
    template<typename T> ComponentMask Mask() { return ComponentMask(1) << Reg<T>(); }
    size_t next = 0;
};
#define REG(T) template<> uint8_t CompReg::Reg<T>() { static uint8_t id = CompReg::I().next++; return id; }

// ============================================================================
// SPARSE SET
// ============================================================================
template<typename T>
struct SparseSet {
    static_assert(std::is_trivially_copyable_v<T>, "POD");
    std::vector<T> dense;
    std::vector<uint32_t> sparse, gen;
    std::vector<Entity> e2d;
    T* Add(Entity e, T c={}) { uint32_t i=EntityIndex(e), g=EntityGeneration(e); if(i>=sparse.size()) sparse.resize(i+1,0); assert(sparse[i]==0); size_t di=dense.size(); dense.push_back(c); gen.push_back(g); e2d.push_back(e); sparse[i]=di+1; return &dense.back(); }
    void Remove(Entity e) { uint32_t i=EntityIndex(e); if(i>=sparse.size()) return; size_t di=sparse[i]; if(!di) return; di--; size_t L=dense.size()-1; if(di!=L){ dense[di]=std::move(dense[L]); gen[di]=gen[L]; Entity me=e2d[L]; e2d[di]=me; sparse[EntityIndex(me)]=di+1; } dense.pop_back(); gen.pop_back(); e2d.pop_back(); sparse[i]=0; }
    T* Get(Entity e) { uint32_t i=EntityIndex(e); if(i>=sparse.size()) return nullptr; size_t di=sparse[i]; if(!di) return nullptr; di--; if(gen[di]!=EntityGeneration(e)) return nullptr; return &dense[di]; }
    const T* Get(Entity e) const { return const_cast<SparseSet*>(this)->Get(e); }
    bool Has(Entity e) const { return Get(e) != nullptr; }
    size_t Size() const { return dense.size(); }
};

// ============================================================================
// COMPONENTES
// ============================================================================
struct Transform { float x=0,y=0; };
struct Velocity { float vx=0,vy=0; };
struct Sprite { int tex=0; };
struct Tag { char name[32]={}; };

REG(Transform) REG(Velocity) REG(Sprite) REG(Tag)

// ============================================================================
// ARCHETYPE STORAGE
// ============================================================================
struct Arch {
    ComponentMask mask = 0;
    std::vector<Entity> entities;
    SparseSet<Transform> T;
    SparseSet<Velocity> V;
    SparseSet<Sprite> S;
    SparseSet<Tag> Tg;

    template<typename C> C* Get(Entity e) {
        if constexpr(std::is_same_v<C,Transform>) return T.Get(e);
        else if constexpr(std::is_same_v<C,Velocity>) return V.Get(e);
        else if constexpr(std::is_same_v<C,Sprite>) return S.Get(e);
        else if constexpr(std::is_same_v<C,Tag>) return Tg.Get(e);
        return nullptr;
    }
    template<typename C> C* Add(Entity e, C c={}) {
        if constexpr(std::is_same_v<C,Transform>) return T.Add(e,c);
        else if constexpr(std::is_same_v<C,Velocity>) return V.Add(e,c);
        else if constexpr(std::is_same_v<C,Sprite>) return S.Add(e,c);
        else if constexpr(std::is_same_v<C,Tag>) return Tg.Add(e,c);
        return nullptr;
    }
    template<typename C> void Remove(Entity e) {
        if constexpr(std::is_same_v<C,Transform>) T.Remove(e);
        else if constexpr(std::is_same_v<C,Velocity>) V.Remove(e);
        else if constexpr(std::is_same_v<C,Sprite>) S.Remove(e);
        else if constexpr(std::is_same_v<C,Tag>) Tg.Remove(e);
    }
};

// ============================================================================
// TODO: Implementa Registry con Archetypes
// ============================================================================
struct Registry {
    std::unordered_map<ComponentMask, Arch> arches;
    std::vector<uint32_t> gen{1};
    std::vector<ComponentMask> masks{0};
    std::vector<Arch*> arch_ptr{nullptr};
    std::vector<uint32_t> dense_idx{0};
    std::vector<uint32_t> free_idx;

    Entity Create() {
        // TU CÓDIGO AQUÍ
        return 0;
    }

    Arch* GetOrCreateArch(ComponentMask mask) {
        // TU CÓDIGO AQUÍ
        (void)mask;
        return nullptr;
    }

    void Migrate(Entity e, ComponentMask new_mask) {
        // TU CÓDIGO AQUÍ
        (void)e; (void)new_mask;
    }

    template<typename C> C& Add(Entity e) {
        ComponentMask cm = CompReg::I().Mask<C>();
        Migrate(e, masks[EntityIndex(e)] | cm);
        // Guarda para los stubs sin resolver: Migrate no crea el archetype y
        // arch_ptr queda en nullptr. Devolvemos una instancia descartable para
        // que el test falle en su aserción en vez de desreferenciar nullptr.
        Arch* a = arch_ptr[EntityIndex(e)];
        if(!a) { static C descartable{}; return descartable; }
        C* c = a->Add<C>(e);
        if(!c) { static C descartable{}; return descartable; }
        return *c;
    }
    template<typename C> void Remove(Entity e) {
        ComponentMask cm = CompReg::I().Mask<C>();
        Migrate(e, masks[EntityIndex(e)] & ~cm);
    }
    template<typename C> C* Get(Entity e) {
        Arch* a = arch_ptr[EntityIndex(e)];
        return a ? a->Get<C>(e) : nullptr;
    }
    void Destroy(Entity e) {
        // TU CÓDIGO AQUÍ
        (void)e;
    }

    // Query: iterar archetypes que match mask
    template<typename... Cs>
    struct QueryView {
        Registry* reg;
        ComponentMask with_mask = 0;
        QueryView(Registry* r) : reg(r) {
            // Fold expression para construir mask
            ( [&]{ with_mask |= CompReg::I().Mask<Cs>(); }(), ... );
        }
        struct Iter {
            Registry* r; ComponentMask m;
            std::vector<Arch*> matching; size_t ai=0, ei=0; Entity cur=0;
            Iter(Registry* reg, ComponentMask mask, bool end) : r(reg), m(mask) {
                if(!end){ for(auto& [k,v]:r->arches) if((v.mask&m)==m) matching.push_back(&v); Advance(); }
            }
            void Advance(){ while(ai<matching.size()){ Arch* a=matching[ai]; if(ei<a->entities.size()){ cur=a->entities[ei]; return; } ai++; ei=0; } cur=0; }
            Entity operator*() const { return cur; }
            Iter& operator++(){ ei++; Advance(); return *this; }
            bool operator!=(const Iter& o) const { return cur!=o.cur; }
        };
        Iter begin(){ return Iter(reg, with_mask, false); }
        Iter end(){ return Iter(reg, with_mask, true); }
    };
    template<typename... Cs> QueryView<Cs...> View() { return QueryView<Cs...>(this); }
};

// ============================================================================
// TESTS
// ============================================================================
TEST_CASE(EntityStartsInEmptyArchetype) {
    Registry reg;
    Entity e = reg.Create();
    ASSERT_EQ(reg.masks[EntityIndex(e)], 0);
    ASSERT_TRUE(reg.arch_ptr[EntityIndex(e)] == nullptr || reg.arch_ptr[EntityIndex(e)]->mask == 0);
}

TEST_CASE(AddComponentMigratesToNewArchetype) {
    Registry reg;
    Entity e = reg.Create();
    reg.Add<Transform>(e) = {10, 20};
    ComponentMask expected = CompReg::I().Mask<Transform>();
    ASSERT_EQ(reg.masks[EntityIndex(e)], expected);
    ASSERT_NE(reg.arch_ptr[EntityIndex(e)], nullptr);
    ASSERT_EQ(reg.arch_ptr[EntityIndex(e)]->mask, expected);
    ASSERT_EQ(reg.arch_ptr[EntityIndex(e)]->entities.size(), 1);
}

TEST_CASE(AddSecondComponentMigratesAgain) {
    Registry reg;
    Entity e = reg.Create();
    reg.Add<Transform>(e);
    reg.Add<Velocity>(e) = {5, 3};
    ComponentMask expected = CompReg::I().Mask<Transform>() | CompReg::I().Mask<Velocity>();
    ASSERT_EQ(reg.masks[EntityIndex(e)], expected);
    // Debe haber 2 archetypes: {T} y {T,V}
    ASSERT_EQ(reg.arches.size(), 2);
}

TEST_CASE(RemoveComponentMigratesToSmallerArchetype) {
    Registry reg;
    Entity e = reg.Create();
    reg.Add<Transform>(e);
    reg.Add<Velocity>(e);
    reg.Remove<Velocity>(e);
    ComponentMask expected = CompReg::I().Mask<Transform>();
    ASSERT_EQ(reg.masks[EntityIndex(e)], expected);
    // Velocity removido
    ASSERT_FALSE(reg.Get<Velocity>(e));
    ASSERT_TRUE(reg.Get<Transform>(e));
}

TEST_CASE(QueryWithReturnsMatchingEntities) {
    Registry reg;
    Entity e1 = reg.Create(); reg.Add<Transform>(e1); reg.Add<Velocity>(e1);
    Entity e2 = reg.Create(); reg.Add<Transform>(e2);
    Entity e3 = reg.Create(); reg.Add<Velocity>(e3);
    Entity e4 = reg.Create(); reg.Add<Transform>(e4); reg.Add<Velocity>(e4); reg.Add<Sprite>(e4);

    int count = 0;
    for(Entity e : reg.View<Transform, Velocity>()){
        count++;
        ASSERT_TRUE(reg.Get<Transform>(e));
        ASSERT_TRUE(reg.Get<Velocity>(e));
    }
    ASSERT_EQ(count, 3);  // e1, e4, y... e2 no tiene Velocity, e3 no tiene Transform
}

TEST_CASE(QueryWithMultipleComponents) {
    Registry reg;
    Entity e1 = reg.Create(); reg.Add<Transform>(e1); reg.Add<Velocity>(e1); reg.Add<Sprite>(e1);
    Entity e2 = reg.Create(); reg.Add<Transform>(e2); reg.Add<Velocity>(e2);
    Entity e3 = reg.Create(); reg.Add<Sprite>(e3);

    int count = 0;
    for(Entity e : reg.View<Transform, Velocity, Sprite>()){
        count++;
        ASSERT_TRUE(reg.Get<Transform>(e));
        ASSERT_TRUE(reg.Get<Velocity>(e));
        ASSERT_TRUE(reg.Get<Sprite>(e));
    }
    ASSERT_EQ(count, 1);  // Solo e1
}

TEST_CASE(MigrationPreservesComponentData) {
    Registry reg;
    Entity e = reg.Create();
    reg.Add<Transform>(e) = {100, 200};
    reg.Add<Velocity>(e) = {10, 5};
    reg.Add<Sprite>(e) = {42};

    // Migrar quitando Velocity
    reg.Remove<Velocity>(e);
    auto* t = reg.Get<Transform>(e);
    auto* s = reg.Get<Sprite>(e);
    ASSERT_TRUE(t);
    ASSERT_NEAR(t->x, 100.0f, 0.001f);
    ASSERT_NEAR(t->y, 200.0f, 0.001f);
    ASSERT_TRUE(s);
    ASSERT_EQ(s->tex, 42);
    ASSERT_FALSE(reg.Get<Velocity>(e));
}

TEST_CASE(DestroyEntityRemovesFromArchetype) {
    Registry reg;
    Entity e = reg.Create();
    reg.Add<Transform>(e);
    reg.Add<Velocity>(e);
    Arch* arch = reg.arch_ptr[EntityIndex(e)];
    // Con los stubs sin resolver no hay archetype (nullptr): se falla limpio.
    ASSERT_TRUE(arch != nullptr);
    ASSERT_EQ(arch->entities.size(), 1);
    reg.Destroy(e);
    ASSERT_EQ(arch->entities.size(), 0);
    ASSERT_EQ(reg.arch_ptr[EntityIndex(e)], nullptr);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 02 (ECS Archetypes)...\n";
    return RUN_ALL_TESTS();
}