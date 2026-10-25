/* =============================================================================
   MÓDULO 17 - LECCIÓN 01: Wrappers RAII para recursos de raylib
   =============================================================================
   TEMA 1 de TEMAS.md: Wrappers RAII para recursos de raylib (Texture2D, Sound, Font)

   raylib (versión 5.5) usa C API con manejo manual de memoria:
   - LoadTexture/UnloadTexture
   - LoadSound/UnloadSound
   - LoadFont/UnloadFont
   - LoadRenderTexture/UnloadRenderTexture
   - LoadWave/UnloadWave
   - LoadMusicStream/UnloadMusicStream

   En C++ envolvemos cada recurso en un tipo RAII que llama a Unload* en su destructor.
   Esto evita fugas y hace el código exception-safe.

   DISEÑO DE LOS WRAPPERS:
   - Move-only (no copy): propiedad exclusiva del recurso
   - operator bool() para chequear validez
   - get() para acceso al handle crudo cuando raylib lo requiere
   - reset() para liberación manual anticipada
   - Conversión implícita al tipo crudo (opcional, con cuidado)

   CMake usa FetchContent para raylib 5.5 (tag concreto, no master).
   Ver: https://github.com/raysan5/raylib/releases/tag/5.5
   ============================================================================= */

#include <iostream>
#include <memory>
#include <utility>
#include <cassert>

// -----------------------------------------------------------------------------
// Forward declarations de tipos de raylib (simulados para compilar sin raylib)
// En código real: #include "raylib.h"
// -----------------------------------------------------------------------------
struct Texture2D { int id; int width; int height; int mipmaps; int format; };
struct Sound     { int stream; int sampleRate; int sampleSize; int channels; };
struct Font      { int baseSize; int glyphCount; int glyphPadding; Texture2D texture; void* recs; void* glyphs; };
struct RenderTexture2D { int id; Texture2D texture; };
struct Wave      { int frameCount; int sampleRate; int sampleSize; int channels; };
struct Music     { int ctxType; void* ctxData; int looping; int sampleRate; int sampleSize; int channels; };

// Stubs de raylib para que compile sin la librería (solo sintaxis)
inline Texture2D LoadTexture(const char*) { return Texture2D{1, 64, 64, 1, 0}; }
inline void UnloadTexture(Texture2D) {}
inline Sound LoadSound(const char*) { return Sound{1, 44100, 16, 2}; }
inline void UnloadSound(Sound) {}
inline Font LoadFont(const char*) { return Font{16, 95, 2, {}, nullptr, nullptr}; }
inline void UnloadFont(Font) {}
inline RenderTexture2D LoadRenderTexture(int, int) { return RenderTexture2D{1, {}}; }
inline void UnloadRenderTexture(RenderTexture2D) {}
inline Wave LoadWave(const char*) { return Wave{1024, 44100, 16, 2}; }
inline void UnloadWave(Wave) {}
inline Music LoadMusicStream(const char*) { return Music{0, nullptr, 0, 44100, 16, 2}; }
inline void UnloadMusicStream(Music) {}

// -----------------------------------------------------------------------------
// 1. Wrapper genérico RAII para cualquier recurso con Load/Unload
// -----------------------------------------------------------------------------
template <typename Handle, void (*UnloadFn)(Handle)>
class RaylibResource {
    Handle handle_;
    bool valid_ = false;

public:
    // Constructor por defecto: recurso inválido (null)
    RaylibResource() = default;

    // Constructor desde handle crudo (toma posesión)
    explicit RaylibResource(Handle h) : handle_(h), valid_(true) {}

    // No copiable: propiedad exclusiva
    RaylibResource(const RaylibResource&) = delete;
    RaylibResource& operator=(const RaylibResource&) = delete;

    // Movible: transfiere propiedad
    RaylibResource(RaylibResource&& other) noexcept
        : handle_(other.handle_), valid_(other.valid_) {
        other.handle_ = Handle{};
        other.valid_ = false;
    }

    RaylibResource& operator=(RaylibResource&& other) noexcept {
        if (this != &other) {
            reset();
            handle_ = other.handle_;
            valid_ = other.valid_;
            other.handle_ = Handle{};
            other.valid_ = false;
        }
        return *this;
    }

    // Destructor: libera el recurso automáticamente
    ~RaylibResource() { reset(); }

    // Liberación manual anticipada
    void reset() {
        if (valid_) {
            UnloadFn(handle_);
            handle_ = Handle{};
            valid_ = false;
        }
    }

    // Acceso al handle crudo
    Handle get() const { return handle_; }
    Handle* get_ptr() { return &handle_; }
    const Handle* get_ptr() const { return &handle_; }

    // Conversión implícita a handle (útil para pasar a API de raylib)
    operator Handle() const { return handle_; }

    // Chequeo de validez
    explicit operator bool() const { return valid_; }
    bool is_valid() const { return valid_; }

    // Swap
    void swap(RaylibResource& other) noexcept {
        std::swap(handle_, other.handle_);
        std::swap(valid_, other.valid_);
    }
};

// -----------------------------------------------------------------------------
// 2. Alias concretos para cada tipo de recurso
//
// IMPORTANTE: NO podemos llamar al alias igual que el tipo C de raylib.
// "using Sound = Wrapper<Sound, ...>" es una declaracion conflictiva: el alias
// y la struct no pueden tener el mismo nombre en el mismo scope. Por eso los
// wrappers llevan sufijo RAII (igual que el resto del curso).
// -----------------------------------------------------------------------------
using TextureRAII  = RaylibResource<Texture2D, UnloadTexture>;
using SoundRAII    = RaylibResource<Sound, UnloadSound>;
using FontRAII     = RaylibResource<Font, UnloadFont>;
using RenderTexRAII = RaylibResource<RenderTexture2D, UnloadRenderTexture>;
using WaveRAII     = RaylibResource<Wave, UnloadWave>;
using MusicRAII    = RaylibResource<Music, UnloadMusicStream>;

// -----------------------------------------------------------------------------
// 3. Factory functions para crear recursos (ocultan la API C)
// -----------------------------------------------------------------------------
inline TextureRAII LoadTextureRAII(const char* fileName) {
    return TextureRAII{LoadTexture(fileName)};
}

inline SoundRAII LoadSoundRAII(const char* fileName) {
    return SoundRAII{LoadSound(fileName)};
}

inline FontRAII LoadFontRAII(const char* fileName) {
    return FontRAII{LoadFont(fileName)};
}

inline RenderTexRAII LoadRenderTextureRAII(int width, int height) {
    return RenderTexRAII{LoadRenderTexture(width, height)};
}

inline WaveRAII LoadWaveRAII(const char* fileName) {
    return WaveRAII{LoadWave(fileName)};
}

inline MusicRAII LoadMusicStreamRAII(const char* fileName) {
    return MusicRAII{LoadMusicStream(fileName)};
}

// -----------------------------------------------------------------------------
// 4. Resource Manager con cache y reference counting (avanzado)
// -----------------------------------------------------------------------------
#include <string>
#include <unordered_map>
#include <shared_mutex>
#include <mutex>

template <typename Resource>
class ResourceManager {
    struct Entry {
        Resource resource;
        int ref_count = 0;
    };

    std::unordered_map<std::string, Entry> cache_;
    mutable std::shared_mutex mutex_;

public:
    // Obtiene o carga un recurso, incrementa ref count
    Resource GetOrLoad(const std::string& key, auto&& loader) {
        std::unique_lock lock(mutex_);
        auto it = cache_.find(key);
        if (it != cache_.end()) {
            ++it->second.ref_count;
            return Resource{std::move(it->second.resource)}; // move out para devolver
        }
        Resource res = loader();
        if (res) {
            cache_.emplace(key, Entry{std::move(res), 1});
            return Resource{std::move(cache_[key].resource)};
        }
        return Resource{};
    }

    // Devuelve un recurso existente sin cargar (no incrementa ref count)
    Resource GetExisting(const std::string& key) const {
        std::shared_lock lock(mutex_);
        auto it = cache_.find(key);
        if (it != cache_.end()) {
            return Resource{it->second.resource}; // copy del handle (si es move-only, esto falla)
        }
        return Resource{};
    }

    // Libera recurso si ref count llega a 0
    void Release(const std::string& key) {
        std::unique_lock lock(mutex_);
        auto it = cache_.find(key);
        if (it != cache_.end()) {
            if (--it->second.ref_count == 0) {
                it->second.resource.reset();
                cache_.erase(it);
            }
        }
    }

    void Clear() {
        std::unique_lock lock(mutex_);
        for (auto& [_, entry] : cache_) {
            entry.resource.reset();
        }
        cache_.clear();
    }
};

// -----------------------------------------------------------------------------
// 5. Ejemplo de uso completo
// -----------------------------------------------------------------------------
void demo_basic_wrappers() {
    std::cout << "=== Wrappers RAII básicos ===\n";

    // Carga automática con factory
    TextureRAII tex = LoadTextureRAII("assets/sprite.png");
    SoundRAII snd = LoadSoundRAII("assets/jump.wav");
    FontRAII font = LoadFontRAII("assets/font.png");

    // Chequeo de validez
    if (tex) {
        std::cout << "Texture cargada: " << tex.get().width << "x" << tex.get().height << "\n";
    }
    if (snd) {
        std::cout << "Sound cargado: " << snd.get().sampleRate << " Hz\n";
    }
    if (font) {
        std::cout << "Font cargada: baseSize=" << font.get().baseSize << "\n";
    }

    // Move semantics: transferencia de propiedad
    TextureRAII tex2 = std::move(tex);
    assert(!tex);      // tex ahora inválido
    assert(tex2);      // tex2 tiene el recurso

    // Reset manual
    tex2.reset();
    assert(!tex2);

    // Scope guard: se libera al salir del scope
    {
        TextureRAII temp = LoadTextureRAII("assets/temp.png");
        // usa temp...
    } // UnloadTexture llamado automáticamente aquí

    std::cout << "Wrappers básicos OK\n\n";
}

void demo_resource_manager() {
    std::cout << "=== Resource Manager con cache ===\n";

    ResourceManager<TextureRAII> texManager;

    // Primera carga
    TextureRAII t1 = texManager.GetOrLoad("player", []{ return LoadTextureRAII("assets/player.png"); });
    std::cout << "t1 válido: " << bool(t1) << "\n";

    // Segunda petición: devuelve del cache (en implementación real haría copy del handle)
    // Nota: con move-only types, GetExisting necesitaría shared_ptr o similar
    // Para demo simplificado, mostramos la idea

    texManager.Release("player");
    texManager.Clear();

    std::cout << "Resource Manager OK\n\n";
}

// -----------------------------------------------------------------------------
// 6. Consideraciones avanzadas
// -----------------------------------------------------------------------------
/*
 * REFERENCE COUNTING REAL:
 * Para reference counting real con move-only types, se necesita:
 *   std::shared_ptr<Resource> con custom deleter que llame Unload*
 *   o un handle tipo intrusive_ptr.
 *
 * THREAD SAFETY:
 *   std::shared_mutex permite lectores concurrentes, escritor exclusivo.
 *   En engine real: carga en hilo background, swap atomico al terminar.
 *
 * HOT RELOAD:
 *   - Watcher de archivos (inotify, ReadDirectoryChangesW)
 *   - Al detectar cambio: cargar nuevo recurso, swap atomico en cache
 *   - Los handles existentes siguen válidos hasta que se recarguen
 *
 * ASSET PIPELINE:
 *   - Formato intermedio (ej. KTX2 para texturas, OGG para audio)
 *   - Procesamiento offline: compresión, mipmaps, atlas packing
 *   - Manifest JSON con hashes para validación de integridad
 */

int main() {
    std::cout << "=== LECCIÓN 01: Wrappers RAII para raylib ===\n\n";

    demo_basic_wrappers();
    demo_resource_manager();

    std::cout << "=== FIN LECCIÓN 01 ===\n";
    return 0;
}