/* =============================================================================
   MÓDULO 22 - EJERCICIO 08: Serialización — Flatbuffers, Prefabs, Scene Diff
   =============================================================================
   DIFICULTAD: Medio-Alto
   APOYO: Lección 12 (Serialización Flatbuffers/Prefabs/Diff)

   CONSIGNA:
   Implementa serialización binaria estilo flatbuffers:
   - BlobWriter/BlobReader para componentes POD
   - SerializeEntity/DeserializeEntity con component mask
   - PrefabManager: RegisterPrefab, Instantiate con overrides
   - SceneDiffer: ComputeDiff(old_scene, new_scene) → lista de ops

   TESTS:
   - Round-trip serialización/deserialización preserva datos
   - Prefab template + overrides produce entidad correcta
   - Scene diff detecta added/removed/modified entities
   - Component-level diff (solo componentes que cambiaron)
   ============================================================================= */
#include <vector>
#include <unordered_map>
#include <string>
#include <optional>
#include <cstdint>
#include <cassert>
#include <algorithm>
#include <cstring>
#include "../../include/test_framework.hpp"

using Entity = uint32_t;

// ============================================================================
// BLOB WRITER/READER (estilo flatbuffers simplificado)
// ============================================================================
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
};

// ============================================================================
// COMPONENTES
// ============================================================================
struct Transform { float x=0,y=0,rot=0,sx=1,sy=1; };
struct Velocity { float vx=0,vy=0; };
struct Sprite { uint32_t tex_id=0; std::string tex_path; };
struct Health { int32_t hp=100, max=100; };
struct Tag { std::string name; };

enum CompType : uint8_t { CT_Transform=1, CT_Velocity=2, CT_Sprite=3, CT_Health=4, CT_Tag=5 };

// ============================================================================
// TODO: Serialización de componentes
// ============================================================================
void SerializeTransform(BlobWriter& w, const Transform& t) { w.Write(t.x); w.Write(t.y); w.Write(t.rot); w.Write(t.sx); w.Write(t.sy); }
void SerializeVelocity(BlobWriter& w, const Velocity& v) { w.Write(v.vx); w.Write(v.vy); }
void SerializeSprite(BlobWriter& w, const Sprite& s) { w.Write(s.tex_id); w.WriteString(s.tex_path); }
void SerializeHealth(BlobWriter& w, const Health& h) { w.Write(h.hp); w.Write(h.max); }
void SerializeTag(BlobWriter& w, const Tag& t) { w.WriteString(t.name); }

bool DeserializeTransform(BlobReader& r, Transform& t) { return r.Read(t.x) && r.Read(t.y) && r.Read(t.rot) && r.Read(t.sx) && r.Read(t.sy); }
bool DeserializeVelocity(BlobReader& r, Velocity& v) { return r.Read(v.vx) && r.Read(v.vy); }
bool DeserializeSprite(BlobReader& r, Sprite& s) { return r.Read(s.tex_id) && r.ReadString(s.tex_path); }
bool DeserializeHealth(BlobReader& r, Health& h) { return r.Read(h.hp) && r.Read(h.max); }
bool DeserializeTag(BlobReader& r, Tag& t) { return r.ReadString(t.name); }

// ============================================================================
// ENTITY SERIALIZATION
// ============================================================================
struct DeserializedEntity {
    Entity id = 0, parent = 0;
    std::optional<Transform> transform;
    std::optional<Velocity> velocity;
    std::optional<Sprite> sprite;
    std::optional<Health> health;
    std::optional<Tag> tag;
};

// TODO: Implementa SerializeEntity y DeserializeEntity
void SerializeEntity(BlobWriter& w, Entity id, Entity parent,
                     const Transform* t, const Velocity* v,
                     const Sprite* s, const Health* h, const Tag* tg) {
    // TU CÓDIGO AQUÍ:
    // 1. Write id, parent
    // 2. Compute component mask (bitmask)
    // 3. Write mask
    // 4. Para cada bit set, serializar componente correspondiente
    (void)w; (void)id; (void)parent; (void)t; (void)v; (void)s; (void)h; (void)tg;
}

bool DeserializeEntity(BlobReader& r, DeserializedEntity& out) {
    // TU CÓDIGO AQUÍ:
    // 1. Read id, parent
    // 2. Read mask
    // 3. Para cada bit set, deserializar componente
    (void)r; (void)out; return false;
}

// ============================================================================
// PREFAB MANAGER
// ============================================================================
struct Prefab {
    std::string name;
    DeserializedEntity template_entity;
};

struct PrefabInstance {
    std::string prefab_name;
    Entity entity_id;
    DeserializedEntity overrides;
};

class PrefabManager {
    std::unordered_map<std::string, Prefab> prefabs_;

public:
    // TU CÓDIGO AQUÍ: RegisterPrefab, Instantiate, SerializePrefabs, DeserializePrefabs
    void RegisterPrefab(const std::string& name, const DeserializedEntity& tmpl) {
        (void)name; (void)tmpl;
    }

    DeserializedEntity Instantiate(const PrefabInstance& inst) const {
        // TU CÓDIGO AQUÍ: merge template + overrides
        (void)inst; return {};
    }

    void SerializePrefabs(BlobWriter& w) const { (void)w; }
    bool DeserializePrefabs(BlobReader& r) { (void)r; return false; }
};

// ============================================================================
// SCENE DIFF
// ============================================================================
enum class DiffOpType { AddEntity, RemoveEntity, ModifyComponent, ChangeParent };

inline std::ostream& operator<<(std::ostream& os, DiffOpType t) {
    switch(t) {
        case DiffOpType::AddEntity: return os << "AddEntity";
        case DiffOpType::RemoveEntity: return os << "RemoveEntity";
        case DiffOpType::ModifyComponent: return os << "ModifyComponent";
        case DiffOpType::ChangeParent: return os << "ChangeParent";
    }
    return os << "Unknown";
}

struct DiffOp {
    DiffOpType type;
    Entity entity_id;
    uint8_t component_type = 0;
    std::vector<uint8_t> old_data, new_data;
    Entity old_parent = 0, new_parent = 0;
};

class SceneDiffer {
public:
    // TU CÓDIGO AQUÍ: ComputeDiff(old_scene, new_scene)
    static std::vector<DiffOp> ComputeDiff(
        const std::vector<DeserializedEntity>& old_scene,
        const std::vector<DeserializedEntity>& new_scene) {
        (void)old_scene; (void)new_scene; return {};
    }
};

// ============================================================================
// TESTS
// ============================================================================
TEST_CASE(EntitySerializeDeserializeRoundTrip) {
    BlobWriter w;
    Transform t{10.5f, 20.25f, 0.785f, 2.0f, 1.5f};
    Velocity v{100.0f, -50.0f};
    Sprite s{42, "player.png"};
    Health h{75, 100};
    Tag tg{"Player"};

    SerializeEntity(w, 1, 0, &t, &v, &s, &h, &tg);

    BlobReader r(w.Data(), w.Size());
    DeserializedEntity e;
    ASSERT_TRUE(DeserializeEntity(r, e));

    ASSERT_EQ(e.id, 1);
    ASSERT_TRUE(e.transform.has_value());
    ASSERT_NEAR(e.transform->x, 10.5f, 0.001f);
    ASSERT_NEAR(e.transform->rot, 0.785f, 0.001f);
    ASSERT_TRUE(e.velocity.has_value());
    ASSERT_NEAR(e.velocity->vx, 100.0f, 0.001f);
    ASSERT_TRUE(e.sprite.has_value());
    ASSERT_EQ(e.sprite->tex_id, 42);
    ASSERT_EQ(e.sprite->tex_path, "player.png");
    ASSERT_TRUE(e.health.has_value());
    ASSERT_EQ(e.health->hp, 75);
    ASSERT_TRUE(e.tag.has_value());
    ASSERT_EQ(e.tag->name, "Player");
}

TEST_CASE(EntityWithoutOptionalComponents) {
    BlobWriter w;
    Transform t{5, 5};
    SerializeEntity(w, 2, 0, &t, nullptr, nullptr, nullptr, nullptr);

    BlobReader r(w.Data(), w.Size());
    DeserializedEntity e;
    ASSERT_TRUE(DeserializeEntity(r, e));

    ASSERT_EQ(e.id, 2);
    ASSERT_TRUE(e.transform.has_value());
    ASSERT_FALSE(e.velocity.has_value());
    ASSERT_FALSE(e.sprite.has_value());
    ASSERT_FALSE(e.health.has_value());
    ASSERT_FALSE(e.tag.has_value());
}

TEST_CASE(PrefabTemplateWithOverrides) {
    PrefabManager pm;
    DeserializedEntity tmpl;
    tmpl.transform = Transform{0, 0, 0, 1, 1};
    tmpl.sprite = Sprite{1, "enemy.png"};
    tmpl.health = Health{100, 100};
    tmpl.tag = Tag{"Enemy"};
    pm.RegisterPrefab("Enemy", tmpl);

    PrefabInstance inst{"Enemy", 99, {}};
    inst.overrides.transform = Transform{100, 200, 0, 1, 1};
    inst.overrides.tag = Tag{"Boss"};

    auto result = pm.Instantiate(inst);

    ASSERT_EQ(result.id, 99);
    ASSERT_TRUE(result.transform.has_value());
    ASSERT_NEAR(result.transform->x, 100.0f, 0.001f);
    ASSERT_NEAR(result.transform->y, 200.0f, 0.001f);
    // Sprite y health vienen del template
    ASSERT_TRUE(result.sprite.has_value());
    ASSERT_EQ(result.sprite->tex_id, 1);
    ASSERT_TRUE(result.health.has_value());
    ASSERT_EQ(result.health->hp, 100);
    // Tag sobrescrito
    ASSERT_TRUE(result.tag.has_value());
    ASSERT_EQ(result.tag->name, "Boss");
}

TEST_CASE(PrefabWithoutOverrideUsesTemplate) {
    PrefabManager pm;
    DeserializedEntity tmpl;
    tmpl.transform = Transform{10, 10};
    tmpl.health = Health{50, 50};
    pm.RegisterPrefab("Projectile", tmpl);

    PrefabInstance inst{"Projectile", 1, {}};  // Sin overrides
    auto result = pm.Instantiate(inst);

    ASSERT_TRUE(result.transform.has_value());
    ASSERT_NEAR(result.transform->x, 10.0f, 0.001f);
    ASSERT_TRUE(result.health.has_value());
    ASSERT_EQ(result.health->hp, 50);
}

TEST_CASE(SceneDiffDetectsAddedEntity) {
    std::vector<DeserializedEntity> old_scene;
    std::vector<DeserializedEntity> new_scene;
    DeserializedEntity e; e.id = 1; e.transform = Transform{0,0};
    new_scene.push_back(e);

    auto diffs = SceneDiffer::ComputeDiff(old_scene, new_scene);
    ASSERT_EQ(diffs.size(), 1);
    ASSERT_EQ(diffs[0].type, DiffOpType::AddEntity);
    ASSERT_EQ(diffs[0].entity_id, 1);
}

TEST_CASE(SceneDiffDetectsRemovedEntity) {
    std::vector<DeserializedEntity> old_scene, new_scene;
    DeserializedEntity e; e.id = 1; e.transform = Transform{0,0};
    old_scene.push_back(e);

    auto diffs = SceneDiffer::ComputeDiff(old_scene, new_scene);
    ASSERT_EQ(diffs.size(), 1);
    ASSERT_EQ(diffs[0].type, DiffOpType::RemoveEntity);
    ASSERT_EQ(diffs[0].entity_id, 1);
}

TEST_CASE(SceneDiffDetectsModifiedComponent) {
    std::vector<DeserializedEntity> old_scene, new_scene;
    DeserializedEntity e1; e1.id = 1; e1.transform = Transform{0, 0};
    DeserializedEntity e2; e2.id = 1; e2.transform = Transform{100, 100};
    old_scene.push_back(e1); new_scene.push_back(e2);

    auto diffs = SceneDiffer::ComputeDiff(old_scene, new_scene);
    ASSERT_EQ(diffs.size(), 1);
    ASSERT_EQ(diffs[0].type, DiffOpType::ModifyComponent);
    ASSERT_EQ(diffs[0].entity_id, 1);
    ASSERT_EQ(diffs[0].component_type, CT_Transform);
}

TEST_CASE(SceneDiffDetectsParentChange) {
    std::vector<DeserializedEntity> old_scene, new_scene;
    DeserializedEntity e1; e1.id = 1; e1.parent = 0; e1.transform = Transform{0,0};
    DeserializedEntity e2; e2.id = 1; e2.parent = 5; e2.transform = Transform{0,0};
    old_scene.push_back(e1); new_scene.push_back(e2);

    auto diffs = SceneDiffer::ComputeDiff(old_scene, new_scene);
    ASSERT_EQ(diffs.size(), 1);
    ASSERT_EQ(diffs[0].type, DiffOpType::ChangeParent);
    ASSERT_EQ(diffs[0].old_parent, 0);
    ASSERT_EQ(diffs[0].new_parent, 5);
}

TEST_CASE(SceneDiffMultipleChanges) {
    std::vector<DeserializedEntity> old_scene, new_scene;
    // old: entity 1 (transform), entity 2 (transform)
    DeserializedEntity e1; e1.id = 1; e1.transform = Transform{0,0};
    DeserializedEntity e2; e2.id = 2; e2.transform = Transform{10,10};
    old_scene.push_back(e1); old_scene.push_back(e2);

    // new: entity 1 (transform changed), entity 3 (added), entity 2 removed
    DeserializedEntity n1; n1.id = 1; n1.transform = Transform{100,100};
    DeserializedEntity n3; n3.id = 3; n3.transform = Transform{0,0};
    new_scene.push_back(n1); new_scene.push_back(n3);

    auto diffs = SceneDiffer::ComputeDiff(old_scene, new_scene);
    // Debe detectar: modify entity 1, remove entity 2, add entity 3
    ASSERT_EQ(diffs.size(), 3);
    int modify=0, remove=0, add=0;
    for(auto& d: diffs){
        if(d.type==DiffOpType::ModifyComponent) modify++;
        else if(d.type==DiffOpType::RemoveEntity) remove++;
        else if(d.type==DiffOpType::AddEntity) add++;
    }
    ASSERT_EQ(modify, 1);
    ASSERT_EQ(remove, 1);
    ASSERT_EQ(add, 1);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 08 (Serialización)...\n";
    return RUN_ALL_TESTS();
}