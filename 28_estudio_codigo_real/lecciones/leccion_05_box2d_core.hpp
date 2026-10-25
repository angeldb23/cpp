/*
 * MÓDULO 28: Estudio de Código Real — Arquitectura de Librerías
 * LECCIÓN 05: Estudio de Box2D 2.4.x — Core Physics Engine Architecture
 *
 * TEMAS:
 * - Box2D 2.4.x: C++ API (b2World, b2Body, b2Fixture, b2Joint)
 * - Box2D 3.x usa C API (nombres diferentes, comparar)
 * - Solver: iterative PGS (Projected Gauss-Seidel) para constraints
 * - Broad-phase: Dynamic AABB Tree (SAP-style)
* - Narrow-phase: SAT/GJK para colisión polígono-círculo
* - Island solving: agrupar bodies conectados para solver paralelo
* - Continuous collision detection (CCD): TOI (Time of Impact) con conservative advancement
* - Contact listener: BeginContact, EndContact, PreSolve, PostSolve
* - Filtros: categoryBits, maskBits, groupIndex
* - Sensors: fixtures sin respuesta física
* - Body types: static, kinematic, dynamic
* - Joints: distance, revolute, prismatic, weld, wheel, rope, motor, friction
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <cmath>
#include <algorithm>
#include <vector>
#include <functional>

namespace modulo28_box2d {

// ============================================================================
// ARQUITECTURA GENERAL
// ============================================================================

/*
 * Box2D 2.4.x estructura principal:
 * 
 * b2World: contenedor principal
 *   - Step(dt, velIter, posIter): avanza simulación
 *   - CreateBody/DestroyBody
 *   - CreateJoint/DestroyJoint
 *   - QueryAABB, RayCast
 *   - SetContactListener, SetDebugDraw
 * 
 * b2Body: cuerpo rígido
 *   - Tipo: static (masa infinita), kinematic (velocidad controlada), dynamic (simulado)
 *   - Transform: posición + rotación (b2Transform)
 *   - Velocity: linear + angular
 *   - Fuerza/torque acumuladores
 *   - Fixtures adjuntos (shapes + propiedades materiales)
 * 
 * b2Fixture: shape + propiedades de colisión
 *   - Shape: b2CircleShape, b2PolygonShape, b2ChainShape, b2EdgeShape
 *   - Density, friction, restitution
 *   - Filter: categoryBits, maskBits, groupIndex
 *   - Sensor flag
 * 
 * b2Joint: constraint entre dos bodies
 *   - Tipos: distance, revolute, prismatic, weld, wheel, rope, motor, friction
 *   - Limits, motors, springs
 *   - Break force/torque
 */

// ============================================================================
// MATH TYPES (Box2D usa sus propios tipos)
// ============================================================================

struct b2Vec2 {
    float x, y;
    constexpr b2Vec2() : x(0), y(0) {}
    constexpr b2Vec2(float x_, float y_) : x(x_), y(y_) {}
    
    constexpr b2Vec2 operator+(const b2Vec2& v) const { return {x + v.x, y + v.y}; }
    constexpr b2Vec2 operator-(const b2Vec2& v) const { return {x - v.x, y - v.y}; }
    constexpr b2Vec2 operator*(float s) const { return {x * s, y * s}; }
    constexpr b2Vec2 operator/(float s) const { return {x / s, y / s}; }
    constexpr b2Vec2 operator-() const { return {-x, -y}; }
    constexpr b2Vec2& operator+=(const b2Vec2& v) { x += v.x; y += v.y; return *this; }
    constexpr b2Vec2& operator-=(const b2Vec2& v) { x -= v.x; y -= v.y; return *this; }
    constexpr b2Vec2& operator*=(float s) { x *= s; y *= s; return *this; }
};

inline float b2Dot(const b2Vec2& a, const b2Vec2& b) { return a.x * b.x + a.y * b.y; }
inline float b2Cross(const b2Vec2& a, const b2Vec2& b) { return a.x * b.y - a.y * b.x; }
inline b2Vec2 b2Cross(float s, const b2Vec2& v) { return {-s * v.y, s * v.x}; }
inline b2Vec2 b2Cross(const b2Vec2& v, float s) { return {s * v.y, -s * v.x}; }

inline float b2Length(const b2Vec2& v) { return std::sqrt(b2Dot(v, v)); }
inline b2Vec2 b2Normalize(const b2Vec2& v) { float l = b2Length(v); return l > 0 ? v / l : b2Vec2(0, 0); }

struct b2Rot {
    float s, c; // sin, cos
    constexpr b2Rot() : s(0), c(1) {}
    constexpr b2Rot(float angle) : s(std::sin(angle)), c(std::cos(angle)) {}
    
    constexpr b2Vec2 operator*(const b2Vec2& v) const {
        return {c * v.x - s * v.y, s * v.x + c * v.y};
    }
};

struct b2Transform {
    b2Vec2 p;
    b2Rot q;
    constexpr b2Transform() : p(0, 0), q(0) {}
    constexpr b2Transform(const b2Vec2& p_, const b2Rot& q_) : p(p_), q(q_) {}
    
    constexpr b2Vec2 operator*(const b2Vec2& v) const { return p + q * v; }
};

// ============================================================================
// SHAPES
// ============================================================================

enum class b2ShapeType { Circle, Polygon, Chain, Edge };

struct b2Shape {
    b2ShapeType type;
    float radius; // para redondeo de vértices (polygon skin)
    virtual ~b2Shape() = default;
};

struct b2CircleShape : b2Shape {
    b2Vec2 p; // centro local
    float r;
    b2CircleShape() : p(0, 0), r(1) { type = b2ShapeType::Circle; radius = 0; }
};

struct b2PolygonShape : b2Shape {
    std::vector<b2Vec2> vertices;
    std::vector<b2Vec2> normals; // outward normals
    int count = 0;
    
    b2PolygonShape() { type = b2ShapeType::Polygon; radius = 0.01f; } // skin radius
    
    // Set as box (centered at origin)
    void SetAsBox(float hx, float hy) {
        vertices = {{-hx, -hy}, {hx, -hy}, {hx, hy}, {-hx, hy}};
        count = 4;
        normals = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};
    }
    
    void SetAsBox(float hx, float hy, const b2Vec2& center, float angle) {
        SetAsBox(hx, hy);
        b2Rot rot(angle);
        for (auto& v : vertices) v = rot * v + center;
    }
};

// ============================================================================
// BODY DEFINITION
// ============================================================================

enum class b2BodyType { Static, Kinematic, Dynamic };

struct b2BodyDef {
    b2BodyType type = b2BodyType::Static;
    b2Vec2 position = {0, 0};
    float angle = 0;
    b2Vec2 linearVelocity = {0, 0};
    float angularVelocity = 0;
    float linearDamping = 0;
    float angularDamping = 0;
    bool allowSleep = true;
    bool awake = true;
    bool fixedRotation = false;
    bool bullet = false; // CCD
    bool enabled = true;
    void* userData = nullptr;
};

// ============================================================================
// FIXTURE DEFINITION
// ============================================================================

struct b2Filter {
    uint16_t categoryBits = 0x0001;
    uint16_t maskBits = 0xFFFF;
    int16_t groupIndex = 0;
    
    bool ShouldCollide(const b2Filter& other) const {
        if (groupIndex != 0 && groupIndex == other.groupIndex) {
            return groupIndex > 0;
        }
        return (categoryBits & other.maskBits) != 0 && (other.categoryBits & maskBits) != 0;
    }
};

struct b2FixtureDef {
    b2Shape* shape = nullptr;
    float density = 0;
    float friction = 0.2f;
    float restitution = 0;
    float restitutionThreshold = 1.0f; // mínimo para restitución
    b2Filter filter;
    bool isSensor = false;
    void* userData = nullptr;
};

// ============================================================================
// WORLD & STEPPING
// ============================================================================

struct b2World {
    b2Vec2 gravity = {0, -9.8f};
    bool allowSleep = true;
    bool warmStarting = true;
    bool continuousPhysics = true;
    bool subStepping = false;
    
    // Solver settings
    int velocityIterations = 8;
    int positionIterations = 3;
    
    // Para simulación paso a paso
    void Step(float dt, int velIter, int posIter) {
        // 1. Broad-phase: detectar pares potenciales
        // 2. Narrow-phase: generar contactos
        // 3. Resolver contactos + joints (PGS)
        // 4. Integrar velocidades
        // 5. Resolver posiciones (penetrations)
        // 6. Continuous collision (TOI) para bullets
        // 7. Sleep management
    }
    
    class b2Body* CreateBody(const b2BodyDef* def) { return nullptr; }
    void DestroyBody(class b2Body* body) {}
    
    class b2Joint* CreateJoint(const class b2JointDef* def) { return nullptr; }
    void DestroyJoint(class b2Joint* joint) {}
    
    // Queries
    template<typename Callback>
    void QueryAABB(Callback callback, const struct b2AABB& aabb) {}
    
    template<typename Callback>
    void RayCast(Callback callback, const b2Vec2& p1, const b2Vec2& p2) {}
    
    // Contact listener
    class b2ContactListener* contactListener = nullptr;
    void SetContactListener(class b2ContactListener* listener) { contactListener = listener; }
};

// ============================================================================
// CONTACT LISTENER
// ============================================================================

struct b2Contact;
struct b2Manifold;

class b2ContactListener {
public:
    virtual ~b2ContactListener() = default;
    virtual void BeginContact(b2Contact* contact) {}
    virtual void EndContact(b2Contact* contact) {}
    virtual void PreSolve(b2Contact* contact, const b2Manifold* oldManifold) {}
    virtual void PostSolve(b2Contact* contact, const struct b2ContactImpulse* impulse) {}
};

// ============================================================================
// JOINTS (resumen)
// ============================================================================

enum class b2JointType { Distance, Revolute, Prismatic, Weld, Wheel, Rope, Motor, Friction };

struct b2JointDef {
    b2JointType type;
    class b2Body* bodyA = nullptr;
    class b2Body* bodyB = nullptr;
    bool collideConnected = false;
    void* userData = nullptr;
};

// Revolute joint (hinge)
struct b2RevoluteJointDef : b2JointDef {
    b2Vec2 localAnchorA = {0, 0};
    b2Vec2 localAnchorB = {0, 0};
    float referenceAngle = 0;
    bool enableLimit = false;
    float lowerAngle = 0, upperAngle = 0;
    bool enableMotor = false;
    float motorSpeed = 0;
    float maxMotorTorque = 0;
};

// ============================================================================
// BROAD-PHASE: DYNAMIC AABB TREE
// ============================================================================

struct b2AABB {
    b2Vec2 lowerBound, upperBound;
    
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
        b2AABB out;
        out.lowerBound = {std::min(a.lowerBound.x, b.lowerBound.x),
                          std::min(a.lowerBound.y, b.lowerBound.y)};
        out.upperBound = {std::max(a.upperBound.x, b.upperBound.x),
                          std::max(a.upperBound.y, b.upperBound.y)};
        return out;
    }
};

struct b2TreeNode {
    b2AABB aabb;
    void* userData = nullptr;
    int parent = -1;
    int child1 = -1, child2 = -1;
    int height = 0;
    bool IsLeaf() const { return child1 == -1; }
};

class b2DynamicTree {
    std::vector<b2TreeNode> nodes;
    int root = -1;
    int freeList = -1;
    int nodeCapacity = 0;
    int nodeCount = 0;
    
public:
    int CreateProxy(const b2AABB& aabb, void* userData) { return -1; }
    void DestroyProxy(int proxyId) {}
    void MoveProxy(int proxyId, const b2AABB& aabb, const b2Vec2& displacement) {}
    
    template<typename Callback>
    void Query(Callback callback, const b2AABB& aabb) const {}
    
    template<typename Callback>
    void RayCast(Callback callback, const b2Vec2& p1, const b2Vec2& p2) const {}
};

// ============================================================================
// ISLAND SOLVING
// ============================================================================

/*
 * Island: grupo de bodies conectados por contactos/joints
 * Se resuelven juntos porque sus ecuaciones están acopladas
 * 
 * Algoritmo:
 * 1. Build islands: BFS/DFS desde bodies awake a través de contactos/joints
 * 2. Para cada island: resolver velocidades (PGS) + posiciones
 * 3. Si island pequeño, resolver directo; si grande, considerar paralelismo
 * 4. Update sleep state
 */

struct b2ContactConstraint {
    b2Vec2 normal;
    b2Vec2 pointA, pointB;
    float separation;
    float normalMass, tangentMass;
    float normalImpulse, tangentImpulse;
    float friction, restitution;
    int indexA, indexB; // índices en arrays de velocidades del island
};

// Velocidad solver (PGS - Projected Gauss-Seidel)
struct b2VelocitySolver {
    void Solve(std::vector<b2ContactConstraint>& contacts, 
               std::vector<b2Body*>& bodies,
               float dt, float inv_dt) {
        // Warm start: aplicar impulsos previos
        // Iterations:
        //   for each contact: resolver velocidad normal + tangencial
        //   for each joint: resolver constraint
        // Store impulses for warm start next frame
    }
};

// ============================================================================
// CONTINUOUS COLLISION (CCD)
// ============================================================================

/*
 * TOI (Time of Impact) para bodies "bullet"
 * 
 * Conservative advancement:
 * 1. Calcular bound de movimiento (AABB swept)
 * 2. Si AABBs overlap en t=1, done
 * 3. Sino, avanzar por pasos conservadores hasta tocar
 * 4. Máximo sub-steps para evitar infinite loop
 */

struct b2TOIInput {
    b2AABB proxyA, proxyB;
    b2Vec2 sweepA, sweepB; // movimiento lineal + angular
    float tMax = 1.0f;
};

struct b2TOIOutput {
    enum State { Separated, Touching, Overlapped } state;
    float t = 1.0f; // tiempo de impacto [0, 1]
};

// ============================================================================
// EJEMPLO DE USO
// ============================================================================

/*
int main() {
    b2World world;
    world.gravity = {0, -9.8f};
    
    // Ground
    b2BodyDef groundDef;
    groundDef.type = b2BodyType::Static;
    groundDef.position = {0, -10};
    b2Body* ground = world.CreateBody(&groundDef);
    
    b2PolygonShape groundShape;
    groundShape.SetAsBox(50, 10);
    b2FixtureDef groundFixture;
    groundFixture.shape = &groundShape;
    ground->CreateFixture(&groundFixture);
    
    // Dynamic box
    b2BodyDef boxDef;
    boxDef.type = b2BodyType::Dynamic;
    boxDef.position = {0, 4};
    b2Body* box = world.CreateBody(&boxDef);
    
    b2PolygonShape boxShape;
    boxShape.SetAsBox(1, 1);
    b2FixtureDef boxFixture;
    boxFixture.shape = &boxShape;
    boxFixture.density = 1;
    boxFixture.friction = 0.3f;
    box->CreateFixture(&boxFixture);
    
    // Simulate
    for (int i = 0; i < 60; ++i) {
        world.Step(1.0f/60.0f, 8, 3);
        printf("Box pos: %.2f, %.2f\n", box->GetPosition().x, box->GetPosition().y);
    }
    
    return 0;
}
*/

} // namespace modulo28_box2d