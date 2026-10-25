/* =============================================================================
   MÓDULO 17 - LECCIÓN 06: Colisiones 2D Simples — AABB, Círculo, Point-in-Rect
   =============================================================================
   TEMA 6 de TEMAS.md (1ra lección): Colisiones 2D simples

   CONCEPTOS CLAVE:
   - Point-in-Rect: el test más barato, 4 comparaciones (o 2 si el punto está fuera)
   - AABB (Axis-Aligned Bounding Box): overlap por separación de ejes
   - Círculo vs círculo: comparar la distancia contra la suma de radios
   - Círculo vs AABB: el punto del rect más cercano al círculo (clamp)
   - Penetración (MTV): cuánto y hacia dónde corregir el overlap
   - Por qué empezar con AABB/círculo: son exactos, baratos y sin math (no hace
     falta SAT ni proyecciones: eso es materia del módulo 23)

   MUERTE FILOSÓFICA DEL "if (x1 < x2 && ...)":
   Todo colisión se puede escribir como un test de SEPARACIÓN en cada eje.
   AABB A y B se tocan si NO hay un eje que las separe:
     A.max.x >= B.min.x  &&  B.max.x >= A.min.x   (eje X)
     A.max.y >= B.min.y  &&  B.max.y >= A.min.y   (eje Y)
   Si alguno falla -> hay hueco -> no colisión. Es más barato salir temprano
   (early exit) que calcular todo.

   NOTA: Código compila sin raylib (stubs). Para probar: linkear -lraylib.
   ============================================================================= */

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>

// -----------------------------------------------------------------------------
// Stubs raylib
// -----------------------------------------------------------------------------
struct Vector2 { float x, y; };
struct Rectangle { float x, y, width, height; };
struct Color { unsigned char r, g, b, a; };

inline void DrawRectangleRec(Rectangle, Color) {}
inline void DrawRectangleLinesEx(Rectangle, float, Color) {}
inline void DrawCircle(int, int, float, Color) {}
inline void DrawCircleV(Vector2, float, Color) {}
inline void DrawLine(int, int, int, int, Color) {}
inline void DrawText(const char*, int, int, int, Color) {}

// -----------------------------------------------------------------------------
// 1. Point-in-Rect: el test más fundamental
// -----------------------------------------------------------------------------
/* MATEMÁTICA:
   Un rectángulo en raylib es {x, y, width, height} donde
   (x, y) es la esquina SUPERIOR IZQUIERDA.
   El punto (p.x, p.y) está dentro si:
     x <= p.x <= x + width       y       y <= p.y <= y + height

   CONVENCIÓN IMPORTANTE (fronteras inclusivas vs exclusivas):
   - En coordenadas continuas (floats), tocar el borde cuenta como colisión
     (usamos <= y >= para que el "toco por un pixel" no se escape).
   - Para raycasts finos se usa la exclusiva; para física de juego, la inclusiva
     evita que dos rects que comparten cara pasen "a través". */

bool PointsInRect(const Rectangle& r, Vector2 p) {
    return p.x >= r.x && p.x <= r.x + r.width &&
           p.y >= r.y && p.y <= r.y + r.height;
}

// -----------------------------------------------------------------------------
// 2. AABB vs AABB: separación por ejes
// -----------------------------------------------------------------------------
/* Overlap de AABBs. Dos variantes equivalentes:

   Variante 1 (por esquinas, estilo raylib):
     r1.x + r1.width  >= r2.x        y        r2.x + r2.width >= r1.x
     r1.y + r1.height >= r2.y        y        r2.y + r2.height >= r1.y

   Variante 2 (por puntos extremos):
     A.max >= B.min && B.max >= A.min (en cada eje)

   Son idénticas; solo cambia la forma de registrarse. La variante 2 es la que
   se usa en motores reales porque el AABB se guarda como {min, max} y el
   análisis de colisiones (SAT del módulo 23) es LA MISMA segunda parte:
   busqué EJES que separen. Con un AABB ya hay solo 2 ejes; con OBBs (rects
   rotados) son 4, con polígonos uno por cara. */

inline float RectMinX(const Rectangle& r) { return r.x; }
inline float RectMaxX(const Rectangle& r) { return r.x + r.width; }
inline float RectMinY(const Rectangle& r) { return r.y; }
inline float RectMaxY(const Rectangle& r) { return r.y + r.height; }

bool RectsOverlap(const Rectangle& a, const Rectangle& b) {
    // El min del eje debe estar ANTES del max del otro y viceversa
    if (RectMaxX(a) < RectMinX(b) || RectMaxX(b) < RectMinX(a)) return false;  // eje X separa
    if (RectMaxY(a) < RectMinY(b) || RectMaxY(b) < RectMinY(a)) return false;  // eje Y separa
    return true;
}

// Penetración (Minimum Translation Vector en AABB): cuánto corregir por eje.
// Devuelve el vector más CORTO para separar los dos rects.
Vector2 GetPenetration(const Rectangle& a, const Rectangle& b) {
    // Cantidad de solapamiento en cada eje
    float overlapX = std::min(RectMaxX(a), RectMaxX(b)) - std::max(RectMinX(a), RectMinX(b));
    float overlapY = std::min(RectMaxY(a), RectMaxY(b)) - std::max(RectMinY(a), RectMinY(b));

    // ¿Empujar hacia la izquierda o derecha (arriba o abajo)?
    // Empujamos del LADO del centro de A: si A tiene su centro a la izquierda
    // del centro de B, corregimos hacia la izquierda.
    float acx = RectMinX(a) + a.width * 0.5f;
    float bcx = RectMinX(b) + b.width * 0.5f;
    float acy = RectMinY(a) + a.height * 0.5f;
    float bcy = RectMinY(b) + b.height * 0.5f;

    Vector2 mtv{};
    if (overlapX <= overlapY) {
        mtv.x = (acx < bcx) ? -overlapX : overlapX;
        mtv.y = 0.0f;
    } else {
        mtv.x = 0.0f;
        mtv.y = (acy < bcy) ? -overlapY : overlapY;
    }
    return mtv;
}

// -----------------------------------------------------------------------------
// 3. Círculo vs círculo: solo radio y distancia
// -----------------------------------------------------------------------------
struct Circle {
    Vector2 center;
    float radius;
};

/* El test más práctico NUNCA compara distancias exactas (sqrt es caro):
   Pythagoras: sumo radios al cuadrado y comparo contra la distancia al cuadrado.
     colisión <=> dist(c1, c2) <= r1 + r2
                <=> dist²(c1, c2) <= (r1 + r2)²
   Ahorro: ni siquiera se computa sqrt. La magnitud del orden de magnitud no
   importa: si dist² <= suma², dist <= suma (ambas positivas). */

bool CirclesOverlap(const Circle& a, const Circle& b) {
    float dx = b.center.x - a.center.x;
    float dy = b.center.y - a.center.y;
    float rSum = a.radius + b.radius;
    return dx*dx + dy*dy <= rSum * rSum;
}

// Penetración radial: cuánto penetra y en qué dirección separar
Vector2 GetCirclePenetration(const Circle& a, const Circle& b) {
    float dx = b.center.x - a.center.x;
    float dy = b.center.y - a.center.y;
    float dist = std::sqrt(dx*dx + dy*dy);
    float rSum = a.radius + b.radius;

    if (dist < 0.001f) {
        // Círculos en el mismo lugar: salida arbitraria (derecha)
        return {-rSum, 0.0f};
    }

    float push = rSum - dist;              // cuánto SOBRA
    // Normaliza y escala: empuja B alejándose de A (o A alejándose de B, es lo mismo al revés)
    return Vector2{ dx / dist * push, dy / dist * push };
}

// -----------------------------------------------------------------------------
// 4. Círculo vs AABB: closest point (punto del rect más cercano)
// -----------------------------------------------------------------------------
/* IDEA: si el círculo toca al rect, el punto del rect MÁS CERCANO al centro
   del círculo está a una distancia menor al radio. Como estamos en AABB, ese
   punto se obtiene "clampeando" (acotando) el centro del círculo al rect:

   closest.x = clamp(circle.center.x, rect.minX, rect.maxX)
   closest.y = clamp(circle.center.y, rect.minY, rect.maxY)

   Luego es el test point-in-circle entre el centro y closest.

   Analogía mental: imagine el rect como una caja; el punto más
   cercano de su contorno al centro del círculo es la proyección del centro
   sobre la caja. Si el centro YA está dentro del rect, clamp
   devuelve el propio centro y la distancia es 0 -> colisión directa. */

Vector2 ClosestPointOnRect(const Rectangle& r, Vector2 p) {
    return Vector2{
        std::clamp(p.x, RectMinX(r), RectMaxX(r)),
        std::clamp(p.y, RectMinY(r), RectMaxY(r))
    };
}

bool RectCircleOverlap(const Rectangle& r, const Circle& c) {
    Vector2 closest = ClosestPointOnRect(r, c.center);
    float dx = c.center.x - closest.x;
    float dy = c.center.y - closest.y;
    return dx*dx + dy*dy <= c.radius * c.radius;  // sqrt evitado
}

// Penetración círculo-rect: empuja el círculo por la normal del punto closest
Vector2 GetRectCirclePenetration(const Rectangle& r, const Circle& c) {
    Vector2 closest = ClosestPointOnRect(r, c.center);
    float dx = c.center.x - closest.x;
    float dy = c.center.y - closest.y;
    float distSq = dx*dx + dy*dy;

    if (distSq < 0.001f) {
        // Centro dentro del rect: dirección hacia el borde más cercano
        // (aquí simplificamos: salimos hacia la izquierda; el Módulo 23 lo
        // extiende con las 4 facicie del AABB)
        float left   = c.center.x - RectMinX(r);
        float right  = RectMaxX(r) - c.center.x;
        float top    = c.center.y - RectMinY(r);
        float bottom = RectMaxY(r) - c.center.y;
        float m = std::min({left, right, top, bottom});
        if (m == left)   return {-(left + c.radius), 0};
        if (m == right)  return { (right + c.radius), 0};
        if (m == top)    return {0, -(top + c.radius)};
        return {0, (bottom + c.radius)};
    }

    float dist = std::sqrt(distSq);
    float push = c.radius - dist;
    return { dx / dist * push, dy / dist * push };
}

// -----------------------------------------------------------------------------
// 5. Helpers de utilidad general (spatial queries del juego)
// -----------------------------------------------------------------------------
bool PointInCircle(const Circle& c, Vector2 p) {
    float dx = c.center.x - p.x;
    float dy = c.center.y - p.y;
    return dx*dx + dy*dy <= c.radius * c.radius;
}

// Distancia² entre dos puntos (helper general: se usa MUCHO en juegos)
inline float DistSq(Vector2 a, Vector2 b) {
    float dx = b.x - a.x, dy = b.y - a.y;
    return dx*dx + dy*dy;
}

// Sección "transversal": AABB de transformación (wrapper de un objeto)
// Cada entidad del juego tiene, internamente, una "AABB de colisión" que
// deriva de su posición (translate) y, a veces, de su tamaño.
Rectangle GetAABBFromCenter(Vector2 center, float w, float h) {
    return Rectangle{ center.x - w * 0.5f, center.y - h * 0.5f, w, h };
}

// -----------------------------------------------------------------------------
// Demo
// -----------------------------------------------------------------------------
void demo_colisiones() {
    std::cout << "=== Colisiones 2D Simples ===\n\n";

    // Point-in-Rect
    Rectangle wall{100, 100, 50, 50};
    std::cout << "Point({120,130}) in rect: " << (PointsInRect(wall, {120, 130}) ? "SI" : "NO") << "\n";
    std::cout << "Point({155,130}) in rect: " << (PointsInRect(wall, {155, 130}) ? "SI" : "NO") << "\n";

    // AABB vs AABB
    Rectangle player{120, 120, 30, 30};
    Rectangle ennemi {135, 133, 30, 30};
    std::cout << "AABB vs AABB: " << (RectsOverlap(player, ennemi) ? "TOCAN" : "separados") << "\n";

    // Penetración
    Vector2 mtv = GetPenetration(player, ennemi);
    std::cout << "Penetración (MTV): (" << mtv.x << ", " << mtv.y << ")\n";

    // Círculo vs círculo
    Circle a{{0, 0}, 10}, b{{15, 0}, 6}, c{{30, 0}, 6};
    std::cout << "Círculos (r=10, r=6) a 15: " << (CirclesOverlap(a, b) ? "TOCAN" : "separados") << "\n";
    std::cout << "Círculos (r=10, r=6) a 30: " << (CirclesOverlap(a, c) ? "TOCAN" : "separados") << "\n";

    // Penetración radial
    Vector2 cp = GetCirclePenetration(a, b);
    std::cout << "Penetración círculo: (" << cp.x << ", " << cp.y << ")\n";

    // Círculo vs AABB
    Rectangle box{200, 200, 40, 40};
    Circle d1{{240, 220}, 5};   // centro en el borde derecho: toca
    Circle d2{{250, 220}, 5};   // a 10 px del borde: hueco de 5 -> NO alcanza
    Circle d3{{260, 220}, 5};   // aún más lejos: separados
    std::cout << "Circle(240,220,r5) vs box: " << (RectCircleOverlap(box, d1) ? "TOCAN" : "separados") << "\n";
    std::cout << "Circle(250,220,r5) vs box: " << (RectCircleOverlap(box, d2) ? "TOCAN" : "separados") << "\n";
    std::cout << "Circle(260,220,r5) vs box: " << (RectCircleOverlap(box, d3) ? "TOCAN" : "separados") << "\n";

    std::cout << "\n=== FIN LECCIÓN 06 ===\n";
}

int main() {
    demo_colisiones();
    return 0;
}
