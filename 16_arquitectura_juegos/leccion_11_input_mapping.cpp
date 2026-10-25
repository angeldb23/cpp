/* =============================================================================
   MÓDULO 16 - LECCIÓN 11: Input — Polling vs Eventos, Mapeo a Acciones Lógicas
   =============================================================================
   TEMA 7 de TEMAS.md: Input polling vs eventos, teclado/ratón/gamepad, acciones.

   POLLING VS EVENTOS:
   -------------------
   POLLING (raylib, SDL_GetKeyboardState):
   - Cada frame: consulta estado actual de todas las teclas/botones.
   - Ventaja: simple, estado inmediato, sin cola de eventos, determinista.
   - Ideal para: movimiento continuo, juego arcade, frame fijo.

   EVENTOS (SDL_PollEvent, GLFW callbacks, Windows messages):
   - Sistema operativo/en ventana empuja eventos (KeyDown, KeyUp, MouseMove...).
   - Ventaja: bajo overhead si pocos inputs, captura texto/IME, eventos de ventana.
   - Ideal para: UI, escritura de texto, menús, eventos discretos (click, tecla).

   RAYLIB USA POLLING:
   -------------------
   - IsKeyDown(KEY) -> true mientras tecla presionada (continuo).
   - IsKeyPressed(KEY) -> true SOLO en frame que se presiona (edge).
   - IsKeyReleased(KEY) -> true SOLO en frame que se suelta.
   - GetMousePosition(), IsMouseButtonDown(), GetMouseWheelMove().
   - GetGamepadAxisMovement(), IsGamepadButtonDown().

   MAPEO A ACCIONES LÓGICAS (Action Mapping):
   ------------------------------------------
   // Separar INPUT FÍSICO (teclas) de ACCIONES DE JUEGO (saltar, mover).
   enum class Action { MoveLeft, MoveRight, Jump, Attack, Pause, MenuUp, MenuDown };

   struct InputMapping {
       int key;           // Tecla física (KEY_A, KEY_SPACE...)
       int gamepadBtn;    // Botón gamepad (GAMEPAD_BUTTON_A...)
       Action action;     // Acción lógica
   };

   // Configuración (data-driven, cargable de JSON)
   InputMapping mappings[] = {
       {KEY_LEFT,  GAMEPAD_BUTTON_LEFT_FACE_LEFT,  Action::MoveLeft},
       {KEY_RIGHT, GAMEPAD_BUTTON_LEFT_FACE_RIGHT, Action::MoveRight},
       {KEY_UP,    GAMEPAD_BUTTON_LEFT_FACE_UP,    Action::Jump},
       {KEY_Z,     GAMEPAD_BUTTON_RIGHT_FACE_DOWN, Action::Attack},
       {KEY_ESCAPE, 0,                             Action::Pause},
       {KEY_W,     GAMEPAD_BUTTON_LEFT_FACE_UP,    Action::MenuUp},
       {KEY_S,     GAMEPAD_BUTTON_LEFT_FACE_DOWN,  Action::MenuDown},
   };

   struct InputState {
       bool current[ActionCount] = {};   // Estado este frame
       bool previous[ActionCount] = {};  // Estado frame anterior
   };

   void UpdateInput(InputState& input, const InputMapping* maps, size_t count) {
       // Copiar current -> previous
       std::copy(std::begin(input.current), std::end(input.current),
                 std::begin(input.previous));

       // Polling teclado + gamepad
       for (size_t i = 0; i < count; ++i) {
           bool down = false;
           if (maps[i].key != 0) down = down || IsKeyDown(maps[i].key);
           if (maps[i].gamepadBtn != 0 && IsGamepadAvailable(0))
               down = down || IsGamepadButtonDown(0, maps[i].gamepadBtn);

           input.current[maps[i].action] = down;
       }
   }

   // Helpers para detectar edges
   bool IsActionPressed(const InputState& input, Action a) {
       return input.current[static_cast<int>(a)] && !input.previous[static_cast<int>(a)];
   }
   bool IsActionReleased(const InputState& input, Action a) {
       return !input.current[static_cast<int>(a)] && input.previous[static_cast<int>(a)];
   }
   bool IsActionHeld(const InputState& input, Action a) {
       return input.current[static_cast<int>(a)];
   }

   VENTAJAS DEL MAPEO:
   -------------------
   1. RECONFIGURABLE: Cambiar teclas sin tocar código de juego (options menu).
   2. MULTI-DISPOSITIVO: Teclado + gamepad + touch mapean a mismas acciones.
   3. TESTABLE: InputState es datos puros; simular inputs en tests sin raylib.
   4. REPLAY: Grabar sequence de InputState para replay determinista.
   5. NETWORK: Enviar InputState (bits) en vez de teclas crudas.

   GAMEPAD (raylib):
   -----------------
   - IsGamepadAvailable(0): verifica gamepad 0 conectado.
   - GetGamepadAxisCount(0): ejes (stick X/Y, triggers).
   - GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X): -1.0 a 1.0.
   - Dead zone: ignorar valores < 0.1-0.2 para sticks.

   TEXTO/IME (Eventos):
   --------------------
   - Polling NO sirve para texto (repetición, composición, Unicode).
   - raylib: GetCharPressed() devuelve codepoints Unicode (evento interno).
   - Para chat/nombres: usar GetCharPressed() en loop hasta 0.

   EJEMPLO COMPLETO:
   -----------------
   InputState input;
   while (!WindowShouldClose()) {
       UpdateInput(input, mappings, MAP_COUNT);

       if (IsActionPressed(input, Action::Pause)) TogglePause();
       if (IsActionHeld(input, Action::MoveLeft))  player.vel.x = -speed;
       if (IsActionHeld(input, Action::MoveRight)) player.vel.x = speed;
       if (IsActionPressed(input, Action::Jump) && onGround) player.Jump();
   }

   ============================================================================= */

#include <iostream>
#include <array>
#include <algorithm>

enum class Action {
    MoveLeft, MoveRight, Jump, Attack, Pause, MenuUp, MenuDown, Count
};
constexpr size_t ActionCount = static_cast<size_t>(Action::Count);

struct InputMapping {
    int key;         // Tecla (simulado con int)
    int gamepadBtn;  // Botón gamepad
    Action action;
};

struct InputState {
    std::array<bool, ActionCount> current{};
    std::array<bool, ActionCount> previous{};
};

// Simulación de raylib IsKeyDown
bool IsKeyDown(int key) {
    // En demo: teclas 1-7 = acciones 0-6
    static std::array<bool, 8> keys{};
    return (key >= 1 && key <= 7) ? keys[key] : false;
}

void SimulateKeyPress(int key, bool down) {
    static std::array<bool, 8> keys{};
    if (key >= 1 && key <= 7) keys[key] = down;
}

void UpdateInput(InputState& input, const InputMapping* maps, size_t count) {
    input.previous = input.current;
    for (size_t i = 0; i < count; ++i) {
        bool down = IsKeyDown(maps[i].key);
        input.current[static_cast<size_t>(maps[i].action)] = down;
    }
}

bool IsActionPressed(const InputState& input, Action a) {
    size_t idx = static_cast<size_t>(a);
    return input.current[idx] && !input.previous[idx];
}
bool IsActionReleased(const InputState& input, Action a) {
    size_t idx = static_cast<size_t>(a);
    return !input.current[idx] && input.previous[idx];
}
bool IsActionHeld(const InputState& input, Action a) {
    return input.current[static_cast<size_t>(a)];
}

const char* ActionName(Action a) {
    static const char* names[] = {"MoveLeft", "MoveRight", "Jump", "Attack",
                                   "Pause", "MenuUp", "MenuDown"};
    return names[static_cast<size_t>(a)];
}

int main() {
    std::cout << "=== LECCIÓN 11: Input Polling + Action Mapping ===\n";

    InputMapping mappings[] = {
        {1, 0, Action::MoveLeft},   // Tecla '1' = Izquierda
        {2, 0, Action::MoveRight},  // Tecla '2' = Derecha
        {3, 0, Action::Jump},       // Tecla '3' = Saltar
        {4, 0, Action::Attack},     // Tecla '4' = Atacar
        {5, 0, Action::Pause},      // Tecla '5' = Pausa
    };

    InputState input;

    // Simular frames
    std::cout << "\nFrame 1: Usuario presiona '1' (MoveLeft) y '3' (Jump)\n";
    SimulateKeyPress(1, true);
    SimulateKeyPress(3, true);
    UpdateInput(input, mappings, 5);
    for (int i = 0; i < 5; ++i) {
        Action a = static_cast<Action>(i);
        if (input.current[i]) std::cout << "  " << ActionName(a) << ": HELD\n";
    }
    if (IsActionPressed(input, Action::Jump)) std::cout << "  -> JUMP PRESSED (edge)!\n";

    std::cout << "\nFrame 2: Usuario suelta '1', mantiene '3'\n";
    SimulateKeyPress(1, false);
    UpdateInput(input, mappings, 5);
    for (int i = 0; i < 5; ++i) {
        if (input.current[i]) std::cout << "  " << ActionName(static_cast<Action>(i)) << ": HELD\n";
    }
    if (IsActionReleased(input, Action::MoveLeft)) std::cout << "  -> MoveLeft RELEASED\n";

    std::cout << "\nFrame 3: Usuario presiona '5' (Pause)\n";
    SimulateKeyPress(5, true);
    UpdateInput(input, mappings, 5);
    if (IsActionPressed(input, Action::Pause)) std::cout << "  -> PAUSE PRESSED -> TogglePause()\n";

    std::cout << "\nVentajas: reconfigurable, multi-device, testable, replay, network.\n";
    std::cout << "=== FIN LECCIÓN 11 ===\n";
    return 0;
}