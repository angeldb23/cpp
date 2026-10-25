/* =============================================================================
   MÓDULO 22 - LECCIÓN 12: Serialización — Flatbuffers, Schema Evolution,
   Prefabs, Instancing, Scene Diff
   =============================================================================
   TEMA 7 de TEMAS.md: Serialización — escena a JSON/flatbuffers, prefabs,
   instancing y diff de escena.

   FLATBUFFERS:
   ------------
   - Serialización binaria zero-copy: leer directamente del buffer sin parse/alloc.
   - Schema (.fbs) define structs/tables; flatc genera código C++ (y otros lenguajes).
   - Backward/forward compatibility: campos opcionales, deprecated, nuevos campos.
   - Usado en: TensorFlow, Android, juegos (Unity DOTS, Bevy, Godot 4).

   SCHEMA EVOLUTION:
   -----------------
   - Agregar campos al final (optional, default value).
   - NUNCA reordenar ni borrar campos existentes.
   - `deprecated` para campos obsoletos (se ignoran al leer).
   - Unions para polimorfismo (componentes de tipos variables).

   PREFABS EN FLATBUFFERS:
   -----------------------
   Prefab = tabla con componentes por defecto.
   Instancia = referencia a prefab + overrides (solo campos que cambian).
   - Ahorra espacio: miles de instancias comparten datos base.
   - Override: flatbuffer con campos presentes = sobrescribe; ausentes = usa default.

   SCENE DIFF (revisado con flatbuffers):
   --------------------------------------
   - Comparar buffers flatbuffers byte-a-byte (rápido, pero no semántico).
   - Diff semántico: iterar tablas, comparar campos por ID.
   - Output: lista de operaciones (Add/Remove/Modify) aplicables al buffer destino.

   ESTA LECCIÓN: Schema flatbuffers para ECS, codegen, serialization/deserialization,
   prefab instantiation con overrides, scene diff algorithm.
   ============================================================================= */
#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include <memory>
#include <functional>
#include <cstdint>
#include <cassert>
#include <algorithm>
#include <fstream>

// ============================================================================
// FLATBUFFERS MINI-IMPLEMENTATION (educativa; en real: #include <flatbuffers/flatbuffers.h>)
// ============================================================================
/*
// Schema ejemplo (schema.fbs):
namespace MyGame;

table Transform {
  x:float = 0; y:float = 0; rot:float = 0; sx:float = 1; sy:float = 1;
}

table Velocity {
  vx:float = 0; vy:float = 0;
}

table Sprite {
  tex_id:uint32 = 0; tex_path:string;
}

table Health {
  hp:int32 = 100; max:int32 = 100;
}

table Tag {
  name:string;
}

table Entity {
  id:uint32;
  parent:uint32 = 0;
  transform:Transform;
  velocity:Velocity;
  sprite:Sprite;
  health:Health;
  tag:Tag;
}

table Scene {
  entities:[Entity];
  prefabs:[Prefab];
}

table Prefab {
  name:string;
  entity:Entity;  // Plantilla
}

root_type Scene;
*/

// En real: flatc genera esto. Aquí simulamos la API.
namespace flatbuffers {
    class FlatBufferBuilder {
        std::vector<uint8_t> buf_;
    public:
        FlatBufferBuilder(size_t initial_size = 1024) { buf_.reserve(initial_size); }
        uint8_t* GetBufferPointer() { return buf_.data(); }
        size_t GetSize() const { return buf_.size(); }
        std::vector<uint8_t> Release() { return std::move(buf_); }
        void Finish(size_t root) { /* root offset */ }
        template<typename T> size_t CreateStruct(const T&) { return 0; }
        size_t CreateString(const char* s) { return 0; }
        template<typename T> size_t CreateVector(const std::vector<T>&) { return 0; }
    };

    template<typename T> struct Vector { const T* data() const { return nullptr; } size_t size() const { return 0; } };
    struct String { const char* c_str() const { return ""; } size_t size() const { return 0; } };
    struct Verifier { Verifier(const uint8_t*, size_t) {} bool Verify() { return true; } };
}

// ============================================================================
// ECS TIPOS (para serializar)
// ============================================================================
using Entity = uint32_t;
struct Transform { float x=0,y=0,rot=0,sx=1,sy=1; };
struct Velocity { float vx=0,vy=0; };
struct Sprite { uint32_t tex_id=0; std::string tex_path; };
struct Health { int32_t hp=100, max=100; };
struct Tag { std::string name; };

// ============================================================================
// SERIALIZACIÓN MANUAL ESTILO FLATBUFFERS (zero-copy concept)
// ============================================================================
// En real: usar flatbuffers::FlatBufferBuilder y generated code.
// Aquí: demostramos el patrón de escritura/lectura sin dependencias.

struct BlobWriter {
    std::vector<uint8_t> data;
    template<typename T> void Write(const T& v) {
        size_t off = data.size(); data.resize(off + sizeof(T));
        std::memcpy(data.data() + off, &v, sizeof(T));
    }
    void WriteString(const std::string& s) {
        Write<uint32_t>(s.size());
        data.insert(data.end(), s.begin(), s.end());
    }
    template<typename T> void WriteVector(const std::vector<T>& v) {
        Write<uint32_t>(v.size());
        for(const auto& e : v) Write(e);
    }
    const uint8_t* Data() const { return data.data(); }
    size_t Size() const { return data.size(); }
};

struct BlobReader {
    const uint8_t* data; size_t size, pos = 0;
    BlobReader(const uint8_t* d, size_t s) : data(d), size(s) {}
    template<typename T> bool Read(T& v) {
        if(pos + sizeof(T) > size) return false;
        std::memcpy(&v, data + pos, sizeof(T)); pos += sizeof(T); return true;
    }
    bool ReadString(std::string& s) {
        uint32_t len; if(!Read(len)) return false;
        if(pos + len > size) return false;
        s.assign(reinterpret_cast<const char*>(data + pos), len); pos += len; return true;
    }
    template<typename T> bool ReadVector(std::vector<T>& v) {
        uint32_t count; if(!Read(count)) return false;
        v.resize(count);
        for(uint32_t i=0;i<count;++i) if(!Read(v[i])) return false;
        return true;
    }
    bool Skip(size_t bytes) { if(pos + bytes > size) return false; pos += bytes; return true; }
};

// ============================================================================
// SERIALIZAR COMPONENTES
// ============================================================================
void SerializeTransform(BlobWriter& w, const Transform& t) {
    w.Write(t.x); w.Write(t.y); w.Write(t.rot); w.Write(t.sx); w.Write(t.sy);
}
void SerializeVelocity(BlobWriter& w, const Velocity& v) { w.Write(v.vx); w.Write(v.vy); }
void SerializeSprite(BlobWriter& w, const Sprite& s) { w.Write(s.tex_id); w.WriteString(s.tex_path); }
void SerializeHealth(BlobWriter& w, const Health& h) { w.Write(h.hp); w.Write(h.max); }
void SerializeTag(BlobWriter& w, const Tag& t) { w.WriteString(t.name); }

// Component type IDs (deben coincidir en reader)
enum CompType : uint8_t { CT_Transform=1, CT_Velocity=2, CT_Sprite=3, CT_Health=4, CT_Tag=5 };

void SerializeEntity(BlobWriter& w, Entity id, Entity parent,
                     const Transform* t, const Velocity* v,
                     const Sprite* s, const Health* h, const Tag* tg) {
    w.Write(id); w.Write(parent);
    // Component mask (bitmask de qué componentes presentes)
    uint8_t mask = 0;
    if(t) mask |= 1<<CT_Transform;
    if(v) mask |= 1<<CT_Velocity;
    if(s) mask |= 1<<CT_Sprite;
    if(h) mask |= 1<<CT_Health;
    if(tg) mask |= 1<<CT_Tag;
    w.Write(mask);
    if(t) SerializeTransform(w, *t);
    if(v) SerializeVelocity(w, *v);
    if(s) SerializeSprite(w, *s);
    if(h) SerializeHealth(w, *h);
    if(tg) SerializeTag(w, *tg);
}

// ============================================================================
// DESERIALIZACIÓN
// ============================================================================
bool DeserializeTransform(BlobReader& r, Transform& t) {
    return r.Read(t.x) && r.Read(t.y) && r.Read(t.rot) && r.Read(t.sx) && r.Read(t.sy);
}
bool DeserializeVelocity(BlobReader& r, Velocity& v) { return r.Read(v.vx) && r.Read(v.vy); }
bool DeserializeSprite(BlobReader& r, Sprite& s) { return r.Read(s.tex_id) && r.ReadString(s.tex_path); }
bool DeserializeHealth(BlobReader& r, Health& h) { return r.Read(h.hp) && r.Read(h.max); }
bool DeserializeTag(BlobReader& r, Tag& t) { return r.ReadString(t.name); }

struct DeserializedEntity {
    Entity id = 0, parent = 0;
    std::optional<Transform> transform;
    std::optional<Velocity> velocity;
    std::optional<Sprite> sprite;
    std::optional<Health> health;
    std::optional<Tag> tag;
};

bool DeserializeEntity(BlobReader& r, DeserializedEntity& out) {
    if(!r.Read(out.id)) return false;
    if(!r.Read(out.parent)) return false;
    uint8_t mask; if(!r.Read(mask)) return false;
    if(mask & (1<<CT_Transform)) { Transform t; if(!DeserializeTransform(r, t)) return false; out.transform = t; }
    if(mask & (1<<CT_Velocity)) { Velocity v; if(!DeserializeVelocity(r, v)) return false; out.velocity = v; }
    if(mask & (1<<CT_Sprite)) { Sprite s; if(!DeserializeSprite(r, s)) return false; out.sprite = s; }
    if(mask & (1<<CT_Health)) { Health h; if(!DeserializeHealth(r, h)) return false; out.health = h; }
    if(mask & (1<<CT_Tag)) { Tag t; if(!DeserializeTag(r, t)) return false; out.tag = t; }
    return true;
}

// ============================================================================
// PREFAB SYSTEM CON OVERRIDES
// ============================================================================
struct Prefab {
    std::string name;
    DeserializedEntity template_entity;  // Valores por defecto
};

struct PrefabInstance {
    std::string prefab_name;
    Entity entity_id;
    DeserializedEntity overrides;  // Solo campos que difieren del prefab
};

class PrefabManager {
    std::unordered_map<std::string, Prefab> prefabs_;

public:
    void RegisterPrefab(const std::string& name, const DeserializedEntity& tmpl) {
        prefabs_[name] = {name, tmpl};
    }

    // Aplicar prefab + overrides → entidad completa
    DeserializedEntity Instantiate(const PrefabInstance& inst) const {
        auto it = prefabs_.find(inst.prefab_name);
        if(it == prefabs_.end()) return {};

        DeserializedEntity result = it->second.template_entity;
        result.id = inst.entity_id;  // Nuevo ID único

        // Merge overrides (campos presentes en override sobrescriben)
        if(inst.overrides.transform) result.transform = inst.overrides.transform;
        if(inst.overrides.velocity) result.velocity = inst.overrides.velocity;
        if(inst.overrides.sprite) result.sprite = inst.overrides.sprite;
        if(inst.overrides.health) result.health = inst.overrides.health;
        if(inst.overrides.tag) result.tag = inst.overrides.tag;
        if(inst.overrides.parent != 0) result.parent = inst.overrides.parent;  // 0 = no override

        return result;
    }

    // Serializar prefabs a blob
    void SerializePrefabs(BlobWriter& w) const {
        w.Write<uint32_t>(prefabs_.size());
        for(const auto& [name, prefab] : prefabs_){
            w.WriteString(name);
            // Serializar template entity
            SerializeEntity(w, 0, 0,
                prefab.template_entity.transform ? &*prefab.template_entity.transform : nullptr,
                prefab.template_entity.velocity ? &*prefab.template_entity.velocity : nullptr,
                prefab.template_entity.sprite ? &*prefab.template_entity.sprite : nullptr,
                prefab.template_entity.health ? &*prefab.template_entity.health : nullptr,
                prefab.template_entity.tag ? &*prefab.template_entity.tag : nullptr);
        }
    }

    bool DeserializePrefabs(BlobReader& r) {
        uint32_t count; if(!r.Read(count)) return false;
        for(uint32_t i=0;i<count;++i){
            std::string name; if(!r.ReadString(name)) return false;
            DeserializedEntity tmpl; if(!DeserializeEntity(r, tmpl)) return false;
            prefabs_[name] = {name, tmpl};
        }
        return true;
    }
};

// ============================================================================
// SCENE DIFF (semántico)
// ============================================================================
enum class DiffOpType { AddEntity, RemoveEntity, ModifyComponent, ChangeParent };

struct DiffOp {
    DiffOpType type;
    Entity entity_id;
    uint8_t component_type;  // CompType
    std::vector<uint8_t> old_data;  // Serializado
    std::vector<uint8_t> new_data;
    Entity old_parent = 0, new_parent = 0;
};

class SceneDiffer {
public:
    // Comparar dos escenas (vectores de entidades deserializadas)
    static std::vector<DiffOp> ComputeDiff(
        const std::vector<DeserializedEntity>& old_scene,
        const std::vector<DeserializedEntity>& new_scene) {

        std::unordered_map<Entity, const DeserializedEntity*> old_map, new_map;
        for(const auto& e : old_scene) old_map[e.id] = &e;
        for(const auto& e : new_scene) new_map[e.id] = &e;

        std::vector<DiffOp> diffs;

        // Entidades en new pero no en old → Add
        for(const auto& e : new_scene){
            if(!old_map.count(e.id)){
                DiffOp op; op.type = DiffOpType::AddEntity; op.entity_id = e.id;
                // Serializar entidad completa como new_data
                BlobWriter w; SerializeEntity(w, e.id, e.parent,
                    e.transform?&*e.transform:nullptr, e.velocity?&*e.velocity:nullptr,
                    e.sprite?&*e.sprite:nullptr, e.health?&*e.health:nullptr, e.tag?&*e.tag:nullptr);
                op.new_data.assign(w.Data(), w.Data() + w.Size());
                diffs.push_back(op);
            }
        }

        // Entidades en old pero no en new → Remove
        for(const auto& e : old_scene){
            if(!new_map.count(e.id)){
                diffs.push_back({DiffOpType::RemoveEntity, e.id, 0, {}, {}});
            }
        }

        // Entidades en ambas → comparar componentes
        for(const auto& e : old_scene){
            auto it = new_map.find(e.id);
            if(it == new_map.end()) continue;
            const DeserializedEntity* ne = it->second;

            auto compare_opt = [&](const auto& old_opt, const auto& new_opt, uint8_t ct){
                bool old_has = old_opt.has_value();
                bool new_has = new_opt.has_value();
                if(old_has && new_has){
                    // Comparar serializado
                    BlobWriter ow, nw;
                    SerializeComponent(ow, *old_opt, ct);
                    SerializeComponent(nw, *new_opt, ct);
                    if(std::string_view((char*)ow.Data(), ow.Size()) !=
                       std::string_view((char*)nw.Data(), nw.Size())){
                        DiffOp op; op.type = DiffOpType::ModifyComponent;
                        op.entity_id = e.id; op.component_type = ct;
                        op.old_data.assign(ow.Data(), ow.Data()+ow.Size());
                        op.new_data.assign(nw.Data(), nw.Data()+nw.Size());
                        diffs.push_back(op);
                    }
                }else if(old_has && !new_has){
                    BlobWriter ow; SerializeComponent(ow, *old_opt, ct);
                    diffs.push_back({DiffOpType::ModifyComponent, e.id, ct,
                                     {ow.Data(), ow.Data()+ow.Size()}, {}});
                }else if(!old_has && new_has){
                    BlobWriter nw; SerializeComponent(nw, *new_opt, ct);
                    diffs.push_back({DiffOpType::ModifyComponent, e.id, ct,
                                     {}, {nw.Data(), nw.Data()+nw.Size()}});
                }
            };

            compare_opt(e.transform, ne->transform, CT_Transform);
            compare_opt(e.velocity, ne->velocity, CT_Velocity);
            compare_opt(e.sprite, ne->sprite, CT_Sprite);
            compare_opt(e.health, ne->health, CT_Health);
            compare_opt(e.tag, ne->tag, CT_Tag);

            if(e.parent != ne->parent){
                diffs.push_back({DiffOpType::ChangeParent, e.id, 0, {}, {}, e.parent, ne->parent});
            }
        }
        return diffs;
    }

private:
    template<typename T>
    static void SerializeComponent(BlobWriter& w, const T& v, uint8_t ct) {
        if constexpr(std::is_same_v<T, Transform>) SerializeTransform(w, v);
        else if constexpr(std::is_same_v<T, Velocity>) SerializeVelocity(w, v);
        else if constexpr(std::is_same_v<T, Sprite>) SerializeSprite(w, v);
        else if constexpr(std::is_same_v<T, Health>) SerializeHealth(w, v);
        else if constexpr(std::is_same_v<T, Tag>) SerializeTag(w, v);
    }
};

// ============================================================================
// VERIFICACIÓN DE SINTAXIS
// ============================================================================
int main() {
    std::cout << "=== LECCIÓN 12: Serialización Flatbuffers/Prefabs/Diff (solo sintaxis) ===\n";

    // Crear escena
    std::vector<DeserializedEntity> scene;
    DeserializedEntity e1; e1.id = 1; e1.parent = 0;
    e1.transform = Transform{100, 100, 0, 2, 2};
    e1.tag = Tag{"Player"};
    e1.health = Health{100, 100};
    scene.push_back(e1);

    DeserializedEntity e2; e2.id = 2; e2.parent = 1;
    e2.transform = Transform{50, 0, 0, 1, 1};
    e2.sprite = Sprite{1, "gun.png"};
    scene.push_back(e2);

    // Serializar
    BlobWriter writer;
    writer.Write<uint32_t>(scene.size());
    for(const auto& e : scene){
        SerializeEntity(writer, e.id, e.parent,
            e.transform?&*e.transform:nullptr, e.velocity?&*e.velocity:nullptr,
            e.sprite?&*e.sprite:nullptr, e.health?&*e.health:nullptr, e.tag?&*e.tag:nullptr);
    }
    std::cout << "Serialized scene: " << writer.Size() << " bytes\n";

    // Deserializar
    BlobReader reader(writer.Data(), writer.Size());
    uint32_t count; reader.Read(count);
    std::vector<DeserializedEntity> loaded;
    for(uint32_t i=0;i<count;++i){
        DeserializedEntity e; DeserializeEntity(reader, e); loaded.push_back(e);
    }
    std::cout << "Deserialized " << loaded.size() << " entities\n";

    // Prefabs
    PrefabManager pm;
    pm.RegisterPrefab("Enemy", scene[0]);  // Usar player como template
    PrefabInstance inst{"Enemy", 99, {}};
    inst.overrides.transform = Transform{0, 0, 0, 1, 1};
    inst.overrides.tag = Tag{"Enemy_1"};
    auto instantiated = pm.Instantiate(inst);
    std::cout << "Instantiated Enemy at ID " << instantiated.id
              << " pos=(" << instantiated.transform->x << "," << instantiated.transform->y << ")\n";

    // Diff
    std::vector<DeserializedEntity> new_scene = loaded;
    new_scene[0].transform->x = 999;  // Cambio
    DeserializedEntity e3; e3.id = 3; e3.transform = Transform{0,0}; new_scene.push_back(e3);

    auto diffs = SceneDiffer::ComputeDiff(loaded, new_scene);
    std::cout << "Diffs: " << diffs.size() << "\n";
    for(const auto& d : diffs){
        std::cout << "  Op: " << (int)d.type << " Entity: " << d.entity_id
                  << " Comp: " << (int)d.component_type << "\n";
    }

    std::cout << "=== FIN LECCIÓN 12 ===\n";
    return 0;
}