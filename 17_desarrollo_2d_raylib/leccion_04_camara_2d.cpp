/* =============================================================================
   MÓDULO 17 - LECCIÓN 04: Cámaras 2D — World-to-Screen, Zoom, Seguimiento Suave, Límites
   =============================================================================
   TEMA 4 de TEMAS.md: Cámaras 2D

   CONCEPTOS CLAVE:
   - Camera2D de raylib: offset, target, rotation, zoom
   - World-to-Screen: GetWorldToScreen2D / GetScreenToWorld2D
   - Zoom: escalado alrededor del offset (centro de pantalla por defecto)
   - Seguimiento suave (lerp): cámara interpola hacia target
   - Límites de mundo (clamping): cámara no sale del nivel
   - Shake: offset temporal aleatorio para impacto

   FUNCIONES RAYLIB (verificar en docs 5.5):
   - BeginMode2D(Camera2D) / EndMode2D()
   - GetWorldToScreen2D(Vector2, Camera2D)
   - GetScreenToWorld2D(Vector2, Camera2D)
   - Camera2D { Vector2 offset, target; float rotation, zoom; }

   NOTA: Código compila con stubs. Para probar: -lraylib.
   ============================================================================= */

#include <iostream>
#include <cmath>
#include <algorithm>

// -----------------------------------------------------------------------------
// Stubs raylib
// -----------------------------------------------------------------------------
struct Vector2 { float x, y; };
struct Rectangle { float x, y, width, height; };
struct Camera2D { Vector2 offset; Vector2 target; float rotation; float zoom; };

inline Vector2 GetWorldToScreen2D(Vector2 world, Camera2D cam) {
    // Simplificado: world * zoom - target * zoom + offset
    return Vector2{
        (world.x - cam.target.x) * cam.zoom + cam.offset.x,
        (world.y - cam.target.y) * cam.zoom + cam.offset.y
    };
}

inline Vector2 GetScreenToWorld2D(Vector2 screen, Camera2D cam) {
    return Vector2{
        (screen.x - cam.offset.x) / cam.zoom + cam.target.x,
        (screen.y - cam.offset.y) / cam.zoom + cam.target.y
    };
}

inline void BeginMode2D(Camera2D) {}
inline void EndMode2D() {}

// -----------------------------------------------------------------------------
// 1. Helpers matemáticos
// -----------------------------------------------------------------------------
inline Vector2 Lerp(Vector2 a, Vector2 b, float t) {
    return { a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t };
}

inline Vector2 Clamp(Vector2 v, Vector2 min, Vector2 max) {
    return { std::clamp(v.x, min.x, max.x), std::clamp(v.y, min.y, max.y) };
}

inline float LerpFloat(float a, float b, float t) { return a + (b - a) * t; }

// -----------------------------------------------------------------------------
// 2. Camera2D Wrapper con funcionalidad extendida
// -----------------------------------------------------------------------------
class Camera2DController {
public:
    Camera2D camera;
    Vector2 worldBoundsMin = {0, 0};
    Vector2 worldBoundsMax = {1920, 1080};
    bool boundsEnabled = false;

    // Seguimiento suave
    float followSpeed = 10.0f;      // Velocidad de lerp (mayor = más rápido)
    float followDeadzone = 0.0f;    // Zona muerta alrededor del target

    // Zoom
    float zoomMin = 0.25f;
    float zoomMax = 4.0f;
    float zoomSpeed = 2.0f;         // Velocidad de zoom (lerp)
    float targetZoom = 1.0f;

    // Shake
    float shakeIntensity = 0.0f;
    float shakeDuration = 0.0f;
    float shakeTimer = 0.0f;

    Camera2DController(int screenWidth, int screenHeight) {
        camera.offset = { float(screenWidth) * 0.5f, float(screenHeight) * 0.5f };
        camera.target = { 0, 0 };
        camera.rotation = 0.0f;
        camera.zoom = 1.0f;
        targetZoom = 1.0f;
    }

    // Configura límites del mundo
    void SetWorldBounds(Rectangle bounds) {
        worldBoundsMin = { bounds.x, bounds.y };
        worldBoundsMax = { bounds.x + bounds.width, bounds.y + bounds.height };
        boundsEnabled = true;
    }

    void SetWorldBounds(float x, float y, float w, float h) {
        SetWorldBounds({x, y, w, h});
    }

    void DisableBounds() { boundsEnabled = false; }

    // Target a seguir (jugador)
    void SetTarget(Vector2 targetPos) {
        camera.target = targetPos;
    }

    // Seguimiento instantáneo (sin lerp)
    void SnapToTarget() {
        camera.target = ClampTarget(camera.target);
    }

    // Update: lerp hacia target, zoom, shake, clamping
    void Update(float dt, Vector2 targetPos) {
        // 1. Suavizado de target (lerp)
        if (followSpeed > 0) {
            Vector2 desired = targetPos;

            // Deadzone: solo mueve si target está fuera de zona muerta
            Vector2 diff = { desired.x - camera.target.x, desired.y - camera.target.y };
            float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);

            if (dist > followDeadzone) {
                float t = 1.0f - std::exp(-followSpeed * dt);  // Frame-rate independent lerp
                camera.target = Lerp(camera.target, desired, t);
            }
        } else {
            camera.target = targetPos;
        }

        // 2. Clamping a límites del mundo
        if (boundsEnabled) {
            camera.target = ClampTarget(camera.target);
        }

        // 3. Zoom lerp
        if (std::abs(camera.zoom - targetZoom) > 0.001f) {
            float t = 1.0f - std::exp(-zoomSpeed * dt);
            camera.zoom = LerpFloat(camera.zoom, targetZoom, t);
            camera.zoom = std::clamp(camera.zoom, zoomMin, zoomMax);
            // Re-clamp tras zoom (visible area cambia)
            if (boundsEnabled) camera.target = ClampTarget(camera.target);
        }

        // 4. Shake decay
        if (shakeTimer > 0) {
            shakeTimer -= dt;
            float shakeProgress = 1.0f - shakeTimer / shakeDuration;
            float currentIntensity = shakeIntensity * (1.0f - shakeProgress * shakeProgress); // ease out
            camera.offset.x += (rand() / (float)RAND_MAX * 2 - 1) * currentIntensity;
            camera.offset.y += (rand() / (float)RAND_MAX * 2 - 1) * currentIntensity;
        } else {
            shakeIntensity = 0;
            // Restaurar offset base (centro de pantalla)
            // En implementación real guardaríamos baseOffset
        }
    }

    // Clampa target para que cámara no salga del mundo
    Vector2 ClampTarget(Vector2 target) const {
        if (!boundsEnabled) return target;

        // Calcular half-screen en world coords
        float halfW = camera.offset.x / camera.zoom;
        float halfH = camera.offset.y / camera.zoom;

        float minX = worldBoundsMin.x + halfW;
        float maxX = worldBoundsMax.x - halfW;
        float minY = worldBoundsMin.y + halfH;
        float maxY = worldBoundsMax.y - halfH;

        // Si mundo es más pequeño que pantalla, centrar
        if (minX > maxX) minX = maxX = (worldBoundsMin.x + worldBoundsMax.x) * 0.5f;
        if (minY > maxY) minY = maxY = (worldBoundsMin.y + worldBoundsMax.y) * 0.5f;

        return Clamp(target, {minX, minY}, {maxX, maxY});
    }

    // Zoom hacia/desde punto de pantalla (mouse)
    void ZoomAtScreenPoint(float delta, Vector2 screenPoint) {
        // Convertir punto de pantalla a mundo ANTES del zoom
        Vector2 worldBefore = GetScreenToWorld2D(screenPoint, camera);
        (void)worldBefore;   // didáctico: se usará en la fórmula completa de zoom-al-punto

        targetZoom = std::clamp(targetZoom + delta * zoomSpeed * 0.1f, zoomMin, zoomMax);

        // Convertir MISMO punto de mundo a pantalla DESPUÉS del zoom
        // Ajustar offset para que el punto de mundo quede bajo el mismo pixel de pantalla
        // Fórmula: offset_new = screenPoint - (worldBefore - target) * zoom_new
        // Simplificado: ajustar target para compensar
        // (En raylib real, cambiar zoom alrededor de offset requiere recalcular target)

        // Método práctico: zoom alrededor del centro (offset), luego ajustar target
        // para que worldBefore siga en screenPoint
        // Esto es complejo; ver implementación completa abajo
    }

    // Zoom simple (alrededor del centro)
    void SetZoom(float zoom) {
        targetZoom = std::clamp(zoom, zoomMin, zoomMax);
    }

    void ZoomRelative(float factor) {
        SetZoom(targetZoom * factor);
    }

    // Shake
    void Shake(float intensity, float duration) {
        shakeIntensity = intensity;
        shakeDuration = duration;
        shakeTimer = duration;
    }

    // Coordinate conversion
    Vector2 WorldToScreen(Vector2 world) const {
        return GetWorldToScreen2D(world, camera);
    }

    Vector2 ScreenToWorld(Vector2 screen) const {
        return GetScreenToWorld2D(screen, camera);
    }

    // Visible world rectangle
    Rectangle GetVisibleWorldRect() const {
        Vector2 topLeft = ScreenToWorld({0, 0});
        Vector2 bottomRight = ScreenToWorld({camera.offset.x * 2, camera.offset.y * 2});
        return { topLeft.x, topLeft.y, bottomRight.x - topLeft.x, bottomRight.y - topLeft.y };
    }

    // Verifica si un rectángulo mundo está visible (culling)
    bool IsRectVisible(Rectangle worldRect) const {
        Rectangle view = GetVisibleWorldRect();
        return !(worldRect.x + worldRect.width < view.x ||
                 worldRect.x > view.x + view.width ||
                 worldRect.y + worldRect.height < view.y ||
                 worldRect.y > view.y + view.height);
    }
};

// -----------------------------------------------------------------------------
// 3. Zoom at mouse point (implementación correcta)
// -----------------------------------------------------------------------------
/*
 * ZOOM AT MOUSE:
 * Queremos que el punto de mundo bajo el mouse ANTES del zoom
 * siga bajo el mouse DESPUÉS del zoom.
 *
 * worldBefore = ScreenToWorld(mousePos, camera_old)
 * camera.zoom = newZoom
 * worldAfter = ScreenToWorld(mousePos, camera_new)
 *
 * Queremos worldAfter == worldBefore
 * => mousePos = WorldToScreen(worldBefore, camera_new)
 * => camera_new.offset = mousePos - (worldBefore - camera_new.target) * camera_new.zoom
 *
 * Si mantenemos target fijo y solo cambiamos zoom + offset:
 *   newOffset = mousePos - (worldBefore - target) * newZoom
 */

void ZoomAtPoint(Camera2D& cam, float newZoom, Vector2 screenPoint) {
    Vector2 worldBefore = GetScreenToWorld2D(screenPoint, cam);
    cam.zoom = newZoom;
    cam.offset = {
        screenPoint.x - (worldBefore.x - cam.target.x) * cam.zoom,
        screenPoint.y - (worldBefore.y - cam.target.y) * cam.zoom
    };
}

// -----------------------------------------------------------------------------
// 4. Camera Shake patterns
// -----------------------------------------------------------------------------
enum class ShakePattern {
    Linear,      // Decay lineal
    Quadratic,   // Ease out quadratic
    Sine,        // Ondulación senoidal
    Trauma       // Trauma-based (Squirrel Eiserloh)
};

struct CameraShake {
    float trauma = 0.0f;           // 0..1
    float traumaDecay = 1.0f;      // Por segundo
    float maxOffset = 20.0f;       // Píxeles máximos
    float maxRotation = 0.1f;      // Radianes máximos
    float frequency = 25.0f;       // Hz
    float time = 0.0f;

    void AddTrauma(float amount) { trauma = std::min(1.0f, trauma + amount); }

    void Update(float dt, Camera2D& cam) {
        trauma = std::max(0.0f, trauma - traumaDecay * dt);
        time += dt;

        float shake = trauma * trauma * trauma;  // Cubic para feel natural
        float offsetX = std::sin(time * frequency * 1.0f) * shake * maxOffset;
        float offsetY = std::sin(time * frequency * 1.31f) * shake * maxOffset; // freq distinta
        float rot = std::sin(time * frequency * 0.77f) * shake * maxRotation;

        cam.offset.x += offsetX;
        cam.offset.y += offsetY;
        cam.rotation += rot;
    }
};

// -----------------------------------------------------------------------------
// 5. Múltiples cámaras (split screen, minimapa, UI)
// -----------------------------------------------------------------------------
struct UICamera {
    Camera2D camera;
    UICamera(int w, int h) { (void)w; (void)h; camera.offset = {0,0}; camera.target = {0,0}; camera.zoom = 1; camera.rotation = 0; }
    // UI camera no sigue a nada, zoom fijo, no rotación
};

void demo_camera() {
    std::cout << "=== Camera 2D Demo ===\n\n";

    Camera2DController cam(1280, 720);
    cam.SetWorldBounds(0, 0, 5000, 3000);  // Mundo grande

    // Simula jugador moviéndose
    Vector2 playerPos = { 100, 100 };

    for (int i = 0; i < 10; ++i) {
        // Jugador se mueve
        playerPos.x += 50;
        playerPos.y += 30;

        cam.Update(0.016f, playerPos);

        std::cout << "Frame " << i
                  << " Player: (" << playerPos.x << ", " << playerPos.y << ")"
                  << " CamTarget: (" << cam.camera.target.x << ", " << cam.camera.target.y << ")"
                  << " Zoom: " << cam.camera.zoom << "\n";
    }

    // Test zoom at point
    cam.SetZoom(2.0f);
    cam.Update(0.016f, playerPos);
    std::cout << "\nAfter zoom 2x: CamTarget=(" << cam.camera.target.x << ", " << cam.camera.target.y << ")\n";

    // Test shake
    cam.Shake(10.0f, 0.5f);
    for (int i = 0; i < 5; ++i) {
        cam.Update(0.016f, playerPos);
        std::cout << "Shake " << i << ": offset=(" << cam.camera.offset.x << ", " << cam.camera.offset.y << ")\n";
    }

    // Visible rect
    Rectangle vis = cam.GetVisibleWorldRect();
    std::cout << "\nVisible world: (" << vis.x << ", " << vis.y << ") " << vis.width << "x" << vis.height << "\n";

    // Culling test
    Rectangle entity = { playerPos.x - 16, playerPos.y - 16, 32, 32 };
    std::cout << "Entity visible: " << (cam.IsRectVisible(entity) ? "YES" : "NO") << "\n";

    // Trauma shake
    std::cout << "\n--- Trauma Shake ---\n";
    Camera2D cam2 = { {640,360}, {0,0}, 0, 1 };
    CameraShake trauma;
    trauma.AddTrauma(0.5f);
    for (int i = 0; i < 5; ++i) {
        trauma.Update(0.016f, cam2);
        std::cout << "Trauma: " << trauma.trauma << " rot=" << cam2.rotation << "\n";
    }

    std::cout << "\n=== FIN LECCIÓN 04 ===\n";
}

int main() {
    demo_camera();
    return 0;
}