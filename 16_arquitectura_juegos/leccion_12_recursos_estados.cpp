/* =============================================================================
   MÓDULO 16 - LECCIÓN 12: Gestión de Recursos — Carga/Descarga entre Estados
   =============================================================================
   TEMA 8 de TEMAS.md: Carga/descarga de recursos entre estados y pantalla de carga.

   CICLO DE VIDA DE RECURSOS:
   --------------------------
   - Los recursos (texturas, sonidos, fuentes, modelos) consumen VRAM/RAM.
   - Cargar TODO al inicio = uso memoria alto, tiempo de arranque largo.
   - Descargar al cambiar estado = libera memoria, evita leaks.

   ESTRATEGIAS:
   ------------
   1. GLOBAL: Cargar todo al inicio, nunca descargar (juegos pequeños).
   2. POR ESTADO: Cada estado carga sus assets en Enter(), libera en Exit().
   3. RESOURCE MANAGER: Reference counting, cache, carga perezosa (ver Módulo 22).
   4. STREAMING: Cargar/descargar chunks según posición (mundo abierto).

   POR ESTADO (Simple, efectivo para la mayoría):
   ----------------------------------------------
   class PlayingState : public IGameState {
       Texture2D playerTex, enemyTex, tileset;
       Sound jumpSfx, hitSfx;
       Music levelMusic;

       void Enter() override {
           playerTex = LoadTexture("assets/player.png");
           enemyTex = LoadTexture("assets/enemy.png");
           tileset = LoadTexture("assets/tileset.png");
           jumpSfx = LoadSound("assets/jump.wav");
           hitSfx = LoadSound("assets/hit.wav");
           levelMusic = LoadMusicStream("assets/music.ogg");
           PlayMusicStream(levelMusic);
       }

       void Exit() override {
           UnloadTexture(playerTex);
           UnloadTexture(enemyTex);
           UnloadTexture(tileset);
           UnloadSound(jumpSfx);
           UnloadSound(hitSfx);
           UnloadMusicStream(levelMusic);
       }
   };

   PROBLEMA: CARGA BLOQUEANTE (Frame Hitch):
   -----------------------------------------
   - LoadTexture/LoadSound son SÍNCRONOS (bloquean hilo principal).
   - Texturas grandes (4K) = 50-200ms de freeze visible.
   - Solución: LoadingState + hilo worker (ver L08).

   RESOURCE MANAGER BÁSICO (RAII + Ref Count):
   -------------------------------------------
   template <typename T>
   struct ResourceHandle {
       T* ptr = nullptr;
       int* refCount = nullptr;

       ResourceHandle() = default;
       ResourceHandle(T* p, int* rc) : ptr(p), refCount(rc) { if (rc) ++*rc; }
       ~ResourceHandle() { Release(); }

       ResourceHandle(const ResourceHandle& other) : ptr(other.ptr), refCount(other.refCount) {
           if (refCount) ++*refCount;
       }
       ResourceHandle& operator=(const ResourceHandle& other) {
           Release(); ptr = other.ptr; refCount = other.refCount;
           if (refCount) ++*refCount; return *this;
       }
       ResourceHandle(ResourceHandle&& other) noexcept : ptr(other.ptr), refCount(other.refCount) {
           other.ptr = nullptr; other.refCount = nullptr;
       }

       void Release() {
           if (refCount && --*refCount == 0) {
               delete ptr; delete refCount; ptr = nullptr; refCount = nullptr;
           }
       }
       T* Get() const { return ptr; }
       T& operator*() const { return *ptr; }
       T* operator->() const { return ptr; }
   };

   class ResourceManager {
       std::unordered_map<std::string, ResourceHandle<Texture2D>> textures_;
       std::unordered_map<std::string, ResourceHandle<Sound>> sounds_;

   public:
       ResourceHandle<Texture2D> GetTexture(const std::string& path) {
           auto it = textures_.find(path);
           if (it != textures_.end()) return it->second;

           Texture2D tex = LoadTexture(path.c_str());
           int* rc = new int(1);
           ResourceHandle<Texture2D> handle(&tex, rc);  // Nota: tex es local, copiar
           // En real: almacenar Texture2D en heap o usar shared_ptr
           textures_[path] = handle;
           return handle;
       }
   };

   // NOTA: raylib Texture2D es struct con ID (uint32_t), copiable.
   // ResourceHandle<Texture2D> funciona bien con copy semantics.

   PANTALLA DE CARGA (LoadingState):
   ---------------------------------
   - Estado dedicado que muestra progreso mientras carga assets pesados.
   - Usa hilo worker para I/O + decode (stb_image, miniaudio).
   - Sube a GPU (glTexImage2D) en hilo principal (OpenGL context).

   ESTRUCTURA LOADING STATE:
   -------------------------
   class LoadingState : public IGameState {
       std::thread loadThread_;
       std::atomic<int> loaded_{0}, total_{0};
       std::vector<std::function<void()>> mainThreadTasks_;  // Para GPU upload
       std::mutex taskMutex_;

       void Enter() override {
           // Definir qué cargar
           std::vector<std::string> textures = {"player.png", "enemy.png", "tileset.png"};
           total_ = textures.size();

           loadThread_ = std::thread([this, textures] {
               for (auto& path : textures) {
                   // 1. Cargar datos crudos en hilo worker (stb_image)
                   int w, h, c; unsigned char* data = stbi_load(path.c_str(), &w, &h, &c, 4);
                   // 2. Encolar subida a GPU en main thread
                   {
                       std::lock_guard lock(taskMutex_);
                       mainThreadTasks_.push_back([data, w, h, path]() {
                           Image img{data, w, h, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
                           Texture2D tex = LoadTextureFromImage(img);
                           stbi_image_free(data);
                           ResourceManager::Instance().StoreTexture(path, tex);
                       });
                   }
                   loaded_++;
               }
           });
       }

       void Update(float dt) override {
           // Ejecutar tareas de GPU en main thread
           std::vector<std::function<void()>> tasks;
           { std::lock_guard lock(taskMutex_); tasks.swap(mainThreadTasks_); }
           for (auto& task : tasks) task();

           if (loaded_ >= total_ && loadThread_.joinable()) {
               loadThread_.join();
               // Transicionar al siguiente estado
               StateManager::Instance().Change(std::make_unique<PlayingState>());
           }
       }

       void Render(float) override {
           DrawText("Cargando...", 350, 200, 30, WHITE);
           float p = total_ > 0 ? float(loaded_) / total_ : 0;
           DrawRectangle(200, 250, 400 * p, 30, GREEN);
           DrawRectangleLines(200, 250, 400, 30, WHITE);
       }

       void Exit() override {
           if (loadThread_.joinable()) loadThread_.join();
       }
   };

   ASSETS COMUNES EN RAYLIB:
   -------------------------
   - Texture2D: LoadTexture/UnloadTexture, DrawTexture/V/Ex/Pro/Rec
   - Image: LoadImage/UnloadImage (CPU), ExportImage, ImageResize, ImageCrop
   - Sound: LoadSound/UnloadSound, PlaySound, SetSoundVolume
   - Music: LoadMusicStream/UnloadMusicStream, PlayMusicStream, UpdateMusicStream
   - Font: LoadFont/UnloadFont, DrawTextEx, MeasureTextEx
   - Model: LoadModel/UnloadModel, DrawModel (ver Módulo 20)

   ============================================================================= */

#include <iostream>
#include <unordered_map>
#include <string>
#include <memory>
#include <vector>
#include <atomic>
#include <thread>
#include <mutex>
#include <functional>

// Demo simplificada: Resource Manager con RAII handles
struct Texture2D { unsigned int id; int width, height; int mipmaps; int format; };

Texture2D LoadTextureDummy(const char* path) {
    std::cout << "  [LoadTexture] " << path << "\n";
    return {1, 64, 64, 1, 0};
}
void UnloadTextureDummy(Texture2D tex) {
    std::cout << "  [UnloadTexture] id=" << tex.id << "\n";
}

template <typename T, void(*Deleter)(T)>
class ResourceHandle {
    T resource_;
    std::shared_ptr<int> refCount_;

public:
    ResourceHandle() : resource_{}, refCount_(new int(0)) {}
    explicit ResourceHandle(T res) : resource_(res), refCount_(new int(1)) {}

    ResourceHandle(const ResourceHandle& other) : resource_(other.resource_), refCount_(other.refCount_) {
        if (refCount_) ++*refCount_;
    }
    ResourceHandle& operator=(const ResourceHandle& other) {
        if (this != &other) { Release(); resource_ = other.resource_; refCount_ = other.refCount_; if (refCount_) ++*refCount_; }
        return *this;
    }
    ResourceHandle(ResourceHandle&& other) noexcept : resource_(other.resource_), refCount_(std::move(other.refCount_)) {
        other.resource_ = {}; other.refCount_ = nullptr;
    }

    ~ResourceHandle() { Release(); }

    void Release() {
        if (refCount_ && --*refCount_ == 0) { Deleter(resource_); resource_ = {}; }
    }
    T* Get() { return &resource_; }
    const T* Get() const { return &resource_; }
    T& operator*() { return resource_; }
    const T& operator*() const { return resource_; }
};

using TextureHandle = ResourceHandle<Texture2D, UnloadTextureDummy>;

class ResourceManager {
    std::unordered_map<std::string, TextureHandle> cache_;
public:
    TextureHandle GetTexture(const std::string& path) {
        auto it = cache_.find(path);
        if (it != cache_.end()) {
            std::cout << "  [Cache HIT] " << path << "\n";
            return it->second;
        }
        std::cout << "  [Cache MISS] Cargando " << path << "\n";
        TextureHandle handle(LoadTextureDummy(path.c_str()));
        cache_[path] = handle;
        return handle;
    }
    void Clear() { cache_.clear(); }
};

struct IGameState {
    virtual ~IGameState() = default;
    virtual void Enter(ResourceManager& rm) = 0;
    virtual void Exit(ResourceManager& rm) = 0;
    virtual void Update(float) = 0;
    virtual void Render() = 0;
};

struct MenuState : IGameState {
    TextureHandle bgTex;
    void Enter(ResourceManager& rm) override { bgTex = rm.GetTexture("menu_bg.png"); std::cout << "[Menu] Enter\n"; }
    void Exit(ResourceManager& rm) override { bgTex.Release(); std::cout << "[Menu] Exit (bg liberado)\n"; }
    void Update(float) override {}
    void Render() override { std::cout << "[Menu] Render con bg\n"; }
};

struct PlayingState : IGameState {
    TextureHandle playerTex, enemyTex;
    void Enter(ResourceManager& rm) override {
        playerTex = rm.GetTexture("player.png");
        enemyTex = rm.GetTexture("enemy.png");
        std::cout << "[Playing] Enter\n";
    }
    void Exit(ResourceManager& rm) override { playerTex.Release(); enemyTex.Release(); std::cout << "[Playing] Exit\n"; }
    void Update(float) override {}
    void Render() override { std::cout << "[Playing] Render player+enemy\n"; }
};

int main() {
    std::cout << "=== LECCIÓN 12: Gestión de Recursos entre Estados ===\n";

    ResourceManager rm;
    std::vector<std::unique_ptr<IGameState>> stack;

    auto PushState = [&](std::unique_ptr<IGameState> s) {
        if (!stack.empty()) stack.back()->Exit(rm);
        stack.push_back(std::move(s));
        stack.back()->Enter(rm);
    };
    auto PopState = [&]() {
        if (!stack.empty()) { stack.back()->Exit(rm); stack.pop_back(); }
        if (!stack.empty()) stack.back()->Enter(rm);
    };

    std::cout << "\n--- Push Menu ---\n";
    PushState(std::make_unique<MenuState>());
    stack.back()->Render();

    std::cout << "\n--- Push Playing (Menu debajo) ---\n";
    PushState(std::make_unique<PlayingState>());
    stack.back()->Render();

    std::cout << "\n--- Pop Playing (vuelve a Menu) ---\n";
    PopState();
    stack.back()->Render();

    std::cout << "\n--- Pop Menu (salir) ---\n";
    PopState();

    std::cout << "\nCache final: " << (rm.GetTexture("player.png").Get()->id ? "tiene refs" : "limpia") << "\n";
    std::cout << "=== FIN LECCIÓN 12 ===\n";
    return 0;
}