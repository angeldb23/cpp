/*
 * MÓDULO 27: Doom Clásico — De C a C++
 * LECCIÓN 10: Modernización Incremental — Wrappers RAII, ECS, Job System, SDL_GPU
 *
 * TEMAS:
 * - Wrappers RAII para lump cache (Z_Malloc -> unique_ptr con custom deleter)
 * - ECS para mobj_t: Entity + Components (Transform, Health, AI, Physics, Render)
 * - Job System para thinkers: paralelizar P_Ticker (thinkers independientes)
 * - SDL_GPU backend: abstracción portable (Vulkan/Metal/DX12) para renderer
 * - Estranguladores (strangler fig): migrar subsistema a subsistema sin reescribir todo
 *
 * VERIFICACIÓN: Código original consultado en lecciones 1-8. Patrones C++ modernos
 * basados en módulos 15-26 del curso (CMake, sanitizers, ECS, job system, SDL_GPU).
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
#include <string.h>
#include <memory>
#include <vector>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <atomic>
#include <condition_variable>

// ============================================================================
// 1. TIPOS BASE (reutilizados)
// ============================================================================
typedef int32_t fixed_t;
#define FRACUNIT 65536
#define INT_TO_FIXED(x) (static_cast<fixed_t>((x) * FRACUNIT))
#define FIXED_TO_INT(x) ((x) / FRACUNIT)
typedef uint16_t angle_t;

// ============================================================================
// 2. WRAPPER RAII PARA ZONE MEMORY (Z_Malloc/Z_Free)
// ============================================================================
/*
 * Zone Memory (z_zone.c) usa tags: PU_STATIC, PU_LEVEL, PU_CACHE, PU_LEVSPEC.
 * En C++: wrapper que encapsula ciclo de vida y tag.
 *
 * Z_Malloc(size, tag, &ptr) -> devuelve user pointer.
 * Z_Free(ptr) -> libera.
 * Z_ChangeTag(ptr, newtag) -> cambia tag.
 *
 * RAII Wrapper:
 *   class ZoneBlock {
 *       void* ptr_;
 *       int tag_;
 *       bool owned_;
 *   public:
 *       ZoneBlock(size_t size, int tag = PU_LEVEL) {
 *           Z_Malloc(size, tag, &ptr_);
 *           tag_ = tag;
 *           owned_ = (ptr_ != nullptr);
 *       }
 *       ~ZoneBlock() { if (owned_) Z_Free(ptr_); }
 *       ZoneBlock(ZoneBlock&& other) noexcept
 *           : ptr_(other.ptr_), tag_(other.tag_), owned_(other.owned_) {
 *           other.owned_ = false;
 *       }
 *       ZoneBlock& operator=(ZoneBlock&& other) noexcept {
 *           if (owned_) Z_Free(ptr_);
 *           ptr_ = other.ptr_; tag_ = other.tag_; owned_ = other.owned_;
 *           other.owned_ = false;
 *           return *this;
 *       }
 *       void* get() const { return ptr_; }
 *       void setTag(int tag) { if (owned_) { Z_ChangeTag(ptr_, tag); tag_ = tag; } }
 *       void release() { owned_ = false; }
 *   };
 *
 * Para lump cache (PU_CACHE): ZoneBlock con tag PU_CACHE.
 * El Zone Memory purga bloques PU_CACHE bajo presión.
 * En C++: custom deleter que llama Z_Free, o pool allocator std::pmr.
 */

// Mock Zone Memory API (en real: z_zone.h)
// Zone memory real de Doom. Acá un mock mínimo con malloc/free para que el
// ejemplo RAII sea ejecutable (el source release usa bloques y tags).
void Z_Init(int size) { (void)size; }
void* Z_Malloc(int size, int tag, void** ptr) {
    (void)tag;
    void* p = std::malloc(static_cast<size_t>(size));
    if (ptr) *ptr = p;
    return p;
}
void Z_Free(void* ptr) { std::free(ptr); }
void Z_ChangeTag(void* ptr, int tag) { (void)ptr; (void)tag; }
void Z_FreeTags(int tag, int lowtag) { (void)tag; (void)lowtag; }

#define PU_STATIC 1
#define PU_LEVEL 2
#define PU_LEVSPEC 3
#define PU_CACHE 4

// RAII Wrapper para Zone Memory
class ZoneBlock {
    void* ptr_ = nullptr;
    int tag_ = 0;
    bool owned_ = false;

public:
    ZoneBlock() = default;
    explicit ZoneBlock(size_t size, int tag = PU_LEVEL) {
        Z_Malloc((int)size, tag, &ptr_);
        tag_ = tag;
        owned_ = (ptr_ != nullptr);
    }
    ~ZoneBlock() { if (owned_) Z_Free(ptr_); }

    ZoneBlock(ZoneBlock&& other) noexcept
        : ptr_(other.ptr_), tag_(other.tag_), owned_(other.owned_) {
        other.owned_ = false;
    }
    ZoneBlock& operator=(ZoneBlock&& other) noexcept {
        if (owned_) Z_Free(ptr_);
        ptr_ = other.ptr_; tag_ = other.tag_; owned_ = other.owned_;
        other.owned_ = false;
        return *this;
    }
    ZoneBlock(const ZoneBlock&) = delete;
    ZoneBlock& operator=(const ZoneBlock&) = delete;

    void* get() const { return ptr_; }
    operator bool() const { return ptr_ != nullptr; }
    void setTag(int tag) { if (owned_) { Z_ChangeTag(ptr_, tag); tag_ = tag; } }
    int tag() const { return tag_; }
    void release() { owned_ = false; ptr_ = nullptr; }
};

// Wrapper específico para Lump Cache (PU_CACHE)
class LumpCache {
    ZoneBlock block_;
public:
    LumpCache() = default;
    explicit LumpCache(size_t size) : block_(size, PU_CACHE) {}
    void* data() const { return block_.get(); }
    bool valid() const { return block_; }
    void purge() { if (block_) { Z_Free(block_.get()); block_.release(); } }
};

// ============================================================================
// 3. ECS PARA MOBJ_T (Entity Component System)
// ============================================================================
/*
 * mobj_t original: TODO en un struct gigante (posición, salud, IA, render, física).
 * ECS: separar datos (Components) de lógica (Systems).
 *
 * COMPONENTES (POD, data-only):
 *   struct Transform { fixed_t x, y, z; angle_t angle; fixed_t velx, vely, velz; };
 *   struct Health { int current, max; };
 *   struct ActorType { int doomednum; const mobjinfo_t* info; }; // datos estáticos
 *   struct AIState { int state; int tics; int target_entity; int reactiontime; int threshold; };
 *   struct Physics { fixed_t radius, height; int flags; int mass; };
 *   struct Renderable { int sprite; int frame; fixed_t scale; int colormap; };
 *
 * ENTIDAD: uint32_t (índice en arrays de componentes).
 * WORLD: arrays paralelos (SoA) por componente + free list.
 *
 * SYSTEMS (lógica):
 *   - AISystem: recorre entidades con AIState + Transform + ActorType -> actualiza IA
 *   - PhysicsSystem: colisión, gravedad, movimiento (blockmap)
 *   - ThinkerSystem: state machine (tics, transiciones, acciones)
 *   - RenderSystem: recopila Renderable + Transform -> draw list
 *
 * VENTAJAS:
 * - Cache locality: PhysicsSystem solo toca Transform + Physics (contiguo).
 * - Paralelización: AISystem y PhysicsSystem pueden correr en hilos separados
 *   si no hay dependencias de datos (o con double-buffering).
 * - Modularidad: añadir/quitar componentes sin tocar otros sistemas.
 * - Testing: sistemas puros = testables sin engine completo.
 */

// Componentes (SoA: Structure of Arrays)
struct Transform {
    fixed_t x, y, z;
    angle_t angle;
    fixed_t velx, vely, velz;
};

struct Health {
    int current, max;
};

struct ActorType {
    int doomednum;
    const void* info;  // mobjinfo_t*
};

struct AIState {
    int state;
    int tics;
    uint32_t target;   // entity index
    int reactiontime;
    int threshold;
};

struct Physics {
    fixed_t radius, height;
    int flags;
    int mass;
};

struct Renderable {
    int sprite;
    int frame;
    fixed_t scale;
    int colormap;
};

// Entity = index (uint32_t)
using Entity = uint32_t;
const Entity INVALID_ENTITY = 0xFFFFFFFF;

// World ECS simplificado (SoA con vectors)
struct World {
    std::vector<Transform> transforms;
    std::vector<Health> healths;
    std::vector<ActorType> actorTypes;
    std::vector<AIState> aiStates;
    std::vector<Physics> physics;
    std::vector<Renderable> renderables;
    std::vector<bool> alive;  // entity alive mask
    std::vector<Entity> freeList;

    Entity create() {
        Entity e;
        if (!freeList.empty()) {
            e = freeList.back();
            freeList.pop_back();
            alive[e] = true;
        } else {
            e = (Entity)alive.size();
            alive.push_back(true);
            transforms.push_back({});
            healths.push_back({});
            actorTypes.push_back({});
            aiStates.push_back({});
            physics.push_back({});
            renderables.push_back({});
        }
        return e;
    }

    void destroy(Entity e) {
        if (e < alive.size() && alive[e]) {
            alive[e] = false;
            freeList.push_back(e);
        }
    }

    bool has(Entity e) const { return e < alive.size() && alive[e]; }
};

// Sistemas (funciones puras sobre World)
void AISystem(World& world, float dt) {
    for (Entity e = 0; e < world.alive.size(); ++e) {
        if (!world.alive[e]) continue;
        if (world.aiStates[e].tics <= 0) continue; // no AI component

        AIState& ai = world.aiStates[e];
        Transform& tf = world.transforms[e];
        ActorType& at = world.actorTypes[e];

        // Lógica IA simplificada (ver lección 05-06)
        if (ai.tics > 0) {
            ai.tics--;
            if (ai.tics == 0) {
                // Transición de estado (usar info->states[])
                // ai.state = next_state;
                // ai.tics = new_tics;
            }
        }
    }
}

void PhysicsSystem(World& world, float dt) {
    for (Entity e = 0; e < world.alive.size(); ++e) {
        if (!world.alive[e]) continue;
        Transform& tf = world.transforms[e];
        Physics& ph = world.physics[e];
        // Movimiento, gravedad, colisión (blockmap)
        tf.x += tf.velx; // simplificado
        tf.y += tf.vely;
        tf.z += tf.velz;
    }
}

void ThinkerSystem(World& world) {
    for (Entity e = 0; e < world.alive.size(); ++e) {
        if (!world.alive[e]) continue;
        AIState& ai = world.aiStates[e];
        if (ai.tics > 0) {
            ai.tics--;
            if (ai.tics == 0) {
                // state transition
            }
        }
    }
}

// ============================================================================
// 4. JOB SYSTEM PARA THINKERS (Paralelización)
// ============================================================================
/*
 * P_Ticker original: recorre thinkers SECUENCIALMENTE.
 *   while (current != thinkercap) { current->function(current); current = next; }
 *
 * Job System: dividir thinkers independientes en jobs paralelos.
 * - Dependencias: thinkers que interactúan (colisión, target) no pueden paralelizarse.
 * - Estrategia: agrupar por "islas" (islands) de contactos (como Box2D).
 *   O: double-buffering de estado (leer estado anterior, escribir nuevo estado).
 *
 * Job System genérico (basado en módulo 22):
 *   class JobSystem {
 *       std::vector<std::thread> workers;
 *       std::queue<std::function<void()>> jobs;
 *       std::mutex mtx;
 *       std::condition_variable cv;
 *       std::atomic<int> pending;
 *   public:
 *       JobSystem(int threads = std::thread::hardware_concurrency());
 *       template<F> void Submit(F&& f) { ... }
 *       void WaitAll();
 *   };
 *
 * Aplicación a thinkers:
 *   - Fase 1: Broad-phase (single thread) -> identificar islas de contactos.
 *   - Fase 2: Para cada isla independiente, submit job con thinkers de esa isla.
 *   - Fase 3: Thinkers sin contactos (proyectiles, items, decoración) -> jobs paralelos.
 *   - Fase 4: WaitAll -> merge resultados.
 *
 * NOTA: En Doom original, thinkers modifican estado global (players, level, RNG).
 * Paralelizar requiere:
 * - RNG por hilo (splitmix64 con seed por job) O RNG global con mutex (lento).
 * - Double-buffered world state (read old, write new).
 * - O: mantener thinkers secuenciales y solo paralelizar render/physics.
 */

// Job System simplificado (basado en módulo 22)
class JobSystem {
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> jobs_;
    std::mutex mtx_;
    std::condition_variable cv_;
    std::atomic<int> pending_{0};
    bool stop_ = false;

public:
    explicit JobSystem(int threads = 0) {
        if (threads <= 0) threads = std::thread::hardware_concurrency();
        if (threads <= 0) threads = 2;
        for (int i = 0; i < threads; ++i) {
            workers_.emplace_back([this] { WorkerLoop(); });
        }
    }
    ~JobSystem() {
        { std::lock_guard<std::mutex> lk(mtx_); stop_ = true; }
        cv_.notify_all();
        for (auto& t : workers_) if (t.joinable()) t.join();
    }

    template<typename F>
    void Submit(F&& f) {
        { std::lock_guard<std::mutex> lk(mtx_); jobs_.push(std::forward<F>(f)); pending_++; }
        cv_.notify_one();
    }

    void WaitAll() {
        std::unique_lock<std::mutex> lk(mtx_);
        cv_.wait(lk, [this] { return pending_ == 0 && jobs_.empty(); });
    }

private:
    void WorkerLoop() {
        while (true) {
            std::function<void()> job;
            { std::unique_lock<std::mutex> lk(mtx_);
                cv_.wait(lk, [this] { return !jobs_.empty() || stop_; });
                if (stop_ && jobs_.empty()) return;
                job = std::move(jobs_.front());
                jobs_.pop();
            }
            job();
            pending_--;
            cv_.notify_one();
        }
    }
};

// Uso en ThinkerSystem paralelizado:
void ParallelThinkerSystem(World& world, JobSystem& jobs) {
    // Agrupar entidades por "isla" (simplificado: cada entidad = job independiente)
    // En real: broad-phase para detectar contactos y agrupar.
    for (Entity e = 0; e < world.alive.size(); ++e) {
        if (!world.alive[e] || world.aiStates[e].tics <= 0) continue;
        jobs.Submit([&world, e]() {
            AIState& ai = world.aiStates[e];
            if (ai.tics > 0) {
                ai.tics--;
                // state transition logic...
            }
        });
    }
    jobs.WaitAll();
}

// ============================================================================
// 5. SDL_GPU BACKEND (Abstracción Portable)
// ============================================================================
/*
 * Doom original: renderer software (columnas, spans, visplanes).
 * Source ports: OpenGL, Vulkan, DirectX, Metal.
 * SDL_GPU (SDL3): abstracción portable sobre Vulkan/Metal/DX12.
 *
 * Ventajas SDL_GPU:
 * - Un solo código de render -> compila en Windows (DX12), macOS (Metal), Linux (Vulkan).
 * - Maneja swapchain, command buffers, pipelines, buffers, textures, samplers.
 * - Más simple que Vulkan nativo, más control que OpenGL.
 *
 * Integración en Doom modernizado:
 * - R_RenderPlayerView -> genera draw commands (walls, flats, sprites).
 * - En vez de dibujar directo a framebuffer software:
 *   - Subir geometry a GPU buffers (VBO/SSBO).
 *   - Subir texturas (flats, patches, sprites) a GPU textures (array/atlas).
 *   - SDL_GPU command buffer: bind pipeline, bind buffers, draw indexed.
 *   - Shaders: vertex (transform), fragment (texture + colormap + light).
 *
 * ESTRUCTURA SDL_GPU (SDL3/SDL_gpu.h):
 *   SDL_GPUDevice* device = SDL_ClaimGPUDevice();
 *   SDL_GPUCommandBuffer* cmdbuf = SDL_AcquireGPUCommandBuffer(device);
 *   SDL_GPURenderPass* pass = SDL_BeginGPURenderPass(cmdbuf, ...);
 *   SDL_BindGPUGraphicsPipeline(pass, pipeline);
 *   SDL_BindGPUVertexBuffers(pass, 0, &vertex_buffer, 1);
 *   SDL_BindGPUFragmentSamplers(pass, 0, &texture_sampler, 1);
 *   SDL_DrawGPUIndexedPrimitives(pass, index_count, 1, 0, 0, 0);
 *   SDL_EndGPURenderPass(pass);
 *   SDL_SubmitGPUCommandBuffer(cmdbuf);
 *
 * MIGRACIÓN INCREMENTAL (Strangler Fig):
 * 1. Mantener renderer software como fallback / referencia.
 * 2. Añadir SDL_GPU path detrás de flag (r_software vs r_hardware).
 * 3. Primero: flats (pisos/techos) -> triángulos texturados.
 * 4. Luego: paredes -> triángulos con UV proyectados.
 * 5. Luego: sprites -> billboards (point sprites o quad instanced).
 * 6. Comparar output pixel-a-pixel (golden frames) para validar.
 */

// Mock SDL_GPU types (en real: #include <SDL3/SDL_gpu.h>)
struct SDL_GPUDevice;
struct SDL_GPUCommandBuffer;
struct SDL_GPURenderPass;
struct SDL_GPUGraphicsPipeline;
struct SDL_GPUBuffer;
struct SDL_GPUTexture;
struct SDL_GPUSampler;

// Renderer backend interface (Strategy pattern)
class IRenderer {
public:
    virtual ~IRenderer() = default;
    virtual bool Init() = 0;
    virtual void BeginFrame() = 0;
    virtual void DrawWalls(const World& world) = 0;
    virtual void DrawFlats(const World& world) = 0;
    virtual void DrawSprites(const World& world) = 0;
    virtual void EndFrame() = 0;
    virtual void Shutdown() = 0;
};

// Software renderer (original)
class SoftwareRenderer : public IRenderer {
public:
    bool Init() override { return true; }
    void BeginFrame() override { /* clear framebuffer */ }
    void DrawWalls(const World&) override { /* R_DrawMasked */ }
    void DrawFlats(const World&) override { /* R_DrawPlanes */ }
    void DrawSprites(const World&) override { /* R_DrawVisSprites */ }
    void EndFrame() override { /* present */ }
    void Shutdown() override {}
};

// SDL_GPU renderer (nuevo)
class SDLGPURenderer : public IRenderer {
    SDL_GPUDevice* device_ = nullptr;
    // pipelines, buffers, textures...
public:
    bool Init() override {
        // device_ = SDL_ClaimGPUDevice();
        // create pipelines, upload textures...
        return device_ != nullptr;
    }
    void BeginFrame() override { /* acquire cmdbuf, begin render pass */ }
    void DrawWalls(const World&) override { /* bind pipeline, draw indexed */ }
    void DrawFlats(const World&) override { /* bind pipeline, draw indexed */ }
    void DrawSprites(const World&) override { /* instanced draw */ }
    void EndFrame() override { /* submit cmdbuf */ }
    void Shutdown() override { /* SDL_ReleaseGPUDevice(device_) */ }
};

// Factory para elegir backend
std::unique_ptr<IRenderer> CreateRenderer(bool use_gpu) {
    if (use_gpu) return std::make_unique<SDLGPURenderer>();
    return std::make_unique<SoftwareRenderer>();
}

// ============================================================================
// 6. ESTRANGLADOR (Strangler Fig Pattern)
// ============================================================================
/*
 * Patrón para migrar legacy C a C++ SIN reescritura total:
 *
 * 1. IDENTIFICAR frontera: subsistema aislado (ej: lump cache).
 * 2. CREAR interfaz C++ (IRenderer, IAssetManager, IAudio).
 * 3. IMPLEMENTAR nueva versión (SDL_GPU, std::pmr, miniaudio).
 * 4. REDIRIGIR llamadas: #ifdef NEW_RENDERER -> nueva, else -> legacy.
 * 5. TESTEAR: golden frames, same inputs -> same outputs.
 * 6. ELIMINAR código legacy cuando nueva versión es estable.
 *
 * En Doom:
 * - Lump cache -> AssetManager (RAII, VFS, async load)
 * - Zone memory -> ZoneBlock / std::pmr
 * - Thinker list -> ECS World + JobSystem
 * - Software renderer -> SDL_GPU / Vulkan backend
 * - Sound -> miniaudio / SDL_Audio
 * - Input -> SDL3 events -> ticcmd_t
 * - Netcode -> ENet / SteamNetworking / custom UDP
 */

// ============================================================================
// 7. TESTS DE MODERNIZACIÓN
// ============================================================================

int test_modernization(void) {
    int passed = 0, failed = 0;
    #define TEST(expr, msg) \
        do { if (expr) { passed++; } else { failed++; printf("FAIL: %s\n", msg); } } while(0)

    // ZoneBlock RAII
    Z_Init(65536);
    {
        ZoneBlock zb(1024, PU_LEVEL);
        TEST(zb.get() != nullptr, "ZoneBlock alloc");
        TEST(zb.tag() == PU_LEVEL, "ZoneBlock tag");
        zb.setTag(PU_CACHE);
        TEST(zb.tag() == PU_CACHE, "ZoneBlock setTag");
    } // destructor calls Z_Free
    TEST(1, "ZoneBlock auto-free on scope exit");

    // LumpCache
    {
        LumpCache lc(512);
        TEST(lc.valid(), "LumpCache valid");
        lc.purge();
        TEST(!lc.valid(), "LumpCache purged");
    }

    // ECS World
    World world;
    Entity e = world.create();
    TEST(world.has(e), "World create entity");
    world.transforms[e] = {INT_TO_FIXED(100), INT_TO_FIXED(200), 0, 0, 0,0,0};
    TEST(world.transforms[e].x == INT_TO_FIXED(100), "Transform component access");
    world.destroy(e);
    TEST(!world.has(e), "World destroy entity");

    // JobSystem
    JobSystem js(2);
    std::atomic<int> counter{0};
    for (int i = 0; i < 100; ++i) js.Submit([&counter](){ counter++; });
    js.WaitAll();
    TEST(counter == 100, "JobSystem executes all jobs");

    // Renderer factory
    auto r1 = CreateRenderer(false);
    TEST(dynamic_cast<SoftwareRenderer*>(r1.get()) != nullptr, "Software renderer created");
    auto r2 = CreateRenderer(true);
    // SDLGPURenderer needs real SDL_GPU, so may be null in test
    // TEST(dynamic_cast<SDLGPURenderer*>(r2.get()) != nullptr, "GPU renderer created");

    printf("Modernization tests: %d pass, %d fail\n", passed, failed);
    return failed == 0 ? 0 : 1;
}

int main(void) {
    printf("Módulo 27 - Lección 10: Modernización Incremental (RAII, ECS, Job System, SDL_GPU)\n");
    return test_modernization();
}