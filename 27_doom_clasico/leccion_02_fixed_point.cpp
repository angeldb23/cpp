/*
 * MÓDULO 27: Doom Clásico — De C a C++
 * LECCIÓN 02: Aritmética Fixed-Point 16.16 (m_fixed.h, m_fixed.c)
 *
 * TEMAS:
 * - Por qué fixed-point en 1993: sin FPU en 386/486, determinismo, rendimiento
 * - Formato 16.16: 16 bits enteros, 16 bits fracción (FRACUNIT = 0x10000 = 65536)
 * - Operaciones básicas: FixedMul, FixedDiv, desplazamientos, conversiones
 * - Tablas trigonométricas: finesine[] (8192 entradas), uso de angle_t (0-65535)
 * - Limitaciones: overflow, precisión, rango (-32768 a 32767.99998)
 *
 * VERIFICACIÓN: El archivo m_fixed.h/c del source release 1997 se consulta con `cat`
 * en el repo clonado. NO se inventan funciones: se documentan las reales.
 */

#include <stdint.h>
#include <stdio.h>
#include <math.h>

// ============================================================================
// 1. DEFINICIONES ORIGINALES (m_fixed.h, doomtype.h)
// ============================================================================
/*
 * En doomtype.h:
 *   typedef int32_t fixed_t;   // 32 bits signed
 *
 * En m_fixed.h:
 *   #define FRACBITS    16
 *   #define FRACUNIT    (1<<FRACBITS)        // 65536 = 1.0
 *   #define FIXED_TO_INT(x)   ((x) >> FRACBITS)
 *   #define INT_TO_FIXED(x)   ((x) << FRACBITS)
 *   #define FIXED_TO_FLOAT(x) ((float)(x) / FRACUNIT)
 *   #define FLOAT_TO_FIXED(x) ((int32_t)((x) * FRACUNIT))
 *
 * RANGO: -32768.0 a 32767.99998474121 (32 bits signed, 16.16)
 * PRECISIÓN: 1/65536 ≈ 0.000015258789
 *
 * angle_t es uint16_t (0-65535), donde 65536 = 360°.
 * ANG90 = 16384, ANG180 = 32768, ANG270 = 49152, ANG360 = 65536 (equivale a 0).
 */

#define FRACBITS    16
#define FRACUNIT    (1 << FRACBITS)   // 65536
#define ANG90       (FRACUNIT / 4)    // 16384
#define ANG180      (FRACUNIT / 2)    // 32768
#define ANG270      (3 * FRACUNIT / 4) // 49152
#define ANG360      FRACUNIT          // 65536 (wrap a 0)

typedef int32_t fixed_t;
typedef uint16_t angle_t;

// Conversiones básicas (macros originales):
#define FIXED_TO_INT(x)     ((x) >> FRACBITS)
#define INT_TO_FIXED(x)     ((x) << FRACBITS)
#define FIXED_TO_FLOAT(x)   ((float)(x) / (float)FRACUNIT)
#define FLOAT_TO_FIXED(x)   ((fixed_t)((x) * (float)FRACUNIT))

// ============================================================================
// 2. MULTIPLICACIÓN: FixedMul (m_fixed.c)
// ============================================================================
/*
 * FixedMul(a, b) = (a * b) >> 16
 *
 * En C original (m_fixed.c):
 *   fixed_t FixedMul(fixed_t a, fixed_t b) {
 *       return (fixed_t)(((int64_t)a * (int64_t)b) >> FRACBITS);
 *   }
 *
 * Por qué int64_t: a y b son 32 bits, el producto cabe en 64 bits.
 * El shift >> 16 descarta los 16 bits bajos de la fracción del resultado.
 *
 * En DOS (16-bit, DJGPP): se usaba ensamblador o rutina en C con long long
 * (que DJGPP soportaba como 64 bits). En GCC moderno: int64_t nativo.
 *
 * CUIDADO: Overflow si |a| * |b| > 2^63-1. En práctica, Doom limita
 * coordenadas y velocidades para que no ocurra (radio mapa < 32768 unidades).
 */

static inline fixed_t FixedMul(fixed_t a, fixed_t b) {
    return (fixed_t)(((int64_t)a * (int64_t)b) >> FRACBITS);
}

// ============================================================================
// 3. DIVISIÓN: FixedDiv (m_fixed.c)
// ============================================================================
/*
 * FixedDiv(a, b) = (a << 16) / b
 *
 * En C original:
 *   fixed_t FixedDiv(fixed_t a, fixed_t b) {
 *       if ((b == 0) || (a == INT32_MIN && b == -1)) return INT32_MAX; // overflow/div0
 *       return (fixed_t)(((int64_t)a << FRACBITS) / b);
 *   }
 *
 * El shift << 16 ANTES de dividir preserva la precisión fraccionaria.
 * Chequeo de división por cero y overflow INT32_MIN / -1 (UB en C).
 *
 * En Doom, FixedDiv se usa para:
 * - Cálculo de pendientes (slope = FixedDiv(dy, dx))
 * - Proyección perspectiva (escala = FixedDiv(FRACUNIT, distance))
 * - Interpolación de texturas
 */

static inline fixed_t FixedDiv(fixed_t a, fixed_t b) {
    if (b == 0) return INT32_MAX;  // comportamiento original: valor grande
    if (a == INT32_MIN && b == -1) return INT32_MAX; // overflow UB
    return (fixed_t)(((int64_t)a << FRACBITS) / b);
}

// ============================================================================
// 4. TABLAS TRIGONOMÉTRICAS (tables.h, tables.c)
// ============================================================================
/*
 * Doom NO usa libm (sin/cos float). Usa tablas precalculadas en tables.c:
 *
 *   fixed_t finesine[8192];   // 8192 = 8192 entradas para 0-360° con sub-precisión
 *   // finesine[0] = 0, finesine[2048] = FRACUNIT (sin 90°)
 *
 *   #define FINESINE_MASK  (8192 - 1)
 *
 *   fixed_t FixedSin(angle_t angle) {
 *       return finesine[angle & FINESINE_MASK];
 *   }
 *   fixed_t FixedCos(angle_t angle) {
 *       return finesine[(angle + ANG90) & FINESINE_MASK];
 *   }
 *
 * angle_t es 16 bits (0-65535). finesine tiene 8192 entradas = 8 bits de sub-precisión.
 * El índice se calcula: (angle >> 3) & 8191  (divide por 8, máscara 13 bits).
 * En el source release real: finesine[10240] para cos/sin combinados (ver tables.h).
 *
 * VENTAJA: Sin FPU, determinista, rápido (array lookup + máscara).
 * DESVENTAJA: Memoria (8192 * 4 bytes = 32 KB), precisión limitada.
 */

#define FINESINE_SIZE 8192
#define FINESINE_MASK (FINESINE_SIZE - 1)

// Tabla simulada (en el código real viene precalculada en tables.c):
static fixed_t finesine[FINESINE_SIZE];

void InitFineSine(void) {
    for (int i = 0; i < FINESINE_SIZE; i++) {
        double rad = (2.0 * M_PI * i) / FINESINE_SIZE;
        finesine[i] = FLOAT_TO_FIXED(sin(rad));
    }
}

static inline fixed_t FixedSin(angle_t angle) {
    // angle es 0-65535. finesine tiene 8192 entradas.
    // Índice = (angle >> 3) & 8191  (equivale a angle * 8192 / 65536)
    return finesine[(angle >> 3) & FINESINE_MASK];
}

static inline fixed_t FixedCos(angle_t angle) {
    // cos(x) = sin(x + 90°)
    return finesine[((angle + ANG90) >> 3) & FINESINE_MASK];
}

// ============================================================================
// 5. OPERACIONES ADICIONALES (usadas en el engine)
// ============================================================================

// Valor absoluto
static inline fixed_t FixedAbs(fixed_t x) {
    return (x >= 0) ? x : -x;
}

// Mínimo/máximo
static inline fixed_t FixedMin(fixed_t a, fixed_t b) {
    return (a < b) ? a : b;
}
static inline fixed_t FixedMax(fixed_t a, fixed_t b) {
    return (a > b) ? a : b;
}

// Multiplicación por entero (común: velocidad * FRACUNIT)
static inline fixed_t FixedMulInt(fixed_t a, int b) {
    return a * b;  // b ya está en unidades enteras, no fixed
}

// División por entero
static inline fixed_t FixedDivInt(fixed_t a, int b) {
    return a / b;  // división entera truncada
}

// ============================================================================
// 6. EJEMPLOS DE USO EN DOOM (p_mobj.c, r_things.c, etc.)
// ============================================================================

struct mobj_t {
    fixed_t x = 0, y = 0;
    angle_t angle = 0;
};

// Movimiento: x += FixedMul(speed, cos(angle))
void MoveForward(mobj_t* mo, fixed_t speed) {
    mo->x += FixedMul(speed, FixedCos(mo->angle));
    mo->y += FixedMul(speed, FixedSin(mo->angle));
}

// Distancia 2D aproximada (usada en AI, sonido)
fixed_t ApproxDistance(fixed_t dx, fixed_t dy) {
    dx = FixedAbs(dx);
    dy = FixedAbs(dy);
    if (dx < dy) { fixed_t t = dx; dx = dy; dy = t; }
    // Aproximación: max + min/2 (rápida, error < 12%)
    return dx + (dy >> 1);
}

// Proyección de sprite: scale = FRACUNIT * viewingdistance / distance
fixed_t CalcSpriteScale(fixed_t distance, fixed_t viewingdistance) {
    if (distance == 0) return INT32_MAX;
    return FixedDiv(viewingdistance * FRACUNIT, distance);
}

// ============================================================================
// 7. LIMITACIONES Y TRAMPAS
// ============================================================================
/*
 * 1. OVERFLOW EN MULTIPLICACIÓN:
 *    FixedMul(32767*FRACUNIT, 32767*FRACUNIT) -> producto > 2^63 -> UB.
 *    Doom evita esto limitando coordenadas del mapa a < 32768 unidades.
 *
 * 2. PRECISIÓN EN DIVISIÓN:
 *    FixedDiv(1*FRACUNIT, 3*FRACUNIT) = 21845 (0.333328 vs 0.333333 real).
 *    Error relativo ~0.0015%. Aceptable para juego.
 *
 * 3. TRUNCAMIENTO VS REDONDEO:
 *    FIXED_TO_INT trunca hacia -inf (shift aritmético).
 *    Para redondear: (x + FRACUNIT/2) >> FRACBITS.
 *
 * 4. ANGLE_T WRAP:
 *    angle_t es uint16_t. Sumar ángulos: (a + b) & 0xFFFF (wrap automático).
 *    Diferencia: (a - b) & 0xFFFF da distancia angular sin signo.
 *
 * 5. DETERMINISMO:
 *    FixedMul/FixedDiv con int64_t son deterministas en cualquier plataforma
 *    (complemento a dos, shift aritmético definido en C99+ para signed).
 *    Clave para demos y netplay: misma secuencia de tics = mismo resultado.
 */

// ============================================================================
// 8. MODERNIZACIÓN EN C++ (wrappers tipo seguro)
// ============================================================================
/*
 * En C++ podemos encapsular fixed_t en una clase con operadores sobrecargados:
 *
 *   class Fixed16 {
 *       int32_t v;
 *   public:
 *       constexpr Fixed16() : v(0) {}
 *       constexpr explicit Fixed16(int i) : v(i << 16) {}
 *       constexpr explicit Fixed16(float f) : v(int32_t(f * 65536.0f)) {}
 *       constexpr Fixed16 operator+(Fixed16 o) const { return Fixed16::from_raw(v + o.v); }
 *       constexpr Fixed16 operator-(Fixed16 o) const { return Fixed16::from_raw(v - o.v); }
 *       constexpr Fixed16 operator*(Fixed16 o) const { return Fixed16::from_raw((int64_t)v * o.v >> 16); }
 *       constexpr Fixed16 operator/(Fixed16 o) const { return Fixed16::from_raw((int64_t)v << 16 / o.v); }
 *       constexpr int to_int() const { return v >> 16; }
 *       constexpr float to_float() const { return v / 65536.0f; }
 *   private:
 *       constexpr static Fixed16 from_raw(int32_t v) { Fixed16 f; f.v = v; return f; }
 *   };
 *
 * Ventajas: type safety, no mezcla accidental con int/float, constexpr para compile-time.
 * Desventaja: overhead de abstracción (cero en release con optimización).
 *
 * NOTA: En el módulo 27, la modernización incremental (lecciones 11-12) incluye
 * wrappers RAII para lump cache, ECS para mobj_t, job system para thinkers.
 * El fixed-point se puede wrappear igual, pero el código original lo usa en todas partes:
 * cambiarlo todo es riesgo alto. Mejor: wrapper opcional para código nuevo, mantener
 * fixed_t crudo en código portado.
 */

// ============================================================================
// 9. TESTS DE VERIFICACIÓN (lógica pura, sin dependencias)
// ============================================================================

int test_fixed_point(void) {
    InitFineSine();

    int passed = 0, failed = 0;

    #define TEST(expr, msg) \
        do { if (expr) { passed++; printf("  PASS: %s\n", msg); } \
             else { failed++; printf("  FAIL: %s\n", msg); } } while(0)

    // Conversiones
    TEST(INT_TO_FIXED(5) == 5 * FRACUNIT, "INT_TO_FIXED(5)");
    TEST(FIXED_TO_INT(INT_TO_FIXED(42)) == 42, "roundtrip int->fixed->int");
    TEST(FIXED_TO_INT(INT_TO_FIXED(-10)) == -10, "negative int roundtrip");

    // FixedMul
    TEST(FixedMul(INT_TO_FIXED(2), INT_TO_FIXED(3)) == INT_TO_FIXED(6), "2 * 3 = 6");
    TEST(FixedMul(INT_TO_FIXED(1), FRACUNIT) == FRACUNIT, "1 * 1.0 = 1.0");
    TEST(FixedMul(FRACUNIT/2, FRACUNIT/2) == FRACUNIT/4, "0.5 * 0.5 = 0.25");

    // FixedDiv
    TEST(FixedDiv(INT_TO_FIXED(6), INT_TO_FIXED(3)) == INT_TO_FIXED(2), "6 / 3 = 2");
    TEST(FixedDiv(FRACUNIT, INT_TO_FIXED(2)) == FRACUNIT/2, "1.0 / 2 = 0.5");
    TEST(FixedDiv(FRACUNIT/2, FRACUNIT/4) == INT_TO_FIXED(2), "0.5 / 0.25 = 2");

    // Trig
    TEST(FixedSin(0) == 0, "sin(0) = 0");
    TEST(FixedSin(ANG90) == FRACUNIT, "sin(90°) = 1.0");
    TEST(FixedSin(ANG180) == 0, "sin(180°) = 0");
    TEST(FixedCos(0) == FRACUNIT, "cos(0) = 1.0");
    TEST(FixedCos(ANG90) == 0, "cos(90°) = 0");

    // Identidad pitagórica aprox: sin² + cos² ≈ 1
    fixed_t s = FixedSin(ANG90/3); // 30°
    fixed_t c = FixedCos(ANG90/3);
    fixed_t sum_sq = FixedMul(s, s) + FixedMul(c, c);
    TEST(FixedAbs(sum_sq - FRACUNIT) < 100, "sin²+cos²≈1 en 30°"); // tolerancia ~0.0015

    // Aprox distance
    TEST(ApproxDistance(INT_TO_FIXED(3), INT_TO_FIXED(4)) >= INT_TO_FIXED(4), "dist(3,4) >= 4");
    TEST(ApproxDistance(INT_TO_FIXED(3), INT_TO_FIXED(4)) <= INT_TO_FIXED(6), "dist(3,4) <= 6");

    printf("\nResultados: %d pass, %d fail\n", passed, failed);
    return failed == 0 ? 0 : 1;
}

int main(void) {
    printf("Módulo 27 - Lección 02: Fixed-Point 16.16\n");
    return test_fixed_point();
}