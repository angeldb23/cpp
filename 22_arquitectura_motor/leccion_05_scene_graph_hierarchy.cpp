/* =============================================================================
   MÓDULO 22 - LECCIÓN 05: Scene Graph — Transform Hierarchy, Dirty Propagation,
   World Matrix Cache
   =============================================================================
   TEMA 3 de TEMAS.md (1/2): Scene Graph — transform hierarchy, dirty propagation,
   world matrix cache y serialización de jerarquía.

   SCENE GRAPH EN MOTORES:
   -----------------------
   Árbol de entidades con relación padre-hijo. Transform local → world por multiplicación.
   - Local transform: posición, rotación, escala relativa al padre.
   - World transform: transform absoluta en espacio global.
   - Cambio en padre → todos los hijos se marcan "dirty" (world matrix inválida).

   DIRTY PROPAGATION (propagación de suciedad):
   -------------------------------------------
   Opción A: Eager (inmediata) — al cambiar local, recalcular world recursivamente.
     - Simple, pero O(N) por cambio aunque no se use world.
   Opción B: Lazy (diferida) — marcar dirty; recalcular solo cuando se accede a world.
     - Mejor si muchos cambios pero pocos reads; requiere flag + version/timestamp.
   Opción C: Frame-deferred — acumular dirty entities; recalcular todo en batch al final del frame.
     - Cache-friendly: un solo pass DFS/BFS por frame; ideal para job system.

   WORLD MATRIX CACHE:
   -------------------
   Matriz 3x3 (2D) o 4x4 (3D) column-major.
   World = Parent.World * Local (orden: scale → rotate → translate).
   En 2D con float: 9 floats (3x3) o 16 floats (4x4).
   Cache: almacenar en componente WorldTransform { float m[9]; uint32_t version; }.

   JERARQUÍA EN ECS:
   -----------------
   Componente Parent { Entity parent; };
   Componente Children { std::vector<Entity> children; } (o linked list first/next).
   Componente LocalTransform { float x,y,rot,sx,sy; };
   Componente WorldTransform { float m[9]; bool dirty; };

   Sistema TransformUpdate:
   1. Recopilar raíces (sin Parent o Parent inválido).
   2. DFS/BFS: para cada entidad, World = Parent.World * Local.
   3. Marcar dirty=false tras calcular.
   4. Parallelizable por subárboles independientes.

   SERIALIZACIÓN DE JERARQUÍA:
   ---------------------------
   JSON: array de entidades con {id, parent_id, local_transform, components...}
   Flatbuffers: schema con tabla Entity { parent:uint32; local:Transform; ... }
   Prefabs: plantilla de entidad + hijos; instancing = clonar subárbol con nuevos IDs.

   ESTA LECCIÓN: SceneGraph component + TransformSystem con dirty propagation + world cache.
   ============================================================================= */
#include <iostream>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <cassert>
#include <cmath>
#include <array>
#include <queue>

// ============================================================================
// TIPOS BASE (reutiliza de lecciones anteriores)
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
struct CompReg {
    static CompReg& I() { static CompReg r; return r; }
    template<typename T> uint8_t Reg() { static uint8_t id = next++; return id; }
    template<typename T> ComponentMask Mask() const { return ComponentMask(1) << Reg<T>(); }
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
};

// ============================================================================
// COMPONENTES SCENE GRAPH
// ============================================================================
struct LocalTransform {
    float x=0, y=0, rot=0, sx=1, sy=1;
};

struct WorldTransform {
    // Matriz 3x3 column-major 2D: [sx*cos, -sy*sin, 0; sx*sin, sy*cos, 0; x, y, 1]
    float m[9] = {1,0,0, 0,1,0, 0,0,1};
    uint32_t version = 0;  // Incrementa cada vez que se recalcula
};

struct Parent { Entity parent = 0; };
struct FirstChild { Entity child = 0; };
struct NextSibling { Entity next = 0; };
struct PrevSibling { Entity prev = 0; };  // Para remoción O(1)

struct DirtyFlag { bool dirty = true; };  // World transform necesita recálculo

REG(LocalTransform) REG(WorldTransform) REG(Parent)
REG(FirstChild) REG(NextSibling) REG(PrevSibling) REG(DirtyFlag)

// ============================================================================
// ARCHETYPE STORAGE (simplificado para demo)
// ============================================================================
struct Arch {
    ComponentMask mask = 0;
    std::vector<Entity> entities;
    SparseSet<LocalTransform> local;
    SparseSet<WorldTransform> world;
    SparseSet<Parent> parent;
    SparseSet<FirstChild> first;
    SparseSet<NextSibling> next;
    SparseSet<PrevSibling> prev;
    SparseSet<DirtyFlag> dirty;

    template<typename C> C* Get(Entity e) {
        if constexpr(std::is_same_v<C,LocalTransform>) return local.Get(e);
        else if constexpr(std::is_same_v<C,WorldTransform>) return world.Get(e);
        else if constexpr(std::is_same_v<C,Parent>) return parent.Get(e);
        else if constexpr(std::is_same_v<C,FirstChild>) return first.Get(e);
        else if constexpr(std::is_same_v<C,NextSibling>) return next.Get(e);
        else if constexpr(std::is_same_v<C,PrevSibling>) return prev.Get(e);
        else if constexpr(std::is_same_v<C,DirtyFlag>) return dirty.Get(e);
        return nullptr;
    }
    template<typename C> C* Add(Entity e, C c={}) {
        if constexpr(std::is_same_v<C,LocalTransform>) return local.Add(e,c);
        else if constexpr(std::is_same_v<C,WorldTransform>) return world.Add(e,c);
        else if constexpr(std::is_same_v<C,Parent>) return parent.Add(e,c);
        else if constexpr(std::is_same_v<C,FirstChild>) return first.Add(e,c);
        else if constexpr(std::is_same_v<C,NextSibling>) return next.Add(e,c);
        else if constexpr(std::is_same_v<C,PrevSibling>) return prev.Add(e,c);
        else if constexpr(std::is_same_v<C,DirtyFlag>) return dirty.Add(e,c);
        return nullptr;
    }
    template<typename C> void Remove(Entity e) {
        if constexpr(std::is_same_v<C,LocalTransform>) local.Remove(e);
        else if constexpr(std::is_same_v<C,WorldTransform>) world.Remove(e);
        else if constexpr(std::is_same_v<C,Parent>) parent.Remove(e);
        else if constexpr(std::is_same_v<C,FirstChild>) first.Remove(e);
        else if constexpr(std::is_same_v<C,NextSibling>) next.Remove(e);
        else if constexpr(std::is_same_v<C,PrevSibling>) prev.Remove(e);
        else if constexpr(std::is_same_v<C,DirtyFlag>) dirty.Remove(e);
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
            if((oa->mask&REG(LocalTransform))&&(nm&REG(LocalTransform))) if(auto* p=oa->Get<LocalTransform>(e)) na->Add<LocalTransform>(e,*p);
            if((oa->mask&REG(WorldTransform))&&(nm&REG(WorldTransform))) if(auto* p=oa->Get<WorldTransform>(e)) na->Add<WorldTransform>(e,*p);
            if((oa->mask&REG(Parent))&&(nm&REG(Parent))) if(auto* p=oa->Get<Parent>(e)) na->Add<Parent>(e,*p);
            if((oa->mask&REG(FirstChild))&&(nm&REG(FirstChild))) if(auto* p=oa->Get<FirstChild>(e)) na->Add<FirstChild>(e,*p);
            if((oa->mask&REG(NextSibling))&&(nm&REG(NextSibling))) if(auto* p=oa->Get<NextSibling>(e)) na->Add<NextSibling>(e,*p);
            if((oa->mask&REG(PrevSibling))&&(nm&REG(PrevSibling))) if(auto* p=oa->Get<PrevSibling>(e)) na->Add<PrevSibling>(e,*p);
            if((oa->mask&REG(DirtyFlag))&&(nm&REG(DirtyFlag))) if(auto* p=oa->Get<DirtyFlag>(e)) na->Add<DirtyFlag>(e,*p);
            size_t L=oa->entities.size()-1; if(odi!=L){ Entity me=oa->entities[L]; oa->entities[odi]=me; dense_idx[EntityIndex(me)]=odi; } oa->entities.pop_back();
        }
        arch_ptr[i]=na; masks[i]=nm; dense_idx[i]=na->entities.size(); na->entities.push_back(e);
    }
    template<typename C> C& Add(Entity e){ Migrate(e, masks[EntityIndex(e)]|REG(C)); return *arch_ptr[EntityIndex(e)]->Add<C>(e); }
    template<typename C> void Remove(Entity e){ Migrate(e, masks[EntityIndex(e)]&~REG(C)); }
    template<typename C> C* Get(Entity e){ Arch* a=arch_ptr[EntityIndex(e)]; return a?a->Get<C>(e):nullptr; }
    void Destroy(Entity e){ uint32_t i=EntityIndex(e); Arch* a=arch_ptr[i]; if(a){ size_t di=dense_idx[i], L=a->entities.size()-1; if(di!=L){ Entity me=a->entities[L]; a->entities[di]=me; dense_idx[EntityIndex(me)]=di; } a->entities.pop_back(); } arch_ptr[i]=nullptr; masks[i]=0; free_idx.push_back(i); }

    // Query helper
    template<typename... Cs> struct View {
        Registry* r; ComponentMask wm=0;
        View(Registry* reg): r(reg) { (wm|=REG(Cs),...); }
        struct It { Registry* r; ComponentMask m; std::vector<Arch*> as; size_t ai=0, ei=0; Entity cur=0;
            It(Registry* reg, ComponentMask mask, bool end): r(reg), m(mask) {
                if(!end){ for(auto& [k,v]:r->arches) if((v.mask&m)==m) as.push_back(&v); Advance(); }
            }
            void Advance(){ while(ai<as.size()){ Arch* a=as[ai]; if(ei<a->entities.size()){ cur=a->entities[ei]; return; } ai++; ei=0; } cur=0; }
            Entity operator*() const { return cur; }
            It& operator++(){ ei++; Advance(); return *this; }
            bool operator!=(const It& o) const { return cur!=o.cur; }
        };
        It begin(){ return It(r, wm, false); } It end(){ return It(r, wm, true); }
    };
    template<typename... Cs> View<Cs...> View() { return View<Cs...>(this); }
};

// ============================================================================
// MATH HELPERS (2D column-major 3x3)
// ============================================================================
inline void Mat3Identity(float m[9]) { m[0]=1; m[1]=0; m[2]=0; m[3]=0; m[4]=1; m[5]=0; m[6]=0; m[7]=0; m[8]=1; }
inline void Mat3FromTRS(float m[9], float x, float y, float rot, float sx, float sy) {
    float c = std::cos(rot), s = std::sin(rot);
    m[0] = sx * c;  m[3] = -sy * s;  m[6] = x;
    m[1] = sx * s;  m[4] = sy * c;   m[7] = y;
    m[2] = 0;       m[5] = 0;        m[8] = 1;
}
inline void Mat3Mul(float dst[9], const float a[9], const float b[9]) {
    // dst = a * b (column-major: a * b significa aplicar b primero, luego a)
    for(int col=0; col<3; ++col){
        for(int row=0; row<3; ++row){
            dst[col*3+row] = a[0*3+row]*b[col*3+0] + a[1*3+row]*b[col*3+1] + a[2*3+row]*b[col*3+2];
        }
    }
}
inline void Mat3MulVec(float out[2], const float m[9], float x, float y) {
    out[0] = m[0]*x + m[3]*y + m[6];
    out[1] = m[1]*x + m[4]*y + m[7];
}

// ============================================================================
// SCENE GRAPH OPERATIONS
// ============================================================================
void SetParent(Registry& reg, Entity child, Entity new_parent) {
    // 1. Quitar del padre anterior
    if(auto* old_p = reg.Get<Parent>(child)) {
        Entity old_parent = old_p->parent;
        if(old_parent) {
            // Unlink de lista de hermanos
            if(auto* pc = reg.Get<PrevSibling>(child)) {
                Entity prev = pc->prev;
                Entity next = reg.Get<NextSibling>(child) ? reg.Get<NextSibling>(child)->next : 0;
                if(prev) {
                    if(auto* pn = reg.Get<NextSibling>(prev)) pn->next = next;
                } else {
                    // Era first child
                    if(auto* fc = reg.Get<FirstChild>(old_parent)) fc->child = next;
                }
                if(next) {
                    if(auto* pn = reg.Get<PrevSibling>(next)) pn->prev = prev;
                }
            }
        }
    }

    // 2. Link al nuevo padre
    if(new_parent) {
        // Añadir al final de la lista de hijos
        Entity last_child = 0;
        if(auto* fc = reg.Get<FirstChild>(new_parent)) {
            Entity cur = fc->child;
            while(cur) {
                last_child = cur;
                auto* ns = reg.Get<NextSibling>(cur);
                cur = ns ? ns->next : 0;
            }
        }
        if(last_child) {
            reg.Add<NextSibling>(last_child).next = child;
            reg.Add<PrevSibling>(child).prev = last_child;
        } else {
            reg.Add<FirstChild>(new_parent).child = child;
        }
        reg.Add<Parent>(child).parent = new_parent;
    } else {
        reg.Remove<Parent>(child);
        reg.Remove<PrevSibling>(child);
        reg.Remove<NextSibling>(child);
    }

    // 3. Marcar dirty en subárbol
    auto MarkDirty = [&](auto&& self, Entity e) -> void {
        if(auto* d = reg.Get<DirtyFlag>(e)) d->dirty = true;
        else reg.Add<DirtyFlag>(e).dirty = true;
        // Recorrer hijos
        Entity cur = 0;
        if(auto* fc = reg.Get<FirstChild>(e)) cur = fc->child;
        while(cur) {
            self(self, cur);
            auto* ns = reg.Get<NextSibling>(cur);
            cur = ns ? ns->next : 0;
        }
    };
    MarkDirty(MarkDirty, child);
}

void UpdateWorldTransforms(Registry& reg) {
    // Recopilar raíces (entidades con LocalTransform pero sin Parent válido)
    std::vector<Entity> roots;
    for(Entity e: reg.View<LocalTransform>()) {
        if(!reg.Get<Parent>(e)) roots.push_back(e);
    }

    // DFS iterativo con stack para calcular world matrices
    std::vector<std::pair<Entity, const float*>> stack;  // (entity, parent_world_matrix)
    float identity[9]; Mat3Identity(identity);

    for(Entity root: roots) {
        stack.clear();
        stack.emplace_back(root, identity);

        while(!stack.empty()) {
            auto [e, parent_world] = stack.back(); stack.pop_back();

            auto* local = reg.Get<LocalTransform>(e);
            auto* world = reg.Get<WorldTransform>(e);
            auto* dirty = reg.Get<DirtyFlag>(e);

            if(!local || !world) continue;

            bool is_dirty = dirty && dirty->dirty;
            // También recalcular si parent cambió (version mismatch)
            // Simplificación: recalcular siempre si dirty o si no tiene world válido
            if(is_dirty || world->version == 0) {
                float local_mat[9];
                Mat3FromTRS(local_mat, local->x, local->y, local->rot, local->sx, local->sy);
                Mat3Mul(world->m, parent_world, local_mat);  // world = parent * local
                world->version++;
                if(dirty) dirty->dirty = false;
            }

            // Push hijos con world matrix actual
            Entity cur = 0;
            if(auto* fc = reg.Get<FirstChild>(e)) cur = fc->child;
            while(cur) {
                stack.emplace_back(cur, world->m);
                auto* ns = reg.Get<NextSibling>(cur);
                cur = ns ? ns->next : 0;
            }
        }
    }
}

// Helpers para GetWorldPosition, etc.
bool GetWorldPosition(Registry& reg, Entity e, float& out_x, float& out_y) {
    auto* w = reg.Get<WorldTransform>(e);
    if(!w) return false;
    out_x = w->m[6]; out_y = w->m[7];
    return true;
}

// ============================================================================
// SERIALIZACIÓN BÁSICA (JSON-like struct)
// ============================================================================
struct SerializedEntity {
    Entity id;
    Entity parent;
    LocalTransform local;
    // ... otros componentes
};

std::vector<SerializedEntity> SerializeScene(Registry& reg) {
    std::vector<SerializedEntity> out;
    for(Entity e: reg.View<LocalTransform>()) {
        Entity parent = 0;
        if(auto* p = reg.Get<Parent>(e)) parent = p->parent;
        out.push_back({e, parent, *reg.Get<LocalTransform>(e)});
    }
    return out;
}

// ============================================================================
// VERIFICACIÓN DE SINTAXIS
// ============================================================================
int main() {
    std::cout << "=== LECCIÓN 05: Scene Graph (solo sintaxis) ===\n";

    Registry reg;

    // Raíz
    Entity root = reg.Create();
    reg.Add<LocalTransform>(root) = {100, 100, 0, 2, 2};
    reg.Add<WorldTransform>(root);
    reg.Add<DirtyFlag>(root);

    // Hijo 1
    Entity child1 = reg.Create();
    reg.Add<LocalTransform>(child1) = {50, 0, 0.5f, 1, 1};
    reg.Add<WorldTransform>(child1);
    reg.Add<DirtyFlag>(child1);
    SetParent(reg, child1, root);

    // Hijo 2
    Entity child2 = reg.Create();
    reg.Add<LocalTransform>(child2) = {-30, 40, 0, 0.5f, 0.5f};
    reg.Add<WorldTransform>(child2);
    reg.Add<DirtyFlag>(child2);
    SetParent(reg, child2, root);

    // Nieto
    Entity grandchild = reg.Create();
    reg.Add<LocalTransform>(grandchild) = {10, 10, 0, 1, 1};
    reg.Add<WorldTransform>(grandchild);
    reg.Add<DirtyFlag>(grandchild);
    SetParent(reg, grandchild, child1);

    // Update world transforms
    UpdateWorldTransforms(reg);

    float wx, wy;
    GetWorldPosition(reg, root, wx, wy); std::cout << "Root world: " << wx << ", " << wy << "\n";
    GetWorldPosition(reg, child1, wx, wy); std::cout << "Child1 world: " << wx << ", " << wy << "\n";
    GetWorldPosition(reg, child2, wx, wy); std::cout << "Child2 world: " << wx << ", " << wy << "\n";
    GetWorldPosition(reg, grandchild, wx, wy); std::cout << "Grandchild world: " << wx << ", " << wy << "\n";

    // Serializar
    auto serialized = SerializeScene(reg);
    std::cout << "Serialized " << serialized.size() << " entities\n";

    std::cout << "=== FIN LECCIÓN 05 ===\n";
    return 0;
}