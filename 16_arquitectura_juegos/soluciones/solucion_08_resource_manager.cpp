/* =============================================================================
   MÓDULO 16 - SOLUCIÓN 08: Resource Manager — RAII Handles + Cache + Ref Count
   =============================================================================
   Implementación correcta: shared_ptr con custom deleter en el control block.
   - Cache guarda shared_ptr con deleter
   - Handles copian el shared_ptr (mismo control block, mismo deleter)
   - Último shared_ptr destruido llama deleter automáticamente
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

/*
 * ResourceHandle: wrapper simple sobre shared_ptr<T>
 * El deleter ya está en el control block del shared_ptr.
 */
template <typename T>
class ResourceHandle {
    std::shared_ptr<T> ptr_;

public:
    ResourceHandle() = default;
    explicit ResourceHandle(std::shared_ptr<T> p) : ptr_(std::move(p)) {}

    ResourceHandle(const ResourceHandle&) = default;
    ResourceHandle& operator=(const ResourceHandle&) = default;
    ResourceHandle(ResourceHandle&&) noexcept = default;
    ResourceHandle& operator=(ResourceHandle&&) noexcept = default;

    T* Get() { return ptr_.get(); }
    const T* Get() const { return ptr_.get(); }
    T& operator*() { return *ptr_; }
    const T& operator*() const { return *ptr_; }
    T* operator->() { return ptr_.get(); }
    const T* operator->() const { return ptr_.get(); }
    explicit operator bool() const { return ptr_ != nullptr; }
    const std::shared_ptr<T>& SharedPtr() const { return ptr_; }
};

class ResourceManager {
    // Cache con shared_ptr que YA TIENE el deleter en su control block
    std::unordered_map<std::string, std::shared_ptr<Texture>> cache_;

    // Helper para crear shared_ptr con deleter
    static std::shared_ptr<Texture> MakeSharedWithDeleter(Texture tex, const std::string& key) {
        return std::shared_ptr<Texture>(
            new Texture(std::move(tex)),
            [key](Texture* t) { UnloadTextureSim(*t); delete t; }
        );
    }

public:
    using TextureHandle = ResourceHandle<Texture>;

    TextureHandle GetOrLoad(const std::string& key) {
        auto it = cache_.find(key);
        if (it != cache_.end()) {
            return TextureHandle(it->second);
        }
        Texture tex = LoadTextureSim(key);
        auto shared = MakeSharedWithDeleter(std::move(tex), key);
        cache_[key] = shared;
        return TextureHandle(shared);
    }

    TextureHandle GetExisting(const std::string& key) {
        auto it = cache_.find(key);
        if (it != cache_.end()) {
            return TextureHandle(it->second);
        }
        return TextureHandle();
    }

    void Clear() {
        cache_.clear();  // Destruye shared_ptrs -> llama deleter si refcount=0
    }

    size_t CacheSize() const { return cache_.size(); }
};

// ============= TESTS =============
TEST_CASE(LoadResourceCreatesAndCaches) {
    g_unloadLog.clear();
    g_textureCounter = 0;
    ResourceManager rm;
    auto h1 = rm.GetOrLoad("player.png");
    ASSERT_TRUE(h1);
    ASSERT_EQ(h1->path, "player.png");
    ASSERT_EQ(rm.CacheSize(), 1u);
    ASSERT_EQ(g_textureCounter, 1);
}

TEST_CASE(GetOrLoadReusesCached) {
    g_unloadLog.clear();
    g_textureCounter = 0;
    ResourceManager rm;
    auto h1 = rm.GetOrLoad("enemy.png");
    auto h2 = rm.GetOrLoad("enemy.png");
    ASSERT_EQ(h1->id, h2->id);
    ASSERT_EQ(rm.CacheSize(), 1u);
    ASSERT_EQ(g_textureCounter, 1);
}

TEST_CASE(HandleCopyIncrementsRefCount) {
    g_unloadLog.clear();
    g_textureCounter = 0;
    ResourceManager rm;
    auto h1 = rm.GetOrLoad("tile.png");
    {
        auto h2 = h1;
        ASSERT_EQ(rm.CacheSize(), 1u);
    }
    ASSERT_TRUE(h1);
    ASSERT_TRUE(g_unloadLog.empty());
}

TEST_CASE(LastDestroyCallsUnload) {
    g_unloadLog.clear();
    g_textureCounter = 0;
    ResourceManager rm;
    {
        auto h1 = rm.GetOrLoad("temp.png");
        auto h2 = h1;
    }
    // Cache mantiene shared_ptr, unload no ocurre hasta Clear()
    ASSERT_EQ(rm.CacheSize(), 1u);
    rm.Clear();
    ASSERT_EQ(g_unloadLog.size(), 1u);
    ASSERT_EQ(g_unloadLog[0], "Unload: temp.png");
}

TEST_CASE(ClearUnloadsAll) {
    g_unloadLog.clear();
    g_textureCounter = 0;
    ResourceManager rm;
    rm.GetOrLoad("a.png");
    rm.GetOrLoad("b.png");
    ASSERT_EQ(rm.CacheSize(), 2u);
    rm.Clear();
    ASSERT_EQ(rm.CacheSize(), 0u);
    ASSERT_EQ(g_unloadLog.size(), 2u);
}

TEST_CASE(GetExistingReturnsEmptyIfNotLoaded) {
    g_textureCounter = 0;
    ResourceManager rm;
    auto h = rm.GetExisting("nonexistent.png");
    ASSERT_FALSE(h);
}

int main() {
    std::cout << "Solución 08 (Resource Manager)...\n";
    return RUN_ALL_TESTS();
}