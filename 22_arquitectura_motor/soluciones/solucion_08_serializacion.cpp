/* =============================================================================
   MÓDULO 22 - SOLUCIÓN 08: Serialización — Flatbuffers, Prefabs, Scene Diff
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

struct Transform { float x=0,y=0,rot=0,sx=1,sy=1; };
struct Velocity { float vx=0,vy=0; };
struct Sprite { uint32_t tex_id=0; std::string tex_path; };
struct Health { int32_t hp=100, max=100; };
struct Tag { std::string name; };

enum CompType : uint8_t { CT_Transform=1, CT_Velocity=2, CT_Sprite=3, CT_Health=4, CT_Tag=5 };

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

struct DeserializedEntity {
    Entity id = 0, parent = 0;
    std::optional<Transform> transform;
    std::optional<Velocity> velocity;
    std::optional<Sprite> sprite;
    std::optional<Health> health;
    std::optional<Tag> tag;
};

void SerializeEntity(BlobWriter& w, Entity id, Entity parent,
                     const Transform* t, const Velocity* v,
                     const Sprite* s, const Health* h, const Tag* tg) {
    w.Write(id); w.Write(parent);
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
    void RegisterPrefab(const std::string& name, const DeserializedEntity& tmpl) {
        prefabs_[name] = {name, tmpl};
    }

    DeserializedEntity Instantiate(const PrefabInstance& inst) const {
        auto it = prefabs_.find(inst.prefab_name);
        if(it == prefabs_.end()) return {};

        DeserializedEntity result = it->second.template_entity;
        result.id = inst.entity_id;

        if(inst.overrides.transform) result.transform = inst.overrides.transform;
        if(inst.overrides.velocity) result.velocity = inst.overrides.velocity;
        if(inst.overrides.sprite) result.sprite = inst.overrides.sprite;
        if(inst.overrides.health) result.health = inst.overrides.health;
        if(inst.overrides.tag) result.tag = inst.overrides.tag;
        if(inst.overrides.parent != 0) result.parent = inst.overrides.parent;

        return result;
    }

    void SerializePrefabs(BlobWriter& w) const {
        w.Write<uint32_t>(prefabs_.size());
        for(const auto& [name, prefab] : prefabs_){
            w.WriteString(name);
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
    static void SerializeComponent(BlobWriter& w, const std::optional<Transform>& o) { if(o) SerializeTransform(w, *o); }
    static void SerializeComponent(BlobWriter& w, const std::optional<Velocity>& o) { if(o) SerializeVelocity(w, *o); }
    static void SerializeComponent(BlobWriter& w, const std::optional<Sprite>& o) { if(o) SerializeSprite(w, *o); }
    static void SerializeComponent(BlobWriter& w, const std::optional<Health>& o) { if(o) SerializeHealth(w, *o); }
    static void SerializeComponent(BlobWriter& w, const std::optional<Tag>& o) { if(o) SerializeTag(w, *o); }

    static bool CompareOptional(const auto& a, const auto& b) {
        if(a.has_value() && b.has_value()){
            BlobWriter wa, wb;
            SerializeComponent(wa, a);
            SerializeComponent(wb, b);
            return wa.Size() == wb.Size() && std::memcmp(wa.Data(), wb.Data(), wa.Size()) == 0;
        }
        return a.has_value() == b.has_value();
    }

public:
    static std::vector<DiffOp> ComputeDiff(
        const std::vector<DeserializedEntity>& old_scene,
        const std::vector<DeserializedEntity>& new_scene) {

        std::unordered_map<Entity, const DeserializedEntity*> old_map, new_map;
        for(const auto& e : old_scene) old_map[e.id] = &e;
        for(const auto& e : new_scene) new_map[e.id] = &e;

        std::vector<DiffOp> diffs;

        for(const auto& e : new_scene){
            if(!old_map.count(e.id)){
                DiffOp op; op.type = DiffOpType::AddEntity; op.entity_id = e.id;
                BlobWriter w; SerializeEntity(w, e.id, e.parent,
                    e.transform?&*e.transform:nullptr, e.velocity?&*e.velocity:nullptr,
                    e.sprite?&*e.sprite:nullptr, e.health?&*e.health:nullptr, e.tag?&*e.tag:nullptr);
                op.new_data.assign(w.Data(), w.Data() + w.Size());
                diffs.push_back(op);
            }
        }

        for(const auto& e : old_scene){
            if(!new_map.count(e.id)){
                diffs.push_back({DiffOpType::RemoveEntity, e.id, 0, {}, {}});
            }
        }

        for(const auto& e : old_scene){
            auto it = new_map.find(e.id);
            if(it == new_map.end()) continue;
            const DeserializedEntity* ne = it->second;

            auto check = [&](const auto& old_o, const auto& new_o, uint8_t ct){
                bool old_has = old_o.has_value();
                bool new_has = new_o.has_value();
                if(old_has && new_has){
                    BlobWriter ow, nw;
                    SerializeComponent(ow, old_o);
                    SerializeComponent(nw, new_o);
                    if(!CompareOptional(old_o, new_o)){
                        diffs.push_back({DiffOpType::ModifyComponent, e.id, ct,
                                         {ow.Data(), ow.Data()+ow.Size()},
                                         {nw.Data(), nw.Data()+nw.Size()}});
                    }
                }else if(old_has && !new_has){
                    BlobWriter ow; SerializeComponent(ow, old_o);
                    diffs.push_back({DiffOpType::ModifyComponent, e.id, ct,
                                     {ow.Data(), ow.Data()+ow.Size()}, {}});
                }else if(!old_has && new_has){
                    BlobWriter nw; SerializeComponent(nw, new_o);
                    diffs.push_back({DiffOpType::ModifyComponent, e.id, ct,
                                     {}, {nw.Data(), nw.Data()+nw.Size()}});
                }
            };

            check(e.transform, ne->transform, CT_Transform);
            check(e.velocity, ne->velocity, CT_Velocity);
            check(e.sprite, ne->sprite, CT_Sprite);
            check(e.health, ne->health, CT_Health);
            check(e.tag, ne->tag, CT_Tag);

            if(e.parent != ne->parent){
                diffs.push_back({DiffOpType::ChangeParent, e.id, 0, {}, {}, e.parent, ne->parent});
            }
        }
        return diffs;
    }
};

// TESTS
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
    ASSERT_TRUE(result.sprite.has_value());
    ASSERT_EQ(result.sprite->tex_id, 1);
    ASSERT_TRUE(result.health.has_value());
    ASSERT_EQ(result.health->hp, 100);
    ASSERT_TRUE(result.tag.has_value());
    ASSERT_EQ(result.tag->name, "Boss");
}

TEST_CASE(PrefabWithoutOverrideUsesTemplate) {
    PrefabManager pm;
    DeserializedEntity tmpl;
    tmpl.transform = Transform{10, 10};
    tmpl.health = Health{50, 50};
    pm.RegisterPrefab("Projectile", tmpl);

    PrefabInstance inst{"Projectile", 1, {}};
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
    DeserializedEntity e1; e1.id = 1; e1.transform = Transform{0,0};
    DeserializedEntity e2; e2.id = 2; e2.transform = Transform{10,10};
    old_scene.push_back(e1); old_scene.push_back(e2);

    DeserializedEntity n1; n1.id = 1; n1.transform = Transform{100,100};
    DeserializedEntity n3; n3.id = 3; n3.transform = Transform{0,0};
    new_scene.push_back(n1); new_scene.push_back(n3);

    auto diffs = SceneDiffer::ComputeDiff(old_scene, new_scene);
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
    std::cout << "Solución 08 (Serialización)...\n";
    return RUN_ALL_TESTS();
}