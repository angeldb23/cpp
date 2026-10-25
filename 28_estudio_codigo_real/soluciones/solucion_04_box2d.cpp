/*
 * MÓDULO 28: Estudio de Código Real — Arquitectura de Librerías
 * SOLUCIÓN 04: Box2D — AABB Tree, SAT, Contact Constraints, Island Building
 */

#include <cstddef>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <cmath>
#include <stack>
#include <queue>
#include <fstream>
#include "../../include/test_framework.hpp"

static std::ofstream debug_log("/tmp/box2d_debug.log", std::ios::app);
#define DLOG(msg) do { debug_log << msg << std::endl; debug_log.flush(); } while(0)

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
    constexpr b2Vec2& operator+=(const b2Vec2& v) { x += v.x; y += v.y; return *this; }
    constexpr b2Vec2& operator-=(const b2Vec2& v) { x -= v.x; y -= v.y; return *this; }
    constexpr float& operator[](int i) { return i == 0 ? x : y; }
    constexpr const float& operator[](int i) const { return i == 0 ? x : y; }
};

inline float b2Dot(const b2Vec2& a, const b2Vec2& b) { return a.x * b.x + a.y * b.y; }
inline float b2Cross(const b2Vec2& a, const b2Vec2& b) { return a.x * b.y - a.y * b.x; }
inline b2Vec2 b2Cross(float s, const b2Vec2& v) { return {-s * v.y, s * v.x}; }
inline b2Vec2 b2Cross(const b2Vec2& v, float s) { return {s * v.y, -s * v.x}; }
inline float b2Length(const b2Vec2& v) { return std::sqrt(b2Dot(v, v)); }
inline b2Vec2 b2Normalize(const b2Vec2& v) { float l = b2Length(v); return l > 0 ? v / l : b2Vec2(0, 0); }

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
    
    b2AABB Expand(float margin) const {
        return {lowerBound - b2Vec2(margin, margin), upperBound + b2Vec2(margin, margin)};
    }
};

// ============================================================================
// 1. DYNAMIC AABB TREE
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
    
    int AllocateNode() {
        if (m_freeList != -1) {
            int id = m_freeList;
            m_freeList = m_nodes[id].parent;
            m_nodes[id] = {};
            return id;
        }
        if (m_nodeCount == m_nodeCapacity) {
            m_nodeCapacity *= 2;
            m_nodes.resize(m_nodeCapacity);
        }
        if (m_nodeCount >= (int)m_nodes.size()) {
            m_nodes.resize(m_nodeCount + 1);
        }
        return m_nodeCount++;
    }
    
    void FreeNode(int id) {
        if (id >= 0 && id < (int)m_nodes.size()) {
            m_nodes[id].parent = m_freeList;
            m_freeList = id;
        }
    }
    
    // Debug accessor with bounds check
    b2TreeNode& Node(int id) {
        if (id < 0 || id >= (int)m_nodes.size()) {
            throw std::out_of_range("Node index out of bounds");
        }
        return m_nodes[id];
    }
    
    const b2TreeNode& Node(int id) const {
        if (id < 0 || id >= (int)m_nodes.size()) {
            throw std::out_of_range("Node index out of bounds");
        }
        return m_nodes[id];
    }
    
    int CreateProxy(const b2AABB& aabb, void* userData) {
        int id = AllocateNode();
        b2TreeNode& node = m_nodes[id];
        node.aabb = aabb.Expand(m_fatMargin);
        node.userData = userData;
        node.height = 0;
        InsertLeaf(id);
        return id;
    }
    
    void DestroyProxy(int proxyId) {
        RemoveLeaf(proxyId);
        FreeNode(proxyId);
    }
    
    void MoveProxy(int proxyId, const b2AABB& aabb, const b2Vec2& displacement) {
        b2TreeNode& node = m_nodes[proxyId];
        DLOG("MoveProxy: proxyId=" << proxyId << ", oldAABB=[" << node.aabb.lowerBound.x << "," << node.aabb.lowerBound.y << "][" << node.aabb.upperBound.x << "," << node.aabb.upperBound.y << "] -> newAABB=[" << aabb.lowerBound.x << "," << aabb.lowerBound.y << "][" << aabb.upperBound.x << "," << aabb.upperBound.y << "]");
        
        if (node.aabb.Contains(aabb.Expand(m_fatMargin))) return;
        
        RemoveLeaf(proxyId);
        
        // Swept AABB: union of old and new positions, expanded by margin
        b2AABB newFatAABB = aabb.Expand(m_fatMargin);
        b2AABB oldFatAABB = node.aabb; // already has fat margin from previous
        
        // Combine old and new to cover swept area
        b2AABB sweptAABB = b2AABB::Combine(oldFatAABB, newFatAABB);
        
        // Also expand by displacement direction for conservative advancement
        b2Vec2 absDisp = {std::abs(displacement.x), std::abs(displacement.y)};
        sweptAABB.lowerBound -= absDisp;
        sweptAABB.upperBound += absDisp;
        
        node.aabb = sweptAABB;
        node.height = 0;
        InsertLeaf(proxyId);
        DLOG("MoveProxy done: root=" << m_root);
    }
    
    template<typename Callback>
    void Query(Callback callback, const b2AABB& aabb) const {
        if (m_root == -1) return;
        std::vector<int> stack = {m_root};
        while (!stack.empty()) {
            int id = stack.back(); stack.pop_back();
            if (id == -1 || id >= (int)m_nodes.size()) continue;
            
            const b2TreeNode& node = m_nodes[id];
            DLOG("Query: visiting node " << id << ", aabb=[" << node.aabb.lowerBound.x << "," << node.aabb.lowerBound.y << "][" << node.aabb.upperBound.x << "," << node.aabb.upperBound.y << "], leaf=" << node.IsLeaf() << ", userData=" << node.userData);
            
            if (!node.aabb.Overlaps(aabb)) continue;
            
            if (node.IsLeaf()) {
                DLOG("Query: leaf hit id=" << id << ", userData=" << node.userData);
                if (!callback(node.userData)) break;
            } else {
                if (node.child1 != -1) stack.push_back(node.child1);
                if (node.child2 != -1) stack.push_back(node.child2);
            }
        }
    }
    
    template<typename Callback>
    void RayCast(Callback callback, const b2Vec2& p1, const b2Vec2& p2) const {
        if (m_root == -1) return;
        b2Vec2 d = p2 - p1;
        std::vector<int> stack = {m_root};
        while (!stack.empty()) {
            int id = stack.back(); stack.pop_back();
            if (id == -1 || id >= (int)m_nodes.size()) continue;
            
            const b2TreeNode& node = m_nodes[id];
            float tmin = 0, tmax = 1;
            if (!RayAABB(p1, d, node.aabb, tmin, tmax)) continue;
            
            if (node.IsLeaf()) {
                if (!callback(node.userData, tmin)) break;
            } else {
                if (node.child1 != -1) stack.push_back(node.child1);
                if (node.child2 != -1) stack.push_back(node.child2);
            }
        }
    }
    
    int GetRoot() const { return m_root; }
    const b2TreeNode& GetNode(int id) const { return m_nodes[id]; }
    int GetNodeCount() const { return m_nodeCount; }
    
private:
    bool RayAABB(const b2Vec2& p, const b2Vec2& d, const b2AABB& aabb, float& tmin, float& tmax) const {
        for (int i = 0; i < 2; ++i) {
            float invD = 1.0f / d[i];
            float t1 = (aabb.lowerBound[i] - p[i]) * invD;
            float t2 = (aabb.upperBound[i] - p[i]) * invD;
            if (t1 > t2) std::swap(t1, t2);
            tmin = std::max(tmin, t1);
            tmax = std::min(tmax, t2);
            if (tmin > tmax) return false;
        }
        return true;
    }
    
    void InsertLeaf(int leaf) {
        if (m_root == -1) {
            m_root = leaf;
            m_nodes[leaf].parent = -1;
            return;
        }
        
        b2AABB leafAABB = m_nodes[leaf].aabb;
        int index = m_root;
        
        while (!m_nodes[index].IsLeaf()) {
            int c1 = m_nodes[index].child1;
            int c2 = m_nodes[index].child2;
            
            float area = m_nodes[index].aabb.GetPerimeter();
            b2AABB combined = b2AABB::Combine(m_nodes[index].aabb, leafAABB);
            float combinedArea = combined.GetPerimeter();
            
            float cost1 = b2AABB::Combine(m_nodes[c1].aabb, leafAABB).GetPerimeter();
            float cost2 = b2AABB::Combine(m_nodes[c2].aabb, leafAABB).GetPerimeter();
            
            if (cost1 < cost2) index = c1;
            else index = c2;
        }
        
        int oldParent = m_nodes[index].parent;
        int newParent = AllocateNode();
        m_nodes[newParent].aabb = b2AABB::Combine(m_nodes[index].aabb, leafAABB);
        m_nodes[newParent].height = m_nodes[index].height + 1;
        
        m_nodes[newParent].child1 = index;
        m_nodes[newParent].child2 = leaf;
        m_nodes[index].parent = newParent;
        m_nodes[leaf].parent = newParent;
        
        if (oldParent != -1) {
            if (m_nodes[oldParent].child1 == index) m_nodes[oldParent].child1 = newParent;
            else m_nodes[oldParent].child2 = newParent;
            m_nodes[newParent].parent = oldParent;
        } else {
            m_root = newParent;
            m_nodes[newParent].parent = -1;
        }
        
        index = m_nodes[newParent].parent;
        while (index != -1) {
            int c1 = m_nodes[index].child1;
            int c2 = m_nodes[index].child2;
            m_nodes[index].aabb = b2AABB::Combine(m_nodes[c1].aabb, m_nodes[c2].aabb);
            m_nodes[index].height = 1 + std::max(m_nodes[c1].height, m_nodes[c2].height);
            index = m_nodes[index].parent;
        }
    }
    
    void RemoveLeaf(int leaf) {
        if (leaf == m_root) {
            m_root = -1;
            return;
        }
        
        int parent = m_nodes[leaf].parent;
        int grandParent = m_nodes[parent].parent;
        int sibling = (m_nodes[parent].child1 == leaf) ? 
                      m_nodes[parent].child2 : m_nodes[parent].child1;
        
        if (grandParent != -1) {
            if (m_nodes[grandParent].child1 == parent)
                m_nodes[grandParent].child1 = sibling;
            else
                m_nodes[grandParent].child2 = sibling;
            m_nodes[sibling].parent = grandParent;
            
            int index = grandParent;
            while (index != -1) {
                int c1 = m_nodes[index].child1;
                int c2 = m_nodes[index].child2;
                m_nodes[index].aabb = b2AABB::Combine(m_nodes[c1].aabb, m_nodes[c2].aabb);
                m_nodes[index].height = 1 + std::max(m_nodes[c1].height, m_nodes[c2].height);
                index = m_nodes[index].parent;
            }
        } else {
            m_root = sibling;
            m_nodes[sibling].parent = -1;
        }
        
        FreeNode(parent);
    }
};

// ============================================================================
// 2. SAT (SEPARATING AXIS THEOREM)
// ============================================================================

struct b2Polygon {
    std::vector<b2Vec2> vertices;
    std::vector<b2Vec2> normals;
    int count = 0;
    
    void BuildNormals() {
        normals.resize(count);
        for (int i = 0; i < count; ++i) {
            int j = (i + 1) % count;
            b2Vec2 edge = vertices[j] - vertices[i];
            normals[i] = b2Normalize(b2Cross(edge, 1.0f));
        }
    }
    
    void Project(const b2Vec2& axis, float& min, float& max) const {
        min = max = b2Dot(vertices[0], axis);
        for (int i = 1; i < count; ++i) {
            float proj = b2Dot(vertices[i], axis);
            min = std::min(min, proj);
            max = std::max(max, proj);
        }
    }
};

struct b2SATResult {
    bool hit = false;
    b2Vec2 normal = {0, 0};
    float depth = 0;
    int axisIndex = -1;
};

b2SATResult b2SAT(const b2Polygon& A, const b2Polygon& B) {
    b2SATResult result;
    float minDepth = 1e30f;
    int minAxis = -1;
    b2Vec2 minNormal = {0, 0};
    
    for (int i = 0; i < A.count; ++i) {
        const b2Vec2& axis = A.normals[i];
        float minA, maxA, minB, maxB;
        A.Project(axis, minA, maxA);
        B.Project(axis, minB, maxB);
        
        if (maxA < minB || maxB < minA) {
            return {false, {0,0}, 0, -1};
        }
        
        float depth = std::min(maxA, maxB) - std::max(minA, minB);
        if (depth < minDepth) {
            minDepth = depth;
            minAxis = i;
            minNormal = axis;
        }
    }
    
    for (int i = 0; i < B.count; ++i) {
        const b2Vec2& axis = B.normals[i];
        float minA, maxA, minB, maxB;
        A.Project(axis, minA, maxA);
        B.Project(axis, minB, maxB);
        
        if (maxA < minB || maxB < minA) {
            return {false, {0,0}, 0, -1};
        }
        
        float depth = std::min(maxA, maxB) - std::max(minA, minB);
        if (depth < minDepth) {
            minDepth = depth;
            minAxis = A.count + i;
            minNormal = axis;
        }
    }
    
    b2Vec2 centerA = {0,0}, centerB = {0,0};
    for (auto& v : A.vertices) centerA = centerA + v;
    for (auto& v : B.vertices) centerB = centerB + v;
    centerA = centerA / A.count;
    centerB = centerB / B.count;
    b2Vec2 toB = centerB - centerA;
    if (b2Dot(minNormal, toB) < 0) minNormal = minNormal * -1.0f;
    
    result.hit = true;
    result.normal = minNormal;
    result.depth = minDepth;
    result.axisIndex = minAxis;
    return result;
}

// ============================================================================
// 3. CONTACT CONSTRAINT
// ============================================================================

struct b2ManifoldPoint {
    b2Vec2 localPoint;
    float normalImpulse = 0;
    float tangentImpulse = 0;
    uint32_t id = 0;
};

struct b2Manifold {
    b2Vec2 localNormal;
    b2Vec2 localPoint;
    std::vector<b2ManifoldPoint> points;
    int pointCount = 0;
    int type = 0;
};

struct b2ContactConstraint {
    int indexA = -1, indexB = -1;
    b2Vec2 normal = {0, 0};
    b2Vec2 tangent = {0, 0};
    float invMassA = 0, invMassB = 0;
    float invIA = 0, invIB = 0;
    b2Vec2 rA = {0, 0}, rB = {0, 0};
    float normalMass = 0;
    float tangentMass = 0;
    float bias = 0;
    float restitution = 0;
    float friction = 0;
    float normalImpulse = 0;
    float tangentImpulse = 0;
};

b2ContactConstraint b2BuildContactConstraint(
    const b2Manifold& manifold,
    const b2Transform& xfA, const b2Transform& xfB,
    float invMassA, float invMassB,
    float invIA, float invIB,
    float restitution, float friction,
    float dt) {
    
    b2ContactConstraint cc;
    cc.indexA = 0;
    cc.indexB = 1;
    cc.invMassA = invMassA;
    cc.invMassB = invMassB;
    cc.invIA = invIA;
    cc.invIB = invIB;
    cc.restitution = restitution;
    cc.friction = friction;
    
    cc.normal = xfA.q * manifold.localNormal;
    cc.tangent = b2Cross(cc.normal, 1.0f);
    
    const b2ManifoldPoint& mp = manifold.points[0];
    b2Vec2 worldPoint = xfA * mp.localPoint;
    
    cc.rA = worldPoint - xfA.p;
    cc.rB = worldPoint - xfB.p;
    
    float rnA = b2Cross(cc.rA, cc.normal);
    float rnB = b2Cross(cc.rB, cc.normal);
    cc.normalMass = invMassA + invMassB + invIA * rnA * rnA + invIB * rnB * rnB;
    if (cc.normalMass > 0) cc.normalMass = 1.0f / cc.normalMass;
    
    float rtA = b2Cross(cc.rA, cc.tangent);
    float rtB = b2Cross(cc.rB, cc.tangent);
    cc.tangentMass = invMassA + invMassB + invIA * rtA * rtA + invIB * rtB * rtB;
    if (cc.tangentMass > 0) cc.tangentMass = 1.0f / cc.tangentMass;
    
    cc.bias = 0.2f * 0.01f / dt;
    
    return cc;
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
    bool touching = false;
    
    constexpr b2Contact() = default;
    constexpr b2Contact(int a, int b, bool t) : bodyA(a), bodyB(b), touching(t) {}
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

void b2BuildIslands(
    const std::vector<b2Body>& bodies,
    const std::vector<b2Contact>& contacts,
    const std::vector<b2Joint>& joints,
    std::vector<b2Island>& islands) {
    
    int n = bodies.size();
    std::vector<char> visited(n, 0);
    std::vector<int> stack;
    
    for (int i = 0; i < n; ++i) {
        if (visited[i] || !bodies[i].awake || bodies[i].isStatic) continue;
        
        b2Island island;
        stack.clear();
        stack.push_back(i);
        visited[i] = 1;
        
        while (!stack.empty()) {
            int bodyIdx = stack.back(); stack.pop_back();
            island.bodies.push_back(bodyIdx);
            
            for (int c = 0; c < (int)contacts.size(); ++c) {
                if (!contacts[c].touching) continue;
                int other = -1;
                if (contacts[c].bodyA == bodyIdx) other = contacts[c].bodyB;
                else if (contacts[c].bodyB == bodyIdx) other = contacts[c].bodyA;
                else continue;
                
                if (!visited[other] && bodies[other].awake && !bodies[other].isStatic) {
                    visited[other] = 1;
                    stack.push_back(other);
                }
                island.contacts.push_back(c);
            }
            
            for (int j = 0; j < (int)joints.size(); ++j) {
                int other = -1;
                if (joints[j].bodyA == bodyIdx) other = joints[j].bodyB;
                else if (joints[j].bodyB == bodyIdx) other = joints[j].bodyA;
                else continue;
                
                if (!joints[j].collideConnected && visited[other]) continue;
                if (!visited[other] && bodies[other].awake && !bodies[other].isStatic) {
                    visited[other] = 1;
                    stack.push_back(other);
                }
                island.joints.push_back(j);
            }
        }
        
        if (!island.bodies.empty()) {
            std::sort(island.contacts.begin(), island.contacts.end());
            island.contacts.erase(std::unique(island.contacts.begin(), island.contacts.end()), island.contacts.end());
            std::sort(island.joints.begin(), island.joints.end());
            island.joints.erase(std::unique(island.joints.begin(), island.joints.end()), island.joints.end());
            
            islands.push_back(std::move(island));
        }
    }
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
    
    ASSERT_EQ(tree.GetNodeCount(), 3);
    ASSERT_NE(tree.GetRoot(), -1);
    
    tree.DestroyProxy(id1);
    tree.DestroyProxy(id2);
    ASSERT_EQ(tree.GetRoot(), -1);
}

TEST_CASE(TestAABBTreeQuery) {
    b2DynamicTree tree;
    
    b2AABB a1{{0, 0}, {2, 2}};
    b2AABB a2{{3, 3}, {5, 5}};
    b2AABB a3{{1, 1}, {4, 4}};
    
    int id1 = tree.CreateProxy(a1, (void*)10);
    int id2 = tree.CreateProxy(a2, (void*)20);
    int id3 = tree.CreateProxy(a3, (void*)30);
    
    int found = 0;
    b2AABB query{{0.5f, 0.5f}, {1.5f, 1.5f}};
    tree.Query([&found](void* data) {
        found += (int)(intptr_t)data;
        return true;
    }, query);
    ASSERT_EQ(found, 40);
    
    found = 0;
    b2AABB query2{{3.5f, 3.5f}, {4.5f, 4.5f}};
    tree.Query([&found](void* data) {
        found += (int)(intptr_t)data;
        return true;
    }, query2);
    ASSERT_EQ(found, 50);
}

TEST_CASE(TestAABBTreeMoveProxy) {
    b2DynamicTree tree;
    b2AABB a{{0, 0}, {1, 1}};
    int id = tree.CreateProxy(a, (void*)42);
    
    b2AABB newAabb{{10, 10}, {11, 11}};
    tree.MoveProxy(id, newAabb, {10, 10});
    
    int found = 0;
    tree.Query([&found](void* data) {
        found = (int)(intptr_t)data;
        return false;
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
    
    B.vertices = {{1, 0}, {2, 0}, {2, 1}, {1, 1}};
    B.count = 4; B.BuildNormals();
    
    auto result = b2SAT(A, B);
    ASSERT_TRUE(result.hit);
    ASSERT_TRUE(result.depth >= 0);
}

TEST_CASE(TestContactConstraint) {
    b2Manifold manifold;
    manifold.localNormal = {0, 1};
    manifold.localPoint = {0, 0};
    manifold.points.push_back({{0, 0}, 0, 0, 0});
    manifold.pointCount = 1;
    
    b2Transform xfA, xfB;
    xfA.p = {0, 0}; xfA.q = b2Rot(0.0f);
    xfB.p = {0, -1}; xfB.q = b2Rot(0.0f);
    
    b2ContactConstraint cc = b2BuildContactConstraint(
        manifold, xfA, xfB,
        1.0f, 1.0f,
        1.0f, 1.0f,
        0.0f, 0.3f,
        1.0f/60.0f
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
    bodies[2].id = 2; bodies[2].awake = false; bodies[2].isStatic = false;
    bodies[3].id = 3; bodies[3].awake = true;  bodies[3].isStatic = true;
    
    std::vector<b2Contact> contacts;
    contacts.emplace_back(0, 1, true);
    contacts.emplace_back(1, 2, true);
    contacts.emplace_back(2, 3, true);
    
    std::vector<b2Joint> joints;
    
    std::vector<b2Island> islands;
    b2BuildIslands(bodies, contacts, joints, islands);
    
    ASSERT_EQ(islands.size(), 1);
    ASSERT_EQ(islands[0].bodies.size(), 2);
    ASSERT_TRUE(std::find(islands[0].bodies.begin(), islands[0].bodies.end(), 0) != islands[0].bodies.end());
    ASSERT_TRUE(std::find(islands[0].bodies.begin(), islands[0].bodies.end(), 1) != islands[0].bodies.end());
}

int main() { return RUN_ALL_TESTS(); }