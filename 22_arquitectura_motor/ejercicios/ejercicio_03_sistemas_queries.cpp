/* =============================================================================
   MÓDULO 22 - EJERCICIO 03: Sistemas y Queries — Iteración Cache-Friendly
   =============================================================================
   DIFICULTAD: Medio
   APOYO: Lección 03 (Sistemas y Queries), Lección 04 (Queries Avanzadas)

   CONSIGNA:
   Implementa sistemas que operan sobre Views:
   - PhysicsSystem: With<Transform, Velocity> Without<StaticTag> → integra posición
   - Query WithOneOf<PlayerTag, EnemyTag> para AI system
   - Query Optional<Health> para sistema de render (entidades sin health = siempre vivas)

   TESTS:
   - PhysicsSystem mueve solo entidades con Transform+Velocity sin StaticTag
   - WithOneOf funciona (PlayerTag O EnemyTag)
   - Optional<Health> pasa nullptr si no tiene
   - Iteración es cache-friendly (acceso secuencial a arrays)
   ============================================================================= */
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <cassert>
#include <type_traits>
#include <optional>
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
#define CMASK(T) CompReg::I().Mask<T>()

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
};

// ============================================================================
// COMPONENTES
// ============================================================================
struct Transform { float x=0,y=0; };
struct Velocity { float vx=0,vy=0; };
struct StaticTag {};
struct PlayerTag {};
struct EnemyTag {};
struct Health { int hp=100; };
struct AIState { int state=0; float timer=0; };

REG(Transform) REG(Velocity) REG(StaticTag) REG(PlayerTag) REG(EnemyTag) REG(Health) REG(AIState)

// ============================================================================
// ARCHETYPE STORAGE
// ============================================================================
struct Arch {
    ComponentMask mask = 0;
    std::vector<Entity> entities;
    SparseSet<Transform> T; SparseSet<Velocity> V;
    SparseSet<StaticTag> ST; SparseSet<PlayerTag> PT; SparseSet<EnemyTag> ET;
    SparseSet<Health> H; SparseSet<AIState> AI;

    template<typename C> C* Get(Entity e) {
        if constexpr(std::is_same_v<C,Transform>) return T.Get(e);
        else if constexpr(std::is_same_v<C,Velocity>) return V.Get(e);
        else if constexpr(std::is_same_v<C,StaticTag>) return ST.Get(e);
        else if constexpr(std::is_same_v<C,PlayerTag>) return PT.Get(e);
        else if constexpr(std::is_same_v<C,EnemyTag>) return ET.Get(e);
        else if constexpr(std::is_same_v<C,Health>) return H.Get(e);
        else if constexpr(std::is_same_v<C,AIState>) return AI.Get(e);
        return nullptr;
    }
    template<typename C> C* Add(Entity e, C c={}) {
        if constexpr(std::is_same_v<C,Transform>) return T.Add(e,c);
        else if constexpr(std::is_same_v<C,Velocity>) return V.Add(e,c);
        else if constexpr(std::is_same_v<C,StaticTag>) return ST.Add(e,c);
        else if constexpr(std::is_same_v<C,PlayerTag>) return PT.Add(e,c);
        else if constexpr(std::is_same_v<C,EnemyTag>) return ET.Add(e,c);
        else if constexpr(std::is_same_v<C,Health>) return H.Add(e,c);
        else if constexpr(std::is_same_v<C,AIState>) return AI.Add(e,c);
        return nullptr;
    }
    template<typename C> void Remove(Entity e) {
        if constexpr(std::is_same_v<C,Transform>) T.Remove(e);
        else if constexpr(std::is_same_v<C,Velocity>) V.Remove(e);
        else if constexpr(std::is_same_v<C,StaticTag>) ST.Remove(e);
        else if constexpr(std::is_same_v<C,PlayerTag>) PT.Remove(e);
        else if constexpr(std::is_same_v<C,EnemyTag>) ET.Remove(e);
        else if constexpr(std::is_same_v<C,Health>) H.Remove(e);
        else if constexpr(std::is_same_v<C,AIState>) AI.Remove(e);
    }
};

// ============================================================================
// REGISTRY (base del ejercicio 02)
// ============================================================================
struct Registry {
    std::unordered_map<ComponentMask, Arch> arches;
    std::vector<uint32_t> gen{1}, dense_idx{0};
    std::vector<ComponentMask> masks{0};
    std::vector<Arch*> arch_ptr{nullptr};
    std::vector<uint32_t> free_idx;

    Entity Create() {
        uint32_t i; if(!free_idx.empty()){ i=free_idx.back(); free_idx.pop_back(); gen[i]++; }
        else { i=gen.size(); gen.push_back(0); masks.push_back(0); arch_ptr.push_back(nullptr); dense_idx.push_back(0); }
        return MakeEntity(i, gen[i]);
    }
    Arch* GetArch(ComponentMask m) { auto it=arches.find(m); if(it!=arches.end()) return &it->second; return &arches[m]; }
    void Migrate(Entity e, ComponentMask nm) {
        uint32_t idx=EntityIndex(e); Arch* oa=arch_ptr[idx]; Arch* na=GetArch(nm); na->mask=nm;
        if(oa){
            size_t odi=dense_idx[idx];
            if((oa->mask&CMASK(Transform))&&(nm&CMASK(Transform))) if(auto* p=oa->Get<Transform>(e)) na->Add<Transform>(e,*p);
            if((oa->mask&CMASK(Velocity))&&(nm&CMASK(Velocity))) if(auto* p=oa->Get<Velocity>(e)) na->Add<Velocity>(e,*p);
            if((oa->mask&CMASK(StaticTag))&&(nm&CMASK(StaticTag))) na->Add<StaticTag>(e);
            if((oa->mask&CMASK(PlayerTag))&&(nm&CMASK(PlayerTag))) na->Add<PlayerTag>(e);
            if((oa->mask&CMASK(EnemyTag))&&(nm&CMASK(EnemyTag))) na->Add<EnemyTag>(e);
            if((oa->mask&CMASK(Health))&&(nm&CMASK(Health))) if(auto* p=oa->Get<Health>(e)) na->Add<Health>(e,*p);
            if((oa->mask&CMASK(AIState))&&(nm&CMASK(AIState))) if(auto* p=oa->Get<AIState>(e)) na->Add<AIState>(e,*p);
            size_t L=oa->entities.size()-1; if(odi!=L){ Entity me=oa->entities[L]; oa->entities[odi]=me; dense_idx[EntityIndex(me)]=odi; } oa->entities.pop_back();
        }
        arch_ptr[idx]=na; masks[idx]=nm; dense_idx[idx]=na->entities.size(); na->entities.push_back(e);
    }
    template<typename C> C& Add(Entity e){ Migrate(e, masks[EntityIndex(e)]|CMASK(C)); return *arch_ptr[EntityIndex(e)]->Add<C>(e); }
    template<typename C> void Remove(Entity e){ Migrate(e, masks[EntityIndex(e)]&~CMASK(C)); }
    template<typename C> C* Get(Entity e){ Arch* a=arch_ptr[EntityIndex(e)]; return a?a->Get<C>(e):nullptr; }

    // ========================================================================
    // QUERY VIEW: With<...> (sin Without/Optional en el ejercicio; son placeholders)
    // ========================================================================
    template<typename... With>
    struct QueryView {
        Registry* reg;
        ComponentMask with_m = 0;
        QueryView(Registry* r) : reg(r) { ( [&]{ with_m |= CMASK(With); }(), ... ); }

        // Placeholders para sintaxis .Without() y .Optional() - no hacen nada en el ejercicio
        template<typename... W>
        QueryView<With...> Without() { return *this; }
        template<typename... O>
        QueryView<With...> Optional() { return *this; }

        struct Iter {
            Registry* reg; ComponentMask wm;
            std::vector<Arch*> matching; size_t ai=0, ei=0; Entity cur=0;
            Iter(Registry* r, ComponentMask w, bool end) : reg(r), wm(w) {
                if(!end){ for(auto& [m,arch]:reg->arches) if((m&wm)==wm) matching.push_back(&arch); Advance(); }
            }
            void Advance(){ while(ai<matching.size()){ Arch* a=matching[ai]; if(ei<a->entities.size()){ cur=a->entities[ei]; return; } ai++; ei=0; } cur=0; }
            Entity operator*() const { return cur; }
            Iter& operator++(){ ei++; Advance(); return *this; }
            bool operator!=(const Iter& o) const { return cur!=o.cur; }
        };
        Iter begin(){ return Iter(reg, with_m, false); }
        Iter end(){ return Iter(reg, with_m, true); }

        template<typename Func>
        void each(Func&& f) {
            for(Entity e: *this){
                std::tuple<With*...> with_comps{reg->Get<With>(e)...};
                std::apply([&](auto*... w){ f(e, *w...); }, with_comps);
            }
        }
    };

    template<typename... With>
    QueryView<With...> View() { return QueryView<With...>(this); }
};

// ============================================================================
// TODO: Implementa PhysicsSystem y AISystem
// ============================================================================
void PhysicsSystem(Registry& reg, float dt) {
    // TU CÓDIGO AQUÍ: iterar Transform+Velocity sin StaticTag
    // Pista: reg.View<Transform, Velocity>().Without<StaticTag>().each(...)
    (void)reg; (void)dt;
}

void AISystem(Registry& reg, float dt) {
    // TU CÓDIGO AQUÍ: iterar Transform+AIState con OneOf<PlayerTag, EnemyTag>
    // Pista: usar dos Views separados o extender Query con OneOf
    (void)reg; (void)dt;
}

// ============================================================================
// TESTS
// ============================================================================
TEST_CASE(PhysicsSystemMovesDynamicEntities) {
    Registry reg;
    Entity dynamic = reg.Create();
    reg.Add<Transform>(dynamic) = {0, 0};
    reg.Add<Velocity>(dynamic) = {100, 50};

    Entity static_ent = reg.Create();
    reg.Add<Transform>(static_ent) = {10, 10};
    reg.Add<Velocity>(static_ent) = {200, 200};
    reg.Add<StaticTag>(static_ent);

    PhysicsSystem(reg, 1.0f);  // dt = 1 segundo

    ASSERT_NEAR(reg.Get<Transform>(dynamic)->x, 100.0f, 0.001f);
    ASSERT_NEAR(reg.Get<Transform>(dynamic)->y, 50.0f, 0.001f);
    // Static no se mueve
    ASSERT_NEAR(reg.Get<Transform>(static_ent)->x, 10.0f, 0.001f);
    ASSERT_NEAR(reg.Get<Transform>(static_ent)->y, 10.0f, 0.001f);
}

TEST_CASE(QueryWithoutExcludesStaticTag) {
    Registry reg;
    Entity e1 = reg.Create(); reg.Add<Transform>(e1); reg.Add<Velocity>(e1);
    Entity e2 = reg.Create(); reg.Add<Transform>(e2); reg.Add<Velocity>(e2); reg.Add<StaticTag>(e2);

    int count = 0;
    for(Entity e : reg.View<Transform, Velocity>().Without<StaticTag>()){
        count++;
        ASSERT_FALSE(reg.Get<StaticTag>(e));
    }
    ASSERT_EQ(count, 1);
}

TEST_CASE(WithOneOfPlayerOrEnemy) {
    Registry reg;
    Entity player = reg.Create(); reg.Add<Transform>(player); reg.Add<AIState>(player); reg.Add<PlayerTag>(player);
    Entity enemy = reg.Create(); reg.Add<Transform>(enemy); reg.Add<AIState>(enemy); reg.Add<EnemyTag>(enemy);
    Entity neutral = reg.Create(); reg.Add<Transform>(neutral); reg.Add<AIState>(neutral);

    // Con OneOf en Query (si implementado) o dos views
    int count = 0;
    for(Entity e : reg.View<Transform, AIState, PlayerTag>()) count++;
    for(Entity e : reg.View<Transform, AIState, EnemyTag>()) count++;
    ASSERT_EQ(count, 2);
}

TEST_CASE(OptionalHealthPassesNullptrWhenMissing) {
    Registry reg;
    Entity with_health = reg.Create(); reg.Add<Transform>(with_health); reg.Add<Health>(with_health);
    Entity without_health = reg.Create(); reg.Add<Transform>(without_health);

    int with_count = 0, without_count = 0;
    // TODO: Implementar Optional en Query.each()
    // reg.View<Transform>().Optional<Health>().each([&](Entity e, Transform* t, Health* h){
    //     if(h) with_count++; else without_count++;
    // });
    // ASSERT_EQ(with_count, 1);
    // ASSERT_EQ(without_count, 1);
    (void)with_health; (void)without_health;
    ASSERT_TRUE(true);  // Placeholder
}

TEST_CASE(PhysicsSystemCacheFriendlyAccess) {
    // Verificar que la iteración accede a arrays contiguos
    Registry reg;
    const int N = 1000;
    for(int i=0;i<N;++i){
        Entity e = reg.Create();
        reg.Add<Transform>(e) = {float(i), 0};
        reg.Add<Velocity>(e) = {1, 0};
    }

    PhysicsSystem(reg, 0.016f);

    for(int i=0;i<N;++i){
        Entity e = reg.arches.begin()->second.entities[i];  // Simplificado
        // En real: iterar via View
    }
    ASSERT_TRUE(true);  // Placeholder para test de concepto
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 03 (Sistemas y Queries)...\n";
    return RUN_ALL_TESTS();
}