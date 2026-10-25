/*
 * MÓDULO 28: Estudio de Código Real — Arquitectura de Librerías
 * EJERCICIO 04: Box2D — AABB Tree, SAT, Contact Constraints, Island Building
 *
 * OBJETIVO: Implementar componentes core de Box2D: broad-phase AABB tree,
 * narrow-phase SAT, contact constraint generation y island building.
 */

#include <cstddef>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <cmath>
#include <stack>
#include <queue>
#include "../../include/test_framework.hpp"

// ============================================================================
// TIPOS BASE
// ============================================================================

struct b2Vec2 {
    float x, y;
    constexpr b2Vec2() : x(0), y(0) {}
    constexpr b2Vec2(float x_, float y_) : x(x_), y(y_) {}
    constexpr b2Vec2 operator+(const b2Vec2& v) const { return {x + v.x, y + v.y}; }
    constexpr b2Vec2 operator-(const b2Vec2& v) const { return {x - v.x, y - v.y}; }
    constexpr b2Vec2 operator*(float s) const { return {x * s, y * s}; }
    constexpr b2Vec2 operator/(float s) const { return {x / s, y / s}; }
};

inline float b2Dot(const b2Vec2& a, const b2Vec2& b) { return a.x * b.x + a.y * b.y; }
inline float b2Cross(const b2Vec2& a, const b2Vec2& b) { return a.x * b.y - a.y * b.x; }
inline b2Vec2 b2Cross(float s, const b2Vec2& v) { return {-s * v.y, s * v.x}; }
inline b2Vec2 b2Cross(const b2Vec2& v, float s) { return {s * v.y, -s * v.x}; }
inline float b2Length(const b2Vec2& v) { return std::sqrt(b2Dot(v, v)); }
inline b2Vec2 b2Normalize(const b2Vec2& v) { float l = b2Length(v); return l > 0 ? v / l : b2Vec2(0, 0); }

struct b2AABB {
    b2Vec2 lowerBound, upperBound;
    
    constexpr b2AABB() : lowerBound(0,0), upperBound(0,0) {}
    constexpr b2AABB(const b2Vec2& l, const b2Vec2& u) : lowerBound(l), upperBound(u) {}
    
    bool Contains(const b2AABB& other) const {
        return lowerBound.x <= other.lowerBound.x && lowerBound.y <= other.lowerBound.y &&
               upperBound.x >= other.upperBound.x && upperBound.y >= other.upperBound.y;
    }
    
    bool Overlaps(const b2AABB& other) const {
        return lowerBound.x < other.upperBound.x && upperBound.x > other.lowerBound.x &&
               lowerBound.y < other.upperBound.y && upperBound.y > other.lowerBound.y;
    }
    
    b2Vec2 GetCenter() const { return (lowerBound + upperBound) * 0.5f; }
    b2Vec2 GetExtents() const { return (upperBound - lowerBound) * 0.5f; }
    float GetPerimeter() const { 
        b2Vec2 e = GetExtents(); 
        return 2.0f * (e.x + e.y); 
    }
    
    static b2AABB Combine(const b2AABB& a, const b2AABB& b) {
        b2AABB c;
        c.lowerBound.x = std::min(a.lowerBound.x, b.lowerBound.x);
        c.lowerBound.y = std::min(a.lowerBound.y, b.lowerBound.y);
        c.upperBound.x = std::max(a.upperBound.x, b.upperBound.x);
        c.upperBound.y = std::max(a.upperBound.y, b.upperBound.y);
        return c;
    }
    
    // Expand by margin
    b2AABB Expand(float margin) const {
        return {lowerBound - b2Vec2(margin, margin), upperBound + b2Vec2(margin, margin)};
    }
};

struct b2Rot {
    float s, c;
    constexpr b2Rot() : s(0), c(1) {}
    constexpr b2Rot(float angle) : s(std::sin(angle)), c(std::cos(angle)) {}
    constexpr b2Rot(float s_, float c_) : s(s_), c(c_) {}
    constexpr b2Vec2 operator*(const b2Vec2& v) const { return {c * v.x - s * v.y, s * v.x + c * v.y}; }
};

struct b2Transform {
    b2Vec2 p;
    b2Rot q;
    constexpr b2Transform() : p(0, 0), q(0) {}
    constexpr b2Transform(const b2Vec2& p_, const b2Rot& q_) : p(p_), q(q_) {}
    constexpr b2Vec2 operator*(const b2Vec2& v) const { return p + q * v; }
};

// ============================================================================
// 1. DYNAMIC AABB TREE (BROAD-PHASE)
// ============================================================================

struct b2TreeNode {
    b2AABB aabb;
    void* userData = nullptr;
    int parent = -1;
    int child1 = -1, child2 = -1;
    int height = 0;
    bool IsLeaf() const { return child1 == -1; }
};

class b2DynamicTree {
    std::vector<b2TreeNode> m_nodes;
    int m_root = -1;
    int m_freeList = -1;
    int m_nodeCapacity = 16;
    int m_nodeCount = 0;
    float m_fatMargin = 0.1f;
    
public:
    b2DynamicTree() { m_nodes.reserve(m_nodeCapacity); }
    
    // TODO: AllocateNode
    int AllocateNode() { return -1; }
    
    // TODO: FreeNode
    void FreeNode(int id) {}
    
    // TODO: CreateProxy
    int CreateProxy(const b2AABB& aabb, void* userData) { return -1; }
    
    // TODO: DestroyProxy
    void DestroyProxy(int proxyId) {}
    
    // TODO: MoveProxy - implementar swept AABB para movimiento
    void MoveProxy(int proxyId, const b2AABB& aabb, const b2Vec2& displacement) {
        // 1. Check if already contains new AABB (with margin)
        // 2. RemoveLeaf(proxyId)
        // 3. Compute swept AABB = Combine(oldFatAABB, newFatAABB) expanded by |displacement|
        // 4. Update node.aabb = sweptAABB, node.height = 0
        // 5. InsertLeaf(proxyId)
    }
    
    // TODO: Query
    template<typename Callback>
    void Query(Callback callback, const b2AABB& aabb) const {}
    
    // TODO: RayCast
    template<typename Callback>
    void RayCast(Callback callback, const b2Vec2& p1, const b2Vec2& p2) const {}
    
    // Helpers para tests
    int GetRoot() const { return m_root; }
    const b2TreeNode& GetNode(int id) const { return m_nodes[id]; }
    int GetNodeCount() const { return m_nodeCount; }
};

// ============================================================================
// 2. SAT (SEPARATING AXIS THEOREM) PARA POLÍGONOS CONVEXOS
// ============================================================================

struct b2Polygon {
    std::vector<b2Vec2> vertices; // CCW order
    std::vector<b2Vec2> normals;  // outward normals (unit length)
    int count = 0;
    
    // Build normals from vertices
    void BuildNormals() {
        normals.resize(count);
        for (int i = 0; i < count; ++i) {
            int j = (i + 1) % count;
            b2Vec2 edge = vertices[j] - vertices[i];
            normals[i] = b2Normalize(b2Cross(edge, 1.0f)); // outward normal
        }
    }
};

// TODO: b2SATResult
struct b2SATResult {
    bool hit = false;
    b2Vec2 normal = {0, 0}; // from A to B
    float depth = 0;
    int axisIndex = -1; // which axis gave min penetration
};

// TODO: b2SAT - test collision between two polygons
b2SATResult b2SAT(const b2Polygon& A, const b2Polygon& B) {
    return {false, {0,0}, 0, -1};
}

// ============================================================================
// 3. CONTACT CONSTRAINT GENERATION
// ============================================================================

struct b2ManifoldPoint {
    b2Vec2 localPoint;   // en espacio local del body A
    float normalImpulse = 0;
    float tangentImpulse = 0;
    uint32_t id = 0;     // feature pair para warm starting
};

struct b2Manifold {
    b2Vec2 localNormal;  // en espacio local del body A
    b2Vec2 localPoint;   // centro de contacto en espacio local A
    std::vector<b2ManifoldPoint> points;
    int pointCount = 0;
    int type = 0; // 0=circles, 1=faceA, 2=faceB
};

// TODO: b2ContactConstraint - constraint para velocity solver
struct b2ContactConstraint {
    int indexA = -1, indexB = -1; // índices en velocity arrays
    b2Vec2 normal = {0, 0};       // world space normal (A -> B)
    b2Vec2 tangent = {0, 0};      // world space tangent
    float invMassA = 0, invMassB = 0;
    float invIA = 0, invIB = 0;
    b2Vec2 rA = {0, 0}, rB = {0, 0}; // radio desde COM a punto contacto
    float normalMass = 0;
    float tangentMass = 0;
    float bias = 0;
    float restitution = 0;
    float friction = 0;
    float normalImpulse = 0;
    float tangentImpulse = 0;
};

// TODO: b2BuildContactConstraint - genera constraint desde manifold
b2ContactConstraint b2BuildContactConstraint(
    const b2Manifold& manifold,
    const b2Transform& xfA, const b2Transform& xfB,
    float invMassA, float invMassB,
    float invIA, float invIB,
    float restitution, float friction,
    float dt) {
    return {};
}

// ============================================================================
// 4. ISLAND BUILDING
// ============================================================================

struct b2Body {
    int id = -1;
    b2Vec2 position = {0, 0};
    float angle = 0;
    b2Vec2 linearVelocity = {0, 0};
    float angularVelocity = 0;
    float invMass = 1, invI = 1;
    bool awake = true;
    bool isStatic = false;
};

struct b2Contact {
    int bodyA = -1, bodyB = -1;
    b2Manifold manifold;
    bool touching = false;
};

struct b2Joint {
    int bodyA = -1, bodyB = -1;
    bool collideConnected = false;
};

struct b2Island {
    std::vector<int> bodies;
    std::vector<int> contacts;
    std::vector<int> joints;
};

// TODO: b2BuildIslands - construye islas de bodies conectados
void b2BuildIslands(
    const std::vector<b2Body>& bodies,
    const std::vector<b2Contact>& contacts,
    const std::vector<b2Joint>& joints,
    std::vector<b2Island>& islands) {
    // DFS/BFS desde cada body awake no visitado
    // A través de contacts y joints
    // Solo bodies awake
}

// ============================================================================
// TESTS (NO MODIFICAR)
// ============================================================================

TEST_CASE(TestAABBTreeBasic) {
    b2DynamicTree tree;
    
    b2AABB aabb1{{0, 0}, {1, 1}};
    int id1 = tree.CreateProxy(aabb1, (void*)1);
    ASSERT_TRUE(id1 >= 0);
    
    b2AABB aabb2{{2, 2}, {3, 3}};
    int id2 = tree.CreateProxy(aabb2, (void*)2);
    ASSERT_TRUE(id2 >= 0);
    
    ASSERT_EQ(tree.GetNodeCount(), 3); // 2 leaves + 1 internal
    ASSERT_NE(tree.GetRoot(), -1);
    
    tree.DestroyProxy(id1);
    tree.DestroyProxy(id2);
    ASSERT_EQ(tree.GetRoot(), -1);
}

TEST_CASE(TestAABBTreeQuery) {
    b2DynamicTree tree;
    
    b2AABB a1{{0, 0}, {2, 2}};
    b2AABB a2{{3, 3}, {5, 5}};
    b2AABB a3{{1, 1}, {4, 4}}; // overlaps both
    
    int id1 = tree.CreateProxy(a1, (void*)10);
    int id2 = tree.CreateProxy(a2, (void*)20);
    int id3 = tree.CreateProxy(a3, (void*)30);
    
    int found = 0;
    b2AABB query{{0.5f, 0.5f}, {1.5f, 1.5f}};
    tree.Query([&found](void* data) {
        found += (int)(intptr_t)data;
        return true;
    }, query);
    ASSERT_EQ(found, 10 + 30); // a1 and a3 overlap query
    
    found = 0;
    b2AABB query2{{3.5f, 3.5f}, {4.5f, 4.5f}};
    tree.Query([&found](void* data) {
        found += (int)(intptr_t)data;
        return true;
    }, query2);
    ASSERT_EQ(found, 20 + 30); // a2 and a3
}

TEST_CASE(TestAABBTreeMoveProxy) {
    b2DynamicTree tree;
    b2AABB a{{0, 0}, {1, 1}};
    int id = tree.CreateProxy(a, (void*)42);
    
    // Move far away
    b2AABB newAabb{{10, 10}, {11, 11}};
    tree.MoveProxy(id, newAabb, {10, 10});
    
    int found = 0;
    tree.Query([&found](void* data) {
        found = (int)(intptr_t)data;
        return false; // stop after first
    }, newAabb);
    ASSERT_EQ(found, 42);
}

TEST_CASE(TestSATSeparated) {
    b2Polygon A, B;
    A.vertices = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};
    A.count = 4; A.BuildNormals();
    
    B.vertices = {{3, 0}, {4, 0}, {4, 1}, {3, 1}};
    B.count = 4; B.BuildNormals();
    
    auto result = b2SAT(A, B);
    ASSERT_FALSE(result.hit);
}

TEST_CASE(TestSATOverlapping) {
    b2Polygon A, B;
    A.vertices = {{0, 0}, {2, 0}, {2, 2}, {0, 2}};
    A.count = 4; A.BuildNormals();
    
    B.vertices = {{1, 1}, {3, 1}, {3, 3}, {1, 3}};
    B.count = 4; B.BuildNormals();
    
    auto result = b2SAT(A, B);
    ASSERT_TRUE(result.hit);
    ASSERT_TRUE(result.depth > 0);
    ASSERT_TRUE(b2Length(result.normal) > 0.99f);
}

TEST_CASE(TestSATEdgeCase) {
    b2Polygon A, B;
    A.vertices = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};
    A.count = 4; A.BuildNormals();
    
    B.vertices = {{1, 0}, {2, 0}, {2, 1}, {1, 1}}; // touching at edge
    B.count = 4; B.BuildNormals();
    
    auto result = b2SAT(A, B);
    ASSERT_TRUE(result.hit); // touching counts as hit
    ASSERT_TRUE(result.depth >= 0);
}

TEST_CASE(TestContactConstraint) {
    b2Manifold manifold;
    manifold.localNormal = {0, 1};
    manifold.localPoint = {0, 0};
    manifold.points.push_back({{0, 0}, 0, 0, 0});
    manifold.pointCount = 1;
    
    b2Transform xfA, xfB;
    xfA.p = {0, 0}; xfA.q = {0, 1};
    xfB.p = {0, -1}; xfB.q = {0, 1};
    
    b2ContactConstraint cc = b2BuildContactConstraint(
        manifold, xfA, xfB,
        1.0f, 1.0f, // invMass
        1.0f, 1.0f, // invI
        0.0f, 0.3f, // restitution, friction
        1.0f/60.0f  // dt
    );
    
    ASSERT_EQ(cc.indexA, 0);
    ASSERT_EQ(cc.indexB, 1);
    ASSERT_TRUE(cc.normalMass > 0);
    ASSERT_TRUE(cc.tangentMass > 0);
    ASSERT_EQ(cc.restitution, 0.0f);
    ASSERT_EQ(cc.friction, 0.3f);
}

TEST_CASE(TestIslandBuilding) {
    std::vector<b2Body> bodies(4);
    bodies[0].id = 0; bodies[0].awake = true;  bodies[0].isStatic = false;
    bodies[1].id = 1; bodies[1].awake = true;  bodies[1].isStatic = false;
    bodies[2].id = 2; bodies[2].awake = false; bodies[2].isStatic = false; // sleeping
    bodies[3].id = 3; bodies[3].awake = true;  bodies[3].isStatic = true;  // static
    
    std::vector<b2Contact> contacts;
    contacts.push_back({0, 1, {}, true});  // 0-1 connected
    contacts.push_back({1, 2, {}, true});  // 1-2 connected (but 2 sleeping)
    contacts.push_back({2, 3, {}, true});  // 2-3 connected
    
    std::vector<b2Joint> joints;
    
    std::vector<b2Island> islands;
    b2BuildIslands(bodies, contacts, joints, islands);
    
    // Island 0: bodies 0,1 (awake, connected)
    // Body 2 is sleeping -> not included
    // Body 3 is static -> not in island (or separate static island)
    
    ASSERT_EQ(islands.size(), 1);
    ASSERT_EQ(islands[0].bodies.size(), 2);
    ASSERT_TRUE(std::find(islands[0].bodies.begin(), islands[0].bodies.end(), 0) != islands[0].bodies.end());
    ASSERT_TRUE(std::find(islands[0].bodies.begin(), islands[0].bodies.end(), 1) != islands[0].bodies.end());
}

int main() { return RUN_ALL_TESTS(); }