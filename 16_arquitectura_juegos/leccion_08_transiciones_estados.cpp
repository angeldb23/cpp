/* =============================================================================
   MÓDULO 16 - LECCIÓN 08: Transiciones entre Estados — Fade, Loading, Transición Suave
   =============================================================================
   TEMA 5 de TEMAS.md (2da lección): Transiciones y pantallas de carga.

   TIPOS DE TRANSICIÓN:
   --------------------
   1. INMEDIATA (Cut): Change(state) - instantáneo, para menús simples.
   2. FADE IN/OUT:   Alpha 0->1 (negro) -> Change -> 1->0. Suave, profesional.
   3. SLIDE/WIPE:    Deslizar nuevo estado sobre viejo (UI estilo móvil).
   4. LOADING SCREEN: Estado intermedio que carga assets async + barra progreso.

   FADE SYSTEM (Genérico):
   -----------------------
   enum class FadePhase { None, FadeOut, FadeIn };

   struct FadeTransition {
       FadePhase phase = FadePhase::None;
       float alpha = 0.0f;          // 0 = transparente, 1 = negro sólido
       float speed = 2.0f;          // Segundos para fade completo
       std::function<void()> onMid; // Callback al llegar a alpha=1 (cambio real)

       void Start(std::function<void()> midCallback) {
           phase = FadePhase::FadeOut;
           alpha = 0.0f;
           onMid = std::move(midCallback);
       }

       void Update(float dt) {
           if (phase == FadePhase::None) return;

           float delta = dt / speed;
           if (phase == FadePhase::FadeOut) {
               alpha += delta;
               if (alpha >= 1.0f) {
                   alpha = 1.0f;
                   phase = FadePhase::FadeIn;
                   if (onMid) { onMid(); onMid = nullptr; }  // Cambio de estado AQUÍ
               }
           } else { // FadeIn
               alpha -= delta;
               if (alpha <= 0.0f) {
                   alpha = 0.0f;
                   phase = FadePhase::None;
               }
           }
       }

       void Render() const {
           if (phase != FadePhase::None && alpha > 0.0f) {
               // DrawRectangle(0, 0, screenW, screenH, {0, 0, 0, (unsigned char)(alpha * 255)});
           }
       }

       bool IsActive() const { return phase != FadePhase::None; }
   };

   USO EN STATE MANAGER:
   ---------------------
   void ChangeWithFade(std::unique_ptr<IGameState> newState) {
       fade.Start([this, newState = std::move(newState)]() mutable {
           stateManager.Change(std::move(newState));
       });
   }

   // En game loop:
   fade.Update(dt);
   if (!fade.IsActive()) currentState->Update(dt);
   currentState->Render(alpha);
   fade.Render();  // Dibuja overlay negro arriba de todo

   LOADING STATE (Carga Asíncrona):
   --------------------------------
   - Para assets grandes (texturas, audio, mapas) que bloquearían el frame.
   - Hilo worker carga en background; main thread muestra progreso.
   - raylib: LoadTexture/LoadSound son bloqueantes; usar LoadTextureFromImage
     con imagen cargada en hilo aparte, o LoadTextureAsync (raylib 5.5+).

   ESTRUCTURA LOADINGSTATE:
   ------------------------
   class LoadingState : public IGameState {
       std::thread loaderThread_;
       std::atomic<float> progress_{0.0f};
       std::function<void()> onComplete_;
       std::unique_ptr<IGameState> nextState_;

       void Enter() override {
           progress_ = 0.0f;
           loaderThread_ = std::thread([this] {
               // Cargar assets pesados aquí
               LoadHeavyAssets();
               progress_ = 1.0f;
           });
       }

       void Update(float dt) override {
           if (progress_ >= 1.0f && loaderThread_.joinable()) {
               loaderThread_.join();
               if (onComplete_) onComplete_();  // Transicionar al siguiente estado
           }
       }

       void Render(float) override {
           DrawText("Cargando...", 350, 200, 30, WHITE);
           DrawRectangle(200, 250, 400 * progress_, 30, GREEN);
           DrawRectangleLines(200, 250, 400, 30, WHITE);
       }

       void Exit() override {
           if (loaderThread_.joinable()) loaderThread_.join();
       }
   };

   TRANSICIÓN COMPLETA: Menu -> Loading -> Playing
   -----------------------------------------------
   1. Usuario pulsa "Jugar" en Menu
   2. FadeOut (negro)
   3. Callback mid: Push(LoadingState) con onComplete = [&]{ FadeIn; Change(Playing); }
   4. LoadingState carga assets, actualiza progress
   5. Al terminar: callback -> FadeIn + Change(Playing)
   6. Playing aparece con FadeIn desde negro

   CONSIDERACIONES THREAD-SAFETY:
   ------------------------------
   - raylib NO es thread-safe para funciones de GPU (LoadTexture, Draw*).
   - Cargar datos crudos (stb_image, miniaudio decode) en hilo worker.
   - Subir a GPU (glTexImage2D, raylib LoadTextureFromImage) en main thread.
   - Usar colas thread-safe (std::queue + mutex) para pasar datos al main thread.

   ============================================================================= */

#include <iostream>
#include <functional>
#include <atomic>
#include <thread>
#include <chrono>

enum class FadePhase { None, FadeOut, FadeIn };

struct FadeTransition {
    FadePhase phase = FadePhase::None;
    float alpha = 0.0f;
    float speed = 1.0f;  // 1 segundo por fade
    std::function<void()> onMid;

    void Start(std::function<void()> mid) {
        phase = FadePhase::FadeOut;
        alpha = 0.0f;
        onMid = std::move(mid);
    }

    void Update(float dt) {
        if (phase == FadePhase::None) return;
        float delta = dt / speed;
        if (phase == FadePhase::FadeOut) {
            alpha += delta;
            if (alpha >= 1.0f) {
                alpha = 1.0f;
                phase = FadePhase::FadeIn;
                if (onMid) { onMid(); onMid = nullptr; }
            }
        } else {
            alpha -= delta;
            if (alpha <= 0.0f) {
                alpha = 0.0f;
                phase = FadePhase::None;
            }
        }
    }

    bool IsActive() const { return phase != FadePhase::None; }
    void Render() const {
        if (phase != FadePhase::None && alpha > 0.0f) {
            std::cout << "  [Fade Render] alpha=" << alpha << " (negro overlay)\n";
        }
    }
};

// Demo de transición con fade
int main() {
    std::cout << "=== LECCIÓN 08: Transiciones entre Estados ===\n";

    FadeTransition fade;
    std::string currentState = "Menu";
    bool loadingDone = false;

    auto startTransition = [&](const char* nextState) {
        std::cout << "\n--- Iniciando transición: " << currentState << " -> " << nextState << " ---\n";
        fade.Start([&]() {
            std::cout << "  [FADE MID] Cambio real de estado\n";
            currentState = nextState;
            loadingDone = false;
            // Simular carga async
            std::thread([&]() {
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                loadingDone = true;
            }).detach();
        });
    };

    // Simular game loop por 3 segundos
    float dt = 0.016f;  // 60 FPS fijo para demo
    for (int frame = 0; frame < 180; ++frame) {  // ~3 segundos
        if (frame == 30) startTransition("Loading");
        if (frame == 90 && currentState == "Loading" && loadingDone) {
            std::cout << "  [Loading complete] Transicionando a Playing\n";
            fade.Start([&]() { currentState = "Playing"; });
        }

        fade.Update(dt);
        if (!fade.IsActive()) {
            // Update estado actual
        }
        fade.Render();

        if (frame % 30 == 0) {
            std::cout << "  Frame " << frame << ": estado=" << currentState
                      << ", fade=" << (fade.IsActive() ? "activo" : "inactivo") << "\n";
        }
    }

    std::cout << "\nEstado final: " << currentState << "\n";
    std::cout << "=== FIN LECCIÓN 08 ===\n";
    return 0;
}