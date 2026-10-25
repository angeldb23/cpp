/* =============================================================================
   MÓDULO 22 - EJERCICIO 05: Resource Manager — Handles, Ref Counting, Async Load
   =============================================================================
   DIFICULTAD: Medio-Alto
   APOYO: Lección 07 (Resource Manager), Lección 08 (Async/Deps/Cache)

   CONSIGNA:
   Implementa ResourceManager con:
   - AssetHandle (id + generation)
   - AssetSlot con ref_count, data (type-erased), generation
   - Load<T>(path) síncrono: retorna handle, carga si no existe
   - LoadAsync<T>(path, callback) asíncrono: retorna handle inmediato, callback al terminar
   - Get<T>(handle) type-safe access
   - Acquire/Release para ref counting manual
   - RAII AssetRef<T> que auto acquire/release

   TESTS:
   - Load mismo path retorna mismo handle (dedup)
   - Acquire/Release incrementa/decrementa ref_count
   - LoadAsync retorna handle válido antes de cargar
   - Callback se ejecuta cuando carga termina
   - AssetRef RAII libera al salir de scope
   ============================================================================= */
#include <vector>
#include <unordered_map>
#include <string>
#include <memory>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <queue>
#include <condition_variable>
#include <atomic>
#include <cstdint>
#include <cassert>
#include "../../include/test_framework.hpp"

struct AssetHandle { uint32_t id = 0, gen = 0; bool IsValid() const { return id != 0; } };
struct AssetHandleHash { size_t operator()(AssetHandle h) const noexcept { return (size_t(h.id)<<32)^h.gen; } };

struct AssetSlot {
    std::shared_ptr<void> data;
    std::atomic<uint32_t> ref_count{0};
    uint32_t gen = 0;
    std::string path, type;
    bool loading = false;
    std::vector<std::function<void(AssetHandle)>> callbacks;

    AssetSlot() = default;
    AssetSlot(AssetSlot&& other) noexcept
        : data(std::move(other.data))
        , ref_count(other.ref_count.load())
        , gen(other.gen)
        , path(std::move(other.path))
        , type(std::move(other.type))
        , loading(other.loading)
        , callbacks(std::move(other.callbacks)) {}
    AssetSlot& operator=(AssetSlot&& other) noexcept {
        if(this != &other) {
            data = std::move(other.data);
            ref_count.store(other.ref_count.load());
            gen = other.gen;
            path = std::move(other.path);
            type = std::move(other.type);
            loading = other.loading;
            callbacks = std::move(other.callbacks);
        }
        return *this;
    }
    AssetSlot(const AssetSlot&) = delete;
    AssetSlot& operator=(const AssetSlot&) = delete;
};

// ============================================================================
// TODO: Implementa ResourceManager
// ============================================================================
class ResourceManager {
    std::unordered_map<std::string, std::function<std::shared_ptr<void>(const std::string&)>> loaders_;
    std::unordered_map<std::string, AssetHandle> path_to_handle_;
    std::vector<AssetSlot> slots_{1};  // index 0 = null
    std::shared_mutex rw_mutex_;

    struct JobQueue {
        std::mutex m; std::queue<std::function<void()>> q; std::condition_variable cv;
        std::thread worker; std::atomic<bool> stop{false};
        JobQueue(){ worker=std::thread([this]{ Run(); }); }
        ~JobQueue(){ stop=true; cv.notify_one(); if(worker.joinable()) worker.join(); }
        void Push(std::function<void()> f){ std::lock_guard l(m); q.push(std::move(f)); cv.notify_one(); }
        void Run(){ while(!stop){ std::function<void()> f; { std::unique_lock l(m); cv.wait(l,[this]{return !q.empty()||stop;}); if(stop) break; f=std::move(q.front()); q.pop(); } f(); } }
    } job_queue_;

    AssetHandle ReserveSlot(const std::string& path, const std::string& type_name) {
        std::unique_lock lock(rw_mutex_);
        uint32_t id = slots_.size();
        slots_.push_back({});
        slots_[id].gen = 1;
        slots_[id].path = path;
        slots_[id].type = type_name;
        slots_[id].ref_count.store(1);
        AssetHandle h{id, 1};
        path_to_handle_[path] = h;
        return h;
    }

public:
    template<typename T>
    using LoaderFunc = std::function<std::shared_ptr<T>(const std::string&)>;

    template<typename T>
    void RegisterLoader(LoaderFunc<T> loader) {
        loaders_[typeid(T).name()] = [loader](const std::string& p){ return std::static_pointer_cast<void>(loader(p)); };
    }

    // TU CÓDIGO AQUÍ: Implementa Load, LoadAsync, Get, Acquire, Release, etc.
    template<typename T>
    AssetHandle Load(const std::string& path) {
        (void)path; return {};
    }

    template<typename T>
    AssetHandle LoadAsync(const std::string& path, std::function<void(AssetHandle)> cb = {}) {
        (void)path; (void)cb; return {};
    }

    template<typename T>
    std::shared_ptr<T> Get(AssetHandle h) {
        (void)h; return nullptr;
    }

    void Acquire(AssetHandle h) { (void)h; }
    void Release(AssetHandle h) { (void)h; }
    bool IsReady(AssetHandle h) { (void)h; return false; }

    // Test-only accessor
    uint32_t GetRefCountForTesting(AssetHandle h) const {
        if(h.id < slots_.size()) return slots_[h.id].ref_count.load();
        return 0;
    }
};

// ============================================================================
// RAII HANDLE
// ============================================================================
template<typename T>
class AssetRef {
    ResourceManager* mgr_ = nullptr;
    AssetHandle handle_;
public:
    AssetRef() = default;
    AssetRef(ResourceManager* mgr, AssetHandle h) : mgr_(mgr), handle_(h) { if(mgr_) mgr_->Acquire(handle_); }
    AssetRef(const AssetRef& other) : mgr_(other.mgr_), handle_(other.handle_) { if(mgr_) mgr_->Acquire(handle_); }
    AssetRef(AssetRef&& other) noexcept : mgr_(other.mgr_), handle_(other.handle_) { other.mgr_=nullptr; other.handle_={}; }
    ~AssetRef() { if(mgr_) mgr_->Release(handle_); }
    AssetRef& operator=(const AssetRef& other) {
        if(this!=&other){ if(mgr_) mgr_->Release(handle_); mgr_=other.mgr_; handle_=other.handle_; if(mgr_) mgr_->Acquire(handle_); } return *this;
    }
    AssetRef& operator=(AssetRef&& other) noexcept {
        if(this!=&other){ if(mgr_) mgr_->Release(handle_); mgr_=other.mgr_; handle_=other.handle_; other.mgr_=nullptr; other.handle_={}; } return *this;
    }
    T* operator->() { return mgr_ ? mgr_->Get<T>(handle_).get() : nullptr; }
    T* Get() { return operator->(); }
    bool IsValid() const { return handle_.IsValid() && mgr_ && mgr_->IsReady(handle_); }
    AssetHandle Handle() const { return handle_; }
};

// ============================================================================
// TEST LOADERS
// ============================================================================
struct TextureData { int w=0, h=0; std::vector<uint8_t> pixels; };
struct ShaderData { std::string source; };

std::shared_ptr<TextureData> LoadTexture(const std::string& path) {
    auto t = std::make_shared<TextureData>(); t->w = 64; t->h = 64; t->pixels.resize(64*64*4, 255); return t;
}
std::shared_ptr<ShaderData> LoadShader(const std::string& path) {
    auto s = std::make_shared<ShaderData>(); s->source = "shader: " + path; return s;
}

// ============================================================================
// TESTS
// ============================================================================
TEST_CASE(LoadSamePathReturnsSameHandle) {
    ResourceManager rm;
    rm.RegisterLoader<TextureData>(LoadTexture);
    AssetHandle h1 = rm.Load<TextureData>("assets/player.png");
    AssetHandle h2 = rm.Load<TextureData>("assets/player.png");
    ASSERT_EQ(h1.id, h2.id);
    ASSERT_EQ(h1.gen, h2.gen);
}

TEST_CASE(AcquireReleaseRefCount) {
    ResourceManager rm;
    rm.RegisterLoader<TextureData>(LoadTexture);
    AssetHandle h = rm.Load<TextureData>("assets/test.png");
    ASSERT_EQ(rm.GetRefCountForTesting(h), 1);
    rm.Acquire(h);
    ASSERT_EQ(rm.GetRefCountForTesting(h), 2);
    rm.Release(h);
    ASSERT_EQ(rm.GetRefCountForTesting(h), 1);
}

TEST_CASE(LoadAsyncReturnsHandleImmediately) {
    ResourceManager rm;
    rm.RegisterLoader<ShaderData>(LoadShader);
    bool callback_called = false;
    AssetHandle h = rm.LoadAsync<ShaderData>("assets/test.glsl", [&](AssetHandle){
        callback_called = true;
    });
    ASSERT_TRUE(h.IsValid());
    // Handle válido pero data puede no estar lista aún
    ASSERT_FALSE(rm.IsReady(h));  // Puede ser false inicialmente
}

TEST_CASE(AsyncCallbackExecutesOnLoad) {
    ResourceManager rm;
    rm.RegisterLoader<TextureData>(LoadTexture);
    std::atomic<int> callback_count{0};
    AssetHandle h = rm.LoadAsync<TextureData>("assets/cb_test.png", [&](AssetHandle){
        callback_count++;
    });
    // Esperar a que termine (en real: wait o poll)
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    // Callback debe haberse ejecutado
    ASSERT_GE(callback_count.load(), 1);
    ASSERT_TRUE(rm.IsReady(h));
}

TEST_CASE(AssetRefRAIIAcquiresAndReleases) {
    ResourceManager rm;
    rm.RegisterLoader<TextureData>(LoadTexture);
    AssetHandle handle;
    {
        AssetRef<TextureData> ref(&rm, rm.Load<TextureData>("assets/raii.png"));
        handle = ref.Handle();
        ASSERT_EQ(rm.GetRefCountForTesting(handle), 2);  // Load + Ref
        ASSERT_TRUE(ref.IsValid());
    }  // ref destruido aquí
    ASSERT_EQ(rm.GetRefCountForTesting(handle), 1);  // Solo Load
}

TEST_CASE(GetReturnsTypeSafePointer) {
    ResourceManager rm;
    rm.RegisterLoader<TextureData>(LoadTexture);
    rm.RegisterLoader<ShaderData>(LoadShader);
    AssetHandle tex_h = rm.Load<TextureData>("assets/tex.png");
    AssetHandle sh_h = rm.Load<ShaderData>("assets/sh.glsl");

    auto tex = rm.Get<TextureData>(tex_h);
    auto sh = rm.Get<ShaderData>(sh_h);
    ASSERT_TRUE(tex);
    ASSERT_EQ(tex->w, 64);
    ASSERT_TRUE(sh);
    ASSERT_TRUE(sh->source.find("sh.glsl") != std::string::npos);

    // Wrong type returns nullptr
    auto wrong = rm.Get<ShaderData>(tex_h);
    ASSERT_FALSE(wrong);
}

TEST_CASE(GenerationInvalidatesOldHandles) {
    ResourceManager rm;
    rm.RegisterLoader<TextureData>(LoadTexture);
    AssetHandle h = rm.Load<TextureData>("assets/gen_test.png");
    uint32_t old_gen = h.gen;
    // Simular reload (en real: Reload method)
    // Para test: crear nuevo slot manualmente
    // Aquí solo verificamos que Get falla con gen incorrecto
    AssetHandle bad_h{h.id, old_gen + 1};
    ASSERT_FALSE(rm.Get<TextureData>(bad_h));
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 05 (Resource Manager)...\n";
    return RUN_ALL_TESTS();
}