/* =============================================================================
   MÓDULO 17 - LECCIÓN 08: Partículas — Emitter, Lifetime, Color/Size over Time,
   Blend Aditivo y Pooling
   =============================================================================
   TEMA 7 de TEMAS.md: Partículas

   CONCEPTOS CLAVE:
   - Particle: POD con posición, velocidad, vida y parámetros de desvanecimiento
   - Pooling: vector pre-aloqueado con swap-remove; NUNCA new/delete en el frame
   - Emitter: emite N partículas por segundo (acumulador de emisión) + bursts
   - Color/Size OVER TIME: interpolación lineal entre (start, end) según t
     de vida (t = age / lifetime, va de 0 a 1)
   - Blend aditivo: BLEND_ADDITIVE suma colores -> brillos de fuego/explosiones
     (raylib: BeginBlendMode / EndBlendMode)
   - Física simple: velocidad + gravedad (aceleración) + drag (fricción)

   REGLAS DE RENDIMIENTO (por qué importa el diseño):
   - Millares de partículas actualizadas y dibujadas por frame. Cualquier aloc,
     punteros dispersos o ramas absurdas se multiplican por 10.000.
   - Pool contiguo (vector<Particle>) = transform de la CPU contenta (caché).
   - Swap-remove: al matar una partícula, copio la última encima y reduzco
     size. O(1) y sin mover el resto del array.
   - La emisión por segundo con acumulador: rate * dt se ACUMULA; si el frame
     curve, spawn floor(acc). Nada de "si dt==1/60 emisión de 1" (rompe con
     timestep variable: el módulo 16 último le enseñó por qué).

   NOTA: Código compila sin raylib (stubs). Partículas son lógica pura.
   ============================================================================= */

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <cstdint>

// -----------------------------------------------------------------------------
// Stubs raylib
// -----------------------------------------------------------------------------
struct Vector2 { float x, y; };
struct Color { unsigned char r, g, b, a; };

inline void DrawCircleV(Vector2, float, Color) {}
inline void BeginBlendMode(int) {}
inline void EndBlendMode() {}
constexpr int BLEND_ALPHA = 0;
constexpr int BLEND_ADDITIVE = 1;

// -----------------------------------------------------------------------------
// 1. Particle: POD delgado y copiable
// -----------------------------------------------------------------------------
/* REGLA DATA-ORIENTED (de los módulos 15 y 16): la struct es tan chica que un
   AoS (array of struct) funciona bien; si el juego escala a decenas de miles
   con simulación pesada, SoA separa posiciones/colores a arrays chicos. */
struct Particle {
    Vector2 position;
    Vector2 velocity;
    float age = 0.0f;
    float lifetime = 1.0f;      // años de vida (resta)
    float sizeStart = 4.0f;
    float sizeEnd = 0.0f;
    Color colorStart;
    Color colorEnd;
    float gravity = 0.0f;       // px/s²  (positiva: cae abajo)
    float drag = 0.0f;          // 0 = sin fricción; 4 = bastante friccionada
};

// Interpolación de color por canal (lerp lineal por byte de color)
Color LerpColor(Color a, Color b, float t) {
    t = std::clamp(t, 0.0f, 1.0f);
    return Color{
        static_cast<unsigned char>(a.r + (b.r - a.r) * t),
        static_cast<unsigned char>(a.g + (b.g - a.g) * t),
        static_cast<unsigned char>(a.b + (b.b - a.b) * t),
        static_cast<unsigned char>(a.a + (b.a - a.a) * t)
    };
}

float Lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

// -----------------------------------------------------------------------------
// 2. ParticlePool: pooling con swap-remove
// -----------------------------------------------------------------------------
class ParticlePool {
    std::vector<Particle> pool_;     // pre-aloqueado a max
    size_t aliveCount_ = 0;          // solo los [0, aliveCount_) están vivos

public:
    explicit ParticlePool(size_t maxParticles) {
        pool_.resize(maxParticles);   // UNA sola alocación en el arranque
        aliveCount_ = 0;
    }
    size_t Capacity() const { return pool_.size(); }
    size_t AliveCount() const { return aliveCount_; }

    // Spawn: devuelve nullptr si el pool está lleno (partícula "perdida",
    // preferencia razonable: NO crece el pool en runtime, decide el diseño)
    Particle* Spawn() {
        if (aliveCount_ >= pool_.size()) return nullptr;
        Particle& p = pool_[aliveCount_++];
        return &p;   // se llena con el emitter
    }

    // Terminar una partícula YA (muerte anticipada): swap-remove
    void Kill(size_t index) {
        if (index >= aliveCount_) return;
        if (index != aliveCount_ - 1) {
            pool_[index] = pool_[aliveCount_ - 1];  // swap (move implícito)
        }
        --aliveCount_;
        // El "hueco" queda fuera del rango activo, no se limpia pero se
        // reescribirá al spawnearse de nuevo.
    }

    Particle& At(size_t i) { return pool_[i]; }

    // Update de todas las partículas: integración + aging + muerte
    void Update(float dt) {
        for (size_t i = 0; i < aliveCount_; /* sin ++: puede cambiar */) {
            Particle& p = pool_[i];

            // Física: gravedad (a += g) y drag (v *= 1 - drag*dt)
            p.velocity.y += p.gravity * dt;
            if (p.drag > 0.0f) {
                float factor = std::max(0.0f, 1.0f - p.drag * dt);
                p.velocity.x *= factor;
                p.velocity.y *= factor;
            }
            p.position.x += p.velocity.x * dt;
            p.position.y += p.velocity.y * dt;
            p.age += dt;

            if (p.age >= p.lifetime) {
                Kill(i);   // swap-remove: la última viene A reemplazarla, no++
            } else {
                ++i;
            }
        }
    }
};

// -----------------------------------------------------------------------------
// 3. Emitter: emisión por segundo + explosiones (burst)
// -----------------------------------------------------------------------------
struct EmitterConfig {
    float rate = 20.0f;             // partículas por segundo (emisión continua)
    float lifetime = 0.5f;          // vida base (con jitter, ver debajo)
    float lifetimeJitter = 0.2f;    // variación aleatoria ±
    float speed = 100.0f;
    float angleDeg = 270.0f;        // dirección base (270 = hacia arriba, +y)
    float angleSpreadDeg = 30.0f;   // semihaz de dispersión
    float sizeStart = 5.0f, sizeEnd = 0.0f;
    float gravity = 300.0f;
    float drag = 1.5f;
};

/* NOTA SDL/LIB: aquí generamos aleatoriedad determinista con un LCG simple
   (mismo resultado en cada run, testable). En un juego real se inyecta un
   generador compartido (std::mt19937 del módulo 19: aleatoriedad determinista
   por semilla, para guardar saves reproducibles). */
class RandomLCG {
    uint32_t state_;
public:
    explicit RandomLCG(uint32_t seed) : state_(seed) {}
    float Next01() {
        state_ = state_ * 1664525u + 1013904223u;
        return static_cast<float>(state_ % 10000) / 10000.0f;
    }
    float Range01(float a, float b) { return a + (b - a) * Next01(); }
};

class ParticleEmitter {
    EmitterConfig cfg_;
    Vector2 position_ {0, 0};
    float emitAccumulator_ = 0.0f;   // el acumulador del timestep variable
    RandomLCG rng_;

public:
    explicit ParticleEmitter(EmitterConfig cfg, uint32_t seed = 7)
        : cfg_(cfg), rng_(seed) {}

    void SetPosition(Vector2 p) { position_ = p; }

    // Emisión CONTINUA: rate*dt se acumula y se sppawnean floor(acc) partículas
    void Emit(float dt, ParticlePool& pool) {
        emitAccumulator_ += cfg_.rate * dt;
        while (emitAccumulator_ >= 1.0f) {
            emitAccumulator_ -= 1.0f;
            SpawnOneEmit(pool);
        }
    }

    // BURST: N partículas de una vez (explosiones, impactos)
    void Burst(int count, ParticlePool& pool) {
        for (int i = 0; i < count; ++i) {
            Particle* p = pool.Spawn();
            if (!p) return;   // pool lleno: dropeo silencioso
            FillParticle(*p);
        }
    }

private:
    void FillParticle(Particle& p) {
        // Ángulo en el semicono [angle-spread, angle+spread]
        float angleDeg = cfg_.angleDeg + rng_.Range01(-cfg_.angleSpreadDeg,
                                                      cfg_.angleSpreadDeg);
        float angle = angleDeg * 3.14159265f / 180.0f;
        float speed = cfg_.speed * rng_.Range01(0.7f, 1.3f);
        float lifetime = cfg_.lifetime + rng_.Range01(-cfg_.lifetimeJitter,
                                                       cfg_.lifetimeJitter);
        p.position = position_;
        p.velocity = { std::cos(angle) * speed, std::sin(angle) * speed };
        p.age = 0.0f;
        p.lifetime = lifetime;
        p.sizeStart = cfg_.sizeStart + rng_.Range01(-1.0f, 1.0f);
        p.sizeEnd = cfg_.sizeEnd;
        p.gravity = cfg_.gravity;
        p.drag = cfg_.drag;
    }

    void SpawnOneEmit(ParticlePool& pool) {
        Particle* p = pool.Spawn();
        if (!p) return;
        FillParticle(*p);
    }
};

// -----------------------------------------------------------------------------
// 4. Render: color y tamaño OVER TIME + blend aditivo
// -----------------------------------------------------------------------------
void RenderParticles(ParticlePool& pool, bool additive) {
    if (additive) BeginBlendMode(BLEND_ADDITIVE);
    for (size_t i = 0; i < pool.AliveCount(); ++i) {
        const Particle& p = pool.At(i);
        float t = (p.lifetime > 0.0f) ? p.age / p.lifetime : 1.0f;
        float size = Lerp(p.sizeStart, p.sizeEnd, t);
        Color c = LerpColor(p.colorStart, p.colorEnd, t);
        DrawCircleV(p.position, size, c);
    }
    if (additive) EndBlendMode();
}

// -----------------------------------------------------------------------------
// Demo: simula fuego (emisión continua) y explosión (burst)
// -----------------------------------------------------------------------------
void demo_particulas() {
    std::cout << "=== Partículas ===\n\n";

    ParticlePool pool(500);       // capacidad fija, cero alloc en frame

    // 1) Fuego: emitter continuo apuntando hacia arriba
    EmitterConfig fireCfg;
    fireCfg.rate = 60.0f;         // 60 partículas/seg
    fireCfg.lifetime = 0.8f;
    fireCfg.speed = 120.0f;
    fireCfg.sizeStart = 6.0f;
    fireCfg.sizeEnd = 1.0f;
    ParticleEmitter fire(fireCfg, 123);
    fire.SetPosition({400, 300});

    const float dt = 1.0f / 60.0f;
    int frames = 120;             // 2 segundos
    size_t maxAlive = 0;
    for (int f = 0; f < frames; ++f) {
        fire.Emit(dt, pool);
        pool.Update(dt);
        maxAlive = std::max(maxAlive, pool.AliveCount());
    }

    // Población estabilizada: nacen 60/s, viven 0.8s promedio => ~48 vivas
    std::cout << "Fuego: 120 frames, max vivas=" << maxAlive << "\n";
    std::cout << "Pool: " << pool.AliveCount() << "/" << pool.Capacity() << "\n";

    // 2) Explosión: burst de 150 + física con gravedad
    EmitterConfig boomCfg;
    boomCfg.rate = 0.0f;          // sin emisión continua
    boomCfg.speed = 250.0f;
    boomCfg.gravity = 400.0f;
    ParticleEmitter boom(boomCfg, 456);
    boom.SetPosition({100, 100});
    boom.Burst(150, pool);
    std::cout << "Tras burst: " << pool.AliveCount() << " vivas\n";

    // Simulo 60 frames más: la gravedad cae las partículas, el emitter quedó
    // fuera de cámara y las motas muertas van librando el pool.
    for (int f = 0; f < 60; ++f) {
        boom.SetPosition({-9999, -9999});   // el emitter queda fuera
        pool.Update(dt);
    }
    std::cout << "Tras 1s: " << pool.AliveCount() << " sobrevivientes (mueren progresivamente)\n";

    // 3) Demo del color/size over time (calculado, no dibujado)
    Particle demo{};
    demo.lifetime = 1.0f;
    demo.colorStart = {255, 200, 50, 255};   // amarillo
    demo.colorEnd   = {180, 20, 0, 0};       // rojo oscuro, alpha 0
    demo.sizeStart = 6;
    demo.sizeEnd = 1;
    for (float t = 0.0f; t <= 1.0f + 0.001f; t += 0.25f) {
        Color c = LerpColor(demo.colorStart, demo.colorEnd, t);
        std::cout << "t=" << t << " color=(" << int(c.r) << "," << int(c.g)
                  << "," << int(c.b) << "," << int(c.a) << ") size="
                  << Lerp(demo.sizeStart, demo.sizeEnd, t) << "\n";
    }
    std::cout << "\n=== FIN LECCIÓN 08 ===\n";
}

int main() {
    demo_particulas();
    return 0;
}
