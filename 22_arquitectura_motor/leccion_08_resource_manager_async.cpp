/* =============================================================================
   MÓDULO 22 - LECCIÓN 08: Resource Manager — Carga Asíncrona Avanzada,
   Dependencias, Cache Policies, Memory Budgets
   =============================================================================
   TEMA 4 de TEMAS.md (2/2): Resource Manager — carga asíncrona, reference counting,
   hot-reload y asset registry (continuación).

   CARGA ASÍNCRONA AVANZADA:
   -------------------------
   - Priority queue: assets críticos (frame actual) vs background (streaming).
   - Dependency resolution: shader incluye otros shaders; modelo referencia texturas.
   - Streaming: cargar LODs progresivamente (textura low-res → high-res).
   - Cancellation: cancelar loads obsoletos (cámara se movió, asset ya no visible).

   DEPENDENCY TRACKING:
   --------------------
   Asset A depende de B → B debe cargarse antes que A.
   Grafo dirigido acíclico (DAG) de dependencias.
   Topological sort para orden de carga.
   - Shader → includes (#include "common.glsl")
   - Material → shaders + texturas
   - Model → materiales + meshes + skeleton
   - Scene → entities + prefabs

   CACHE POLICIES:
   ---------------
   - LRU (Least Recently Used): evict menos usado recientemente.
   - LRU por categoría: texturas separadas de shaders.
   - Memory budget: límite total (ej. 512MB VRAM + 1GB RAM).
   - Pinning: assets críticos (UI, player) nunca se evictan.
   - Streaming hints: priority, max distance, LOD bias.

   MEMORY BUDGETS:
   ---------------
   - System RAM budget: para assets CPU-side (texturas sin subir, audio decodificado).
   - VRAM budget: para recursos GPU (texturas subidas, buffers, shaders).
   - Eviction callback: al evictar textura → glDeleteTextures.
   - Resident set: assets actualmente en VRAM vs en disco.

   LOADING SCREENS / PROGRESS:
   ---------------------------
   - Contar assets totales + cargados → progreso 0-100%.
   - Frame budget: max tiempo de carga por frame (ej. 2ms) para no bloquear render.
   - Background thread pool: N workers + job queue con prioridades.

   ESTA LECCIÓN: Dependency graph, topological load order, priority queue,
   LRU cache con memory budget, loading progress tracking.
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
#include <algorithm>
#include <cstdint>
#include <cassert>

// ============================================================================
// TIPOS BASE (reutiliza de lección 07)
// ============================================================================
struct AssetHandle { uint32_t id=0, gen=0; bool IsValid() const { return id!=0; } };
struct AssetHandleHash { size_t operator()(AssetHandle h) const noexcept { return (size_t(h.id)<<32)^h.gen; } };

struct AssetSlot {
    std::shared_ptr<void> data;
    std::atomic<uint32_t> ref_count{0};
    uint32_t gen = 0;
    std::string path, type;
    uint64_t hash = 0;
    bool loading = false;
    std::vector<std::function<void(AssetHandle)>> callbacks;
    // Para cache LRU
    std::atomic<uint64_t> last_access{0};
    bool pinned = false;
    size_t size_bytes = 0;  // Tamaño estimado en memoria
};

// ============================================================================
// DEPENDENCY GRAPH
// ============================================================================
struct AssetNode {
    std::string path;
    std::string type;
    std::vector<std::string> dependencies;  // Paths de dependencias
    std::vector<std::string> dependents;    // Inverso: quién depende de este
    uint32_t indegree = 0;                  // Para topological sort
    AssetHandle handle;
    enum State { Unknown, Registered, Loading, Loaded, Failed } state = Unknown;
    int priority = 0;  // Mayor = más urgente
};

class DependencyGraph {
    std::unordered_map<std::string, AssetNode> nodes_;
    std::mutex m_;

public:
    void AddAsset(const std::string& path, const std::string& type, const std::vector<std::string>& deps) {
        std::lock_guard lock(m_);
        auto& node = nodes_[path];
        node.path = path; node.type = type; node.dependencies = deps;
        for(const auto& dep : deps){
            nodes_[dep].dependents.push_back(path);
        }
    }

    // Kahn's algorithm para topological order
    std::vector<std::string> GetLoadOrder() {
        std::lock_guard lock(m_);
        std::unordered_map<std::string, uint32_t> indeg;
        for(auto& [p, n] : nodes_) {
            indeg[p] = n.dependencies.size();
        }
        std::queue<std::string> q;
        for(auto& [p, d] : indeg) if(d == 0) q.push(p);

        std::vector<std::string> order;
        while(!q.empty()){
            std::string cur = q.front(); q.pop();
            order.push_back(cur);
            auto it = nodes_.find(cur);
            if(it != nodes_.end()){
                for(const auto& dep : it->second.dependents){
                    if(--indeg[dep] == 0) q.push(dep);
                }
            }
        }
        if(order.size() != nodes_.size()){
            std::cerr << "Ciclo en dependencias!\n";
        }
        return order;
    }

    AssetNode* Get(const std::string& path) {
        std::lock_guard lock(m_);
        auto it = nodes_.find(path);
        return it != nodes_.end() ? &it->second : nullptr;
    }
    void SetState(const std::string& path, AssetNode::State s) {
        std::lock_guard lock(m_);
        if(auto it = nodes_.find(path); it != nodes_.end()) it->second.state = s;
    }
    void SetHandle(const std::string& path, AssetHandle h) {
        std::lock_guard lock(m_);
        if(auto it = nodes_.find(path); it != nodes_.end()) it->second.handle = h;
    }
    void SetPriority(const std::string& path, int p) {
        std::lock_guard lock(m_);
        if(auto it = nodes_.find(path); it != nodes_.end()) it->second.priority = p;
    }
};

// ============================================================================
// PRIORITY JOB QUEUE
// ============================================================================
struct Job {
    int priority = 0;
    std::function<void()> work;
    bool operator<(const Job& other) const { return priority < other.priority; }  // max-heap
};

class PriorityJobQueue {
    std::priority_queue<Job> queue_;
    std::mutex m_;
    std::condition_variable cv_;
    std::vector<std::thread> workers_;
    std::atomic<bool> stop_{false};
    std::atomic<int> active_jobs_{0};

public:
    PriorityJobQueue(int num_threads = std::thread::hardware_concurrency()){
        for(int i=0;i<num_threads;++i) workers_.emplace_back([this]{ Worker(); });
    }
    ~PriorityJobQueue(){ stop_=true; cv_.notify_all(); for(auto& t:workers_) if(t.joinable()) t.join(); }

    void Push(int priority, std::function<void()> work){
        std::lock_guard lock(m_);
        queue_.push({priority, std::move(work)});
        cv_.notify_one();
    }

    // Ejecutar jobs hasta time budget (ms) en hilo principal (para loading screen)
    int RunMainThreadBudget(std::chrono::milliseconds budget){
        auto deadline = std::chrono::steady_clock::now() + budget;
        int executed = 0;
        while(std::chrono::steady_clock::now() < deadline){
            Job job;
            { std::lock_guard lock(m_); if(queue_.empty()) break; job = std::move(queue_.top()); queue_.pop(); }
            job.work();
            if(++executed > 1000) break;  // Safety
        }
        return executed;
    }

    int ActiveJobs() const { return active_jobs_.load(); }
    size_t PendingJobs() const { std::lock_guard lock(m_); return queue_.size(); }

private:
    void Worker(){
        while(!stop_){
            Job job;
            { std::unique_lock lock(m_); cv_.wait(lock, [this]{ return !queue_.empty() || stop_; }); if(stop_) break; job = std::move(queue_.top()); queue_.pop(); }
            active_jobs_++;
            job.work();
            active_jobs_--;
        }
    }
};

// ============================================================================
// LRU CACHE CON MEMORY BUDGET
// ============================================================================
class LRUCache {
    struct Entry {
        AssetHandle handle;
        size_t size;
        uint64_t last_access;
        bool pinned;
    };
    std::mutex m_;
    std::unordered_map<AssetHandle, Entry, AssetHandleHash> entries_;
    std::atomic<size_t> total_size_{0};
    size_t budget_bytes_;
    uint64_t time_counter_{0};

public:
    explicit LRUCache(size_t budget) : budget_bytes_(budget) {}

    void Touch(AssetHandle h, size_t size, bool pinned){
        std::lock_guard lock(m_);
        auto& e = entries_[h];
        e.handle = h; e.size = size; e.pinned = pinned;
        e.last_access = ++time_counter_;
        total_size_ += size;  // Simplificación: no resta size anterior
        EvictIfNeeded();
    }

    void Unpin(AssetHandle h){
        std::lock_guard lock(m_);
        if(auto it = entries_.find(h); it != entries_.end()) it->second.pinned = false;
    }

    void Remove(AssetHandle h){
        std::lock_guard lock(m_);
        if(auto it = entries_.find(h); it != entries_.end()){
            total_size_ -= it->second.size;
            entries_.erase(it);
        }
    }

    size_t TotalSize() const { return total_size_.load(); }
    size_t Budget() const { return budget_bytes_; }
    void SetBudget(size_t b){ budget_bytes_ = b; EvictIfNeeded(); }

private:
    void EvictIfNeeded(){
        while(total_size_ > budget_bytes_){
            // Encontrar LRU no pinned
            AssetHandle victim = 0;
            uint64_t oldest = UINT64_MAX;
            for(auto& [h, e] : entries_){
                if(!e.pinned && e.last_access < oldest){
                    oldest = e.last_access; victim = h;
                }
            }
            if(!victim.IsValid()) break;  // Todo pinned o vacío
            // En real: callback para liberar GPU resource
            std::cout << "  [Cache] Evicting asset " << victim.id << " (size: " << entries_[victim].size << ")\n";
            total_size_ -= entries_[victim].size;
            entries_.erase(victim);
        }
    }
};

// ============================================================================
// RESOURCE MANAGER EXTENDIDO
// ============================================================================
class ResourceManager {
    std::unordered_map<std::string, std::function<std::shared_ptr<void>(const std::string&)>> loaders_;
    std::unordered_map<std::string, AssetHandle> path_to_handle_;
    std::vector<AssetSlot> slots_{1};
    std::shared_mutex rw_mutex_;

    DependencyGraph dep_graph_;
    PriorityJobQueue job_queue_;
    LRUCache lru_cache_{256 * 1024 * 1024};  // 256MB budget
    std::atomic<uint64_t> frame_counter_{0};

    std::atomic<int> total_to_load_{0}, loaded_count_{0};

public:
    template<typename T>
    using LoaderFunc = std::function<std::shared_ptr<T>(const std::string&)>;

    template<typename T>
    void RegisterLoader(LoaderFunc<T> loader){
        loaders_[typeid(T).name()] = [loader](const std::string& p){ return std::static_pointer_cast<void>(loader(p)); };
    }

    void RegisterDependency(const std::string& asset, const std::string& dependency){
        dep_graph_.AddAsset(asset, "", {dependency});
    }

    template<typename T>
    AssetHandle Load(const std::string& path){
        std::shared_lock lock(rw_mutex_);
        if(auto it = path_to_handle_.find(path); it != path_to_handle_.end()){
            AssetHandle h = it->second; lock.unlock(); Acquire(h); return h;
        }
        lock.unlock();
        return LoadSync<T>(path);
    }

    template<typename T>
    AssetHandle LoadAsync(const std::string& path, int priority = 0, std::function<void(AssetHandle)> cb = {}){
        std::shared_lock lock(rw_mutex_);
        if(auto it = path_to_handle_.find(path); it != path_to_handle_.end()){
            AssetHandle h = it->second; lock.unlock(); Acquire(h); if(cb && IsReady(h)) cb(h); return h;
        }
        lock.unlock();

        AssetHandle h = ReserveSlot(path, typeid(T).name());
        {
            std::unique_lock ul(rw_mutex_);
            auto& slot = slots_[h.id]; slot.loading = true; if(cb) slot.callbacks.push_back(cb);
        }
        dep_graph_.SetState(path, AssetNode::Loading);
        dep_graph_.SetPriority(path, priority);
        total_to_load_++;

        job_queue_.Push(priority, [this, path, h, type = typeid(T).name(), priority](){
            auto loader_it = loaders_.find(type);
            if(loader_it == loaders_.end()){ FinishLoad(h, {}, false); return; }
            auto data = loader_it->second(path);
            uint64_t hash = ComputeHash(path);
            FinishLoad(h, data, true, hash);
        });
        return h;
    }

    // Cargar grafo completo en orden topológico
    void LoadGraph(const std::vector<std::string>& paths, const std::string& type = "auto"){
        auto order = dep_graph_.GetLoadOrder();
        for(const auto& p : order){
            if(std::find(paths.begin(), paths.end(), p) != paths.end()){
                // Cargar con prioridad alta para dependencias
                int prio = 1000 - dep_graph_.Get(p)->dependencies.size() * 10;
                LoadAsync<void>(p, prio);
            }
        }
    }

    template<typename T>
    std::shared_ptr<T> Get(AssetHandle h){
        std::shared_lock lock(rw_mutex_);
        if(h.id >= slots_.size() || slots_[h.id].gen != h.gen) return nullptr;
        lru_cache_.Touch(h, slots_[h.id].size_bytes, slots_[h.id].pinned);
        return std::static_pointer_cast<T>(slots_[h.id].data);
    }
    template<typename T> T* GetRaw(AssetHandle h){ auto p=Get<T>(h); return p?p.get():nullptr; }
    bool IsReady(AssetHandle h){ std::shared_lock l(rw_mutex_); return h.id<slots_.size() && slots_[h.id].data && slots_[h.id].gen==h.gen; }

    void Acquire(AssetHandle h){ std::shared_lock l(rw_mutex_); if(h.id<slots_.size()&&slots_[h.id].gen==h.gen) slots_[h.id].ref_count.fetch_add(1); }
    void Release(AssetHandle h){ std::shared_lock l(rw_mutex_); if(h.id<slots_.size()&&slots_[h.id].gen==h.gen) if(slots_[h.id].ref_count.fetch_sub(1)==1){ /* unload or cache */ } }

    void Pin(AssetHandle h, bool pin){
        std::unique_lock l(rw_mutex_);
        if(h.id<slots_.size()&&slots_[h.id].gen==h.gen){ slots_[h.id].pinned=pin; lru_cache_.Touch(h, slots_[h.id].size_bytes, pin); }
    }

    float GetLoadProgress() const {
        int total = total_to_load_.load();
        return total > 0 ? float(loaded_count_.load()) / total : 1.0f;
    }

    void UpdateFrameBudget(std::chrono::milliseconds budget){
        frame_counter_++;
        job_queue_.RunMainThreadBudget(budget);
    }

    // Hot-reload
    template<typename T>
    void Reload(const std::string& path){
        auto loader_it = loaders_.find(typeid(T).name());
        if(loader_it == loaders_.end()) return;
        auto new_data = loader_it->second(path);
        uint64_t hash = ComputeHash(path);

        std::shared_lock lock(rw_mutex_);
        auto it = path_to_handle_.find(path);
        if(it == path_to_handle_.end()){ lock.unlock(); Load<T>(path); return; }
        AssetHandle h = it->second; lock.unlock();

        std::unique_lock ul(rw_mutex_);
        if(h.id<slots_.size() && slots_[h.id].gen==h.gen){
            slots_[h.id].data = new_data; slots_[h.id].hash = hash; slots_[h.id].gen++;
            // Notificar dependientes para reload en cascada
            auto* node = dep_graph_.Get(path);
            if(node) for(const auto& dep : node->dependents) Reload<void>(dep);
        }
    }

private:
    AssetHandle ReserveSlot(const std::string& path, const std::string& type){
        std::unique_lock l(rw_mutex_);
        uint32_t id = slots_.size(); slots_.push_back({}); slots_[id].gen=1; slots_[id].path=path; slots_[id].type=type; slots_[id].ref_count.store(1);
        AssetHandle h{id, 1}; path_to_handle_[path]=h; return h;
    }
    void FinishLoad(AssetHandle h, std::shared_ptr<void> data, bool ok, uint64_t hash=0){
        std::unique_lock l(rw_mutex_);
        if(h.id>=slots_.size()||slots_[h.id].gen!=h.gen) return;
        auto& slot = slots_[h.id]; slot.loading=false;
        if(ok){ slot.data=data; slot.hash=hash; slot.size_bytes = EstimateSize(data); }
        for(auto& cb: slot.callbacks) cb(h); slot.callbacks.clear();
        dep_graph_.SetState(slot.path, ok?AssetNode::Loaded:AssetNode::Failed);
        dep_graph_.SetHandle(slot.path, h);
        loaded_count_++;
        if(ok) lru_cache_.Touch(h, slot.size_bytes, slot.pinned);
    }
    size_t EstimateSize(std::shared_ptr<void>){
        return 1024*1024;  // Stub: 1MB por asset
    }
    uint64_t ComputeHash(const std::string& path){
        std::ifstream f(path, std::ios::binary); if(!f) return 0;
        uint64_t h=14695981039346656037ull; char buf[4096];
        while(f.read(buf,sizeof(buf))||f.gcount()) for(std::streamsize i=0;i<f.gcount();++i){ h^=(uint8_t)buf[i]; h*=1099511628211ull; }
        return h;
    }
};

// ============================================================================
// VERIFICACIÓN DE SINTAXIS
// ============================================================================
struct TextureData { int w=0,h=0; std::vector<uint8_t> pixels; };
struct ShaderData { std::string source; };

std::shared_ptr<TextureData> LoadTex(const std::string& p){
    auto t=std::make_shared<TextureData>(); t->w=64; t->h=64; t->pixels.resize(64*64*4);
    std::cout << "  Load: " << p << "\n"; return t;
}
std::shared_ptr<ShaderData> LoadSh(const std::string& p){
    auto s=std::make_shared<ShaderData>(); s->source="shader"; std::cout << "  Load: " << p << "\n"; return s;
}

int main(){
    std::cout << "=== LECCIÓN 08: Resource Manager Async/Deps/Cache (solo sintaxis) ===\n";

    ResourceManager rm;
    rm.RegisterLoader<TextureData>(LoadTex);
    rm.RegisterLoader<ShaderData>(LoadSh);

    // Registrar dependencias: shader incluye common.glsl
    rm.RegisterDependency("assets/basic.glsl", "assets/common.glsl");
    rm.RegisterDependency("assets/player.png", "");  // Sin deps

    // Load graph
    rm.LoadGraph({"assets/basic.glsl", "assets/player.png"});

    // Simular frame loop con budget
    for(int frame=0; frame<10; ++frame){
        rm.UpdateFrameBudget(std::chrono::milliseconds(2));
        float prog = rm.GetLoadProgress();
        std::cout << "Frame " << frame << ": progress " << (prog*100) << "% | pending jobs: " << rm.job_queue_.PendingJobs() << "\n";
        if(prog >= 1.0f) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    // Pin crítico
    AssetHandle tex = rm.Load<TextureData>("assets/player.png");
    rm.Pin(tex, true);
    std::cout << "Pinned texture, cache size: " << rm.lru_cache_.TotalSize() << "\n";

    std::cout << "=== FIN LECCIÓN 08 ===\n";
    return 0;
}