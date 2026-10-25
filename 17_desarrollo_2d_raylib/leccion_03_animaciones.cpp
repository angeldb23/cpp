/* =============================================================================
   MÓDULO 17 - LECCIÓN 03: Animaciones — Frame Data, Timer, Looping, Ping-Pong, State Machine
   =============================================================================
   TEMA 3 de TEMAS.md: Animaciones

   CONCEPTOS CLAVE:
   - Frame Data: duración por frame, rectángulo source, offset/pivot opcional
   - Timer: acumulador de tiempo, avanza frame cuando supera duración
   - Looping: repite desde inicio al terminar
   - Ping-Pong: ida y vuelta (0->1->2->1->0...)
   - Animation State Machine: estados (Idle, Walk, Attack, Hurt) con transiciones

   DISEÑO:
   - AnimationClip: datos inmutables (frames, duración total, loop mode)
   - AnimationPlayer: estado mutable (clip actual, frame actual, timer, speed)
   - AnimationStateMachine: transiciones entre clips con condiciones

   NOTA: Código compila sin raylib (stubs). Para probar: linkear -lraylib.
   ============================================================================= */

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>
#include <memory>

// -----------------------------------------------------------------------------
// Stubs raylib
// -----------------------------------------------------------------------------
struct Vector2 { float x, y; };
struct Rectangle { float x, y, width, height; };
struct Color { unsigned char r, g, b, a; };
struct Texture2D { int id; int width; int height; int mipmaps; int format; };

inline Texture2D LoadTexture(const char*) { return Texture2D{1, 512, 512, 1, 0}; }
inline void UnloadTexture(Texture2D) {}
inline void DrawTexturePro(Texture2D, Rectangle, Rectangle, Vector2, float, Color) {}

// -----------------------------------------------------------------------------
// 1. Frame Data: un frame de animación
// -----------------------------------------------------------------------------
struct AnimationFrame {
    Rectangle source;      // Región en el atlas
    float duration;        // Segundos que dura este frame
    Vector2 offset;        // Offset de dibujo (para alinear sprites de distinta size)
    Vector2 origin;        // Pivote personalizado (opcional, 0,0 = usar default)

    AnimationFrame() : source{0,0,0,0}, duration(0.1f), offset{0,0}, origin{0,0} {}
    AnimationFrame(Rectangle src, float dur)
        : source(src), duration(dur), offset{0,0}, origin{0,0} {}
    AnimationFrame(Rectangle src, float dur, Vector2 off, Vector2 org)
        : source(src), duration(dur), offset(off), origin(org) {}
};

// -----------------------------------------------------------------------------
// 2. Animation Clip: datos inmutables de una animación completa
// -----------------------------------------------------------------------------
enum class LoopMode {
    Once,       // Se queda en último frame
    Loop,       // Vuelve al inicio
    PingPong    // Ida y vuelta
};

struct AnimationClip {
    std::string name;
    std::vector<AnimationFrame> frames;
    LoopMode loopMode = LoopMode::Loop;
    bool hasExitTime = false;      // Para transiciones: puede interrumpirse
    float exitTime = 1.0f;         // Normalizado 0-1 (cuando puede transicionar)

    AnimationClip() = default;
    AnimationClip(std::string n) : name(std::move(n)) {}

    float TotalDuration() const {
        float total = 0;
        for (const auto& f : frames) total += f.duration;
        return total;
    }

    int FrameCount() const { return (int)frames.size(); }

    // Factory: crea clip desde grid uniforme
    static AnimationClip FromGrid(std::string name, Texture2D texture,
                                  int startX, int startY,
                                  int frameW, int frameH,
                                  int cols, int rows,
                                  float frameDuration,
                                  LoopMode mode = LoopMode::Loop,
                                  int padding = 0, int margin = 0) {
        AnimationClip clip(std::move(name));
        // La textura NO se guarda en el clip: los frames solo almacenan rects,
        // la textura se pasa al Draw (permite reusar un clip con varias texturas)
        (void)texture;
        clip.frames.reserve(cols * rows);
        for (int row = 0; row < rows; ++row) {
            for (int col = 0; col < cols; ++col) {
                Rectangle src = {
                    float(margin + startX + col * (frameW + padding)),
                    float(margin + startY + row * (frameH + padding)),
                    float(frameW), float(frameH)
                };
                clip.frames.emplace_back(src, frameDuration);
            }
        }
        clip.loopMode = mode;
        return clip;
    }

    // Factory: frames con duraciones individuales
    static AnimationClip FromFrames(std::string name, std::vector<AnimationFrame> fr,
                                    LoopMode mode = LoopMode::Loop) {
        AnimationClip clip(std::move(name));
        clip.frames = std::move(fr);
        clip.loopMode = mode;
        return clip;
    }
};

// -----------------------------------------------------------------------------
// 3. Animation Player: reproduce un clip
// -----------------------------------------------------------------------------
struct AnimationPlayer {
    const AnimationClip* currentClip = nullptr;
    int currentFrame = 0;
    float frameTimer = 0.0f;
    float speed = 1.0f;           // Multiplicador de velocidad (1.0 = normal)
    bool playing = true;
    bool finished = false;        // True cuando termina en modo Once
    int pingPongDirection = 1;    // 1 = forward, -1 = backward

    AnimationPlayer() = default;

    void Play(const AnimationClip* clip, bool reset = true) {
        if (currentClip == clip && playing && !reset) return;
        currentClip = clip;
        currentFrame = 0;
        frameTimer = 0.0f;
        playing = true;
        finished = false;
        pingPongDirection = 1;
    }

    void Stop() { playing = false; }
    void Pause() { playing = false; }
    void Resume() { playing = true; }
    void SetSpeed(float s) { speed = s; }

    void Update(float dt) {
        if (!playing || !currentClip || currentClip->frames.empty()) return;

        frameTimer += dt * speed;
        const float frameDur = currentClip->frames[currentFrame].duration;

        if (frameTimer >= frameDur) {
            frameTimer -= frameDur;
            AdvanceFrame();
        }
    }

    void AdvanceFrame() {
        if (!currentClip) return;

        const int frameCount = currentClip->FrameCount();
        if (frameCount <= 1) return;

        switch (currentClip->loopMode) {
            case LoopMode::Once:
                if (currentFrame + 1 < frameCount) {
                    currentFrame++;
                } else {
                    finished = true;
                    playing = false;
                }
                break;

            case LoopMode::Loop:
                currentFrame = (currentFrame + 1) % frameCount;
                break;

            case LoopMode::PingPong:
                currentFrame += pingPongDirection;
                if (currentFrame >= frameCount - 1) {
                    currentFrame = frameCount - 1;
                    pingPongDirection = -1;
                } else if (currentFrame <= 0) {
                    currentFrame = 0;
                    pingPongDirection = 1;
                }
                break;
        }
    }

    // Obtiene frame actual para dibujar
    const AnimationFrame* GetCurrentFrame() const {
        if (!currentClip || currentClip->frames.empty()) return nullptr;
        if (currentFrame >= (int)currentClip->frames.size()) return nullptr;
        return &currentClip->frames[currentFrame];
    }

    // Progreso normalizado 0-1
    float GetProgress() const {
        if (!currentClip) return 0;
        float elapsed = 0;
        for (int i = 0; i < currentFrame; ++i) elapsed += currentClip->frames[i].duration;
        elapsed += frameTimer;
        return elapsed / currentClip->TotalDuration();
    }

    // Verifica si está en exit time (para transiciones)
    bool CanTransition() const {
        if (!currentClip) return true;
        return currentClip->hasExitTime && GetProgress() >= currentClip->exitTime;
    }

    // Dibuja frame actual
    void Draw(Texture2D texture, Vector2 position, Vector2 scale = {1,1},
              float rotation = 0, Color tint = {255,255,255,255}) const {
        const AnimationFrame* frame = GetCurrentFrame();
        if (!frame) return;

        Rectangle dest = {
            position.x + frame->offset.x * scale.x,
            position.y + frame->offset.y * scale.y,
            frame->source.width * scale.x,
            frame->source.height * scale.y
        };
        Vector2 origin = frame->origin;
        if (origin.x == 0 && origin.y == 0) {
            origin = { frame->source.width * 0.5f, frame->source.height * 0.5f };
        }
        DrawTexturePro(texture, frame->source, dest, origin, rotation, tint);
    }
};

// -----------------------------------------------------------------------------
// 4. Animation State Machine: transiciones entre clips
// -----------------------------------------------------------------------------
struct AnimationTransition {
    std::string from;
    std::string to;
    std::function<bool()> condition;  // Condición de transición
    bool hasExitTime = false;         // Esperar exit time del clip actual
    float exitTime = 1.0f;
    float crossfadeTime = 0.0f;       // Tiempo de blend (avanzado)

    AnimationTransition() = default;
    AnimationTransition(std::string f, std::string t, std::function<bool()> c)
        : from(std::move(f)), to(std::move(t)), condition(std::move(c)) {}
};

class AnimationStateMachine {
    std::unordered_map<std::string, AnimationClip> clips_;
    std::vector<AnimationTransition> transitions_;
    AnimationPlayer player_;
    std::string currentState_;

public:
    void AddClip(AnimationClip clip) {
        clips_.emplace(clip.name, std::move(clip));
    }

    void AddTransition(AnimationTransition trans) {
        transitions_.push_back(std::move(trans));
    }

    void SetState(const std::string& state, bool force = false) {
        auto it = clips_.find(state);
        if (it == clips_.end()) return;

        if (!force && player_.currentClip == &it->second && player_.playing) return;

        // Buscar transición válida desde estado actual
        if (!currentState_.empty() && !force) {
            for (const auto& trans : transitions_) {
                if (trans.from == currentState_ && trans.to == state) {
                    if (trans.condition && !trans.condition()) return;
                    if (trans.hasExitTime && !player_.CanTransition()) return;
                    break;  // Transición válida encontrada
                }
            }
        }

        player_.Play(&it->second);
        currentState_ = state;
    }

    void Update(float dt) {
        player_.Update(dt);

        // Auto-transiciones (sin condición explícita, solo exit time)
        if (player_.finished || (player_.CanTransition() && !currentState_.empty())) {
            for (const auto& trans : transitions_) {
                if (trans.from == currentState_ && trans.condition == nullptr) {
                    auto it = clips_.find(trans.to);
                    if (it != clips_.end()) {
                        player_.Play(&it->second);
                        currentState_ = trans.to;
                        break;
                    }
                }
            }
        }
    }

    void Draw(Texture2D texture, Vector2 position, Vector2 scale = {1,1}) const {
        player_.Draw(texture, position, scale);
    }

    const std::string& CurrentState() const { return currentState_; }
    const AnimationPlayer& Player() const { return player_; }
    AnimationPlayer& Player() { return player_; }
};

// -----------------------------------------------------------------------------
// 5. Ejemplo: Personaje con Idle, Walk, Attack, Hurt
// -----------------------------------------------------------------------------
void demo_animation_state_machine() {
    std::cout << "=== Animation State Machine ===\n\n";

    Texture2D tex = LoadTexture("assets/character.png");

    AnimationStateMachine sm;

    // Clips (usando FromGrid para simplicidad)
    sm.AddClip(AnimationClip::FromGrid("Idle", tex, 0, 0, 32, 32, 4, 1, 0.2f, LoopMode::Loop));
    sm.AddClip(AnimationClip::FromGrid("Walk", tex, 0, 32, 32, 32, 6, 1, 0.1f, LoopMode::Loop));
    sm.AddClip(AnimationClip::FromGrid("Attack", tex, 0, 64, 32, 32, 4, 1, 0.08f, LoopMode::Once));
    sm.AddClip(AnimationClip::FromGrid("Hurt", tex, 0, 96, 32, 32, 2, 1, 0.15f, LoopMode::Once));

    // Attack tiene exit time al 80% (puede cancelarse en los últimos frames)
    // Nota: los clips viven en el map de la machine (copia por valor); el exit
    // time se configura por transición vía AnimationTransition::hasExitTime.
    // NO guardar punteros a los clips: el map los mueve al insertar (emplace).

    // Transiciones
    bool moving = false;
    bool attacking = false;
    bool hurt = false;

    sm.AddTransition({"Idle", "Walk", [&]{ return moving; }});
    sm.AddTransition({"Walk", "Idle", [&]{ return !moving; }});
    sm.AddTransition({"Idle", "Attack", [&]{ return attacking; }});
    sm.AddTransition({"Walk", "Attack", [&]{ return attacking; }});
    sm.AddTransition({"Attack", "Idle", nullptr});  // Auto al terminar (exit time)
    sm.AddTransition({"Attack", "Hurt", [&]{ return hurt; }});  // Interrupción
    sm.AddTransition({"Hurt", "Idle", nullptr});  // Auto al terminar

    // Simulación
    sm.SetState("Idle");

    for (int tick = 0; tick < 20; ++tick) {
        float dt = 0.016f;

        // Input simulado
        if (tick == 5) moving = true;
        if (tick == 10) { moving = false; attacking = true; }
        if (tick == 13) attacking = false;
        if (tick == 15) { hurt = true; attacking = false; }
        if (tick == 17) hurt = false;

        sm.Update(dt);
        std::cout << "Tick " << tick << ": State=" << sm.CurrentState()
                  << " Frame=" << sm.Player().currentFrame
                  << " Progress=" << sm.Player().GetProgress() << "\n";
    }

    UnloadTexture(tex);
    std::cout << "\n";
}

// -----------------------------------------------------------------------------
// 6. Animation Events (callbacks en frames específicos)
// -----------------------------------------------------------------------------
/*
 * EVENTOS DE ANIMACIÓN:
 * - Footstep en frame 2 y 5 de Walk
 * - Hitbox activada en frame 3 de Attack
 * - Sonido de aterrizaje en frame 1 de Land
 *
 * IMPLEMENTACIÓN:
 *   struct AnimationEvent { int frame; std::string name; std::function<void()> callback; };
 *   En AnimationClip: std::vector<AnimationEvent> events;
 *   En AnimationPlayer::AdvanceFrame(): chequear eventos del nuevo frame y ejecutar.
 */

struct AnimationEvent {
    int frameIndex;
    std::string name;
    std::function<void()> callback;
};

struct AnimationClipWithEvents : AnimationClip {
    std::vector<AnimationEvent> events;

    AnimationClipWithEvents() = default;
    // Reexpone el constructor del base: "Attack" va a AnimationClip(std::string)
    explicit AnimationClipWithEvents(std::string name) : AnimationClip(std::move(name)) {}

    void AddEvent(int frame, std::string name, std::function<void()> cb) {
        events.push_back({frame, std::move(name), std::move(cb)});
    }
};

void demo_animation_events() {
    std::cout << "=== Animation Events ===\n\n";

    AnimationClipWithEvents attack("Attack");
    attack.frames = {
        {{0,0,32,32}, 0.05f}, {{32,0,32,32}, 0.05f},
        {{64,0,32,32}, 0.05f}, {{96,0,32,32}, 0.1f}
    };
    attack.loopMode = LoopMode::Once;

    // Eventos: frame 2 = hitbox on, frame 3 = hitbox off + sound
    attack.AddEvent(2, "HitboxOn", []{ std::cout << "  >> HITBOX ACTIVATED\n"; });
    attack.AddEvent(3, "HitboxOff", []{ std::cout << "  >> HITBOX DEACTIVATED\n"; });
    attack.AddEvent(3, "SwingSound", []{ std::cout << "  >> PLAY SWING SOUND\n"; });

    AnimationPlayer player;
    player.Play(&attack);

    for (int i = 0; i < 10; ++i) {
        player.Update(0.016f);
        if (player.currentFrame < (int)attack.events.size()) {
            for (const auto& ev : attack.events) {
                if (ev.frameIndex == player.currentFrame) {
                    if (ev.callback) ev.callback();
                }
            }
        }
        if (player.finished) break;
    }
    std::cout << "\n";
}

// -----------------------------------------------------------------------------
// 7. Blending / Crossfade (avanzado)
// -----------------------------------------------------------------------------
/*
 * CROSSFADE ENTRE ANIMACIONES:
 * - Mezcla dos clips durante crossfadeTime segundos
 * - Peso: clip_from (1-t) + clip_to (t) donde t = elapsed/crossfadeTime
 * - Requiere dibujar ambos clips con alpha blending
 * - Útil para transiciones suaves (Idle->Walk, Walk->Run)
 *
 * NOTA: Para implementación completa se necesita:
 *   - Skeleton/bone hierarchy (para mezclar transforms, no solo sprites)
 *   - O blend de texturas (más simple pero menos flexible)
 */

void demo_crossfade_concept() {
    std::cout << "=== Crossfade Concept ===\n";
    std::cout << R"(
Crossfade entre Idle y Walk:
  t = 0.0: 100% Idle, 0% Walk
  t = 0.5: 50% Idle, 50% Walk (dibuja ambos con alpha)
  t = 1.0: 0% Idle, 100% Walk

Implementación simple (sprite-based):
  DrawTexturePro(idleFrame, ..., tint = {255,255,255, 255*(1-t)});
  DrawTexturePro(walkFrame, ..., tint = {255,255,255, 255*t});

Para skeletal animation: mezclar transforms de huesos (matrices).
)" << "\n";
}

int main() {
    demo_animation_state_machine();
    demo_animation_events();
    demo_crossfade_concept();

    std::cout << "=== FIN LECCIÓN 03 ===\n";
    return 0;
}