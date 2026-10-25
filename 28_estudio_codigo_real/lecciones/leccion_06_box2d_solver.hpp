/*
 * MÓDULO 28: Estudio de Código Real — Arquitectura de Librerías
 * LECCIÓN 06: Box2D — Solver PGS, Broad-Phase Tree, CCD, Island Architecture
 *
 * TEMAS:
 * - Velocity solver: PGS (Projected Gauss-Seidel) con warm starting
 * - Position solver: pseudo-velocities para corrección de penetración
 * - Broad-phase: Dynamic AABB Tree con SAH (Surface Area Heuristic)
 * - Narrow-phase: SAT para polígonos, GJK/EPA para formas convexas
 * - Continuous Collision Detection (CCD): TOI con conservative advancement
* - Island solving: paralelización por islas independientes
* - Contact caching: persistencia de contactos entre frames
* - Sub-stepping para estabilidad
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>
#include <algorithm>

#include "leccion_05_box2d_core.hpp"

namespace modulo28_box2d_detail {

using modulo28_box2d::b2AABB;
using modulo28_box2d::b2Cross;
using modulo28_box2d::b2Dot;
using modulo28_box2d::b2Vec2;

// ============================================================================
// VELOCITY SOLVER: PGS (PROJECTED GAUSS-SEIDEL)
// ============================================================================

/*
 * PGS resuelve: J * v = -b  con constraints de desigualdad (impulses >= 0 para contactos)
 * 
 * Formulación:
 *   Para cada contacto: v_rel_normal >= 0 (separación) -> impulse >= 0
 *   Para fricción: |impulse_t| <= mu * impulse_n (cono de Coulomb)
 * 
 * Iteración PGS:
 *   1. Para cada constraint, calcular impulse que satisface constraint localmente
 *   2. Proyectar: clamp impulse a región factible
 *   3. Acumular delta-v a bodies
 *   4. Repetir N iteraciones
 * 
 * Warm starting: usar impulses del frame anterior como initial guess
 *   - Mejora convergencia drásticamente (contactos persistentes)
 *   - Especialmente importante para stacking
 */

struct b2VelocityConstraint {
    // Contact constraint
    struct Contact {
        int indexA, indexB;      // índices en velocity arrays
        b2Vec2 normal;           // normal de contacto (A -> B)
        b2Vec2 tangent;          // tangente (perp a normal)
        float normalMass;        // masa efectiva normal
        float tangentMass;       // masa efectiva tangencial
        float bias;              // Baumgarte bias para penetración
        float restitution;       // coeficiente restitución
        float friction;          // coeficiente fricción
        float normalImpulse;     // impulso normal acumulado
        float tangentImpulse;    // impulso tangencial acumulado
    };
    
    // Joint constraint (simplificado)
    struct Joint {
        int indexA, indexB;
        b2Vec2 J;                // Jacobiano (simplificado 2D)
        float mass;              // masa efectiva
        float bias;
        float impulse;
        float lowerLimit, upperLimit; // para limits
    };
    
    std::vector<Contact> contacts;
    std::vector<Joint> joints;
};

// Velocity state para solver
struct b2VelocityState {
    b2Vec2 v;    // linear velocity
    float w;     // angular velocity
    
    void ApplyImpulse(const b2Vec2& impulse, const b2Vec2& r, float invMass, float invI) {
        v += impulse * invMass;
        w += b2Cross(r, impulse) * invI;
    }
};

// PGS Solver implementation
class b2PGSVelocitySolver {
    std::vector<b2VelocityState>* m_velocities;
    float m_dt;
    float m_inv_dt;
    
public:
    void Initialize(std::vector<b2VelocityState>* velocities, float dt) {
        m_velocities = velocities;
        m_dt = dt;
        m_inv_dt = 1.0f / dt;
    }
    
    void Solve(b2VelocityConstraint& constraint, int iterations) {
        // Warm start
        for (auto& c : constraint.contacts) {
            b2Vec2 P = c.normal * c.normalImpulse + c.tangent * c.tangentImpulse;
            (*m_velocities)[c.indexA].ApplyImpulse(-P, {}, 1, 1); // simplified
            (*m_velocities)[c.indexB].ApplyImpulse(P, {}, 1, 1);
        }
        
        for (int iter = 0; iter < iterations; ++iter) {
            // Solve contacts
            for (auto& c : constraint.contacts) {
                b2VelocityState& vA = (*m_velocities)[c.indexA];
                b2VelocityState& vB = (*m_velocities)[c.indexB];
                
                // Relative velocity at contact
                b2Vec2 dv = vB.v - vA.v + b2Cross(vB.w, {}) - b2Cross(vA.w, {});
                // Simplified - real implementation uses rA, rB
                
                // Normal velocity
                float vn = b2Dot(dv, c.normal);
                
                // Compute normal impulse
                float dPn = c.normalMass * (-vn + c.bias);
                
                // Clamp (accumulated impulse >= 0)
                float newImpulse = c.normalImpulse + dPn;
                if (newImpulse < 0) newImpulse = 0;
                dPn = newImpulse - c.normalImpulse;
                c.normalImpulse = newImpulse;
                
                // Apply normal impulse
                b2Vec2 Pn = c.normal * dPn;
                vA.v -= Pn; // simplified
                vB.v += Pn;
                
                // Tangent (friction)
                float vt = b2Dot(dv, c.tangent);
                float dPt = c.tangentMass * (-vt);
                
                // Friction cone: |Pt| <= mu * Pn
                float maxPt = c.friction * c.normalImpulse;
                float newTangentImpulse = c.tangentImpulse + dPt;
                if (newTangentImpulse > maxPt) newTangentImpulse = maxPt;
                if (newTangentImpulse < -maxPt) newTangentImpulse = -maxPt;
                dPt = newTangentImpulse - c.tangentImpulse;
                c.tangentImpulse = newTangentImpulse;
                
                b2Vec2 Pt = c.tangent * dPt;
                vA.v -= Pt;
                vB.v += Pt;
            }
            
            // Solve joints (similar pattern)
            for (auto& j : constraint.joints) {
                // Joint constraint resolution
            }
        }
    }
};

// ============================================================================
// POSITION SOLVER (PSEUDO-VELOCITIES)
// ============================================================================

/*
 * Position correction: resolver penetraciones usando "pseudo-velocidades"
 * No afecta momentum, solo corrige posiciones
 * 
 * b2PositionConstraint: similar a velocity pero con posiciones
 * Bias = penetration / dt (Baumgarte)
 * 
 * Iteraciones típicas: 3-8 (menos que velocity)
 */

struct b2PositionConstraint {
    int indexA, indexB;
    b2Vec2 normal;
    float separation; // negativo = penetración
    float bias;
};

class b2PseudoVelocitySolver {
public:
    void Solve(std::vector<b2PositionConstraint>& constraints, 
               std::vector<b2Vec2>* positions,
               std::vector<float>* angles,
               int iterations) {
        for (int iter = 0; iter < iterations; ++iter) {
            for (auto& c : constraints) {
                // Simplified position correction
                // Real implementation uses full Jacobian
            }
        }
    }
};

// ============================================================================
// BROAD-PHASE: DYNAMIC AABB TREE CON SAH
// ============================================================================

/*
 * Dynamic AABB Tree:
 * - Binary tree donde hojas = proxies (AABB + userData)
 * - Nodos internos = AABB que engloba a hijos
 * - SAH para balanceo: minimizar SA(parent) = SA(left) + SA(right)
 * - Insertion: descend from root, choose child with minimal area increase
 * - Removal: mark leaf, rebuild path to root
 * - Query: traverse tree, test AABB overlap
 * - Ray cast: traverse with slab test
 * 
 * Optimizaciones:
 * - Fat AABBs (margen) para reducir reinserts
 * - Buffer de nodos free list
 * - Batch queries para cache locality
 */

struct b2TreeNodeDetailed {
    b2AABB aabb;
    void* userData = nullptr;
    int parent = -1;
    int child1 = -1, child2 = -1;
    int height = 0;
    bool IsLeaf() const { return child1 == -1; }
    
    // Surface Area Heuristic cost
    float ComputeCost() const {
        return aabb.GetPerimeter();
    }
};

class b2DynamicTreeDetailed {
    std::vector<b2TreeNodeDetailed> m_nodes;
    int m_root = -1;
    int m_freeList = -1;
    int m_nodeCapacity = 16;
    int m_nodeCount = 0;
    float m_fatMargin = 0.1f; // AABB fattening
    
public:
    b2DynamicTreeDetailed() { m_nodes.reserve(m_nodeCapacity); }
    
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
        return m_nodeCount++;
    }
    
    void FreeNode(int id) {
        m_nodes[id].parent = m_freeList;
        m_freeList = id;
    }
    
    int CreateProxy(const b2AABB& aabb, void* userData) {
        int id = AllocateNode();
        b2TreeNodeDetailed& node = m_nodes[id];
        node.aabb = aabb;
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
        b2TreeNodeDetailed& node = m_nodes[proxyId];
        
        // Check if still contained with margin
        if (node.aabb.Contains(aabb)) return;
        
        RemoveLeaf(proxyId);
        
        // Extend AABB with margin + displacement prediction
        b2AABB fatAABB = aabb;
        fatAABB.lowerBound -= b2Vec2(m_fatMargin, m_fatMargin);
        fatAABB.upperBound += b2Vec2(m_fatMargin, m_fatMargin);
        fatAABB.lowerBound += displacement * 0.5f; // conservative
        fatAABB.upperBound += displacement * 0.5f;
        
        node.aabb = fatAABB;
        InsertLeaf(proxyId);
    }
    
    template<typename Callback>
    void Query(Callback callback, const b2AABB& aabb) const {
        std::vector<int> stack = {m_root};
        while (!stack.empty()) {
            int id = stack.back(); stack.pop_back();
            if (id == -1) continue;
            
            const b2TreeNodeDetailed& node = m_nodes[id];
            if (!node.aabb.Overlaps(aabb)) continue;
            
            if (node.IsLeaf()) {
                if (!callback(node.userData)) break; // allow early exit
            } else {
                stack.push_back(node.child1);
                stack.push_back(node.child2);
            }
        }
    }
    
private:
    void InsertLeaf(int leaf) {
        if (m_root == -1) {
            m_root = leaf;
            m_nodes[leaf].parent = -1;
            return;
        }
        
        // Find best sibling
        b2AABB leafAABB = m_nodes[leaf].aabb;
        int index = m_root;
        
        while (!m_nodes[index].IsLeaf()) {
            int c1 = m_nodes[index].child1;
            int c2 = m_nodes[index].child2;
            
            float cost = m_nodes[index].aabb.GetPerimeter();
            float combinedCost = b2AABB::Combine(m_nodes[index].aabb, leafAABB).GetPerimeter();
            
            float cost1 = b2AABB::Combine(m_nodes[c1].aabb, leafAABB).GetPerimeter();
            float cost2 = b2AABB::Combine(m_nodes[c2].aabb, leafAABB).GetPerimeter();
            
            float inheritCost = 2 * (cost1 + cost2 - cost);
            
            if (inheritCost < cost1 && inheritCost < cost2) {
                // Create new parent
                break;
            }
            
            // Descend to child with smaller cost increase
            if (cost1 < cost2) index = c1;
            else index = c2;
        }
        
        // ... rest of insertion logic
        (void)index;
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
            
            // Update ancestors AABB
            int index = grandParent;
            while (index != -1) {
                int c1 = m_nodes[index].child1;
                int c2 = m_nodes[index].child2;
                m_nodes[index].aabb = b2AABB::Combine(m_nodes[c1].aabb, m_nodes[c2].aabb);
                index = m_nodes[index].parent;
            }
        } else {
            m_root = sibling;
            m_nodes[sibling].parent = -1;
        }
        
        FreeNode(parent);
    }
};

// AABB combine helper
inline b2AABB Combine(const b2AABB& a, const b2AABB& b) {
    b2AABB c;
    c.lowerBound.x = std::min(a.lowerBound.x, b.lowerBound.x);
    c.lowerBound.y = std::min(a.lowerBound.y, b.lowerBound.y);
    c.upperBound.x = std::max(a.upperBound.x, b.upperBound.x);
    c.upperBound.y = std::max(a.upperBound.y, b.upperBound.y);
    return c;
}

// ============================================================================
// NARROW-PHASE: SAT / GJK
// ============================================================================

/*
 * SAT (Separating Axis Theorem) para polígonos convexos:
 * - Para cada edge normal de ambos polígonos, proyectar y testear overlap
 * - Si hay separating axis -> no colisión
 * - Si no -> colisión, calcular penetration axis/depth
 * 
 * GJK (Gilbert-Johnson-Keerthi) para formas convexas genéricas:
 * - Minkowski difference: A - B
 * - Buscar punto más cercano a origen en Minkowski diff
 * - Support function: max(dot(v, x)) para x in shape
 * - Iterativo: construir simplex (point, line, triangle, tetrahedron)
 * 
 * EPA (Expanding Polytope Algorithm) para penetration depth:
 * - Partir del simplex final de GJK
 * - Expandir caras hasta encontrar origen
 * - Dar penetration normal + depth
 */

// Support function interface
struct b2ShapeSupport {
    virtual b2Vec2 GetSupport(const b2Vec2& dir) const = 0;
    virtual ~b2ShapeSupport() = default;
};

// GJK distance
float b2GJKDistance(const b2ShapeSupport& A, const b2ShapeSupport& B, 
                    b2Vec2& closestA, b2Vec2& closestB) {
    // Implementation omitted for brevity
    return 0.0f;
}

// ============================================================================
// CCD: CONSERVATIVE ADVANCEMENT
// ============================================================================

/*
 * Conservative Advancement para TOI:
 * 
 * 1. Calcular distancia actual entre shapes (GJK)
 * 2. Calcular velocidad relativa en dirección de separación
 * 3. Si v_rel <= 0 -> moving apart, no TOI
 * 4. t_advance = distance / v_rel
 * 4. Avanzar shapes por t_advance
 * 5. Repetir hasta distance < tolerance o t >= 1
 * 6. Clamp t a [0, 1]
 * 
 * Problemas:
 * - Rotation hace que distance function sea no-lineal
 * - Conservative: usar bound de rotación (angular velocity * radius)
 * - Max iterations para evitar infinite loop en grazing contacts
 */

struct b2Sweep {
    b2Vec2 c0, c1;      // center positions at t=0, t=1
    float a0, a1;       // angles at t=0, t=1
    float localCenter_x, localCenter_y;
    
    b2Vec2 GetCenter(float t) const { return c0 + (c1 - c0) * t; }
    float GetAngle(float t) const { return a0 + (a1 - a0) * t; }
};

struct b2TOIInputDetailed {
    b2ShapeSupport* proxyA;
    b2ShapeSupport* proxyB;
    b2Sweep sweepA, sweepB;
    float tMax = 1.0f;
};

struct b2TOIOutputDetailed {
    enum State { Separated, Touching, Overlapped, Failed } state;
    float t = 1.0f;
    int iterations = 0;
};

b2TOIOutputDetailed b2TimeOfImpact(const b2TOIInputDetailed& input) {
    b2TOIOutputDetailed output;
    output.state = b2TOIOutputDetailed::Separated;
    output.t = input.tMax;
    
    const int MAX_ITERATIONS = 20;
    const float TOLERANCE = 0.001f;
    
    float t = 0;
    for (int iter = 0; iter < MAX_ITERATIONS; ++iter) {
        output.iterations = iter;
        
        // Get shapes at time t
        // (Would need swept shape support functions)
        
        // Compute distance
        float distance = 0.0f; // b2GJKDistance(...)
        
        if (distance < TOLERANCE) {
            output.state = b2TOIOutputDetailed::Touching;
            output.t = t;
            return output;
        }
        
        if (t >= input.tMax) {
            output.state = b2TOIOutputDetailed::Separated;
            output.t = input.tMax;
            return output;
        }
        
        // Conservative advancement
        // v_rel = relative velocity along normal
        float v_rel = 1.0f; // placeholder
        if (v_rel <= 0) {
            output.state = b2TOIOutputDetailed::Separated;
            output.t = input.tMax;
            return output;
        }
        
        float dt = distance / v_rel;
        t += dt;
        
        if (t > input.tMax) {
            output.state = b2TOIOutputDetailed::Separated;
            output.t = input.tMax;
            return output;
        }
    }
    
    output.state = b2TOIOutputDetailed::Failed;
    output.t = input.tMax;
    return output;
}

// ============================================================================
// ISLAND PARALLELIZATION
// ============================================================================

/*
 * Island = connected component of awake bodies via contacts/joints
 * 
 * Build islands:
 * 1. Mark all awake bodies as unvisited
 * 2. For each unvisited awake body: BFS/DFS through contacts/joints
 * 3. Collect all bodies, contacts, joints in island
 * 4. Solve island independently (can parallelize across islands)
 * 
 * Paralelización:
 * - Thread pool procesa islands independientes en paralelo
 * - Islands grandes se pueden subdividir (domain decomposition)
 * - Synchronization barrier entre velocity/position phases
 */

struct b2Island {
    std::vector<int> bodyIndices;
    std::vector<int> contactIndices;
    std::vector<int> jointIndices;
    
    void Solve(float dt, int velIter, int posIter) {
        // 1. Build velocity constraints
        // 2. PGS velocity solver
        // 3. Integrate velocities -> tentative positions
        // 4. Build position constraints
        // 5. Pseudo-velocity position solver
        // 6. Update final positions
    }
};

class b2IslandManager {
    std::vector<b2Island> m_islands;
    std::vector<int> m_stack; // DFS stack
    
public:
    void BuildIslands(/* world data */) {
        // 1. Clear visited flags
        // 2. For each awake body not visited: DFS
        // 3. Collect into island
    }
    
    void SolveIslands(float dt, int velIter, int posIter) {
        // Can parallelize across islands
        for (auto& island : m_islands) {
            island.Solve(dt, velIter, posIter);
        }
    }
};

} // namespace modulo28_box2d_detail