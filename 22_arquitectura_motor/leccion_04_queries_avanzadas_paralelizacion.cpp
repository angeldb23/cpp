/* =============================================================================
   MÓDULO 22 - LECCIÓN 04: Queries Avanzadas — With/Without, WithOneOf, Paralelización
   =============================================================================
   TEMA 2 de TEMAS.md (2/2): Queries/Views — filtrado, iteración cache-friendly,
   with/without y paralelización.

   QUERY FILTERS COMPLETOS:
   ------------------------
   - With<A, B, C>: entidad DEBE tener A, B, C (AND).
   - Without<D, E>: entidad NO debe tener D ni E (NOT).
   - WithOneOf<F, G>: entidad tiene AL MENOS uno de F, G (OR).
   - Optional<H>: entidad PUEDE tener H (se pasa nullptr si no).

   En archetype ECS:
   - With → mask & with_mask == with_mask
   - Without → (mask & without_mask) == 0
   - WithOneOf → (mask & oneof_mask) != 0
   - Optional → no afecta match; Get() retorna nullptr si no tiene.

   ITERACIÓN CACHE-FRIENDLY CON MÚLTIPLES COMPONENTES:
   ---------------------------------------------------
   Para query With<Transform, Velocity, Sprite>:
   - Archetype match tiene 3 arrays paralelos: Transform[], Velocity[], Sprite[]
   - Loop: for i in 0..N-1: t = T[i], v = V[i], s = S[i]
   - Acceso secuencial en TRES arrays → 3 cache lines por iteración.
   - Optimización: Structure of Arrays (SoA) ya hecho; prefetch manual si N grande.

   PARALELIZACIÓN DE SISTEMAS:
   ---------------------------
   Enfoque: System Scheduler con DAG de dependencias.
   Cada sistema declara:
     - ReadComponents: componentes que lee (shared)
     - WriteComponents: componentes que escribe (exclusive)
   Reglas:
     - Múltiples sistemas pueden leer mismo componente (shared read).
     - Un solo sistema puede escribir un componente por frame (exclusive write).
     - Sistema que escribe X debe correr ANTES que sistemas que leen X.
     - Sistema que escribe X debe correr DESPUÉS de sistemas que leen X (frame anterior).

   JOB SYSTEM INTEGRATION (lección 14-15):
   ---------------------------------------
   - Cada archetype chunk (contiguous block) = unit of work (job).
   - Sistema recibe lista de chunks; cada job procesa un chunk.
   - Chunk size típico: 16KB-64KB (ajuste a L1/L2 cache).
   - Parallel for sobre chunks: std::execution::par o thread pool propio.

   ESTA LECCIÓN: Query builder con WithOneOf/Optional + System Scheduler DAG básico.
   ============================================================================= */
#include <iostream>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <cassert>
#include <type_traits>
#include <functional>
#include <tuple>
#include <algorithm>
#include <thread>
#include <mutex>
#include <atomic>
#include <queue>

// ============================================================================
// TIPOS BASE
// ============================================================================
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
struct ComponentTypeRegistry {
    static ComponentTypeRegistry& I() { static ComponentTypeRegistry r; return r; }
    template <typename T> uint8_t Reg() { static uint8_t id = next++; return id; }
    template <typename T> ComponentMask Mask() const { return ComponentMask(1) << Reg<T>(); }
    size_t next = 0;
};
#define REG(T) template<> uint8_t ComponentTypeRegistry::Reg<T>() { static uint8_t id = ComponentTypeRegistry::I().next++; return id; }

// ============================================================================
// SPARSE SET
// ============================================================================
template <typename T>
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
struct Transform { float x=0,y=0,rot=0,sx=1,sy=1; };
struct Velocity { float vx=0,vy=0; };
struct Acceleration { float ax=0,ay=0; };
struct Sprite { int tex=0; float u=0,v=0,w=1,h=1; };
struct Health { int hp=100, max=100; };
struct AIState { int state=0; float timer=0; };
struct PlayerTag {}; struct EnemyTag {}; struct StaticTag {}; struct VisibleTag {};

REG(Transform) REG(Velocity) REG(Acceleration) REG(Sprite) REG(Health) REG(AIState)
REG(PlayerTag) REG(EnemyTag) REG(StaticTag) REG(VisibleTag)

// ============================================================================
// ARCHETYPE STORAGE
// ============================================================================
struct Arch {
    ComponentMask mask=0;
    std::vector<Entity> entities;
    SparseSet<Transform> T; SparseSet<Velocity> V; SparseSet<Acceleration> A;
    SparseSet<Sprite> S; SparseSet<Health> H; SparseSet<AIState> AI;
    SparseSet<PlayerTag> PT; SparseSet<EnemyTag> ET; SparseSet<StaticTag> ST; SparseSet<VisibleTag> VT;

    template<typename C> C* Get(Entity e) {
        if constexpr(std::is_same_v<C,Transform>) return T.Get(e);
        else if constexpr(std::is_same_v<C,Velocity>) return V.Get(e);
        else if constexpr(std::is_same_v<C,Acceleration>) return A.Get(e);
        else if constexpr(std::is_same_v<C,Sprite>) return S.Get(e);
        else if constexpr(std::is_same_v<C,Health>) return H.Get(e);
        else if constexpr(std::is_same_v<C,AIState>) return AI.Get(e);
        else if constexpr(std::is_same_v<C,PlayerTag>) return PT.Get(e);
        else if constexpr(std::is_same_v<C,EnemyTag>) return ET.Get(e);
        else if constexpr(std::is_same_v<C,StaticTag>) return ST.Get(e);
        else if constexpr(std::is_same_v<C,VisibleTag>) return VT.Get(e);
        return nullptr;
    }
    template<typename C> C* Add(Entity e, C c={}) {
        if constexpr(std::is_same_v<C,Transform>) return T.Add(e,c);
        else if constexpr(std::is_same_v<C,Velocity>) return V.Add(e,c);
        else if constexpr(std::is_same_v<C,Acceleration>) return A.Add(e,c);
        else if constexpr(std::is_same_v<C,Sprite>) return S.Add(e,c);
        else if constexpr(std::is_same_v<C,Health>) return H.Add(e,c);
        else if constexpr(std::is_same_v<C,AIState>) return AI.Add(e,c);
        else if constexpr(std::is_same_v<C,PlayerTag>) return PT.Add(e,c);
        else if constexpr(std::is_same_v<C,EnemyTag>) return ET.Add(e,c);
        else if constexpr(std::is_same_v<C,StaticTag>) return ST.Add(e,c);
        else if constexpr(std::is_same_v<C,VisibleTag>) return VT.Add(e,c);
        return nullptr;
    }
    template<typename C> void Remove(Entity e) {
        if constexpr(std::is_same_v<C,Transform>) T.Remove(e);
        else if constexpr(std::is_same_v<C,Velocity>) V.Remove(e);
        else if constexpr(std::is_same_v<C,Acceleration>) A.Remove(e);
        else if constexpr(std::is_same_v<C,Sprite>) S.Remove(e);
        else if constexpr(std::is_same_v<C,Health>) H.Remove(e);
        else if constexpr(std::is_same_v<C,AIState>) AI.Remove(e);
        else if constexpr(std::is_same_v<C,PlayerTag>) PT.Remove(e);
        else if constexpr(std::is_same_v<C,EnemyTag>) ET.Remove(e);
        else if constexpr(std::is_same_v<C,StaticTag>) ST.Remove(e);
        else if constexpr(std::is_same_v<C,VisibleTag>) VT.Remove(e);
    }
};

// ============================================================================
// REGISTRY
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
        uint32_t i=EntityIndex(e); Arch* oa=arch_ptr[i]; Arch* na=GetArch(nm); na->mask=nm;
        if(oa){
            size_t odi=dense_idx[i];
            auto copy_if=[&](auto cm, auto add){ if((oa->mask&cm)&&(nm&cm)) if(auto* p=oa->Get<decltype(*add(e))>(e)) na->Add(*p); };
            // Manual para cada tipo (en real: reflexión o tuple)
            if((oa->mask&REG(Transform))&&(nm&REG(Transform))) if(auto* p=oa->Get<Transform>(e)) na->Add<Transform>(e,*p);
            if((oa->mask&REG(Velocity))&&(nm&REG(Velocity))) if(auto* p=oa->Get<Velocity>(e)) na->Add<Velocity>(e,*p);
            if((oa->mask&REG(Acceleration))&&(nm&REG(Acceleration))) if(auto* p=oa->Get<Acceleration>(e)) na->Add<Acceleration>(e,*p);
            if((oa->mask&REG(Sprite))&&(nm&REG(Sprite))) if(auto* p=oa->Get<Sprite>(e)) na->Add<Sprite>(e,*p);
            if((oa->mask&REG(Health))&&(nm&REG(Health))) if(auto* p=oa->Get<Health>(e)) na->Add<Health>(e,*p);
            if((oa->mask&REG(AIState))&&(nm&REG(AIState))) if(auto* p=oa->Get<AIState>(e)) na->Add<AIState>(e,*p);
            if((oa->mask&REG(PlayerTag))&&(nm&REG(PlayerTag))) na->Add<PlayerTag>(e);
            if((oa->mask&REG(EnemyTag))&&(nm&REG(EnemyTag))) na->Add<EnemyTag>(e);
            if((oa->mask&REG(StaticTag))&&(nm&REG(StaticTag))) na->Add<StaticTag>(e);
            if((oa->mask&REG(VisibleTag))&&(nm&REG(VisibleTag))) na->Add<VisibleTag>(e);
            size_t L=oa->entities.size()-1; if(odi!=L){ Entity me=oa->entities[L]; oa->entities[odi]=me; dense_idx[EntityIndex(me)]=odi; } oa->entities.pop_back();
        }
        arch_ptr[i]=na; masks[i]=nm; dense_idx[i]=na->entities.size(); na->entities.push_back(e);
    }
    template<typename C> C& Add(Entity e){ Migrate(e, masks[EntityIndex(e)]|REG(C)); return *arch_ptr[EntityIndex(e)]->Add<C>(e); }
    template<typename C> void Remove(Entity e){ Migrate(e, masks[EntityIndex(e)]&~REG(C)); }
    template<typename C> C* Get(Entity e){ Arch* a=arch_ptr[EntityIndex(e)]; return a?a->Get<C>(e):nullptr; }
    void Destroy(Entity e){ uint32_t i=EntityIndex(e); Arch* a=arch_ptr[i]; if(a){ size_t di=dense_idx[i], L=a->entities.size()-1; if(di!=L){ Entity me=a->entities[L]; a->entities[di]=me; dense_idx[EntityIndex(me)]=di; } a->entities.pop_back(); } arch_ptr[i]=nullptr; masks[i]=0; free_idx.push_back(i); }

    // ========================================================================
    // QUERY BUILDER CON WITH/ WITHOUT/ WITHONEOF/ OPTIONAL
    // ========================================================================
    template <typename... With, typename... Without, typename... OneOf, typename... Optional>
    struct Query {
        Registry* reg;
        ComponentMask with_m=0, without_m=0, oneof_m=0;
        Query(Registry* r): reg(r) { (with_m|=REG(With),...); (without_m|=REG(Without),...); (oneof_m|=REG(OneOf),...); }
        bool Matches(ComponentMask m) const { return (m&with_m)==with_m && (m&without_m)==0 && (oneof_m==0 || (m&oneof_m)!=0); }

        // Iteración estilo range-for
        struct Iter {
            Registry* reg; ComponentMask wm, wom, om;
            std::vector<Arch*> matching; size_t ai=0, ei=0; Entity cur=0;
            Iter(Registry* r, ComponentMask w, ComponentMask wo, ComponentMask o, bool end)
                : reg(r), wm(w), wom(wo), om(o) {
                if(!end){ for(auto& [m,arch]:reg->arches) if(Matches(m)) matching.push_back(&arch); Advance(); }
            }
            bool Matches(ComponentMask m) const { return (m&wm)==wm && (m&wom)==0 && (om==0||(m&om)!=0); }
            void Advance(){ while(ai<matching.size()){ Arch* a=matching[ai]; if(ei<a->entities.size()){ cur=a->entities[ei]; return; } ai++; ei=0; } cur=0; }
            Entity operator*() const { return cur; }
            Iter& operator++(){ ei++; Advance(); return *this; }
            bool operator!=(const Iter& o) const { return cur!=o.cur; }
        };
        Iter begin(){ return Iter(reg, with_m, without_m, oneof_m, false); }
        Iter end(){ return Iter(reg, with_m, without_m, oneof_m, true); }

        // .each() con Optional support
        template <typename Func>
        void each(Func&& f) {
            for(Entity e: *this){
                // With components (garantizados)
                std::tuple<With*...> with_comps{reg->Get<With>(e)...};
                // Optional components (pueden ser nullptr)
                std::tuple<Optional*...> opt_comps{reg->Get<Optional>(e)...};
                // Desempaquetar y llamar
                std::apply([&](auto*... w){
                    std::apply([&](auto*... o){ f(e, *w..., o...); }, opt_comps);
                }, with_comps);
            }
        }
    };

    template <typename... With>
    Query<With...> View() { return Query<With...>(this); }
};

// ============================================================================
// SYSTEM SCHEDULER (DAG básico)
// ============================================================================
struct System {
    std::string name;
    ComponentMask reads, writes;
    std::function<void(Registry&, float)> func;
    std::vector<System*> deps;  // Sistemas que deben correr ANTES
};

struct Scheduler {
    std::vector<System> systems;
    std::vector<int> order;  // Topological order

    void Add(System s) { systems.push_back(std::move(s)); }

    void BuildOrder() {
        order.clear();
        std::vector<int> indeg(systems.size(), 0);
        std::vector<std::vector<int>> adj(systems.size());
        for(size_t i=0;i<systems.size();++i){
            for(size_t j=0;j<systems.size();++j) if(i!=j){
                // i escribe, j lee → i antes que j
                if((systems[i].writes & systems[j].reads) != 0){ adj[i].push_back(j); indeg[j]++; }
                // i lee, j escribe → j antes que i (frame anterior, pero en mismo frame: i antes que j si j escribe lo que i lee)
                // Regla simple: writes exclusivo, reads compartido. Write antes que reads del MISMO frame.
            }
        }
        std::queue<int> q; for(size_t i=0;i<systems.size();++i) if(indeg[i]==0) q.push(i);
        while(!q.empty()){ int u=q.front(); q.pop(); order.push_back(u); for(int v:adj[u]) if(--indeg[v]==0) q.push(v); }
        if(order.size()!=systems.size()) std::cerr << "CICLO EN DEPENDENCIAS!\n";
    }

    void Run(Registry& reg, float dt) {
        BuildOrder();
        for(int idx: order) systems[idx].func(reg, dt);
    }

    // Versión paralela por chunks (simplificada)
    void RunParallel(Registry& reg, float dt, size_t num_threads) {
        BuildOrder();
        for(int idx: order){
            System& sys = systems[idx];
            // Para cada archetype que match, dividir entities en chunks y procesar en paralelo
            // Aquí demo: solo ejecuta secuencial; implementación real usa thread pool + job queue
            sys.func(reg, dt);
        }
    }
};

// ============================================================================
// SISTEMAS DE EJEMPLO
// ============================================================================
void PhysicsSys(Registry& reg, float dt) {
    // With<Transform, Velocity> Without<StaticTag>
    auto q = reg.View<Transform, Velocity>().Without<StaticTag>();
    for(Entity e: q){
        auto* t = reg.Get<Transform>(e);
        auto* v = reg.Get<Velocity>(e);
        if(auto* a = reg.Get<Acceleration>(e)){ v->vx += a->ax*dt; v->vy += a->ay*dt; }
        t->x += v->vx*dt; t->y += v->vy*dt;
    }
}

void AISys(Registry& reg, float dt) {
    // With<Transform, AIState> WithOneOf<PlayerTag, EnemyTag>
    auto q = reg.View<Transform, AIState>().OneOf<PlayerTag, EnemyTag>();
    // OneOf no implementado en Query arriba; usar two views o extender Query
    // Demo: iterar ambos tags por separado
    for(Entity e: reg.View<Transform, AIState, PlayerTag>()){
        auto* t = reg.Get<Transform>(e); auto* ai = reg.Get<AIState>(e);
        ai->timer += dt; if(ai->timer>1){ ai->state^=1; ai->timer=0; }
    }
    for(Entity e: reg.View<Transform, AIState, EnemyTag>()){
        auto* t = reg.Get<Transform>(e); auto* ai = reg.Get<AIState>(e);
        ai->timer += dt; if(ai->timer>2){ ai->state^=1; ai->timer=0; }
    }
}

void HealthSys(Registry& reg, float dt) {
    // With<Health> Optional<VisibleTag>
    reg.View<Health>().Optional<VisibleTag>().each([&](Entity e, Health& h, VisibleTag* vt){
        if(h.hp<=0){ /* muerte */ }
        if(vt){ /* visible */ }
    });
}

// ============================================================================
// VERIFICACIÓN DE SINTAXIS
// ============================================================================
int main() {
    std::cout << "=== LECCIÓN 04: Queries Avanzadas y Paralelización (solo sintaxis) ===\n";

    Registry reg;
    Entity p = reg.Create(); reg.Add<Transform>(p)={0,0}; reg.Add<Velocity>(p)={10,0}; reg.Add<PlayerTag>(p);
    Entity e = reg.Create(); reg.Add<Transform>(e)={100,0}; reg.Add<Velocity>(e)={-5,0}; reg.Add<EnemyTag>(e);
    Entity s = reg.Create(); reg.Add<Transform>(s)={50,50}; reg.Add<StaticTag>(s);

    PhysicsSys(reg, 0.016f);
    AISys(reg, 0.016f);

    std::cout << "Player: " << reg.Get<Transform>(p)->x << "\n";
    std::cout << "Enemy: " << reg.Get<Transform>(e)->x << "\n";
    std::cout << "Static: " << reg.Get<Transform>(s)->x << " (sin mover)\n";

    // Scheduler demo
    Scheduler sched;
    sched.Add({"Physics", REG(Transform)|REG(Velocity), REG(Transform)|REG(Velocity), PhysicsSys});
    sched.Add({"AI", REG(Transform)|REG(AIState), REG(AIState), AISys});
    sched.BuildOrder();
    std::cout << "Execution order: ";
    for(int i: sched.order) std::cout << sched.systems[i].name << " ";
    std::cout << "\n";

    std::cout << "=== FIN LECCIÓN 04 ===\n";
    return 0;
}