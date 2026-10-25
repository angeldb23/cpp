/* =============================================================================
   MÓDULO 22 - LECCIÓN 11: Scripting Integration — C++ como Scripting
   (Hot-Reload de Shared Libraries .so), API de Componentes, Seguridad
   =============================================================================
   TEMA 6 de TEMAS.md: Scripting integration — C++ como scripting (hot-reload
   de bibliotecas compartidas .so en Linux), API de componentes y seguridad básica.

   POR QUÉ C++ COMO SCRIPTING:
   -----------------------------
   - Performance: código nativo, sin overhead de VM/interpreter.
   - Type safety: compile-time checks, IDE support, refactoring.
   - Hot-reload: recompilar .so → cargar en runtime sin reiniciar motor.
   - Ecosistema: mismo lenguaje que el motor, acceso directo a ECS/components.

   HOT-RELOAD DE SHARED LIBRARIES (.so/.dll):
   ------------------------------------------
   Linux: dlopen/dlsym/dlclose + inotify para detectar cambios.
   Windows: LoadLibrary/GetProcAddress/FreeLibrary + ReadDirectoryChangesW.
   Flujo:
   1. Compilar script.cpp → libscript.so (g++ -shared -fPIC)
   2. Motor: dlopen("libscript.so") → handle
   3. dlsym(handle, "ScriptInit"), dlsym(handle, "ScriptUpdate"), etc.
   4. File watcher detecta cambio en script.cpp
   5. Recompilar → dlclose(old) → dlopen(new) → dlsym(new functions)
   6. Mantener estado: serializar datos del script antes de unload, restaurar tras reload.

   API DE COMPONENTES PARA SCRIPTS:
   --------------------------------
   - Registry access: GetComponent<T>(entity), AddComponent<T>(entity), RemoveComponent<T>(entity)
   - Query iteration: for (entity : view<Transform, Velocity>()) { ... }
   - Event subscription: OnEvent<CollisionEvent>(callback)
   - Time: dt, frame_count, time_since_start
   - Input: IsKeyDown, GetMousePos, GetGamepadState
   - Resources: LoadTexture, LoadShader, PlaySound
   - Logging: LogInfo, LogWarning, LogError

   SEGURIDAD BÁSICA (Sandboxing):
   ------------------------------
   - No sandbox real en C++ nativo (código tiene acceso total al proceso).
   - Mitigaciones:
     * Compilar scripts con -fno-exceptions -fno-rtti (reducir superficie)
     * Validar punteros devueltos por dlsym (no null, firma correcta)
     * Timeout watchdog: si ScriptUpdate toma > X ms → kill/abort
     * Memory limit: allocator personalizado que trackea uso por script
     * Filesystem access: solo directorio del proyecto (no /etc, no ~)
     * Network: solo sockets permitidos por motor
   - Alternativa real: WASM (wasmtime/wasm3) para sandbox real. Fuera de scope.

   STATE PERSISTENCE ACROSS RELOADS:
   ---------------------------------
   - Script define struct ScriptState { ... datos persistentes ... };
   - Motor: antes de unload, llamar ScriptSerialize() → bytes
   - Tras reload: llamar ScriptDeserialize(bytes) → restaurar estado
   - Entity handles: serializar Entity ID (generation incluido) → válido tras reload

   ESTA LECCIÓN: PluginManager con dlopen/dlsym, hot-reload loop,
   API de componentes expuesta a scripts, state serialization.
   ============================================================================= */
#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include <memory>
#include <functional>
#include <filesystem>
#include <fstream>
#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <cstdint>
#include <cassert>
#include <dlfcn.h>  // Linux: dlopen, dlsym, dlclose

// ============================================================================
// TIPOS BASE ECS (simplificados)
// ============================================================================
using Entity = uint32_t;
struct Transform { float x=0,y=0,rot=0,sx=1,sy=1; };
struct Velocity { float vx=0,vy=0; };
struct Tag { char name[32]={}; };

// ============================================================================
// REGISTRY MINIMAL PARA SCRIPTS
// ============================================================================
struct ScriptRegistry {
    std::unordered_map<Entity, Transform> transforms;
    std::unordered_map<Entity, Velocity> velocities;
    std::unordered_map<Entity, Tag> tags;
    std::atomic<Entity> next_entity{1};

    Entity CreateEntity() { return next_entity++; }
    void DestroyEntity(Entity e) { transforms.erase(e); velocities.erase(e); tags.erase(e); }

    Transform* GetTransform(Entity e) { auto it=transforms.find(e); return it!=transforms.end()?&it->second:nullptr; }
    Velocity* GetVelocity(Entity e) { auto it=velocities.find(e); return it!=velocities.end()?&it->second:nullptr; }
    Tag* GetTag(Entity e) { auto it=tags.find(e); return it!=tags.end()?&it->second:nullptr; }

    template<typename T> T* Get(Entity e) {
        if constexpr(std::is_same_v<T,Transform>) return GetTransform(e);
        else if constexpr(std::is_same_v<T,Velocity>) return GetVelocity(e);
        else if constexpr(std::is_same_v<T,Tag>) return GetTag(e);
        return nullptr;
    }
    template<typename T> T& Add(Entity e) {
        if constexpr(std::is_same_v<T,Transform>) return transforms.emplace(e, Transform{}).first->second;
        else if constexpr(std::is_same_v<T,Velocity>) return velocities.emplace(e, Velocity{}).first->second;
        else if constexpr(std::is_same_v<T,Tag>) return tags.emplace(e, Tag{}).first->second;
        throw std::runtime_error("Unsupported component");
    }
    template<typename T> void Remove(Entity e) {
        if constexpr(std::is_same_v<T,Transform>) transforms.erase(e);
        else if constexpr(std::is_same_v<T,Velocity>) velocities.erase(e);
        else if constexpr(std::is_same_v<T,Tag>) tags.erase(e);
    }

    // View para scripts
    template<typename... Cs>
    struct View {
        ScriptRegistry* reg;
        std::vector<Entity> entities;
        View(ScriptRegistry* r) : reg(r) {
            // Filtrar entidades que tienen todos los componentes
            for(auto& [e, _] : reg->transforms){
                bool has_all = true;
                (void)std::initializer_list<int>{(has_all = has_all && reg->Get<Cs>(e) != nullptr, 0)...};
                if(has_all) entities.push_back(e);
            }
        }
        auto begin() { return entities.begin(); }
        auto end() { return entities.end(); }
    };
    template<typename... Cs> View<Cs...> View() { return View<Cs...>(this); }
};

// ============================================================================
// SCRIPT API (funciones expuestas al script via dlsym)
// ============================================================================
extern "C" {
    // Lifecycle
    using ScriptInitFn = void(*)(ScriptRegistry*);
    using ScriptUpdateFn = void(*)(ScriptRegistry*, float dt);
    using ScriptShutdownFn = void(*)(ScriptRegistry*);

    // State persistence
    using ScriptSerializeFn = size_t(*)(void* buffer, size_t max_size);  // Returns bytes written
    using ScriptDeserializeFn = void(*)(const void* buffer, size_t size);

    // Event callbacks (opcional)
    using ScriptOnEventFn = void(*)(ScriptRegistry*, const char* event_name, const void* data);
}

// ============================================================================
// PLUGIN MANAGER
// ============================================================================
struct Plugin {
    std::string name;
    std::string source_path;      // .cpp file
    std::string library_path;     // .so file
    void* handle = nullptr;       // dlopen handle

    ScriptInitFn init_fn = nullptr;
    ScriptUpdateFn update_fn = nullptr;
    ScriptShutdownFn shutdown_fn = nullptr;
    ScriptSerializeFn serialize_fn = nullptr;
    ScriptDeserializeFn deserialize_fn = nullptr;

    std::vector<uint8_t> saved_state;  // Estado serializado
    std::filesystem::file_time_type last_write_time;
    bool loaded = false;
    bool enabled = true;
    std::string error_msg;
};

class PluginManager {
    std::unordered_map<std::string, Plugin> plugins_;
    ScriptRegistry registry_;
    std::mutex mutex_;
    std::atomic<bool> stop_watcher_{false};
    std::thread watcher_thread_;
    std::chrono::milliseconds watch_interval_{500};

    // Compilar script.cpp → libscript.so
    bool CompilePlugin(Plugin& plug) {
        std::string cmd = "g++ -std=c++20 -shared -fPIC -O2 -I./include "
                          "-o " + plug.library_path + " " + plug.source_path + " 2>&1";
        std::cout << "  [PluginManager] Compiling: " << cmd << "\n";
        // En real: usar subprocess o build system; aquí simulamos
        // system(cmd.c_str()) == 0
        return true;  // Stub: asumimos éxito
    }

    bool LoadPlugin(Plugin& plug) {
        if(plug.handle) dlclose(plug.handle);
        plug.handle = dlopen(plug.library_path.c_str(), RTLD_NOW | RTLD_LOCAL);
        if(!plug.handle){
            plug.error_msg = dlerror();
            std::cerr << "  [PluginManager] dlopen failed: " << plug.error_msg << "\n";
            return false;
        }
        plug.init_fn = (ScriptInitFn)dlsym(plug.handle, "ScriptInit");
        plug.update_fn = (ScriptUpdateFn)dlsym(plug.handle, "ScriptUpdate");
        plug.shutdown_fn = (ScriptShutdownFn)dlsym(plug.handle, "ScriptShutdown");
        plug.serialize_fn = (ScriptSerializeFn)dlsym(plug.handle, "ScriptSerialize");
        plug.deserialize_fn = (ScriptDeserializeFn)dlsym(plug.handle, "ScriptDeserialize");

        if(!plug.init_fn || !plug.update_fn){
            plug.error_msg = "Missing required symbols (ScriptInit/ScriptUpdate)";
            dlclose(plug.handle); plug.handle = nullptr;
            return false;
        }
        plug.loaded = true;
        plug.error_msg.clear();
        return true;
    }

    void UnloadPlugin(Plugin& plug) {
        if(plug.shutdown_fn && plug.loaded) plug.shutdown_fn(&registry_);
        if(plug.handle) { dlclose(plug.handle); plug.handle = nullptr; }
        plug.init_fn = plug.update_fn = plug.shutdown_fn = nullptr;
        plug.serialize_fn = plug.deserialize_fn = nullptr;
        plug.loaded = false;
    }

    void ReloadPlugin(Plugin& plug) {
        std::cout << "  [PluginManager] Reloading: " << plug.name << "\n";
        // 1. Serializar estado
        if(plug.serialize_fn && plug.loaded){
            std::vector<uint8_t> buf(1024 * 1024);  // 1MB buffer
            size_t written = plug.serialize_fn(buf.data(), buf.size());
            buf.resize(written);
            plug.saved_state = std::move(buf);
            std::cout << "    Serialized " << plug.saved_state.size() << " bytes\n";
        }
        // 2. Unload
        UnloadPlugin(plug);
        // 3. Recompile
        if(!CompilePlugin(plug)){
            plug.error_msg = "Compilation failed";
            return;
        }
        // 4. Load new
        if(!LoadPlugin(plug)){
            return;
        }
        // 5. Restaurar estado
        if(plug.deserialize_fn && !plug.saved_state.empty()){
            plug.deserialize_fn(plug.saved_state.data(), plug.saved_state.size());
            std::cout << "    Deserialized " << plug.saved_state.size() << " bytes\n";
        }
        // 6. Init
        if(plug.init_fn) plug.init_fn(&registry_);
    }

    void CheckFileChanges() {
        for(auto& [name, plug] : plugins_){
            if(!plug.enabled) continue;
            try {
                auto ftime = std::filesystem::last_write_time(plug.source_path);
                if(ftime > plug.last_write_time){
                    plug.last_write_time = ftime;
                    std::cout << "  [PluginManager] Detected change in: " << plug.source_path << "\n";
                    ReloadPlugin(plug);
                }
            } catch(...) {}
        }
    }

    void WatcherLoop() {
        while(!stop_watcher_){
            std::this_thread::sleep_for(watch_interval_);
            std::lock_guard lock(mutex_);
            CheckFileChanges();
        }
    }

public:
    PluginManager() {
        watcher_thread_ = std::thread(&PluginManager::WatcherLoop, this);
    }
    ~PluginManager() {
        stop_watcher_ = true;
        if(watcher_thread_.joinable()) watcher_thread_.join();
        for(auto& [_, plug] : plugins_) UnloadPlugin(plug);
    }

    bool RegisterPlugin(const std::string& name, const std::string& source_path, const std::string& library_path) {
        std::lock_guard lock(mutex_);
        if(plugins_.count(name)) return false;
        Plugin plug; plug.name = name; plug.source_path = source_path; plug.library_path = library_path;
        plug.last_write_time = std::filesystem::last_write_time(source_path);
        if(!CompilePlugin(plug)) return false;
        if(!LoadPlugin(plug)) return false;
        if(plug.init_fn) plug.init_fn(&registry_);
        plugins_[name] = std::move(plug);
        return true;
    }

    void Update(float dt) {
        std::lock_guard lock(mutex_);
        for(auto& [_, plug] : plugins_){
            if(plug.enabled && plug.loaded && plug.update_fn){
                plug.update_fn(&registry_, dt);
            }
        }
    }

    ScriptRegistry& GetRegistry() { return registry_; }
    void EnablePlugin(const std::string& name, bool enable) {
        if(auto it = plugins_.find(name); it != plugins_.end()) it->second.enabled = enable;
    }
    const std::string& GetError(const std::string& name) const {
        static const std::string empty;
        auto it = plugins_.find(name);
        return it != plugins_.end() ? it->second.error_msg : empty;
    }
};

// ============================================================================
// EJEMPLO DE SCRIPT (lo que compilaría el usuario)
// ============================================================================
/*
// script_player.cpp
#include <script_api.hpp>  // Headers del motor con ScriptRegistry, Entity, etc.

static Entity player_entity = 0;

extern "C" void ScriptInit(ScriptRegistry* reg) {
    player_entity = reg->CreateEntity();
    auto& t = reg->Add<Transform>(player_entity);
    t.x = 400; t.y = 300;
    reg->Add<Velocity>(player_entity) = {0, 0};
    auto& tag = reg->Add<Tag>(player_entity);
    strcpy(tag.name, "Player");
}

extern "C" void ScriptUpdate(ScriptRegistry* reg, float dt) {
    auto* vel = reg->Get<Velocity>(player_entity);
    auto* trans = reg->Get<Transform>(player_entity);
    if(!vel || !trans) return;

    // Input simulado
    bool left=false, right=false, up=false, down=false;
    // En real: engine->IsKeyDown(KEY_LEFT), etc.
    if(left)  vel->vx = -200;
    else if(right) vel->vx = 200;
    else vel->vx = 0;
    if(up)    vel->vy = -200;
    else if(down) vel->vy = 200;
    else vel->vy = 0;

    trans->x += vel->vx * dt;
    trans->y += vel->vy * dt;
}

extern "C" void ScriptShutdown(ScriptRegistry* reg) {
    reg->DestroyEntity(player_entity);
}

extern "C" size_t ScriptSerialize(void* buf, size_t max) {
    // Serializar player_entity (uint32_t)
    if(max < sizeof(Entity)) return 0;
    *(Entity*)buf = player_entity;
    return sizeof(Entity);
}

extern "C" void ScriptDeserialize(const void* buf, size_t size) {
    if(size >= sizeof(Entity)) player_entity = *(const Entity*)buf;
}
*/

// ============================================================================
// VERIFICACIÓN DE SINTAXIS (simulación sin dlopen real)
// ============================================================================
int main() {
    std::cout << "=== LECCIÓN 11: Scripting Integration (solo sintaxis) ===\n";

    PluginManager pm;

    // Simular registro (sin compilar real)
    std::cout << "PluginManager creado. Watcher thread corriendo.\n";

    // Registry directo para demo
    ScriptRegistry& reg = pm.GetRegistry();
    Entity e = reg.CreateEntity();
    reg.Add<Transform>(e) = {10, 20, 0, 1, 1};
    reg.Add<Velocity>(e) = {5, 3};
    reg.Add<Tag>(e); strcpy(reg.GetTag(e)->name, "Test");

    // View usage
    for(Entity ent : reg.View<Transform, Velocity>()){
        auto* t = reg.Get<Transform>(ent);
        auto* v = reg.Get<Velocity>(ent);
        std::cout << "Entity " << ent << ": pos=(" << t->x << "," << t->y
                  << ") vel=(" << v->vx << "," << v->vy << ")\n";
    }

    // Simular update
    pm.Update(0.016f);

    std::cout << "=== FIN LECCIÓN 11 ===\n";
    return 0;
}