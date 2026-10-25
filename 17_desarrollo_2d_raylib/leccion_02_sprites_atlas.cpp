/* =============================================================================
   MÓDULO 17 - LECCIÓN 02: Sprites — Atlas, Sub-rects, Origen, Escalado, Rotación, Tint
   =============================================================================
   TEMA 2 de TEMAS.md: Sprites

   CONCEPTOS CLAVE:
   - Texture Atlas: una textura grande con múltiples sprites (reduce draw calls y state changes)
   - Source Rectangle (sub-rect): región del atlas a dibujar (x, y, width, height)
   - Origin: punto de pivote para rotación/escalado (0,0 = esquina sup-izq por defecto)
   - Scale: factor de escalado (1.0 = tamaño original)
   - Rotation: ángulo en grados (raylib usa grados, no radianes)
   - Tint: color de multiplicación (WHITE = sin tint)
   - Draw Order / Layers: painter's algorithm (se dibuja en orden, último arriba)

   FUNCIONES RAYLIB RELEVANTES (verificar en docs de raylib 5.5):
   - DrawTexturePro(texture, source, dest, origin, rotation, tint)
   - DrawTextureRec(texture, source, position, tint)  // simplificado
   - DrawTextureV(texture, position, tint)             // sin source rect
   - Rectangle { float x, y, width, height; }
   - Vector2 { float x, y; }

   NOTA: El código compila con -fsyntax-only o compila sin ejecutar (raylib no disponible en CI).
   Para probar a mano: compilar con -lraylib y ejecutar.
   ============================================================================= */

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>

// -----------------------------------------------------------------------------
// Stubs de raylib para compilación sin la librería
// -----------------------------------------------------------------------------
struct Vector2 { float x, y; };
struct Rectangle { float x, y, width, height; };
struct Color { unsigned char r, g, b, a; };
struct Texture2D { int id; int width; int height; int mipmaps; int format; };

const Color WHITE = {255, 255, 255, 255};
const Color RED   = {255, 0, 0, 255};
const Color BLUE  = {0, 0, 255, 255};

inline Texture2D LoadTexture(const char*) { return Texture2D{1, 512, 512, 1, 0}; }
inline void UnloadTexture(Texture2D) {}
inline void DrawTexturePro(Texture2D, Rectangle, Rectangle, Vector2, float, Color) {}
inline void DrawTextureRec(Texture2D, Rectangle, Vector2, Color) {}
inline void DrawTextureV(Texture2D, Vector2, Color) {}
inline void DrawRectangleLinesEx(Rectangle, float, Color) {}

// -----------------------------------------------------------------------------
// 1. Sprite básico: textura + source rect + configuración de dibujo
// -----------------------------------------------------------------------------
struct Sprite {
    Texture2D texture;      // Textura (atlas)
    Rectangle source;       // Sub-rectángulo en el atlas
    Vector2 origin;         // Pivote (relativo al source: 0..1 o píxeles)
    Vector2 scale;          // Factor de escalado
    float rotation;         // Grados
    Color tint;             // Color de multiplicación
    int layer;              // Capa de dibujo (menor = detrás)

    Sprite() : texture{0,0,0,0,0}, source{0,0,0,0}, origin{0,0}, scale{1,1}, rotation(0), tint(WHITE), layer(0) {}

    // Constructor desde atlas con coordenadas de frame
    Sprite(Texture2D tex, float srcX, float srcY, float srcW, float srcH)
        : texture(tex), source{srcX, srcY, srcW, srcH}, origin{0,0}, scale{1,1}, rotation(0), tint(WHITE), layer(0) {}

    // Dibuja en posición mundial (esquina sup-izq del dest rect)
    void Draw(Vector2 position) const {
        Rectangle dest = { position.x, position.y, source.width * scale.x, source.height * scale.y };
        DrawTexturePro(texture, source, dest, origin, rotation, tint);
    }

    // Dibuja centrado en posición (origin = centro del sprite)
    void DrawCentered(Vector2 center) const {
        Rectangle dest = {
            center.x - source.width * scale.x * 0.5f,
            center.y - source.height * scale.y * 0.5f,
            source.width * scale.x,
            source.height * scale.y
        };
        Vector2 centeredOrigin = { source.width * 0.5f, source.height * 0.5f };
        DrawTexturePro(texture, source, dest, centeredOrigin, rotation, tint);
    }

    // Dibuja con pivote personalizado (en píxeles relativos al source)
    void DrawWithOrigin(Vector2 position, Vector2 customOrigin) const {
        Rectangle dest = { position.x, position.y, source.width * scale.x, source.height * scale.y };
        DrawTexturePro(texture, source, dest, customOrigin, rotation, tint);
    }
};

// -----------------------------------------------------------------------------
// 2. Atlas de texturas: grid uniforme de frames
// -----------------------------------------------------------------------------
struct TextureAtlas {
    Texture2D texture;
    int frameWidth;
    int frameHeight;
    int cols;
    int rows;
    int padding;      // Espacio entre frames
    int margin;       // Margen alrededor del grid

    TextureAtlas() : texture{0,0,0,0,0}, frameWidth(0), frameHeight(0), cols(0), rows(0), padding(0), margin(0) {}

    TextureAtlas(Texture2D tex, int fw, int fh, int c, int r, int pad = 0, int mar = 0)
        : texture(tex), frameWidth(fw), frameHeight(fh), cols(c), rows(r), padding(pad), margin(mar) {}

    // Obtiene source rect para un frame índice (row-major: fila * cols + col)
    Rectangle GetFrame(int index) const {
        if (index < 0 || index >= cols * rows) return {0,0,0,0};
        int col = index % cols;
        int row = index / cols;
        return Rectangle{
            float(margin + col * (frameWidth + padding)),
            float(margin + row * (frameHeight + padding)),
            float(frameWidth),
            float(frameHeight)
        };
    }

    // Obtiene source rect por fila y columna
    Rectangle GetFrame(int row, int col) const {
        if (row < 0 || row >= rows || col < 0 || col >= cols) return {0,0,0,0};
        return Rectangle{
            float(margin + col * (frameWidth + padding)),
            float(margin + row * (frameHeight + padding)),
            float(frameWidth),
            float(frameHeight)
        };
    }

    int TotalFrames() const { return cols * rows; }
};

// -----------------------------------------------------------------------------
// 3. Sprite animado simple (para lección 03 se expande)
// -----------------------------------------------------------------------------
struct AnimatedSprite {
    TextureAtlas atlas;
    int currentFrame = 0;
    float frameTimer = 0.0f;
    float frameDuration = 0.1f;  // 10 FPS por defecto
    bool looping = true;
    int startFrame = 0;
    int endFrame = 0;  // 0 = último frame del atlas

    AnimatedSprite() = default;
    AnimatedSprite(TextureAtlas a) : atlas(a), endFrame(a.TotalFrames() - 1) {}

    void Update(float dt) {
        frameTimer += dt;
        if (frameTimer >= frameDuration) {
            frameTimer -= frameDuration;
            currentFrame++;
            if (currentFrame > endFrame) {
                if (looping) currentFrame = startFrame;
                else currentFrame = endFrame;
            }
        }
    }

    void Draw(Vector2 position) const {
        Rectangle src = atlas.GetFrame(currentFrame);
        Rectangle dest = { position.x, position.y,
                          float(atlas.frameWidth), float(atlas.frameHeight) };
        DrawTexturePro(atlas.texture, src, dest, {0,0}, 0, WHITE);
    }

    void DrawScaled(Vector2 position, Vector2 scale) const {
        Rectangle src = atlas.GetFrame(currentFrame);
        Rectangle dest = { position.x, position.y,
                          float(atlas.frameWidth) * scale.x, float(atlas.frameHeight) * scale.y };
        DrawTexturePro(atlas.texture, src, dest, {0,0}, 0, WHITE);
    }
};

// -----------------------------------------------------------------------------
// 4. Sistema de capas (layers) para draw order
// -----------------------------------------------------------------------------
struct LayeredSprite {
    Sprite sprite;
    int layer;  // Menor = detrás, mayor = delante

    LayeredSprite() : layer(0) {}
    LayeredSprite(Sprite s, int l) : sprite(s), layer(l) {}
};

// Comparador para ordenar por layer
inline bool LayerCompare(const LayeredSprite& a, const LayeredSprite& b) {
    return a.layer < b.layer;
}

// Draw all sprites sorted by layer (painter's algorithm)
void DrawLayeredSprites(std::vector<LayeredSprite>& sprites) {
    std::sort(sprites.begin(), sprites.end(), LayerCompare);
    for (auto& ls : sprites) {
        ls.sprite.Draw({0, 0});  // posición simplificada
    }
}

// -----------------------------------------------------------------------------
// 5. Tint y efectos de color
// -----------------------------------------------------------------------------
/*
 * TINT EN RAYLIB:
 * - Color multiplicativo: pixel_final = pixel_textura * tint / 255
 * - WHITE (255,255,255,255) = sin cambio
 * - RED (255,0,0,255) = solo canal rojo
 * - Alpha < 255 = transparencia
 *
 * USOS COMUNES:
 * - Flash daño: tint = RED por 0.1s
 * - Fade in/out: alpha 0->255 o 255->0
 * - Equipo/color swap: tint = color del equipo
 * - Noche/visión nocturna: tint azulado
 * - Silueta: tint = BLACK con alpha
 */

Color LerpColor(Color a, Color b, float t) {
    return Color{
        (unsigned char)(a.r + (b.r - a.r) * t),
        (unsigned char)(a.g + (b.g - a.g) * t),
        (unsigned char)(a.b + (b.b - a.b) * t),
        (unsigned char)(a.a + (b.a - a.a) * t)
    };
}

// -----------------------------------------------------------------------------
// 6. Debug drawing para sprites
// -----------------------------------------------------------------------------
void DrawSpriteDebug(const Sprite& sprite, Vector2 position) {
    Rectangle dest = { position.x, position.y, sprite.source.width * sprite.scale.x, sprite.source.height * sprite.scale.y };
    DrawRectangleLinesEx(dest, 1.0f, RED);

    // Dibuja origin
    Vector2 originWorld = { position.x + sprite.origin.x * sprite.scale.x,
                           position.y + sprite.origin.y * sprite.scale.y };
    // En raylib real acá iría: DrawCircleV(originWorld, 3.0f, BLUE);
    (void)originWorld;  // stub: no hay DrawCircleV disponible en este archivo
}

// -----------------------------------------------------------------------------
// Demo
// -----------------------------------------------------------------------------
void demo_sprites() {
    std::cout << "=== Sprites: Atlas, Sub-rects, Origen, Escalado, Rotación, Tint ===\n\n";

    // Simula carga de atlas
    Texture2D atlasTex = LoadTexture("assets/atlas.png");
    TextureAtlas atlas(atlasTex, 32, 32, 8, 8, 1, 0);  // 32x32 frames, 8x8 grid, 1px padding

    // Sprite estático desde frame 5 (fila 0, col 5)
    Sprite player(atlasTex, atlas.GetFrame(0, 5).x, atlas.GetFrame(0, 5).y, 32, 32);
    player.origin = {16, 16};  // Centro
    player.scale = {2.0f, 2.0f};  // 2x
    player.rotation = 45.0f;  // 45 grados
    player.tint = WHITE;

    // Sprite animado
    AnimatedSprite anim(atlas);
    anim.frameDuration = 0.1f;  // 10 FPS
    anim.startFrame = 0;
    anim.endFrame = 3;  // 4 frames de caminata

    // Simula update
    for (int i = 0; i < 5; ++i) {
        anim.Update(0.016f);  // ~60 FPS
        std::cout << "Frame: " << anim.currentFrame << "\n";
    }

    // Layers
    std::vector<LayeredSprite> layers;
    layers.emplace_back(Sprite{atlasTex, 0,0,32,32}, 0);  // Background
    layers.emplace_back(Sprite{atlasTex, 32,0,32,32}, 10); // Player
    layers.emplace_back(Sprite{atlasTex, 64,0,32,32}, 20); // UI

    std::cout << "Layers ordenados: ";
    std::sort(layers.begin(), layers.end(), LayerCompare);
    for (auto& l : layers) std::cout << l.layer << " ";
    std::cout << "\n\n";

    UnloadTexture(atlasTex);
}

int main() {
    demo_sprites();
    std::cout << "=== FIN LECCIÓN 02 ===\n";
    return 0;
}