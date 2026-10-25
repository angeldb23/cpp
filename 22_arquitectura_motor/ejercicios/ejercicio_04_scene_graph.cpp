/* =============================================================================
   MÓDULO 22 - EJERCICIO 04: Scene Graph — Transform Hierarchy y Dirty Propagation
   =============================================================================
   DIFICULTAD: Medio
   APOYO: Lección 05 (Scene Graph Hierarchy), Lección 06 (Serialización)

   CONSIGNA:
   Implementa Scene Graph con:
   - LocalTransform (posición, rotación, escala local)
   - WorldTransform (matriz 3x3 world, version, dirty flag)
   - Parent/FirstChild/NextSibling/PrevSibling para jerarquía
   - SetParent(child, parent) que relinka y marca dirty en subárbol
   - UpdateWorldTransforms() que recalcula world matrices en orden DFS

   TESTS:
   - Hijo hereda transform del padre (world = parent * local)
   - Cambio en local del padre marca dirty en hijos
   - UpdateWorldTransforms recalcula correctamente
   - Re-parenting (cambiar padre) actualiza ambos subárboles
   - GetWorldPosition retorna posición world correcta
   ============================================================================= */
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <cassert>
#include <cmath>
#include <array>
#include <queue>
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
// COMPONENTES SCENE GRAPH
// ============================================================================
struct LocalTransform { float x=0, y=0, rot=0, sx=1, sy=1; };
struct WorldTransform { float m[9] = {1,0,0, 0,1,0, 0,0,1}; uint32_t version = 0; };
struct Parent { Entity parent = 0; };
struct FirstChild { Entity child = 0; };
struct NextSibling { Entity next = 0; };
struct PrevSibling { Entity prev = 0; };
struct DirtyFlag { bool dirty = true; };

REG(LocalTransform) REG(WorldTransform) REG(Parent)
REG(FirstChild) REG(NextSibling) REG(PrevSibling) REG(DirtyFlag)

// ============================================================================
// ARCHETYPE STORAGE
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
        uint32_t idx=EntityIndex(e); Arch* oa=arch_ptr[idx]; Arch* na=GetArch(nm); na->mask=nm;
        if(oa){
            size_t odi=dense_idx[idx];
            if((oa->mask&CMASK(LocalTransform))&&(nm&CMASK(LocalTransform))) if(auto* p=oa->Get<LocalTransform>(e)) na->Add<LocalTransform>(e,*p);
            if((oa->mask&CMASK(WorldTransform))&&(nm&CMASK(WorldTransform))) if(auto* p=oa->Get<WorldTransform>(e)) na->Add<WorldTransform>(e,*p);
            if((oa->mask&CMASK(Parent))&&(nm&CMASK(Parent))) if(auto* p=oa->Get<Parent>(e)) na->Add<Parent>(e,*p);
            if((oa->mask&CMASK(FirstChild))&&(nm&CMASK(FirstChild))) if(auto* p=oa->Get<FirstChild>(e)) na->Add<FirstChild>(e,*p);
            if((oa->mask&CMASK(NextSibling))&&(nm&CMASK(NextSibling))) if(auto* p=oa->Get<NextSibling>(e)) na->Add<NextSibling>(e,*p);
            if((oa->mask&CMASK(PrevSibling))&&(nm&CMASK(PrevSibling))) if(auto* p=oa->Get<PrevSibling>(e)) na->Add<PrevSibling>(e,*p);
            if((oa->mask&CMASK(DirtyFlag))&&(nm&CMASK(DirtyFlag))) if(auto* p=oa->Get<DirtyFlag>(e)) na->Add<DirtyFlag>(e,*p);
            size_t L=oa->entities.size()-1; if(odi!=L){ Entity me=oa->entities[L]; oa->entities[odi]=me; dense_idx[EntityIndex(me)]=odi; } oa->entities.pop_back();
        }
        arch_ptr[idx]=na; masks[idx]=nm; dense_idx[idx]=na->entities.size(); na->entities.push_back(e);
    }
    template<typename C> C& Add(Entity e){ Migrate(e, masks[EntityIndex(e)]|CMASK(C)); return *arch_ptr[EntityIndex(e)]->Add<C>(e); }
    template<typename C> void Remove(Entity e){ Migrate(e, masks[EntityIndex(e)]&~CMASK(C)); }
    template<typename C> C* Get(Entity e){ Arch* a=arch_ptr[EntityIndex(e)]; return a?a->Get<C>(e):nullptr; }
};

// ============================================================================
// MATH HELPERS
// ============================================================================
inline void Mat3Identity(float m[9]) { m[0]=1; m[1]=0; m[2]=0; m[3]=0; m[4]=1; m[5]=0; m[6]=0; m[7]=0; m[8]=1; }
inline void Mat3FromTRS(float m[9], float x, float y, float rot, float sx, float sy) {
    float c = std::cos(rot), s = std::sin(rot);
    m[0] = sx * c;  m[3] = -sy * s;  m[6] = x;
    m[1] = sx * s;  m[4] = sy * c;   m[7] = y;
    m[2] = 0;       m[5] = 0;        m[8] = 1;
}
inline void Mat3Mul(float dst[9], const float a[9], const float b[9]) {
    for(int col=0; col<3; ++col)
        for(int row=0; row<3; ++row)
            dst[col*3+row] = a[0*3+row]*b[col*3+0] + a[1*3+row]*b[col*3+1] + a[2*3+row]*b[col*3+2];
}

// ============================================================================
// TODO: Implementa SetParent y UpdateWorldTransforms
// ============================================================================
void SetParent(Registry& reg, Entity child, Entity new_parent) {
    // TU CÓDIGO AQUÍ:
    // 1. Unlink del padre anterior (actualizar FirstChild/NextSibling/PrevSibling)
    // 2. Link al nuevo padre (añadir al final de lista de hijos)
    // 3. Marcar dirty en subárbol del child (recursivo)
    (void)reg; (void)child; (void)new_parent;
}

void UpdateWorldTransforms(Registry& reg) {
    // TU CÓDIGO AQUÍ:
    // 1. Recopilar raíces (entidades con LocalTransform pero sin Parent)
    // 2. DFS iterativo con stack: (entity, parent_world_matrix)
    // 3. Para cada entidad: si dirty o version==0, calcular world = parent * local
    // 4. Marcar dirty=false, incrementar version
    // 5. Push hijos con world matrix actual
    (void)reg;
}

bool GetWorldPosition(Registry& reg, Entity e, float& out_x, float& out_y) {
    // TU CÓDIGO AQUÍ: retornar world.m[6], world.m[7]
    (void)reg; (void)e; (void)out_x; (void)out_y;
    return false;
}

// ============================================================================
// TESTS
// ============================================================================
TEST_CASE(ChildInheritsParentTransform) {
    Registry reg;
    Entity root = reg.Create();
    reg.Add<LocalTransform>(root) = {100, 100, 0, 2, 2};
    reg.Add<WorldTransform>(root);
    reg.Add<DirtyFlag>(root);

    Entity child = reg.Create();
    reg.Add<LocalTransform>(child) = {50, 0, 0, 1, 1};
    reg.Add<WorldTransform>(child);
    reg.Add<DirtyFlag>(child);
    SetParent(reg, child, root);

    UpdateWorldTransforms(reg);

    float wx, wy;
    ASSERT_TRUE(GetWorldPosition(reg, child, wx, wy));
    // Child local (50,0) * Parent scale(2,2) = world (100+100, 100+0) = (200, 100)
    ASSERT_NEAR(wx, 200.0f, 0.01f);
    ASSERT_NEAR(wy, 100.0f, 0.01f);
}

TEST_CASE(GrandchildInheritsFullHierarchy) {
    Registry reg;
    Entity root = reg.Create();
    reg.Add<LocalTransform>(root) = {0, 0, 0, 1, 1};
    reg.Add<WorldTransform>(root); reg.Add<DirtyFlag>(root);

    Entity child = reg.Create();
    reg.Add<LocalTransform>(child) = {10, 0, 0, 1, 1};
    reg.Add<WorldTransform>(child); reg.Add<DirtyFlag>(child);
    SetParent(reg, child, root);

    Entity grandchild = reg.Create();
    reg.Add<LocalTransform>(grandchild) = {5, 5, 0, 1, 1};
    reg.Add<WorldTransform>(grandchild); reg.Add<DirtyFlag>(grandchild);
    SetParent(reg, grandchild, child);

    UpdateWorldTransforms(reg);

    float wx, wy;
    ASSERT_TRUE(GetWorldPosition(reg, grandchild, wx, wy));
    // 0+10+5 = 15, 0+0+5 = 5
    ASSERT_NEAR(wx, 15.0f, 0.01f);
    ASSERT_NEAR(wy, 5.0f, 0.01f);
}

TEST_CASE(ReparentingUpdatesBothSubtrees) {
    Registry reg;
    Entity old_parent = reg.Create();
    reg.Add<LocalTransform>(old_parent) = {100, 100, 0, 1, 1};
    reg.Add<WorldTransform>(old_parent); reg.Add<DirtyFlag>(old_parent);

    Entity new_parent = reg.Create();
    reg.Add<LocalTransform>(new_parent) = {200, 200, 0, 1, 1};
    reg.Add<WorldTransform>(new_parent); reg.Add<DirtyFlag>(new_parent);

    Entity child = reg.Create();
    reg.Add<LocalTransform>(child) = {10, 10, 0, 1, 1};
    reg.Add<WorldTransform>(child); reg.Add<DirtyFlag>(child);
    SetParent(reg, child, old_parent);
    UpdateWorldTransforms(reg);

    float wx1, wy1;
    GetWorldPosition(reg, child, wx1, wy1);
    ASSERT_NEAR(wx1, 110.0f, 0.01f);
    ASSERT_NEAR(wy1, 110.0f, 0.01f);

    // Reparent
    SetParent(reg, child, new_parent);
    UpdateWorldTransforms(reg);

    float wx2, wy2;
    GetWorldPosition(reg, child, wx2, wy2);
    ASSERT_NEAR(wx2, 210.0f, 0.01f);
    ASSERT_NEAR(wy2, 210.0f, 0.01f);
}

TEST_CASE(DirtyPropagationOnParentChange) {
    Registry reg;
    Entity root = reg.Create();
    reg.Add<LocalTransform>(root) = {0, 0, 0, 1, 1};
    reg.Add<WorldTransform>(root); reg.Add<DirtyFlag>(root);

    Entity child = reg.Create();
    reg.Add<LocalTransform>(child) = {10, 0, 0, 1, 1};
    reg.Add<WorldTransform>(child); reg.Add<DirtyFlag>(child);
    SetParent(reg, child, root);
    UpdateWorldTransforms(reg);

    // Cambiar local del padre
    auto* root_local = reg.Get<LocalTransform>(root);
    root_local->x = 100;
    // Debe marcar dirty en hijo
    auto* child_dirty = reg.Get<DirtyFlag>(child);
    ASSERT_TRUE(child_dirty);
    ASSERT_TRUE(child_dirty->dirty);

    UpdateWorldTransforms(reg);
    float wx, wy;
    GetWorldPosition(reg, child, wx, wy);
    ASSERT_NEAR(wx, 110.0f, 0.01f);
}

TEST_CASE(RotationAndScaleInheritance) {
    Registry reg;
    Entity root = reg.Create();
    reg.Add<LocalTransform>(root) = {0, 0, 1.5708f, 2, 2};  // 90 deg, scale 2
    reg.Add<WorldTransform>(root); reg.Add<DirtyFlag>(root);

    Entity child = reg.Create();
    reg.Add<LocalTransform>(child) = {10, 0, 0, 1, 1};
    reg.Add<WorldTransform>(child); reg.Add<DirtyFlag>(child);
    SetParent(reg, child, root);

    UpdateWorldTransforms(reg);

    float wx, wy;
    GetWorldPosition(reg, child, wx, wy);
    // Parent: rot 90°, scale 2. Child local (10,0) → rotado 90° = (0,10) * scale 2 = (0,20)
    ASSERT_NEAR(wx, 0.0f, 0.1f);
    ASSERT_NEAR(wy, 20.0f, 0.1f);
}

TEST_CASE(MultipleChildrenIndependent) {
    Registry reg;
    Entity root = reg.Create();
    reg.Add<LocalTransform>(root) = {100, 100, 0, 1, 1};
    reg.Add<WorldTransform>(root); reg.Add<DirtyFlag>(root);

    Entity child1 = reg.Create();
    reg.Add<LocalTransform>(child1) = {10, 0, 0, 1, 1};
    reg.Add<WorldTransform>(child1); reg.Add<DirtyFlag>(child1);
    SetParent(reg, child1, root);

    Entity child2 = reg.Create();
    reg.Add<LocalTransform>(child2) = {0, 20, 0, 1, 1};
    reg.Add<WorldTransform>(child2); reg.Add<DirtyFlag>(child2);
    SetParent(reg, child2, root);

    UpdateWorldTransforms(reg);

    float wx1, wy1, wx2, wy2;
    GetWorldPosition(reg, child1, wx1, wy1);
    GetWorldPosition(reg, child2, wx2, wy2);
    ASSERT_NEAR(wx1, 110.0f, 0.01f); ASSERT_NEAR(wy1, 100.0f, 0.01f);
    ASSERT_NEAR(wx2, 100.0f, 0.01f); ASSERT_NEAR(wy2, 120.0f, 0.01f);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 04 (Scene Graph)...\n";
    return RUN_ALL_TESTS();
}