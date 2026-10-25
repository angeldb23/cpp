/* =============================================================================
   MÓDULO 22 - LECCIÓN 07: Resource Manager — Handle/ID, Carga Asíncrona,
   Reference Counting, Hot-Reload, Asset Registry
   =============================================================================
   TEMA 4 de TEMAS.md (1/2): Resource Manager — handle/ID, carga asíncrona,
   reference counting, hot-reload y asset registry.

   RESOURCE MANAGER EN MOTORES:
   ----------------------------
   Centraliza carga, ciclo de vida y acceso a assets (texturas, shaders, audio, modelos).
   - Handle/ID: identificador opaco (no puntero directo) → permite mover memoria, hot-reload.
   - Reference counting: asset vive mientras haya handles; unload al llegar a 0.
   - Carga asíncrona: job en hilo worker; callback/promise al terminar.
   - Hot-reload: detectar cambio en disco → recargar asset → actualizar handles.
   - Asset registry: mapa path → metadata (tipo, hash, dependencias, estado).

   HANDLE DESIGN:
   --------------
   struct AssetHandle { uint32_t id; uint32_t generation; };
   - id: índice en array de assets del manager.
   - generation: detecta uso tras unload/reload (igual que Entity).
   - Manager: vector<AssetSlot> slots; slot = { ref_count, generation, data_ptr, ... }.

   REFERENCE COUNTING:
   -------------------
   - AssetHandle es copyable (shared ownership).
   - Copiar handle → incrementa ref_count.
   - Destruir handle → decrementa ref_count; si 0 → unload (o keep cached).
   - Weak handle: no incrementa ref_count; para caches/observadores.

   CARGA ASÍNCRONA:
   ----------------
   AssetHandle LoadAsync(path, callback) {
     handle = ReserveSlot();  // Inmediato, handle válido pero data=null
     JobQueue::Push([path, handle]{
         data = LoadFromDisk(path);
         MainThread::Post([handle, data]{ FinishLoad(handle, data); });
     });
     return handle;
   }
   - Handle válido inmediatamente para pasar a sistemas.
   - Polling: handle.IsReady() o callback cuando data lista.

   HOT-RELOAD:
   -----------
   FileWatcher (inotify/ReadDirectoryChangesW) → event (path changed).
   Manager.Reload(path):
     1. Load new data from disk.
     2. Swap pointer in slot (atomic).
     3. Increment generation → invalida handles viejos (opcional: auto-update).
     4. Notify systems (callback/observer) para recrear GPU resources.

   ASSET REGISTRY:
   ---------------
   struct AssetInfo { std::string path; AssetType type; uint64_t hash; std::vector<std::string> deps; };
   std::unordered_map<std::string, AssetInfo> registry;
   - Hash (xxHash/CRC32) para detectar cambios sin stat.
   - Dependencias: shader → includes; modelo → texturas/materiales.

   ESTA LECCIÓN: ResourceManager con handles genéricos, ref-count, async load,
   file watcher stub, hot-reload, asset registry con hash.
   ============================================================================= */
#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include <memory>
#include <functional>
#include <mutex>
#include <thread>
#include <queue>
#include <atomic>
#include <condition_variable>
#include <shared_mutex>
#include <filesystem>
#include <fstream>
#include <cstdint>
#include <cassert>

// ============================================================================
// ASSET HANDLE (genérico, type-erased)
// ============================================================================
struct AssetHandle {
    uint32_t id = 0;
    uint32_t generation = 0;

    bool IsValid() const { return id != 0; }
    bool operator==(const AssetHandle& other) const { return id == other.id && generation == other.generation; }
    bool operator!=(const AssetHandle& other) const { return !(*this == other); }
};

// Hash para unordered_map
struct AssetHandleHash {
    size_t operator()(const AssetHandle& h) const noexcept {
        return (size_t(h.id) << 32) ^ h.generation;
    }
};

// ============================================================================
// ASSET SLOT (slot en el manager, type-erased)
// ============================================================================
struct AssetSlot {
    std::shared_ptr<void> data;      // Datos del asset (type-erased)
    std::atomic<uint32_t> ref_count{0};
    uint32_t generation = 0;
    std::string path;                // Path original
    std::string type_name;           // Tipo para debug/cast
    uint64_t file_hash = 0;          // Hash del archivo en disco
    bool loading = false;            // Carga en progreso
    std::vector<std::function<void(AssetHandle)>> callbacks;  // Para async load
};

// ============================================================================
// RESOURCE MANAGER
// ============================================================================
class ResourceManager {
public:
    // Registrar loader para un tipo
    template<typename T>
    using LoaderFunc = std::function<std::shared_ptr<T>(const std::string& path)>;

    template<typename T>
    void RegisterLoader(LoaderFunc<T> loader) {
        loaders_[typeid(T).name()] = [loader](const std::string& path) -> std::shared_ptr<void> {
            return std::static_pointer_cast<void>(loader(path));
        };
    }

    // Síncrono: bloquea hasta cargar
    template<typename T>
    AssetHandle Load(const std::string& path) {
        std::shared_lock lock(rw_mutex_);
        auto it = path_to_handle_.find(path);
        if(it != path_to_handle_.end()){
            AssetHandle h = it->second;
            lock.unlock();
            Acquire(h);
            return h;
        }
        lock.unlock();

        // No cargado: cargar ahora
        return LoadSync<T>(path);
    }

    // Asíncrono: retorna handle inmediatamente, carga en background
    template<typename T>
    AssetHandle LoadAsync(const std::string& path, std::function<void(AssetHandle)> on_ready = {}) {
        std::shared_lock lock(rw_mutex_);
        auto it = path_to_handle_.find(path);
        if(it != path_to_handle_.end()){
            AssetHandle h = it->second;
            lock.unlock();
            Acquire(h);
            if(on_ready && IsReady<T>(h)) on_ready(h);
            else if(on_ready) AddCallback(h, on_ready);
            return h;
        }
        lock.unlock();

        // Reservar slot
        AssetHandle h = ReserveSlot(path, typeid(T).name());
        {
            std::unique_lock ul(rw_mutex_);
            auto& slot = slots_[h.id];
            slot.loading = true;
            if(on_ready) slot.callbacks.push_back(on_ready);
        }

        // Enqueue job
        job_queue_.Push([this, path, h, type_name = typeid(T).name()](){
            auto loader_it = loaders_.find(type_name);
            if(loader_it == loaders_.end()){
                std::cerr << "No loader for type: " << type_name << "\n";
                FinishLoad(h, std::shared_ptr<void>{}, false);
                return;
            }
            std::shared_ptr<void> data = loader_it->second(path);
            uint64_t hash = ComputeFileHash(path);
            FinishLoad(h, data, true, hash);
        });

        return h;
    }

    // Acceso a datos (type-safe)
    template<typename T>
    std::shared_ptr<T> Get(AssetHandle h) {
        std::shared_lock lock(rw_mutex_);
        if(h.id >= slots_.size()) return nullptr;
        auto& slot = slots_[h.id];
        if(slot.generation != h.generation) return nullptr;
        return std::static_pointer_cast<T>(slot.data);
    }

    template<typename T>
    T* GetRaw(AssetHandle h) {
        auto ptr = Get<T>(h);
        return ptr ? ptr.get() : nullptr;
    }

    bool IsReady(AssetHandle h) {
        std::shared_lock lock(rw_mutex_);
        if(h.id >= slots_.size()) return false;
        return slots_[h.id].data != nullptr && slots_[h.id].generation == h.generation;
    }

    template<typename T>
    bool IsReady(AssetHandle h) { return IsReady(h); }

    // Reference counting manual (para handles no-RAII)
    void Acquire(AssetHandle h) {
        std::shared_lock lock(rw_mutex_);
        if(h.id < slots_.size() && slots_[h.id].generation == h.generation) {
            slots_[h.id].ref_count.fetch_add(1, std::memory_order_relaxed);
        }
    }
    void Release(AssetHandle h) {
        std::shared_lock lock(rw_mutex_);
        if(h.id < slots_.size() && slots_[h.id].generation == h.generation) {
            if(slots_[h.id].ref_count.fetch_sub(1, std::memory_order_acq_rel) == 1){
                // Ref count llegó a 0 → unload (o keep cached)
                // En real: mover a cache LRU o unload inmediato
                // slots_[h.id].data.reset();
            }
        }
    }

    // Hot-reload: recargar asset desde disco
    template<typename T>
    void Reload(const std::string& path) {
        std::shared_lock lock(rw_mutex_);
        auto it = path_to_handle_.find(path);
        if(it == path_to_handle_.end()){
            lock.unlock();
            Load<T>(path);  // Primera carga
            return;
        }
        AssetHandle h = it->second;
        lock.unlock();

        // Cargar nuevo dato
        auto loader_it = loaders_.find(typeid(T).name());
        if(loader_it == loaders_.end()) return;
        std::shared_ptr<void> new_data = loader_it->second(path);
        uint64_t new_hash = ComputeFileHash(path);

        // Atomic swap
        std::unique_lock ul(rw_mutex_);
        if(h.id < slots_.size() && slots_[h.id].generation == h.generation){
            slots_[h.id].data = new_data;
            slots_[h.id].file_hash = new_hash;
            slots_[h.id].generation++;  // Invalida handles viejos
            // Nota: handles existentes ahora tienen generation vieja → Get() falla
            // Opción: notificar a sistemas para que pidan nuevo handle
        }
    }

    // Asset Registry: info de assets conocidos
    struct AssetInfo {
        std::string path;
        std::string type;
        uint64_t hash = 0;
        std::vector<std::string> dependencies;
        bool loaded = false;
    };
    std::unordered_map<std::string, AssetInfo> registry_;

    void ScanDirectory(const std::string& root_path) {
        for(auto& entry : std::filesystem::recursive_directory_iterator(root_path)){
            if(!entry.is_regular_file()) continue;
            std::string path = entry.path().string();
            AssetInfo info; info.path = path; info.hash = ComputeFileHash(path);
            // Inferir tipo por extensión
            auto ext = entry.path().extension().string();
            if(ext==".png"||ext==".jpg"||ext==".dds") info.type="Texture";
            else if(ext==".vert"||ext==".frag"||ext==".glsl") info.type="Shader";
            else if(ext==".obj"||ext==".gltf") info.type="Model";
            else if(ext==".ogg"||ext==".wav") info.type="Audio";
            else info.type="Unknown";
            registry_[path] = info;
        }
    }

    const AssetInfo* GetAssetInfo(const std::string& path) {
        auto it = registry_.find(path);
        return it != registry_.end() ? &it->second : nullptr;
    }

private:
    std::unordered_map<std::string, std::function<std::shared_ptr<void>(const std::string&)>> loaders_;
    std::unordered_map<std::string, AssetHandle> path_to_handle_;
    std::vector<AssetSlot> slots_{1};  // Index 0 = null
    std::shared_mutex rw_mutex_;

    // Job queue simple (single worker thread para demo)
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
        slots_[id].generation = 1;
        slots_[id].path = path;
        slots_[id].type_name = type_name;
        slots_[id].ref_count.store(1, std::memory_order_relaxed);
        AssetHandle h{id, 1};
        path_to_handle_[path] = h;
        return h;
    }

    void AddCallback(AssetHandle h, std::function<void(AssetHandle)> cb) {
        std::unique_lock lock(rw_mutex_);
        if(h.id < slots_.size() && slots_[h.id].generation == h.generation){
            slots_[h.id].callbacks.push_back(cb);
        }
    }

    void FinishLoad(AssetHandle h, std::shared_ptr<void> data, bool success, uint64_t hash = 0) {
        std::unique_lock lock(rw_mutex_);
        if(h.id >= slots_.size() || slots_[h.id].generation != h.generation) return;
        auto& slot = slots_[h.id];
        slot.loading = false;
        if(success){
            slot.data = data;
            slot.file_hash = hash;
        }
        // Ejecutar callbacks
        for(auto& cb : slot.callbacks) cb(h);
        slot.callbacks.clear();
    }

    // xxHash simplificado (en real: xxHash library)
    uint64_t ComputeFileHash(const std::string& path) {
        std::ifstream f(path, std::ios::binary);
        if(!f) return 0;
        uint64_t hash = 14695981039346656037ull;  // FNV offset basis
        char buf[4096];
        while(f.read(buf, sizeof(buf)) || f.gcount()){
            for(std::streamsize i=0; i<f.gcount(); ++i){
                hash ^= (uint8_t)buf[i];
                hash *= 1099511628211ull;  // FNV prime
            }
        }
        return hash;
    }
};

// ============================================================================
// RAII HANDLE (auto acquire/release)
// ============================================================================
template<typename T>
class AssetRef {
    ResourceManager* mgr_ = nullptr;
    AssetHandle handle_;

public:
    AssetRef() = default;
    AssetRef(ResourceManager* mgr, AssetHandle h) : mgr_(mgr), handle_(h) { if(mgr_) mgr_->Acquire(handle_); }
    AssetRef(const AssetRef& other) : mgr_(other.mgr_), handle_(other.handle_) { if(mgr_) mgr_->Acquire(handle_); }
    AssetRef(AssetRef&& other) noexcept : mgr_(other.mgr_), handle_(other.handle_) { other.mgr_ = nullptr; other.handle_ = {}; }
    ~AssetRef() { if(mgr_) mgr_->Release(handle_); }

    AssetRef& operator=(const AssetRef& other) {
        if(this != &other){
            if(mgr_) mgr_->Release(handle_);
            mgr_ = other.mgr_; handle_ = other.handle_;
            if(mgr_) mgr_->Acquire(handle_);
        }
        return *this;
    }
    AssetRef& operator=(AssetRef&& other) noexcept {
        if(this != &other){
            if(mgr_) mgr_->Release(handle_);
            mgr_ = other.mgr_; handle_ = other.handle_;
            other.mgr_ = nullptr; other.handle_ = {};
        }
        return *this;
    }

    T* operator->() { return mgr_ ? mgr_->GetRaw<T>(handle_) : nullptr; }
    T& operator*() { return *operator->(); }
    T* Get() { return operator->(); }
    bool IsValid() const { return handle_.IsValid() && mgr_ && mgr_->IsReady(handle_); }
    AssetHandle Handle() const { return handle_; }
};

// ============================================================================
// EJEMPLO DE USO
// ============================================================================
struct TextureData { int w=0, h=0; std::vector<uint8_t> pixels; };
struct ShaderData { std::string source; };

std::shared_ptr<TextureData> LoadTexture(const std::string& path) {
    // Stub: en real usa stb_image
    auto tex = std::make_shared<TextureData>();
    tex->w = 64; tex->h = 64; tex->pixels.resize(64*64*4, 255);
    std::cout << "  [Loader] Loaded texture: " << path << "\n";
    return tex;
}
std::shared_ptr<ShaderData> LoadShader(const std::string& path) {
    auto sh = std::make_shared<ShaderData>();
    sh->source = "// Shader source from " + path;
    std::cout << "  [Loader] Loaded shader: " << path << "\n";
    return sh;
}

int main() {
    std::cout << "=== LECCIÓN 07: Resource Manager (solo sintaxis) ===\n";

    ResourceManager rm;
    rm.RegisterLoader<TextureData>(LoadTexture);
    rm.RegisterLoader<ShaderData>(LoadShader);

    // Scan (crea registry)
    // rm.ScanDirectory("assets");  // Requiere directorio real

    // Load síncrono
    AssetHandle tex_h = rm.Load<TextureData>("assets/player.png");
    auto tex = rm.Get<TextureData>(tex_h);
    if(tex) std::cout << "Texture loaded: " << tex->w << "x" << tex->h << "\n";

    // Load asíncrono
    AssetHandle shader_h = rm.LoadAsync<ShaderData>("assets/basic.glsl", [](AssetHandle h){
        std::cout << "  [Callback] Shader ready: " << h.id << "\n";
    });
    std::cout << "Async shader handle: " << shader_h.id << " (loading: " << (rm.IsReady(shader_h)?"no":"yes") << ")\n";

    // RAII handle
    {
        AssetRef<TextureData> tex_ref(&rm, rm.Load<TextureData>("assets/enemy.png"));
        std::cout << "RAII texture: " << (tex_ref.IsValid()? "valid" : "invalid") << "\n";
    }  // Release automático al salir del scope

    // Hot-reload (simulado)
    // rm.Reload<TextureData>("assets/player.png");

    std::cout << "=== FIN LECCIÓN 07 ===\n";
    return 0;
}