/* =============================================================================
   MÓDULO 22 - LECCIÓN 06: Scene Graph — Serialización, Prefabs, Instancing, Diff
   =============================================================================
   TEMA 3 de TEMAS.md (2/2): Serialización de jerarquía, prefabs, instancing,
   diff de escena.

   SERIALIZACIÓN DE ESCENA:
   ------------------------
   Formatos:
   - JSON: legible, lento parse, grande en disco. Bueno para assets de desarrollo.
   - Flatbuffers: schema binario, zero-copy read, rápido. Bueno para runtime.
   - Custom binary: máximo control, versionado manual.

   Estructura típica JSON:
   {
     "entities": [
       {"id": 1, "parent": 0, "local": {"x":100,"y":100}, "components": {"Sprite": {"tex":"player.png"}}},
       {"id": 2, "parent": 1, "local": {"x":50,"y":0}, "components": {"Health": {"hp":100}}}
     ],
     "prefabs": {"Enemy": {"components": ["Transform", "Sprite", "AI", "Health"]}}
   }

   PREFABS (Plantillas de Entidad):
   --------------------------------
   Prefab = entidad plantilla con componentes por defecto.
   Instancing = clonar prefab → nuevas entidades con nuevos IDs, mismos valores iniciales.
   - Prefab puede ser jerarquía completa (subárbol).
   - Override: instancias pueden sobrescribir valores específicos.
   - Nested prefabs: prefab dentro de prefab (instanciar recursivamente).

   INSTANCING EFICIENTE:
   ---------------------
   - Clonar solo componentes que difieren del prefab (copy-on-write).
   - Compartir datos inmutables (mesh, texture, material) via handles.
   - En ECS: prefab = archetype + valores por defecto; instancia = entidades en ese archetype.

   DIFF DE ESCENA (Scene Diff):
   ----------------------------
   Comparar escena actual vs guardada → lista de operaciones:
   - EntityAdded(id, components...)
   - EntityRemoved(id)
   - ComponentChanged(id, component_type, old_value, new_value)
   - ParentChanged(id, old_parent, new_parent)
   - TransformChanged(id, old_local, new_local)

   Usos:
   - Undo/Redo: pila de diffs.
   - Network replication: enviar solo diffs.
   - Hot-reload: detectar qué cambió y actualizar solo eso.
   - Level editor: mostrar cambios sin guardar.

   ESTA LECCIÓN: Serialización JSON (nlohmann::json estilo), Prefab system,
   Instancing con override, Scene Diff algorithm.
   ============================================================================= */
#include <iostream>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <cassert>
#include <string>
#include <sstream>
#include <memory>
#include <optional>
#include <variant>
#include <fstream>

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
struct CompReg {
    static CompReg& I() { static CompReg r; return r; }
    template<typename T> uint8_t Reg() { static uint8_t id = next++; return id; }
    template<typename T> ComponentMask Mask() const { return ComponentMask(1) << Reg<T>(); }
    template<typename T> const char* Name() const {
        static const char* names[] = {"Transform","Velocity","Sprite","Health","Tag","Parent"};
        return names[Reg<T>()];
    }
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
// COMPONENTES
// ============================================================================
struct Transform { float x=0,y=0,rot=0,sx=1,sy=1; };
struct Velocity { float vx=0,vy=0; };
struct Sprite { int tex_id=0; char tex_path[64]={}; };
struct Health { int hp=100, max=100; };
struct Tag { char name[32]={}; };
struct Parent { Entity parent=0; };

REG(Transform) REG(Velocity) REG(Sprite) REG(Health) REG(Tag) REG(Parent)

// ============================================================================
// ARCHETYPE STORAGE
// ============================================================================
struct Arch {
    ComponentMask mask=0;
    std::vector<Entity> entities;
    SparseSet<Transform> T; SparseSet<Velocity> V; SparseSet<Sprite> S;
    SparseSet<Health> H; SparseSet<Tag> Tg; SparseSet<Parent> P;

    template<typename C> C* Get(Entity e) {
        if constexpr(std::is_same_v<C,Transform>) return T.Get(e);
        else if constexpr(std::is_same_v<C,Velocity>) return V.Get(e);
        else if constexpr(std::is_same_v<C,Sprite>) return S.Get(e);
        else if constexpr(std::is_same_v<C,Health>) return H.Get(e);
        else if constexpr(std::is_same_v<C,Tag>) return Tg.Get(e);
        else if constexpr(std::is_same_v<C,Parent>) return P.Get(e);
        return nullptr;
    }
    template<typename C> C* Add(Entity e, C c={}) {
        if constexpr(std::is_same_v<C,Transform>) return T.Add(e,c);
        else if constexpr(std::is_same_v<C,Velocity>) return V.Add(e,c);
        else if constexpr(std::is_same_v<C,Sprite>) return S.Add(e,c);
        else if constexpr(std::is_same_v<C,Health>) return H.Add(e,c);
        else if constexpr(std::is_same_v<C,Tag>) return Tg.Add(e,c);
        else if constexpr(std::is_same_v<C,Parent>) return P.Add(e,c);
        return nullptr;
    }
    template<typename C> void Remove(Entity e) {
        if constexpr(std::is_same_v<C,Transform>) T.Remove(e);
        else if constexpr(std::is_same_v<C,Velocity>) V.Remove(e);
        else if constexpr(std::is_same_v<C,Sprite>) S.Remove(e);
        else if constexpr(std::is_same_v<C,Health>) H.Remove(e);
        else if constexpr(std::is_same_v<C,Tag>) Tg.Remove(e);
        else if constexpr(std::is_same_v<C,Parent>) P.Remove(e);
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
    Arch* GetArch(ComponentMask m) { auto it=arches.find(m); if(it!=arches.end()) return &it->second; return &archetypes[m]; }
    void Migrate(Entity e, ComponentMask nm) {
        uint32_t i=EntityIndex(e); Arch* oa=arch_ptr[i]; Arch* na=GetArch(nm); na->mask=nm;
        if(oa){
            size_t odi=dense_idx[i];
            if((oa->mask&REG(Transform))&&(nm&REG(Transform))) if(auto* p=oa->Get<Transform>(e)) na->Add<Transform>(e,*p);
            if((oa->mask&REG(Velocity))&&(nm&REG(Velocity))) if(auto* p=oa->Get<Velocity>(e)) na->Add<Velocity>(e,*p);
            if((oa->mask&REG(Sprite))&&(nm&REG(Sprite))) if(auto* p=oa->Get<Sprite>(e)) na->Add<Sprite>(e,*p);
            if((oa->mask&REG(Health))&&(nm&REG(Health))) if(auto* p=oa->Get<Health>(e)) na->Add<Health>(e,*p);
            if((oa->mask&REG(Tag))&&(nm&REG(Tag))) if(auto* p=oa->Get<Tag>(e)) na->Add<Tag>(e,*p);
            if((oa->mask&REG(Parent))&&(nm&REG(Parent))) if(auto* p=oa->Get<Parent>(e)) na->Add<Parent>(e,*p);
            size_t L=oa->entities.size()-1; if(odi!=L){ Entity me=oa->entities[L]; oa->entities[odi]=me; dense_idx[EntityIndex(me)]=odi; } oa->entities.pop_back();
        }
        arch_ptr[i]=na; masks[i]=nm; dense_idx[i]=na->entities.size(); na->entities.push_back(e);
    }
    template<typename C> C& Add(Entity e){ Migrate(e, masks[EntityIndex(e)]|REG(C)); return *arch_ptr[EntityIndex(e)]->Add<C>(e); }
    template<typename C> void Remove(Entity e){ Migrate(e, masks[EntityIndex(e)]&~REG(C)); }
    template<typename C> C* Get(Entity e){ Arch* a=arch_ptr[EntityIndex(e)]; return a?a->Get<C>(e):nullptr; }
    void Destroy(Entity e){ uint32_t i=EntityIndex(e); Arch* a=arch_ptr[i]; if(a){ size_t di=dense_idx[i], L=a->entities.size()-1; if(di!=L){ Entity me=a->entities[L]; a->entities[di]=me; dense_idx[EntityIndex(me)]=di; } a->entities.pop_back(); } arch_ptr[i]=nullptr; masks[i]=0; free_idx.push_back(i); }

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
// JSON-LIKE SERIALIZATION (manual, sin dependencia externa)
// ============================================================================
// En producción: usar nlohmann::json (header-only) o Flatbuffers.
// Aquí: writer/reader simple para demo.

struct JsonValue {
    enum Type { Null, Bool, Int, Float, String, Array, Object } type = Null;
    bool b = false; int64_t i = 0; double d = 0; std::string s;
    std::vector<JsonValue> arr; std::unordered_map<std::string, JsonValue> obj;

    JsonValue() = default;
    JsonValue(bool v) : type(Bool), b(v) {}
    JsonValue(int v) : type(Int), i(v) {}
    JsonValue(float v) : type(Float), d(v) {}
    JsonValue(const char* v) : type(String), s(v) {}
    JsonValue(std::string v) : type(String), s(std::move(v)) {}

    JsonValue& operator[](const char* key) { type = Object; return obj[key]; }
    JsonValue& operator[](size_t idx) { type = Array; if(idx >= arr.size()) arr.resize(idx+1); return arr[idx]; }

    std::string ToString(int indent = 0) const {
        std::string pad(indent, ' ');
        switch(type){
            case Null: return "null";
            case Bool: return b ? "true" : "false";
            case Int: return std::to_string(i);
            case Float: return std::to_string(d);
            case String: return "\"" + s + "\"";
            case Array: {
                std::string r = "[\n";
                for(size_t i=0;i<arr.size();++i){
                    r += pad + "  " + arr[i].ToString(indent+2);
                    if(i+1<arr.size()) r += ",";
                    r += "\n";
                }
                r += pad + "]"; return r;
            }
            case Object: {
                std::string r = "{\n";
                size_t i=0;
                for(auto& [k,v]: obj){
                    r += pad + "  \"" + k + "\": " + v.ToString(indent+2);
                    if(i+1<obj.size()) r += ",";
                    r += "\n"; i++;
                }
                r += pad + "}"; return r;
            }
        }
        return "null";
    }
};

// Serializar componente individual
JsonValue SerializeComponent(const char* type_name, const void* data) {
    JsonValue v; v["type"] = type_name;
    if(strcmp(type_name, "Transform")==0){
        auto* t = (const Transform*)data;
        v["x"]=t->x; v["y"]=t->y; v["rot"]=t->rot; v["sx"]=t->sx; v["sy"]=t->sy;
    }else if(strcmp(type_name, "Velocity")==0){
        auto* v2 = (const Velocity*)data; v["vx"]=v2->vx; v["vy"]=v2->vy;
    }else if(strcmp(type_name, "Sprite")==0){
        auto* s = (const Sprite*)data; v["tex_id"]=s->tex_id; v["tex_path"]=s->tex_path;
    }else if(strcmp(type_name, "Health")==0){
        auto* h = (const Health*)data; v["hp"]=h->hp; v["max"]=h->max;
    }else if(strcmp(type_name, "Tag")==0){
        auto* t = (const Tag*)data; v["name"]=t->name;
    }else if(strcmp(type_name, "Parent")==0){
        auto* p = (const Parent*)data; v["parent"]= (int)p->parent;
    }
    return v;
}

// Deserializar componente
void DeserializeComponent(const JsonValue& v, void* data) {
    std::string type = v["type"].s;
    if(type=="Transform"){ auto* t=(Transform*)data; t->x=v["x"].d; t->y=v["y"].d; t->rot=v["rot"].d; t->sx=v["sx"].d; t->sy=v["sy"].d; }
    else if(type=="Velocity"){ auto* v2=(Velocity*)data; v2->vx=v["vx"].d; v2->vy=v["vy"].d; }
    else if(type=="Sprite"){ auto* s=(Sprite*)data; s->tex_id=v["tex_id"].i; strncpy(s->tex_path, v["tex_path"].s.c_str(), 63); }
    else if(type=="Health"){ auto* h=(Health*)data; h->hp=v["hp"].i; h->max=v["max"].i; }
    else if(type=="Tag"){ auto* t=(Tag*)data; strncpy(t->name, v["name"].s.c_str(), 31); }
    else if(type=="Parent"){ auto* p=(Parent*)data; p->parent=(Entity)v["parent"].i; }
}

// ============================================================================
// SERIALIZAR ESCENA COMPLETA
// ============================================================================
JsonValue SerializeScene(Registry& reg) {
    JsonValue root; root["entities"] = JsonValue();
    size_t idx = 0;
    for(Entity e: reg.View<Transform>()){
        JsonValue ent; ent["id"] = (int)e;
        if(auto* p = reg.Get<Parent>(e)) ent["parent"] = (int)p->parent; else ent["parent"] = 0;
        JsonValue comps; comps.type = JsonValue::Array;
        // Transform (siempre presente en esta view)
        if(auto* t = reg.Get<Transform>(e)) comps[comps.arr.size()] = SerializeComponent("Transform", t);
        if(auto* v = reg.Get<Velocity>(e)) comps[comps.arr.size()] = SerializeComponent("Velocity", v);
        if(auto* s = reg.Get<Sprite>(e)) comps[comps.arr.size()] = SerializeComponent("Sprite", s);
        if(auto* h = reg.Get<Health>(e)) comps[comps.arr.size()] = SerializeComponent("Health", h);
        if(auto* tg = reg.Get<Tag>(e)) comps[comps.arr.size()] = SerializeComponent("Tag", tg);
        if(auto* p = reg.Get<Parent>(e)) comps[comps.arr.size()] = SerializeComponent("Parent", p);
        ent["components"] = comps;
        root["entities"][idx++] = ent;
    }
    return root;
}

// ============================================================================
// PREFAB SYSTEM
// ============================================================================
struct Prefab {
    std::string name;
    // Archetype mask + valores por defecto por componente
    ComponentMask mask = 0;
    std::unordered_map<std::string, JsonValue> default_components;  // type_name -> JsonValue
};

struct PrefabRegistry {
    std::unordered_map<std::string, Prefab> prefabs;

    void RegisterFromEntity(Registry& reg, const std::string& name, Entity prototype) {
        Prefab pf; pf.name = name;
        Arch* arch = nullptr;
        for(auto& [m, a]: reg.arches) {
            if(std::find(a.entities.begin(), a.entities.end(), prototype) != a.entities.end()){
                arch = &a; pf.mask = m; break;
            }
        }
        if(!arch) return;
        // Capturar valores por defecto
        auto capture = [&](auto& pool, const char* type_name){
            for(size_t i=0;i<pool.dense.size();++i){
                if(pool.dense_to_entity[i] == prototype){
                    pf.default_components[type_name] = SerializeComponent(type_name, &pool.dense[i]);
                    break;
                }
            }
        };
        capture(arch->T, "Transform");
        capture(arch->V, "Velocity");
        capture(arch->S, "Sprite");
        capture(arch->H, "Health");
        capture(arch->Tg, "Tag");
        prefabs[name] = std::move(pf);
    }

    Entity Instantiate(Registry& reg, const std::string& name, Entity parent = 0) {
        auto it = prefabs.find(name);
        if(it == prefabs.end()) return 0;
        const Prefab& pf = it->second;
        Entity e = reg.Create();
        // Añadir componentes con valores por defecto
        for(auto& [type_name, json] : pf.default_components) {
            if(type_name == "Transform") reg.Add<Transform>(e); DeserializeComponent(json, reg.Get<Transform>(e));
            else if(type_name == "Velocity") reg.Add<Velocity>(e); DeserializeComponent(json, reg.Get<Velocity>(e));
            else if(type_name == "Sprite") reg.Add<Sprite>(e); DeserializeComponent(json, reg.Get<Sprite>(e));
            else if(type_name == "Health") reg.Add<Health>(e); DeserializeComponent(json, reg.Get<Health>(e));
            else if(type_name == "Tag") reg.Add<Tag>(e); DeserializeComponent(json, reg.Get<Tag>(e));
        }
        if(parent) {
            reg.Add<Parent>(e).parent = parent;
        }
        return e;
    }
};

// ============================================================================
// SCENE DIFF
// ============================================================================
enum class DiffOpType { EntityAdded, EntityRemoved, ComponentChanged, ParentChanged, TransformChanged };

struct DiffOp {
    DiffOpType type;
    Entity entity;
    std::string component_type;
    JsonValue old_value, new_value;
    Entity old_parent = 0, new_parent = 0;
};

std::vector<DiffOp> ComputeSceneDiff(Registry& old_reg, Registry& new_reg) {
    std::vector<DiffOp> diffs;
    // Build maps entity -> components para ambas escenas
    // Simplificado: asumimos mismos IDs (generation igual)
    // En real: mapear por ID persistente (UUID) no por Entity handle

    // 1. Entidades en old pero no en new → removed
    for(Entity e: old_reg.View<Transform>()){
        if(!new_reg.Get<Transform>(e)){
            diffs.push_back({DiffOpType::EntityRemoved, e, "", {}, {}});
        }
    }
    // 2. Entidades en new pero no en old → added
    for(Entity e: new_reg.View<Transform>()){
        if(!old_reg.Get<Transform>(e)){
            JsonValue ent; ent["id"] = (int)e;
            JsonValue comps; comps.type = JsonValue::Array;
            if(auto* t = new_reg.Get<Transform>(e)) comps[comps.arr.size()] = SerializeComponent("Transform", t);
            if(auto* v = new_reg.Get<Velocity>(e)) comps[comps.arr.size()] = SerializeComponent("Velocity", v);
            if(auto* s = new_reg.Get<Sprite>(e)) comps[comps.arr.size()] = SerializeComponent("Sprite", s);
            if(auto* h = new_reg.Get<Health>(e)) comps[comps.arr.size()] = SerializeComponent("Health", h);
            if(auto* tg = new_reg.Get<Tag>(e)) comps[comps.arr.size()] = SerializeComponent("Tag", tg);
            if(auto* p = new_reg.Get<Parent>(e)) comps[comps.arr.size()] = SerializeComponent("Parent", p);
            ent["components"] = comps;
            diffs.push_back({DiffOpType::EntityAdded, e, "", {}, ent});
        }
    }
    // 3. Entidades en ambas → comparar componentes
    for(Entity e: old_reg.View<Transform>()){
        if(!new_reg.Get<Transform>(e)) continue;
        auto check_comp = [&](const char* type_name, auto getter_old, auto getter_new){
            auto* old_c = getter_old(e);
            auto* new_c = getter_new(e);
            if(old_c && new_c){
                JsonValue old_j = SerializeComponent(type_name, old_c);
                JsonValue new_j = SerializeComponent(type_name, new_c);
                if(old_j.ToString() != new_j.ToString()){
                    diffs.push_back({DiffOpType::ComponentChanged, e, type_name, old_j, new_j});
                }
            }else if(old_c && !new_c){
                diffs.push_back({DiffOpType::ComponentChanged, e, type_name, SerializeComponent(type_name, old_c), {}});
            }else if(!old_c && new_c){
                diffs.push_back({DiffOpType::ComponentChanged, e, type_name, {}, SerializeComponent(type_name, new_c)});
            }
        };
        check_comp("Transform", [&](Entity e){ return old_reg.Get<Transform>(e); }, [&](Entity e){ return new_reg.Get<Transform>(e); });
        check_comp("Velocity", [&](Entity e){ return old_reg.Get<Velocity>(e); }, [&](Entity e){ return new_reg.Get<Velocity>(e); });
        check_comp("Sprite", [&](Entity e){ return old_reg.Get<Sprite>(e); }, [&](Entity e){ return new_reg.Get<Sprite>(e); });
        check_comp("Health", [&](Entity e){ return old_reg.Get<Health>(e); }, [&](Entity e){ return new_reg.Get<Health>(e); });
        check_comp("Tag", [&](Entity e){ return old_reg.Get<Tag>(e); }, [&](Entity e){ return new_reg.Get<Tag>(e); });
        // Parent
        Entity old_p = 0, new_p = 0;
        if(auto* p = old_reg.Get<Parent>(e)) old_p = p->parent;
        if(auto* p = new_reg.Get<Parent>(e)) new_p = p->parent;
        if(old_p != new_p){
            diffs.push_back({DiffOpType::ParentChanged, e, "", {}, {}, old_p, new_p});
        }
    }
    return diffs;
}

// ============================================================================
// VERIFICACIÓN DE SINTAXIS
// ============================================================================
int main() {
    std::cout << "=== LECCIÓN 06: Scene Graph Serialización/Prefabs/Diff (solo sintaxis) ===\n";

    Registry reg;
    Entity e1 = reg.Create();
    reg.Add<Transform>(e1) = {10, 20, 0, 1, 1};
    reg.Add<Tag>(e1); strcpy(reg.Get<Tag>(e1)->name, "Player");
    reg.Add<Health>(e1) = {100, 100};
    reg.Add<Sprite>(e1) = {1, "player.png"};

    Entity e2 = reg.Create();
    reg.Add<Transform>(e2) = {5, 5, 0, 1, 1};
    reg.Add<Tag>(e2); strcpy(reg.Get<Tag>(e2)->name, "Gun");
    reg.Add<Parent>(e2).parent = e1;

    // Serializar
    JsonValue json = SerializeScene(reg);
    std::cout << "Scene JSON:\n" << json.ToString() << "\n";

    // Prefab
    PrefabRegistry prefabs;
    prefabs.RegisterFromEntity(reg, "Player", e1);
    Entity instance = prefabs.Instantiate(reg, "Player");
    std::cout << "Instanced Player as entity: " << instance << "\n";
    if(auto* t = reg.Get<Transform>(instance)) std::cout << "  Transform: " << t->x << "," << t->y << "\n";

    // Diff
    Registry reg2 = reg;  // Copia
    reg2.Get<Transform>(e1)->x = 999;  // Cambio
    Entity e3 = reg2.Create(); reg2.Add<Transform>(e3) = {0,0}; reg2.Add<Tag>(e3); strcpy(reg2.Get<Tag>(e3)->name, "New");
    auto diffs = ComputeSceneDiff(reg, reg2);
    std::cout << "Diffs found: " << diffs.size() << "\n";
    for(auto& d: diffs){
        std::cout << "  Type: " << (int)d.type << " Entity: " << d.entity;
        if(d.type == DiffOpType::ComponentChanged) std::cout << " Comp: " << d.component_type;
        std::cout << "\n";
    }

    std::cout << "=== FIN LECCIÓN 06 ===\n";
    return 0;
}