/* =============================================================================
   MÓDULO 17 - LECCIÓN 05: Audio — miniaudio vía raylib, Streaming vs Loading,
   Canales, Volumen/Pitch, Pooling de Sonidos
   =============================================================================
   TEMA 5 de TEMAS.md: Audio

   CONCEPTOS CLAVE:
   - raylib usa miniaudio internamente (C API)
   - Sound: cargado completo en memoria (WAV, OGG, MP3, FLAC) - para SFX cortos
   - Music: streaming desde disco (OGG, MP3, FLAC, WAV, MOD, XM) - para música larga
   - Audio Device: InitAudioDevice() / CloseAudioDevice() (uno por programa)
   - Canales: raylib maneja pool interno (máx 32 por defecto, configurable)
   - Volumen/Pitch: por Sound/Music o global
   - Pooling: reutilizar Sound instances para SFX frecuentes

   FUNCIONES RAYLIB (verificar en docs 5.5):
   - InitAudioDevice() / CloseAudioDevice()
   - LoadSound/UnloadSound, PlaySound, StopSound, PauseSound, ResumeSound
   - LoadMusicStream/UnloadMusicStream, PlayMusicStream, UpdateMusicStream
   - SetSoundVolume, SetSoundPitch, SetMusicVolume, SetMusicPitch
   - SetMasterVolume
   - IsSoundPlaying, IsMusicStreamPlaying
   - GetSoundLength, GetMusicTimePlayed, GetMusicTimeLength
   - SetAudioStreamBufferSizeDefault (para latencia)

   NOTA: Código compila con stubs. Para probar: -lraylib.
   ============================================================================= */

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <queue>
#include <mutex>
#include <functional>
#include <cmath>
#include <algorithm>
#include <memory>

// -----------------------------------------------------------------------------
// Stubs raylib (simplificados para compilación)
// -----------------------------------------------------------------------------
struct Sound { int id; int sampleRate; int sampleSize; int channels; };
struct Music { int ctxType; void* ctxData; int looping; int sampleRate; int sampleSize; int channels; };
struct AudioStream { void* buffer; int sampleRate; int sampleSize; int channels; };
struct Vector2 { float x, y; };

inline void InitAudioDevice() {}
inline void CloseAudioDevice() {}
inline bool IsAudioDeviceReady() { return true; }

inline Sound LoadSound(const char*) { return Sound{1, 44100, 16, 2}; }
inline void UnloadSound(Sound) {}
inline void PlaySound(Sound) {}
inline void StopSound(Sound) {}
inline void PauseSound(Sound) {}
inline void ResumeSound(Sound) {}
inline bool IsSoundPlaying(Sound) { return false; }
inline void SetSoundVolume(Sound, float) {}
inline void SetSoundPitch(Sound, float) {}
inline void SetSoundPan(Sound, float) {}

inline Music LoadMusicStream(const char*) { return Music{0, nullptr, 0, 44100, 16, 2}; }
inline void UnloadMusicStream(Music) {}
inline void UnloadAudioStream(AudioStream) {}
inline void PlayMusicStream(Music) {}
inline void UpdateMusicStream(Music) {}
inline void StopMusicStream(Music) {}
inline void PauseMusicStream(Music) {}
inline void ResumeMusicStream(Music) {}
inline bool IsMusicStreamPlaying(Music) { return false; }
inline void SetMusicVolume(Music, float) {}
inline void SetMusicPitch(Music, float) {}
inline void SetMusicPan(Music, float) {}
inline float GetMusicTimePlayed(Music) { return 0; }
inline float GetMusicTimeLength(Music) { return 0; }

inline void SetMasterVolume(float) {}

// -----------------------------------------------------------------------------
// 1. RAII Wrappers para Audio (extiende lección 01)
// -----------------------------------------------------------------------------
template <typename Handle, void (*UnloadFn)(Handle)>
class AudioResource {
    Handle handle_;
    bool valid_ = false;
public:
    AudioResource() = default;
    explicit AudioResource(Handle h) : handle_(h), valid_(true) {}
    AudioResource(const AudioResource&) = delete;
    AudioResource& operator=(const AudioResource&) = delete;
    AudioResource(AudioResource&& o) noexcept : handle_(o.handle_), valid_(o.valid_) { o.valid_ = false; }
    AudioResource& operator=(AudioResource&& o) noexcept { if(this!=&o){ reset(); handle_=o.handle_; valid_=o.valid_; o.valid_=false; } return *this; }
    ~AudioResource() { reset(); }
    void reset() { if(valid_){ UnloadFn(handle_); valid_=false; } }
    const Handle& get() const { return handle_; }
    Handle& get() { return handle_; }   // versión mutable: get().looping = true;
    operator Handle() const { return handle_; }
    explicit operator bool() const { return valid_; }
};

using SoundRAII = AudioResource<Sound, UnloadSound>;
using MusicRAII = AudioResource<Music, UnloadMusicStream>;

// Factory functions
inline SoundRAII LoadSoundRAII(const char* f) { return SoundRAII{LoadSound(f)}; }
inline MusicRAII LoadMusicRAII(const char* f) { return MusicRAII{LoadMusicStream(f)}; }

// -----------------------------------------------------------------------------
// 2. Sound Pool: reutilizar instancias para SFX frecuentes
// -----------------------------------------------------------------------------
class SoundPool {
    struct PooledSound {
        SoundRAII sound;
        bool inUse = false;
    };

    std::vector<PooledSound> pool_;
    std::string filePath_;
    size_t maxSize_;

public:
    SoundPool(const char* file, size_t maxInstances = 8)
        : filePath_(file), maxSize_(maxInstances) {
        // Pre-cargar algunos
        for (size_t i = 0; i < std::min(maxSize_, size_t(4)); ++i) {
            pool_.push_back({LoadSoundRAII(filePath_.c_str()), false});
        }
    }

    // Obtiene un sound libre o crea uno nuevo (hasta maxSize)
    SoundRAII* Acquire() {
        for (auto& ps : pool_) {
            if (!ps.inUse && ps.sound) {
                ps.inUse = true;
                return &ps.sound;
            }
        }
        if (pool_.size() < maxSize_) {
            pool_.push_back({LoadSoundRAII(filePath_.c_str()), true});
            return &pool_.back().sound;
        }
        // Pool lleno: robar el más viejo (stop y reusar)
        if (!pool_.empty()) {
            auto& oldest = pool_.front();
            if (oldest.sound) StopSound(oldest.sound);
            oldest.inUse = true;
            return &oldest.sound;
        }
        return nullptr;
    }

    void Release(SoundRAII* sound) {
        for (auto& ps : pool_) {
            if (&ps.sound == sound) {
                ps.inUse = false;
                break;
            }
        }
    }

    // Auto-release: wrapper que devuelve al pool al destruirse
    class ScopedSound {
        SoundPool* pool_;
        SoundRAII* sound_;
    public:
        ScopedSound(SoundPool* p, SoundRAII* s) : pool_(p), sound_(s) {}
        ~ScopedSound() { if (pool_ && sound_) pool_->Release(sound_); }
        SoundRAII* operator->() { return sound_; }
        SoundRAII& operator*() { return *sound_; }
        explicit operator bool() const { return sound_ && *sound_; }
    };

    ScopedSound Play(float volume = 1.0f, float pitch = 1.0f, float pan = 0.0f) {
        SoundRAII* s = Acquire();
        if (s && *s) {
            SetSoundVolume(*s, volume);
            SetSoundPitch(*s, pitch);
            SetSoundPan(*s, pan);
            PlaySound(*s);
        }
        return ScopedSound(this, s);
    }

    void StopAll() {
        for (auto& ps : pool_) {
            if (ps.sound) StopSound(ps.sound);
            ps.inUse = false;
        }
    }
};

// -----------------------------------------------------------------------------
// 3. Music Manager: streaming, crossfade, playlist
// -----------------------------------------------------------------------------
class MusicManager {
    MusicRAII currentMusic_;
    MusicRAII nextMusic_;
    bool crossfading_ = false;
    float crossfadeTime_ = 0.0f;
    float crossfadeDuration_ = 1.0f;
    float currentVolume_ = 1.0f;
    float targetVolume_ = 1.0f;

public:
    MusicManager() = default;

    void Play(const char* file, bool loop = true, float volume = 1.0f) {
        if (currentMusic_ && IsMusicStreamPlaying(currentMusic_)) {
            StopMusicStream(currentMusic_);
        }
        currentMusic_ = LoadMusicRAII(file);
        if (currentMusic_) {
            currentMusic_.get().looping = loop;
            SetMusicVolume(currentMusic_, volume);
            PlayMusicStream(currentMusic_);
            currentVolume_ = volume;
        }
    }

    // Crossfade a nueva música (volumen final configurado al terminar el blend)
    void CrossfadeTo(const char* file, float duration = 1.0f, bool loop = true, float volume = 1.0f) {
        if (currentMusic_ && IsMusicStreamPlaying(currentMusic_)) {
            nextMusic_ = LoadMusicRAII(file);
            if (nextMusic_) {
                nextMusic_.get().looping = loop;
                SetMusicVolume(nextMusic_, 0.0f);
                PlayMusicStream(nextMusic_);
                crossfading_ = true;
                crossfadeDuration_ = duration;
                crossfadeTime_ = 0.0f;
                targetVolume_ = volume;
            }
        } else {
            Play(file, loop, volume);
        }
    }

    void Update(float dt) {
        if (currentMusic_) UpdateMusicStream(currentMusic_);
        if (nextMusic_) UpdateMusicStream(nextMusic_);

        if (crossfading_) {
            crossfadeTime_ += dt;
            float t = std::min(1.0f, crossfadeTime_ / crossfadeDuration_);
            // Curva suave (smoothstep)
            t = t * t * (3.0f - 2.0f * t);

            SetMusicVolume(currentMusic_, currentVolume_ * (1.0f - t));
            SetMusicVolume(nextMusic_, targetVolume_ * t);

            if (t >= 1.0f) {
                StopMusicStream(currentMusic_);
                currentMusic_ = std::move(nextMusic_);
                currentVolume_ = targetVolume_;
                crossfading_ = false;
            }
        }
    }

    void SetVolume(float vol) {
        targetVolume_ = std::clamp(vol, 0.0f, 1.0f);
        if (!crossfading_ && currentMusic_) {
            currentVolume_ = targetVolume_;
            SetMusicVolume(currentMusic_, currentVolume_);
        }
    }

    void Pause() {
        if (currentMusic_) PauseMusicStream(currentMusic_);
        if (nextMusic_) PauseMusicStream(nextMusic_);
    }

    void Resume() {
        if (currentMusic_) ResumeMusicStream(currentMusic_);
        if (nextMusic_) ResumeMusicStream(nextMusic_);
    }

    void Stop() {
        if (currentMusic_) { StopMusicStream(currentMusic_); currentMusic_.reset(); }
        if (nextMusic_) { StopMusicStream(nextMusic_); nextMusic_.reset(); }
        crossfading_ = false;
    }

    float GetProgress() const {
        if (currentMusic_ && GetMusicTimeLength(currentMusic_) > 0) {
            return GetMusicTimePlayed(currentMusic_) / GetMusicTimeLength(currentMusic_);
        }
        return 0;
    }

    bool IsPlaying() const { return currentMusic_ && IsMusicStreamPlaying(currentMusic_); }
};

// -----------------------------------------------------------------------------
// 4. Audio System: integra todo + categorías de volumen
// -----------------------------------------------------------------------------
enum class AudioCategory { Master, Music, SFX, UI, Ambience, Voice };

class AudioSystem {
    std::unordered_map<AudioCategory, float> categoryVolumes_;
    std::unordered_map<std::string, std::unique_ptr<SoundPool>> soundPools_;
    MusicManager music_;
    bool initialized_ = false;

public:
    AudioSystem() {
        categoryVolumes_[AudioCategory::Master] = 1.0f;
        categoryVolumes_[AudioCategory::Music] = 0.7f;
        categoryVolumes_[AudioCategory::SFX] = 1.0f;
        categoryVolumes_[AudioCategory::UI] = 0.8f;
        categoryVolumes_[AudioCategory::Ambience] = 0.5f;
        categoryVolumes_[AudioCategory::Voice] = 1.0f;
    }

    bool Initialize() {
        if (initialized_) return true;
        InitAudioDevice();
        initialized_ = IsAudioDeviceReady();
        return initialized_;
    }

    ~AudioSystem() {
        if (initialized_) CloseAudioDevice();
    }

    // Categoría de volumen
    void SetCategoryVolume(AudioCategory cat, float vol) {
        categoryVolumes_[cat] = std::clamp(vol, 0.0f, 1.0f);
        UpdateMasterVolume();
    }

    float GetCategoryVolume(AudioCategory cat) const {
        auto it = categoryVolumes_.find(cat);
        return it != categoryVolumes_.end() ? it->second : 1.0f;
    }

    float GetEffectiveVolume(AudioCategory cat) const {
        return categoryVolumes_.at(AudioCategory::Master) * categoryVolumes_.at(cat);
    }

    // Registrar sound pool
    void RegisterSoundPool(const std::string& name, const char* file, size_t maxInstances = 8) {
        soundPools_[name] = std::make_unique<SoundPool>(file, maxInstances);
    }

    // Jugar SFX desde pool
    typename SoundPool::ScopedSound PlaySound(const std::string& poolName,
                                               AudioCategory cat = AudioCategory::SFX,
                                               float pitch = 1.0f, float pan = 0.0f) {
        auto it = soundPools_.find(poolName);
        if (it != soundPools_.end()) {
            float vol = GetEffectiveVolume(cat);
            return it->second->Play(vol, pitch, pan);
        }
        return typename SoundPool::ScopedSound(nullptr, nullptr);
    }

    // Music
    void PlayMusic(const char* file, bool loop = true, AudioCategory cat = AudioCategory::Music) {
        float vol = GetEffectiveVolume(cat);
        music_.Play(file, loop, vol);
    }

    void CrossfadeMusic(const char* file, float duration = 1.0f, AudioCategory cat = AudioCategory::Music) {
        float vol = GetEffectiveVolume(cat);
        music_.CrossfadeTo(file, duration, true, vol);
    }

    void Update(float dt) {
        if (initialized_) music_.Update(dt);
    }

    void PauseAll() {
        music_.Pause();
        // Sounds se paran solos al terminar
    }

    void ResumeAll() { music_.Resume(); }

    void StopAll() {
        music_.Stop();
        for (auto& [_, pool] : soundPools_) pool->StopAll();
    }

    void UpdateMasterVolume() {
        float master = categoryVolumes_.at(AudioCategory::Master);
        SetMasterVolume(master);
    }
};

// -----------------------------------------------------------------------------
// 5. Audio Streaming personalizado (para casos avanzados)
// -----------------------------------------------------------------------------
/*
 * AUDIO STREAM PERSONALIZADO (raylib AudioStream):
 * - Permite generar audio en runtime (síntesis, DSP, radio, etc.)
 * - Callback: void Callback(void* buffer, unsigned int frames)
 * - LoadAudioStream(sampleRate, sampleSize, channels)
 * - PlayAudioStream / UpdateAudioStream / StopAudioStream
 * - IsAudioStreamProcessed / UpdateAudioStream
 *
 * USOS:
 * - Síntesis procedural (chiptune, FM, granular)
 * - Efectos DSP en tiempo real (filtros, reverb, delay)
 * - Streaming de red (radio, VoIP)
 * - Mezcla dinámica de múltiples fuentes
 */

struct AudioStreamRAII : AudioResource<AudioStream, &UnloadAudioStream> {
    using AudioResource::AudioResource;
};

// -----------------------------------------------------------------------------
// 6. 3D Audio / Spatial (pan + distance attenuation)
// -----------------------------------------------------------------------------
struct AudioListener {
    Vector2 position = {0, 0};
    Vector2 velocity = {0, 0};
    float orientation = 0;  // radianes
};

struct AudioEmitter {
    Vector2 position = {0, 0};
    float minDistance = 100.0f;   // Distancia para volumen máximo
    float maxDistance = 1000.0f;  // Distancia para silencio
    float rolloffFactor = 1.0f;   // Curva de atenuación
};

void Update3DAudio(const AudioListener& listener, AudioEmitter& emitter,
                   SoundRAII& sound, float baseVolume = 1.0f) {
    // Distancia
    float dx = emitter.position.x - listener.position.x;
    float dy = emitter.position.y - listener.position.y;
    float dist = std::sqrt(dx*dx + dy*dy);

    // Atenuación por distancia (inverse clamped)
    float volume = baseVolume;
    if (dist > emitter.minDistance) {
        float factor = emitter.minDistance / (emitter.minDistance + emitter.rolloffFactor * (dist - emitter.minDistance));
        volume *= std::max(0.0f, factor);
    }
    if (dist > emitter.maxDistance) volume = 0.0f;

    // Pan estéreo simple (-1 izquierda, 1 derecha)
    float pan = 0.0f;
    if (dist > 0.001f) {
        // Ángulo relativo al listener
        float angle = std::atan2(dy, dx) - listener.orientation;
        pan = std::sin(angle);  // -1 a 1
    }

    if (sound) {
        SetSoundVolume(sound, volume);
        SetSoundPan(sound, pan);
    }
}

// -----------------------------------------------------------------------------
// 7. Ducking / Sidechain (bajar música cuando hay voz/SFX importante)
// -----------------------------------------------------------------------------
class DuckingManager {
    MusicManager* music_;
    float duckLevel_ = 0.3f;      // Volumen al hacer duck (0.3 = 30%)
    float attackTime_ = 0.05f;    // Qué tan rápido baja
    float releaseTime_ = 0.5f;    // Qué tan rápido sube
    float currentDuck_ = 1.0f;
    bool ducking_ = false;

public:
    DuckingManager(MusicManager* m) : music_(m) {}

    void TriggerDuck() { ducking_ = true; }
    void ReleaseDuck() { ducking_ = false; }

    void Update(float dt) {
        float target = ducking_ ? duckLevel_ : 1.0f;
        float speed = ducking_ ? (1.0f / attackTime_) : (1.0f / releaseTime_);

        if (currentDuck_ > target) {
            currentDuck_ = std::max(target, currentDuck_ - speed * dt);
        } else if (currentDuck_ < target) {
            currentDuck_ = std::min(target, currentDuck_ + speed * dt);
        }

        if (music_) music_->SetVolume(currentDuck_);
    }
};

// -----------------------------------------------------------------------------
// Demo
// -----------------------------------------------------------------------------
void demo_audio() {
    std::cout << "=== Audio Demo ===\n\n";

    // Simula inicialización
    InitAudioDevice();

    // Sound Pool para salto
    SoundPool jumpPool("assets/jump.wav", 4);
    for (int i = 0; i < 6; ++i) {
        auto scoped = jumpPool.Play(1.0f, 1.0f + i * 0.1f);  // pitch variable
        std::cout << "Jump " << i << ": " << (scoped ? "played" : "pool full") << "\n";
    }

    // Music Manager
    MusicManager music;
    music.Play("assets/music_level1.ogg", true, 0.7f);

    // Simula update loop
    for (int i = 0; i < 5; ++i) {
        music.Update(0.016f);
        std::cout << "Music progress: " << music.GetProgress() << "\n";
    }

    // Crossfade
    music.CrossfadeTo("assets/music_boss.ogg", 2.0f);
    for (int i = 0; i < 10; ++i) {
        music.Update(0.016f);
    }

    // Audio System completo
    AudioSystem audio;
    audio.Initialize();
    audio.RegisterSoundPool("jump", "assets/jump.wav");
    audio.RegisterSoundPool("coin", "assets/coin.wav", 16);
    audio.RegisterSoundPool("explosion", "assets/explosion.wav", 4);

    audio.PlaySound("jump");
    audio.PlaySound("coin");
    audio.PlaySound("coin");  // Múltiples coins
    audio.PlayMusic("assets/theme.ogg");

    // 3D Audio
    AudioListener listener{{400, 300}, {0,0}, 0};
    AudioEmitter emitter{{500, 300}, 100, 1000, 1.0f};
    SoundRAII sound3d = LoadSoundRAII("assets/engine.wav");
    Update3DAudio(listener, emitter, sound3d);
    std::cout << "3D Audio: volume=" << "calculated" << " pan=" << "calculated" << "\n";

    // Ducking
    DuckingManager duck(&music);
    duck.TriggerDuck();
    for (int i = 0; i < 5; ++i) duck.Update(0.016f);
    std::cout << "Ducking volume: " << "reduced" << "\n";
    duck.ReleaseDuck();
    for (int i = 0; i < 10; ++i) duck.Update(0.016f);
    std::cout << "Ducking volume: " << "restored" << "\n";

    CloseAudioDevice();
    std::cout << "\n=== FIN LECCIÓN 05 ===\n";
}

int main() {
    demo_audio();
    return 0;
}