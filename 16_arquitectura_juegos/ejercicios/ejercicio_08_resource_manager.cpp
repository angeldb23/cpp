/* =============================================================================
   MÓDULO 16 - EJERCICIO 08: Resource Manager — RAII Handles + Cache + Ref Count
   =============================================================================
   DIFICULTAD: Avanzado
   APOYO: Lección 12 (Gestión de recursos entre estados)

   CONSIGNA:
   Implementa ResourceManager con handles RAII y reference counting:
   - ResourceHandle<T, Deleter>: copia incrementa refcount, destructor decrementa
   - GetResource(key): si existe en cache, retorna handle existente (refcount++)
   - LoadResource(key): si no existe, crea, guarda en cache, retorna handle
   - Clear(): libera todos (refcount a 0 -> Deleter)

   TYPES:
   - Texture: struct { int id; } con Load/Unload simulados
   - Deleter: función que imprime "Unload: key"

   TESTS:
   - LoadResource carga una vez, GetResource reusa (cache hit)
   - Handle copy incrementa refcount, destroy decrementa
   - Último destroy llama Deleter
   - Clear libera todo

   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_08_resource_manager.cpp -o ej08 && ./ej08
   ============================================================================= */
#include <unordered_map>
#include <string>
#include <memory>
#include <functional>
#include <vector>
#include "../../include/test_framework.hpp"

struct Texture {
    int id = 0;
    std::string path;
};

int g_textureCounter = 0;
std::vector<std::string> g_unloadLog;

Texture LoadTextureSim(const std::string& path) {
    return {++g_textureCounter, path};
}

void UnloadTextureSim(Texture tex) {
    g_unloadLog.push_back("Unload: " + tex.path);
}

// TODO 1: ResourceHandle with ref counting
// TU CÓDIGO AQUÍ:
template <typename T, typename Deleter>
class ResourceHandle {
    T resource_;
    std::shared_ptr<int> refCount_;
    Deleter deleter_;

public:
    ResourceHandle() : resource_{}, refCount_(std::make_shared<int>(0)), deleter_() {}
    ResourceHandle(T res, Deleter d) : resource_(res), refCount_(std::make_shared<int>(1)), deleter_(d) {}

    // Copy
    ResourceHandle(const ResourceHandle& other) : resource_(other.resource_), refCount_(other.refCount_), deleter_(other.deleter_) {
        if (refCount_) ++*refCount_;
    }
    ResourceHandle& operator=(const ResourceHandle& other) {
        if (this != &other) { Release(); resource_ = other.resource_; refCount_ = other.refCount_; deleter_ = other.deleter_; if (refCount_) ++*refCount_; }
        return *this;
    }
    // Move
    ResourceHandle(ResourceHandle&& other) noexcept : resource_(std::move(other.resource_)), refCount_(std::move(other.refCount_)), deleter_(std::move(other.deleter_)) {
        other.resource_ = {}; other.refCount_ = nullptr;
    }

    ~ResourceHandle() { Release(); }

    void Release() {
        if (refCount_ && --*refCount_ == 0 && deleter_) {
            deleter_(resource_);
            resource_ = {};
        }
    }

    T* Get() { return &resource_; }
    const T* Get() const { return &resource_; }
    T& operator*() { return resource_; }
    const T& operator*() const { return resource_; }
    T* operator->() { return &resource_; }
    const T* operator->() const { return &resource_; }
    explicit operator bool() const { return refCount_ && *refCount_ > 0; }
};

// TODO 2: ResourceManager
// TU CÓDIGO AQUÍ:
class ResourceManager {
    std::unordered_map<std::string, ResourceHandle<Texture, std::function<void(Texture)>>> cache_;

public:
    using TextureHandle = ResourceHandle<Texture, std::function<void(Texture)>>;

    // GetOrLoad: si existe en cache, retorna copy (refcount++); si no, carga, guarda, retorna
    TextureHandle GetOrLoad(const std::string& key) {
        // Pista: auto it = cache_.find(key); if (it != end) return it->second;
        //        Texture tex = LoadTextureSim(key);
        //        TextureHandle handle(tex, [key](Texture t){ UnloadTextureSim(t); });
        //        cache_[key] = handle; return handle;
        return TextureHandle();
    }

    // GetExisting: solo si ya cargado (sin cargar nuevo)
    TextureHandle GetExisting(const std::string& key) {
        // Pista: auto it = cache_.find(key); if (it != end) return it->second; return {};
        return TextureHandle();
    }

    void Clear() {
        // Pista: cache_.clear(); (handles se destruyen -> refcount 0 -> Unload)
        cache_.clear();
    }

    size_t CacheSize() const { return cache_.size(); }
};

// ============= TESTS =============
TEST_CASE(LoadResourceCreatesAndCaches) {
    g_unloadLog.clear();
    ResourceManager rm;
    auto h1 = rm.GetOrLoad("player.png");
    ASSERT_TRUE(h1);
    ASSERT_EQ(h1->path, "player.png");
    ASSERT_EQ(rm.CacheSize(), 1u);
    ASSERT_EQ(g_textureCounter, 1);
}

TEST_CASE(GetOrLoadReusesCached) {
    g_unloadLog.clear();
    ResourceManager rm;
    auto h1 = rm.GetOrLoad("enemy.png");
    auto h2 = rm.GetOrLoad("enemy.png");
    ASSERT_EQ(h1->id, h2->id);  // Misma textura
    ASSERT_EQ(rm.CacheSize(), 1u);
    ASSERT_EQ(g_textureCounter, 1);  // No se creó otra
}

TEST_CASE(HandleCopyIncrementsRefCount) {
    g_unloadLog.clear();
    ResourceManager rm;
    auto h1 = rm.GetOrLoad("tile.png");
    {
        auto h2 = h1;  // Copy
        ASSERT_EQ(rm.CacheSize(), 1u);
    }  // h2 destruido
    ASSERT_TRUE(h1);  // h1 sigue vivo
    ASSERT_TRUE(g_unloadLog.empty());  // No unload aún
}

TEST_CASE(LastDestroyCallsUnload) {
    g_unloadLog.clear();
    ResourceManager rm;
    {
        auto h1 = rm.GetOrLoad("temp.png");
        auto h2 = h1;
    }  // Ambos destruidos
    ASSERT_EQ(g_unloadLog.size(), 1u);
    ASSERT_EQ(g_unloadLog[0], "Unload: temp.png");
}

TEST_CASE(ClearUnloadsAll) {
    g_unloadLog.clear();
    ResourceManager rm;
    rm.GetOrLoad("a.png");
    rm.GetOrLoad("b.png");
    ASSERT_EQ(rm.CacheSize(), 2u);
    rm.Clear();
    ASSERT_EQ(rm.CacheSize(), 0u);
    ASSERT_EQ(g_unloadLog.size(), 2u);
}

TEST_CASE(GetExistingReturnsEmptyIfNotLoaded) {
    ResourceManager rm;
    auto h = rm.GetExisting("nonexistent.png");
    ASSERT_FALSE(h);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 08 (Resource Manager)...\n";
    return RUN_ALL_TESTS();
}