/* =============================================================================
   MÓDULO 17 - LECCIÓN 11: Debug Drawing — Colisiones, Pivots, Grid, FPS y
   Consola en Pantalla
   =============================================================================
   TEMA 9 de TEMAS.md: Debug drawing

   POR QUÉ EXISTE:
   Un juego con colisiones "raras", pivotes mal puestos, tasas de disparo
   injustas o stuttering NO se depura a ojo con los sprites; se depura
   visualizando la información INTERNA en tiempo real:
   - AABBs y círculos de colisión con contorno (wireframe)
   - Pivote del sprite (cruz en el origin) y dirección de la velocidad
   - Grid del mundo/celda de colisión (líneas cada N píxeles)
   - FPS y frame time del frame (HUD) + el gráfico del último frame history
   - Consola en pantalla: log de últimos N mensajes con filtro de categoría

   REGLAS DE UN SISTEMA DE DEBUG DRAW:
   1. DEBE apagarse por completo: con la cap OFF no corre ni un Draw. Toggle
      por tecla (F1 en raylib) o por build (NDEBUG quita los cuerpos).
   2. NO entra en el render "normal": se dibuja APARTE (encima de todo, con un
      color por categoría) y puede cruzar cámaras: la parte de mundo se dibuja
      en BeginMode2D/EndMode2D y el HUD después, en espacio de pantalla.
   3. COLECTAR datos en el update, dibujar en el render: no mezclar fases.
   4. Cero alocación por frame idealmente: el ring buffer del log es un deque
      de tamaño fijo que recicla entradas.

   TÉCNICAS raylib (docs 5.5): DrawRectangleLinesEx, DrawLine, DrawCircleLines,
   DrawFPS, DrawText, MeasureText; el grupo GetFrameTime() da el dt real
   de raylib (módulos 16: en game loop manual, la generas vos).

   NOTA: Código compila sin raylib (stubs): registrar los datos "dibujados"
   en un vector de eventos y los imprimir con formato para que la demo
   demuestre QUÉ se dibujaría y por qué.
   ============================================================================= */

#include <iostream>
#include <vector>
#include <string>
#include <deque>
#include <cmath>
#include <algorithm>
#include <cstdio>

// -----------------------------------------------------------------------------
// Stubs raylib
// -----------------------------------------------------------------------------
struct Vector2 { float x, y; };
struct Rectangle { float x, y, width, height; };
struct Color { unsigned char r, g, b, a; };

inline float GetTime() { return 0.0f; }
inline float GetFrameTime() { return 0.016f; }
inline void DrawRectangleLinesEx(Rectangle, float, Color) {}
inline void DrawLine(int, int, int, int, Color) {}
inline void DrawCircleLines(int, int, float, Color) {}
inline void DrawText(const char*, int, int, int, Color) {}
inline int MeasureText(const char*, int) { return 10; }
inline void DrawFPS(int, int) {}

// RECORD: en vez de dibujar de verdad (no hay pantalla en la demo), el
// "recorder" guarda qué se dibujó: los prints de la demo muestran el resultado
// (en un juego real esto no existe: se dibuja directo con raylib).
struct DebugRecord {
    std::string kind;     // "rect", "line", "circle", "text"
    Rectangle rect{};
    Vector2 a{}, b{};
    std::string label;
};
static std::vector<DebugRecord> g_records;
// Para imprimir el conteo: util de demo
int RecordCount(const std::string& kind) {
    int n = 0;
    for (const auto& r : g_records) if (r.kind == kind) ++n;
    return n;
}

// -----------------------------------------------------------------------------
// 1. Primitivas de debug (wrapper con categoría, constante de "encendido")
// -----------------------------------------------------------------------------
struct DebugConfig {
    bool enabled = true;
    bool showCollisions = true;
    bool showPivots = true;
    bool showGrid = false;
    bool showHud = true;
};

// Tinte por categoría (magenta player, verde dado, amarillo muro, gris grid)
struct DebugPalette {
    static Color Red()    { return {255, 60, 60, 255}; }    // peligro/enemy
    static Color Green()  { return {60, 255, 60, 255}; }    // player
    static Color Yellow() { return {255, 255, 0, 255}; }    // triggers
    static Color Gray()   { return {120, 120, 120, 120}; }  // grid
    static Color Cyan()   { return {0, 200, 255, 255}; }    // texto
};

struct DebugDraw {
    DebugConfig cfg;

    void Rect(const Rectangle& r, Color c) {
        if (!cfg.enabled || !cfg.showCollisions) return;
        g_records.push_back({"rect", r, {}, {}, "rect"});
        DrawRectangleLinesEx(r, 1.0f, c);   // en raylib real: wireframe
    }

    void Circle(const Vector2& center, float radius, Color c) {
        if (!cfg.enabled || !cfg.showCollisions) return;
        g_records.push_back({"circle", {}, center, {radius, 0}, ""});
        DrawCircleLines(int(center.x), int(center.y), radius, c);
    }

    // Cruz del pivote del sprite (para ver el origin de la lección 02)
    void Pivot(const Vector2& pos) {
        if (!cfg.enabled || !cfg.showPivots) return;
        DrawLine(int(pos.x - 3), int(pos.y), int(pos.x + 3), int(pos.y), DebugPalette::Green());
        DrawLine(int(pos.x), int(pos.y - 3), int(pos.x), int(pos.y + 3), DebugPalette::Green());
        g_records.push_back({"pivot", {}, pos, {}, "+"});
    }

    // Velocidad como flecha (dirección del movimiento)
    void Velocity(const Vector2& pos, const Vector2& vel, float scale = 0.1f) {
        if (!cfg.enabled || !cfg.showPivots) return;
        DrawLine(int(pos.x), int(pos.y),
                 int(pos.x + vel.x * scale), int(pos.y + vel.y * scale),
                 DebugPalette::Cyan());
        g_records.push_back({"velocity", {}, pos, vel, "vec"});
    }

    // Grilla del mundo (en pantalla: líneas cada N píxeles). Si el mundo es
    // muy grande para dibujar la grilla entera, se recorta al rango visible
    // con el culling de la lección 10.
    void Grid(float worldW, float worldH, float cell) {
        if (!cfg.enabled || !cfg.showGrid) return;
        if (worldW / cell > 200 || worldH / cell > 200) {
            std::cout << "  [debug grid] mundo " << worldW << "px / celda " << cell
                      << " = " << int(worldW / cell) << " líneas: usar culling de cámara\n";
        }
        for (float x = 0.0f; x <= worldW; x += cell) {
            DrawLine(int(x), 0, int(x), int(worldH), DebugPalette::Gray());
        }
        for (float y = 0.0f; y <= worldH; y += cell) {
            DrawLine(0, int(y), int(worldW), int(y), DebugPalette::Gray());
        }
        g_records.push_back({"grid", {}, {}, {}, "grid"});
    }

    void Text(const std::string& s, int x, int y) {
        if (!cfg.enabled) return;
        DrawText(s.c_str(), x, y, 10, DebugPalette::Cyan());
        g_records.push_back({"text", {}, {}, {}, s});
    }
};

// -----------------------------------------------------------------------------
// 2. HUD: FPS, frame time y micro-gráfica del historial
// -----------------------------------------------------------------------------
/* El FPS solo (DrawFPS) engaña: un juego puede tener promedio 60 con spikes
   horribles a 15. El HUD captura el frame time y muestra:
   - el FPS promedio (1/suma del historial * tamaño) y el peor frame
   - una micro-barra con el frame time del último frame (scale de ms a px) */
class DebugHud {
    std::deque<float> frameTimes_;          // ring del último N
    size_t historySize_;
    float worstTime_ = 0.0f;
    float smoothedFps_ = 60.0f;             // media del historial, menos saltos

public:
    explicit DebugHud(size_t historySize = 120) : historySize_(historySize) {}

    void RecordFrame(float frameTime) {
        if (frameTime <= 0.0f) return;
        if (frameTimes_.size() >= historySize_) frameTimes_.pop_front();
        frameTimes_.push_back(frameTime);

        // FPS promedio: 1 tiene que ser el length del historial, no el total
        float total = 0.0f;
        for (float t : frameTimes_) total += t;
        smoothedFps_ = (total > 0.0f) ? float(frameTimes_.size()) / total : 0.0f;
        worstTime_ = std::max(worstTime_, frameTime);
    }

    float Fps()        const { return smoothedFps_; }
    float WorstFrame() const { return worstTime_; }

    // Dibuja el HUD / actualizado por frame (tool: la demo solo lo imprime)
    void Draw(int x, int y) const {
        char buf[64];
        std::snprintf(buf, sizeof(buf), "FPS: %.0f  (worst %.1f ms)",
                      smoothedFps_, worstTime_ * 1000.0f);
        DrawText(buf, x, y, 12, DebugPalette::Cyan());
        DrawFPS(x, y + 16);
        // barra gráfica del último frameTime *  scale
        if (!frameTimes_.empty()) {
            float t = frameTimes_.back();
            float barW = std::min(200.0f, t * 20000.0f);   // 10ms -> 200px
            DrawText("", int(barW), y + 32, 2, DebugPalette::Yellow());
        }
    }

    // Diagnóstico STRING (línea con lo que la demo imprime)
    std::string Describe() const {
        char buf[96];
        std::snprintf(buf, sizeof(buf), "FPS %.1f | worst %.1f ms | history %zu",
                      smoothedFps_, worstTime_ * 1000.0f, frameTimes_.size());
        return buf;
    }
};

// -----------------------------------------------------------------------------
// 3. Consola en pantalla: log ring buffer con filtro
// -----------------------------------------------------------------------------
class ScreenConsole {
    struct Entry {
        std::string category;
        std::string message;
        float life = 0.0f;   // tiempo restante en pantalla (0 = permanente)
    };
    std::deque<Entry> entries_;
    std::string filter_;     // categoría que se muestra ("" = todas)
    size_t maxLines_ = 8;
    float lifetime_ = 5.0f;  // cada línea decae
public:
    explicit ScreenConsole(size_t maxLines = 8, float lifetime = 5.0f)
        : maxLines_(maxLines), lifetime_(lifetime) {}

    void Log(const std::string& cat, const std::string& msg) {
        if (entries_.size() >= maxLines_) entries_.pop_front();
        entries_.push_back({cat, msg, lifetime_});
    }
    void SetFilter(const std::string& cat) { filter_ = cat; }

    void Update(float dt) {
        for (auto& e : entries_) {
            if (e.life > 0.0f) e.life -= dt;
        }
        // Purge de las expiradas (un remove_if + erase, no por intermedio)
        entries_.erase(
            std::remove_if(entries_.begin(), entries_.end(),
                           [](const Entry& e) { return e.life <= 0.0f; }),
            entries_.end());
    }

    void Render(int x, int startY) const {
        int y = startY;
        for (const auto& e : entries_) {
            if (!filter_.empty() && e.category != filter_) continue;
            DrawText(("[" + e.category + "] " + e.message).c_str(), x, y, 10, DebugPalette::Cyan());
            y += 14;   // salto de línea
        }
    }

    std::vector<std::string> Snapshot() const {
        std::vector<std::string> out;
        for (const auto& e : entries_) {
            if (!filter_.empty() && e.category != filter_) continue;  // filtra acá
            out.push_back("[" + e.category + "] " + e.message);
        }
        return out;
    }
};

// -----------------------------------------------------------------------------
// Demo: simula 1 frame y muestra el mensaje de debug que se dibujaría
// -----------------------------------------------------------------------------
void demo_debug_drawing() {
    std::cout << "=== Debug Drawing ===\n\n";

    DebugDraw dbg;

    // 1) Colisiones: AABB del player y del enemigo (wireframe con tinte)
    Rectangle playerAABB{400, 300 - 32, 24, 32};
    Rectangle enemyAABB{420, 296, 16, 24};
    dbg.Rect(playerAABB, DebugPalette::Green());
    dbg.Rect(enemyAABB, DebugPalette::Red());
    // El pivot del sprite del player (centro por defecto)
    dbg.Pivot({playerAABB.x + playerAABB.width / 2.0f, playerAABB.y + playerAABB.height / 2.0f});
    dbg.Velocity({playerAABB.x + playerAABB.width / 2.0f, playerAABB.y + playerAABB.height / 2.0f}, {50, -80});
    std::cout << "Primitivas registradas: rects=" << RecordCount("rect")
              << " pivots=" << RecordCount("pivot")
              << " velocities=" << RecordCount("velocity") << "\n";

    // 2) Toggle OFF: nada se dibuja
    dbg.cfg.enabled = false;
    g_records.clear();
    dbg.Rect(playerAABB, DebugPalette::Green());
    std::cout << "Con enabled=false: se dibujo " << g_records.size() << " primitiva\n";
    dbg.cfg.enabled = true;

    // 3) HUD: simula 120 frames con algunos spikes
    DebugHud hud(120);
    for (int f = 0; f < 120; ++f) {
        float ft = ((f % 40) == 0) ? 0.045f : (0.0140f + 0.0012f * (f % 17));
        hud.RecordFrame(ft);
    }
    std::cout << "HUD: " << hud.Describe() << "\n";
    hud.Draw(10, 10);

    // 4) Consola en pantalla: logs de juego con vida útil y filtro
    ScreenConsole console(6, 10.0f);
    console.Log("col", "player toca piso en (96, 576)");
    console.Log("audio", "play 'jump' con pitch 1.20");
    console.Log("col", "enemigo muerto: 3 restantes en oleada");
    console.Log("col", "player recibe 5 de daño");
    console.Update(0.0f);
    std::cout << "Consola:\n";
    for (const auto& line : console.Snapshot()) {
        std::cout << "  " << line << "\n";
    }
    console.SetFilter("col");
    console.Update(0.0f);
    console.Render(10, 100);
    std::cout << "Filtro 'col': " << console.Snapshot().size() << " líneas visibles\n";

    // Simular decaimiento del lifetime: pasadas 10s TODAS las líneas mueren
    console.Update(10.1f);
    std::cout << "Tras 10s de decay: " << console.Snapshot().size() << " líneas\n";

    // 5) Grid (activado a mano en builds de debug)
    dbg.cfg.showGrid = true;
    g_records.clear();
    dbg.Grid(640.0f, 320.0f, 64.0f);   // 10x5 celdas = 15 líneas
    std::cout << "Grid 640x320 c/64: " << RecordCount("grid") << " consulta registrada\n";

    std::cout << "\n=== FIN LECCIÓN 11 ===\n";
}

int main() {
    demo_debug_drawing();
    return 0;
}
