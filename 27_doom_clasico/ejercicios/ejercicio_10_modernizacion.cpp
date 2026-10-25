/*
 * MÓDULO 27: Doom Clásico — De C a C++
 * EJERCICIO 10: Modernización — RAII Wrappers, ECS, Job System, SDL_GPU
 *
 * BASADO EN: Lección 10 (modernizacion) y Lección 14 (modernizacion_avanzada)
 *
 * OBJETIVO: Implementar wrappers RAII para Zone Memory, ECS básico, Job System.
 */

#include <stdint.h>
#include <stdlib.h>
#include <memory>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <functional>
#include "../../include/test_framework.hpp"

// ============================================================================
// MOCK ZONE MEMORY API
// ============================================================================

#define PU_STATIC 1
#define PU_LEVEL 2
#define PU_CACHE 4

void Z_Init(int size) {}
void* Z_Malloc(int size, int tag, void** ptr) {
    *ptr = malloc(size);
    return *ptr;
}
void Z_Free(void* ptr) { free(ptr); }
void Z_ChangeTag(void* ptr, int tag) { (void)ptr; (void)tag; }

// ============================================================================
// 1. ZONEBLOCK RAII WRAPPER
// ============================================================================

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

    void* get() const {
        // TODO: devolver el puntero administrado por este ZoneBlock
        return nullptr;
    }
    void setTag(int tag) { if (owned_) { Z_ChangeTag(ptr_, tag); tag_ = tag; } }
    int tag() const { return tag_; }
    explicit operator bool() const { return ptr_ != nullptr; }
};

// ============================================================================
// 2. ECS WORLD (SoA)
// ============================================================================

using fixed_t = int32_t;
#define FRACUNIT 65536
#define INT_TO_FIXED(x) ((x) << 16)

struct Transform { fixed_t x, y, z; };
struct Health { int current, max; };
struct AIState { int state, tics; };

using Entity = uint32_t;
const Entity INVALID_ENTITY = 0xFFFFFFFF;

struct World {
    std::vector<Transform> transforms;
    std::vector<Health> healths;
    std::vector<AIState> aiStates;
    std::vector<bool> alive;
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
            aiStates.push_back({});
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

void AISystem(World& world) {}
void PhysicsSystem(World& world) {}

// ============================================================================
// 3. JOB SYSTEM
// ============================================================================

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

// ============================================================================
// 4. RENDERER STRATEGY
// ============================================================================

class IRenderer {
public:
    virtual ~IRenderer() = default;
    virtual bool Init() = 0;
    virtual void BeginFrame() = 0;
    virtual void DrawWalls(const World&) = 0;
    virtual void DrawFlats(const World&) = 0;
    virtual void DrawSprites(const World&) = 0;
    virtual void EndFrame() = 0;
    virtual void Shutdown() = 0;
};

class SoftwareRenderer : public IRenderer {
public:
    bool Init() override { return true; }
    void BeginFrame() override {}
    void DrawWalls(const World&) override {}
    void DrawFlats(const World&) override {}
    void DrawSprites(const World&) override {}
    void EndFrame() override {}
    void Shutdown() override {}
};

class SDLGPURenderer : public IRenderer {
public:
    bool Init() override { return false; }
    void BeginFrame() override {}
    void DrawWalls(const World&) override {}
    void DrawFlats(const World&) override {}
    void DrawSprites(const World&) override {}
    void EndFrame() override {}
    void Shutdown() override {}
};

std::unique_ptr<IRenderer> CreateRenderer(bool use_gpu) {
    if (use_gpu) return std::make_unique<SDLGPURenderer>();
    return std::make_unique<SoftwareRenderer>();
}

// ============================================================================
// TESTS (NO MODIFICAR)
// ============================================================================

TEST_CASE(TestZoneBlock) {
    Z_Init(65536);
    {
        ZoneBlock zb(1024, PU_LEVEL);
        ASSERT_TRUE(zb.get() != nullptr);
        zb.setTag(PU_CACHE);
        ASSERT_TRUE(true);
    }
    ASSERT_TRUE(true);
}

TEST_CASE(TestECSWorld) {
    World world;
    Entity e = world.create();
    ASSERT_TRUE(world.has(e));
    world.transforms[e] = {INT_TO_FIXED(100)};
    ASSERT_EQ(world.transforms[e].x, INT_TO_FIXED(100));
    world.destroy(e);
    ASSERT_FALSE(world.has(e));
}

TEST_CASE(TestJobSystem) {
    JobSystem js(2);
    std::atomic<int> counter{0};
    for (int i = 0; i < 100; ++i) js.Submit([&counter](){ counter.fetch_add(1); });
    js.WaitAll();
    ASSERT_EQ(counter.load(), 100);
}

TEST_CASE(TestRendererFactory) {
    auto r1 = CreateRenderer(false);
    ASSERT_TRUE(r1 != nullptr);
    auto r2 = CreateRenderer(true);
    ASSERT_TRUE(true);
}

int main() {
    return RUN_ALL_TESTS();
}