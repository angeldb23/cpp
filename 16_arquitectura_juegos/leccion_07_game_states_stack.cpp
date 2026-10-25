/* =============================================================================
   MÓDULO 16 - LECCIÓN 07: Estados de Juego — Pila de Estados (Menu, Jugando, Pausa)
   =============================================================================
   TEMA 5 de TEMAS.md (1ra lección): Estados de juego y pila de estados.

   POR QUÉ ESTADOS (State Machine / Stack):
   ----------------------------------------
   - Juegos tienen modos distintos: Menú, Jugando, Pausa, GameOver, Cargando...
   - Cada estado tiene SU propio Input, Update, Render, Enter/Exit.
   - Pila (stack) permite: Pausa sobre Juego, Menú sobre Pausa, transiciones limpias.

   INTERFAZ COMÚN (State):
   -----------------------
   struct IGameState {
       virtual ~IGameState() = default;
       virtual void Enter() {}      // Called al apilar (push)
       virtual void Exit() {}       // Called al desapilar (pop)
       virtual void Update(float dt) = 0;
       virtual void Render(float alpha) = 0;
       virtual void HandleInput() = 0;
   };

   GAME STATE MANAGER (Stack):
   ---------------------------
   class StateManager {
       std::vector<std::unique_ptr<IGameState>> stack_;

   public:
       void Push(std::unique_ptr<IGameState> state) {
           if (!stack_.empty()) stack_.back()->Exit();
           stack_.push_back(std::move(state));
           stack_.back()->Enter();
       }

       void Pop() {
           if (!stack_.empty()) {
               stack_.back()->Exit();
               stack_.pop_back();
               if (!stack_.empty()) stack_.back()->Enter();  // Resume previo
           }
       }

       void Change(std::unique_ptr<IGameState> state) {
           Pop();
           Push(std::move(state));
       }

       IGameState* Current() { return stack_.empty() ? nullptr : stack_.back().get(); }
       bool Empty() const { return stack_.empty(); }
   };

   GAME LOOP CON STATE MANAGER:
   ----------------------------
   StateManager states;
   states.Push(std::make_unique<MenuState>());

   while (!WindowShouldClose() && !states.Empty()) {
       float dt = GetFrameTime();  // O timestep fijo del L05
       auto* current = states.Current();

       current->HandleInput();
       current->Update(dt);
       current->Render(alpha);
   }

   ESTADOS TÍPICOS:
   ----------------
   1. MenuState:   Fondo, botones (Jugar, Opciones, Salir), input navegación.
   2. PlayingState: Gameplay real, física, entidades, HUD.
   3. PauseState:  Overlay semitransparente, botones (Continuar, Menú, Salir).
   4. GameOverState: Pantalla final, score, botón "Reintentar" -> Change(Playing).
   5. LoadingState: Barra de carga, carga async de assets, transiciona a Playing.

   TRANSICIONES COMUNES:
   ---------------------
   - Menu -> Playing:   Push(Playing)  (Menu queda debajo para volver rápido)
   - Playing -> Pause:  Push(Pause)    (Playing pausado debajo)
   - Pause -> Playing:  Pop()          (Resume Playing)
   - Pause -> Menu:     Pop(); Pop()   (Sale de ambos)
   - Playing -> GameOver: Change(GameOver) (Limpia Playing)
   - GameOver -> Menu:  Change(Menu)

   ENTER/EXIT PARA RECURSOS:
   -------------------------
   - Enter(): Cargar assets, iniciar música, resetear timers, suscribir eventos.
   - Exit():  Descargar assets, parar música, limpiar entidades, desuscribir.
   - RAII en miembros del estado (std::unique_ptr, vectores) = limpieza auto.

   EJEMPLO: MENÚ SIMPLE:
   ---------------------
   class MenuState : public IGameState {
       int selected = 0;
       const char* options[3] = {"Jugar", "Opciones", "Salir"};

       void HandleInput() override {
           if (IsKeyPressed(KEY_UP)) selected = (selected + 2) % 3;
           if (IsKeyPressed(KEY_DOWN)) selected = (selected + 1) % 3;
           if (IsKeyPressed(KEY_ENTER)) {
               if (selected == 0) manager_->Push(std::make_unique<PlayingState>());
               else if (selected == 2) manager_->Pop();  // Sale del juego
           }
       }
       void Render(float) override {
           for (int i = 0; i < 3; ++i) {
               Color c = (i == selected) ? YELLOW : WHITE;
               DrawText(options[i], 350, 200 + i * 40, 30, c);
           }
       }
   };

   ============================================================================= */

#include <iostream>
#include <vector>
#include <memory>
#include <string>

// Demo conceptual: máquina de estados con pila (sin raylib)
struct IGameState {
    virtual ~IGameState() = default;
    virtual void Enter() { std::cout << "  [Enter] " << Name() << "\n"; }
    virtual void Exit()  { std::cout << "  [Exit]  " << Name() << "\n"; }
    virtual void Update(float) = 0;
    virtual void Render(float) = 0;
    virtual const char* Name() const = 0;
};

class StateManager {
    std::vector<std::unique_ptr<IGameState>> stack_;
public:
    void Push(std::unique_ptr<IGameState> s) {
        if (!stack_.empty()) stack_.back()->Exit();
        stack_.push_back(std::move(s));
        stack_.back()->Enter();
    }
    void Pop() {
        if (!stack_.empty()) {
            stack_.back()->Exit();
            stack_.pop_back();
            if (!stack_.empty()) stack_.back()->Enter();
        }
    }
    void Change(std::unique_ptr<IGameState> s) { Pop(); Push(std::move(s)); }
    IGameState* Current() { return stack_.empty() ? nullptr : stack_.back().get(); }
    bool Empty() const { return stack_.empty(); }
};

// Estados de ejemplo
struct MenuState : IGameState {
    const char* Name() const override { return "MenuState"; }
    void Update(float) override {}
    void Render(float) override { std::cout << "  [Render] Menu: [Jugar] [Salir]\n"; }
};

struct PlayingState : IGameState {
    const char* Name() const override { return "PlayingState"; }
    void Update(float dt) override { std::cout << "  [Update] Playing dt=" << dt << "\n"; }
    void Render(float alpha) override { std::cout << "  [Render] Playing alpha=" << alpha << "\n"; }
};

struct PauseState : IGameState {
    const char* Name() const override { return "PauseState"; }
    void Update(float) override {}
    void Render(float) override { std::cout << "  [Render] Pause overlay\n"; }
};

int main() {
    std::cout << "=== LECCIÓN 07: Pila de Estados de Juego ===\n";

    StateManager sm;
    sm.Push(std::make_unique<MenuState>());

    // Simular: Menu -> Playing -> Pause -> Playing -> Menu -> Exit
    std::cout << "\n--- Usuario pulsa 'Jugar' ---\n";
    sm.Push(std::make_unique<PlayingState>());
    sm.Current()->Update(0.016f);
    sm.Current()->Render(0.5f);

    std::cout << "\n--- Usuario pulsa ESC (pausa) ---\n";
    sm.Push(std::make_unique<PauseState>());
    sm.Current()->Render(0.0f);

    std::cout << "\n--- Usuario pulsa 'Continuar' ---\n";
    sm.Pop();  // Vuelve a Playing
    sm.Current()->Update(0.016f);
    sm.Current()->Render(0.3f);

    std::cout << "\n--- Usuario pulsa 'Salir al menú' ---\n";
    sm.Pop();  // Sale de Playing
    sm.Current()->Render(0.0f);

    std::cout << "\n--- Usuario pulsa 'Salir del juego' ---\n";
    sm.Pop();  // Stack vacía -> juego termina
    std::cout << "Stack vacía: " << (sm.Empty() ? "sí" : "no") << "\n";

    std::cout << "\n=== FIN LECCIÓN 07 ===\n";
    return 0;
}