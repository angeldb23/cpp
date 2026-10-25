/* =============================================================================
   MÓDULO 19 - LECCIÓN 17: Optimización — SoA para math, SIMD sin forzar y
   fast-inverse-sqrt (y cuándo NO usarlo)
   =============================================================================
   TEMA 10 de TEMAS.md (1 lección): SoA para math, SIMD con GLM (no-forzar),
   fast_inverse_sqrt y cuándo no usarlo.

   == SOA PARA MATEMÁTICA: EL DATA-ORIENTED DEL MÓDULO 16 ====================

   El ejemplo estrella: transformar 100k posiciones por la MISMA matriz:

     struct Entidades {               //  (SoA: estructuras de arrays)
         std::vector<Vec3> posiciones;
         std::vector<Vec3> direcciones;
     };

   Los VEC3 están en ARREGLOS contiguos SEPARADOS: transformar las
   posiciones es UN loop sobre memoria lineal (lleno de caché y fácil de
   autovectorizar con SSE). Con AoS (vector<Entity> con pos+dir+vida+...) el
   loop trae campos de segunda mano del caché por cada entidad y el
   compilador tiene menos margen para vectorizar.

   La demo: MISMOS resultados SoA vs AoS (equivalencia) + estructura que sé
   que el módulo 16 ya estudió. Nota de diseño: Vec3 en SoA son 12 bytes
   (3*float): al subir a GPU (uniforms, módulo 20) la alineación IMPORTA:
   ahí se usa Vec4 alineado (float padding) para que el compilador sepa.

   == SIMD CON GLM: LA REGLA "NO FORZAR" =====================================

   GLM por default YA usa lo que el COMPILADOR le permite por los flags
   (-march=...): estas banderas definen GAPs p. ej. __SSE2__, __AVX2__.
   En x86-64 el SSE2 SIEMPRE está (es la base del ABI x86-64) — así que
   GLM hace lo justo y necesario: SIN you TOCAR NADA.

   REGLA DEL CURSO (la misma del módulo 15 y L02):
     - NUNCA distribuir binarios compilados con -march=native ni -mavx2:
       la CPU del dueño (Celeron N4020) NO TIENE AVX/AVX2 y el binario se
       muere con SIGILL al arrancar.
     - Si algún día recogemos AVX2 a mano (como la L11 del módulo 15),
       guardarlo con #if defined(__AVX2__) + fallback escalar SIEMPRE.
     - GLM_FORCE_PURE solo para comparar/debuguear (si un bug de matemática
       desaparece solo con ese define: compilo a mano en otro modo).

   == FAST INVERSE SQRT (y cuándo NO usarlo) =================================

   El famoso "0x5f3759df" de Quake III: computa 1/sqrt(x) con UNA división
   (una estimation + 2 iteraciones de Newton-Raphson). En 1993-2000 (Pentium
   III...) la división carga GRAN costo, así que valía la pena. HOY:

       - en x86 MODERNO (2004 en adelante): la FPU integra sqrtss/rsqrtss por
         llamada... 1/sqrt es O(1) ningun pipeline>: usar std::sqrt y 1.0f/x
         es MÁS RÁPIDO (el hardware lo INSTANCEA más rápido que la hack)
       - error de la approximation: ~0.1% máximo: durante el JUEGO (colisiones
         con los modules 23 de física) O CMÁX (shooting raycast) el error
         es invisible a la vista... pero ensucia las comparaciones exactas
         (y ensuciar la DETECCIÓN de bugs de matemática: más difícil de debug).
     - el hack solo se justifica en un algoritmo que NO SOBREVIVE de
       equipos antiguos (el caso Doom del módulo 27) O cargas MUY paralelas
       por GPU (los típicos rayos del 26).

   == DEMO =====================================================================

   1) SoA vs AoS: MISMOS resultados, típica estructura diferente.
   2) (opcional, con chrono del módulo 11) medir el tiempo de cada camino:
      en esta máquina los números se ven (no es test determinista: es medición).
   ============================================================================= */

#include "include/mate3d.hpp"
#include <chrono>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <vector>

using namespace mate;

// ==== PRIMITIVAS LOCALES ===================================================
float productoEscalar(Vec3 a, Vec3 b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}
Vec3 productoVectorial(Vec3 a, Vec3 b) {
    return { a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x };
}
float largo(Vec3 a) { return std::sqrt(productoEscalar(a, a)); }
Vec3 normalizar(Vec3 a) {
    float l = largo(a);
    if (l <= 1e-8f) return {0.0f, 0.0f, 0.0f};
    return a / l;
}
Mat4 matrizTraslacion(Vec3 t) {
    Mat4 m = Mat4::identidad();
    m.ponerColumna3(3, t, 1.0f);
    return m;
}
Mat4 matrizEscala(Vec3 s) {
    Mat4 m = Mat4::identidad();
    m.set(0, 0, s.x); m.set(1, 1, s.y); m.set(2, 2, s.z);
    return m;
}
Mat4 matrizRotacionZ(float rad) {
    float c = std::cos(rad), s = std::sin(rad);
    Mat4 m = Mat4::identidad();
    m.ponerColumna(0, { c, s, 0.0f });
    m.ponerColumna(1, {-s, c, 0.0f });
    return m;
}
// ===========================================================================

// ==== SOA ==================================================================
struct EntidadesSoA {
    std::vector<Vec3> posiciones;
    std::vector<Vec3> direcciones;
    void redimensionar(size_t n) {
        posiciones.resize(n);
        direcciones.resize(n);
    }
};

// Transforma TODAS las posiciones de golpe con la MISMA matriz (tight loop).
void transformarPosicionesSoA(EntidadesSoA& es, Mat4 const& m) {
    for (size_t i = 0; i < es.posiciones.size(); ++i)
        es.posiciones[i] = transformarPunto(m, es.posiciones[i]);
}

// ==== AOA (el "cómodo" de un objeto por cada entrada) ======================
struct EntidadAoS {
    Vec3 posicion;
    Vec3 direccion;
    float vida;          // campos que NO se usan en el loop de transformación
    int   etiqueta;
};

void transformarPosicionesAoS(std::vector<EntidadAoS>& es, Mat4 const& m) {
    for (EntidadAoS& e : es)
        e.posicion = transformarPunto(m, e.posicion);
}

// ==== DEMO =================================================================
int main() {
    const size_t N = 200000;
    std::cout << "== EQUIVALENCIA: SoA y AoS dan LOS MISMOS resultados ==\n";
    EntidadesSoA soa;
    soa.redimensionar(N);
    std::vector<EntidadAoS> aos(N);
    for (size_t i = 0; i < N; ++i) {
        Vec3 p{(float)i * 0.001f, (float)(i % 7), 1.0f};
        soa.posiciones[i] = p;
        aos[i].posicion = p;
        aos[i].vida = 1.0f;
        aos[i].etiqueta = (int)i;
    }
    Mat4 w = matrizTraslacion({1.0f, 2.0f, 3.0f}) * matrizRotacionZ(aRadianes(30.0f))
           * matrizEscala({2.0f, 2.0f, 2.0f});
    transformarPosicionesSoA(soa, w);
    transformarPosicionesAoS(aos, w);

    bool iguales = true;
    for (size_t i = 0; i < N; ++i)
        if (!casiIgualVec3(soa.posiciones[i], aos[i].posicion, 1e-5f))
            { iguales = false; break; }
    std::cout << "equipos: resultados idénticos? " << (iguales ? "SI" : "NO")
              << " (" << N << " entidades)\n";

    std::cout << "\n== PERFIL (no test determinista, mide a mano por curiosidad) ==\n";
    using reloj = std::chrono::steady_clock;      // (módulo 11)
    EntidadesSoA b; b.redimensionar(N);
    for (size_t i = 0; i < N; ++i) b.posiciones[i] = {(float)i, 1.0f, 2.0f};
    std::vector<EntidadAoS> a(N);
    for (size_t i = 0; i < N; ++i) a[i].posicion = {(float)i, 1.0f, 2.0f};

    auto t0 = reloj::now();
    transformarPosicionesSoA(b, w);
    auto t1 = reloj::now();
    transformarPosicionesAoS(a, w);
    auto t2 = reloj::now();
    auto ms = [](auto d) { return std::chrono::duration<double, std::milli>(d).count(); };
    std::printf("SoA: %.2f ms | AoS: %.2f ms (una corrida; los números varían)\n",
                ms(t1 - t0), ms(t2 - t1));

    std::cout << "\n== LAS REGLAS (resumen EVERGREEN) ==\n";
    std::cout << "1) Compilación: SIN -march=native, SIN -mavx2 (la CPU del\n"
                 "   dueño no tiene AVX; el default x86-64 es SSE2 seguro).\n"
                 "2) Si/problemas de aceleración a mano (módulo 15/L17): significa\n"
                 "   siempre un guard #if defined(__AVX2__) + fallback escalar.\n"
                 "3) Antes de optimizar MATEMÁTICA: PERFILAR (el bottleneck casi\n"
                 "   siempre es el CACHE o el draw count, no la sqrt).\n"
                 "4) std::sqrt > fast-inverse-sqrt en CPU moderna.\n";

    std::cout << "=== FIN LECCIÓN 17 ===\n";
    return 0;
}
