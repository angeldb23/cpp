/* =============================================================================
   MÓDULO 21 - LECCIÓN 02: Gamma correction y sRGB (parte 2 del tema 1)
   =============================================================================
   TEMA 1 de TEMAS.md (segunda de 2 lecciones): "gamma correction y sRGB".

   CÓMO CORRERLA A MANO:
     g++ -std=c++20 -Wall -Wextra leccion_02_gamma_srgb.cpp \
         $(pkg-config --cflags --libs sdl3) -lOpenGL -o lec02 && ./lec02
   Además IMPRIME una tabla de conversiones sRGB<->lineal: la usás para los
   tests del ejercicio 04.

   --- EL TEATRO FÍSICO ------------------------------------------------------
   El ojo percibe brillo de forma NO lineal: de "10% más claro" a "el doble
   de claro" hay muchos escalones. En los 80-90 los motores aprendieron que si
   guardan color LINEAL y lo mandan tal cual al monitor (que responde con una
   curva exponencial), las sombras se ven chapadas y las luces quemadas.
   Décadas después, TODA imagen "normal" (jpg, png) NO guarda color lineal:
   guarda color sRGB, codificado con una curva de potencia (~1/2.2).

   --- LA CURVA sRGB REAL (IEC 61966-2-1) ------------------------------------
   No es exactamente pow(c, 1/2.2): es una curva POR TRAMOS que deja el tramo
   oscuro lineal (evita que pow amplifique el ruido cerca del negro):
     sRGB(v):    v <= 0.04045    -> v / 12.92
                 v >  0.04045    -> 1.055 * pow(v, 1.0/2.4) - 0.055
     lineal(v):  L <= 0.0031308  -> 12.92 * L
                 L >  0.0031308  -> 1.055 * pow(L, 2.4) - 0.055
   Valores clave: sRGB(0.5) ~= 0.214 (¡NO la mitad!). La mitad sRGB guarda solo
   ~21% de la energía lineal. Esa asimetría es TODO el tema del doble gamma.

   --- EL BUG CLÁSICO: doble gamma -------------------------------------------
   Escena: textura (en sRGB en disco) + shading. Si mandás el byte del disco
   directo al shader:
     1) el shader recibe 0.5 cuando debiera trabajar con 0.214;
     2) la mezcla (ambient + diffuse en un vec3 sumando) ocurre en espacio
        sRGB, donde el promedio NO es el promedio físico: las luces de las
        sombras quedan lavadas (el look "flotando" de los juegos de los 2000
        viene de ahí). La cura son DOS cosas SIMULTÁNEAS:
     1) decodificar la textura A LINEAL al llegar al GPU:
        - GLSL a mano: pow(tex.rgb, 2.2), o la fórmula exacta por tramos;
        - O GL NATIVO: subir la textura con internalformat GL_SRGB8_ALPHA8
          (0x8C43, en glcorearb.h): el sampler devuelve lineal AUTOMÁTICO.
     2) RE-codificar a sRGB al FINAL, cuando el color viaja al monitor:
        - pow(color, 1.0/2.2) manual en el shader final, o bien
        - glEnable(GL_FRAMEBUFFER_SRGB) (0x8DB9) SOLO en el pase final: GL
          codifica la salida sin costo de shader (la vista en el módulo 20
          L11; Mesa lo soporta para contexto 3.3 core).
   Hacer una y saltar la otra produce sombras "under" o luces "over". Hacer
   las dos con la herramienta equivocada produce doble gamma (lava quemada).

   --- APROXIMACIÓN pow(2.2) vs FÓRMULA EXACTA -------------------------------
   pow(x, 2.2) se desvía de la curva real hasta ~6% en el tramo oscuro; con la
   fórmula exacta no hay motivo económico para ignorar el tramo. En el curso:
   la EXACTA se usa en tests y en la tabla impresa; la APROXIMADA se explica
   para que cuando leas "gamma 2.2" en un foro sepas qué aproximan.

   --- QUÉ HACE ESTA DEMO ----------------------------------------------------
   Un plano iluminado con la misma luz Phong de la lección 01, renderizado
   TRES VECES (tres tercios de pantalla) con el MISMO shader:
     IZQUIERDA: textura LINEAL al sampler (sin decodificar): look lavado.
     CENTRO   : textura GL_SRGB8_ALPHA8 (el GPU decodifica al samplear),
                PERO sin recodificar la salida: sombras oscuras y spec frío.
     DERECHA  : textura sRGB + pow(color, 1/2.2) final: la combinación BIEN.

   --- APIs usadas (verificadas contra glcorearb.h local) --------------------
   - GL_SRGB8_ALPHA8 (0x8C43) como internalformat en glTexImage2D.
   - glGetUniformLocation + f21::subirMat/subirVec (helpers de la 01/l21).
   Mismo patrón ejecutable de la lección 01: compila y corré a mano.
   ============================================================================
 */

#include "include/l21.hpp"

#include <iostream>

namespace {

/* Fórmula EXACTA (para la tabla impresa y el ejercicio 04). */
static float srgbALineal(float s) {
    return (s <= 0.04045f) ? s / 12.92f
                           : std::pow((s + 0.055f) / 1.055f, 2.4f);
}
static float linealAsRGB(float L) {
    return (L <= 0.0031308f) ? 12.92f * L
                             : 1.055f * std::pow(L, 1.0f / 2.4f) - 0.055f;
}
/* Aproximación pow(1/2.2) para MOSTRAR la desviación en la tabla. */
static float linealAsRGBAprox(float L) { return std::pow(L, 1.0f / 2.2f); }

const char* VS_FUENTE = R"GLSL(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUV;
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;
out vec3  vPos;
out vec3  vNormal;
out vec2  vUV;
void main() {
    vec4 mundo = uModel * vec4(aPos, 1.0);
    vPos = mundo.xyz;
    vNormal = mat3(uModel) * aNormal;
    vUV = aUV;
    gl_Position = uProj * uView * mundo;
}
)GLSL";

/* FRAGMENTO: la misma luz con DOS interruptores por uniform:
   uDecodifica = 1: el shader pega pow(tex, 2.2) manual (más "a mano").
   uCodifica   = 1: el shader recodifica a sRGB al final. */
const char* FS_MODO = R"GLSL(
#version 330 core
in vec3 vPos;
in vec3 vNormal;
in vec2 vUV;
out vec4 FragColor;

uniform vec3  uLuzPos;
uniform vec3  uPosCamara;
uniform float uDecodifica;
uniform float uCodifica;
uniform sampler2D uAlbedo;        // ¡DECLARAR EL SAMPLER! (si no, no enlaza)
const float GAMMA = 2.2;

void main() {
    vec3 N = normalize(vNormal);
    vec3 L = normalize(uLuzPos - vPos);
    vec3 V = normalize(uPosCamara - vPos);

    vec3 albedo = texture(uAlbedo, vUV).rgb;
    if (uDecodifica > 0.5) {
        // pow(2.2) aproxima sRGB->lineal (la EXACTA sería
        // ((c+0.055)/1.055)^2.4 por canal)
        albedo = pow(albedo, vec3(GAMMA));
    }

    // luz Phong idéntica a la lección 01: SOLO cambia el manejo de gamma
    vec3 color = albedo * 0.10;                       // ambient
    float dif = max(dot(N, L), 0.0);
    color += albedo * dif;                            // diffuse
    float spec = pow(max(dot(reflect(-L, N), V), 0.0), 32.0);
    color += vec3(0.35) * spec;                       // specular

    if (uCodifica > 0.5) {
        color = pow(color, vec3(1.0 / GAMMA));        // -> sRGB al monitor
    }
    FragColor = vec4(color, 1.0);
}
)GLSL";

}  // namespace

/* Sube el ajedrez como textura LINEAL (GL_RGBA8) o sRGB (GL_SRGB8_ALPHA8). */
static GLuint subirTextura(const std::vector<unsigned char>& pix, int w, int h,
                           bool esSRGB) {
    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, esSRGB ? GL_SRGB8_ALPHA8 : GL_RGBA8,
                 w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pix.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    return tex;
}

int main() {
    // La tabla: entender la corrección gamma es matemática ANTES de shader.
    std::cout << "sRGB <-> lineal (fórmula exacta IEC 61966-2-1)\n";
    float valores[] = {0.0f, 0.05f, 0.1f, 0.25f, 0.5f, 0.75f};
    for (float v : valores) {
        float L = srgbALineal(v);
        float s2 = linealAsRGB(L);
        std::cout << "  sRGB " << v << " -> lineal " << L
                  << " -> sRGB devuelto " << s2
                  << "  (ida y vuelta con error "
                  << std::fmax(std::fabs(s2 - v), 0.0f) << ")\n";
    }
    std::cout << "  lineal 0.5 -> sRGB " << linealAsRGB(0.5f)
              << "  (¡NO 0.5!)\n";
    std::cout << "  aprox pow(1/2.2): lineal 0.5 -> " << linealAsRGBAprox(0.5f)
              << "  (la aproximación, para comparar)\n\n";

    glu::Contexto c = glu::abrir("M21 L02: gamma/sRGB (3 columnas)", 900, 400);
    if (c.ventana == nullptr) return 1;
    glEnable(GL_DEPTH_TEST);

    GLuint prog = glu::enlazar(VS_FUENTE, FS_MODO);
    if (prog == 0) { glu::cerrar(c); return 1; }
    glu::Uniform uModel, uView, uProj, uLuzPos, uPosCamara, uDec, uCod;
    uModel.buscar(prog, "uModel");     uView.buscar(prog, "uView");
    uProj.buscar(prog, "uProj");       uLuzPos.buscar(prog, "uLuzPos");
    uPosCamara.buscar(prog, "uPosCamara");
    uDec.buscar(prog, "uDecodifica");  uCod.buscar(prog, "uCodifica");
    GLint albedoLugar = glGetUniformLocation(prog, "uAlbedo");
    glUniform1i(albedoLugar, 0);

    // Ajedrez con valores "como los del disco" (sRGB).
    constexpr int TX = 64, TY = 64;
    std::vector<unsigned char> pix(TX * TY * 4);
    for (int y = 0; y < TY; ++y)
        for (int x = 0; x < TX; ++x) {
            bool par = ((x / 8) + (y / 8)) % 2 == 0;
            float v = par ? 0.85f : 0.35f;
            std::size_t i = (std::size_t(y) * TX + std::size_t(x)) * 4;
            pix[i] = pix[i + 1] = pix[i + 2] =
                static_cast<unsigned char>(v * 255.0f + 0.5f);
            pix[i + 3] = 255;
        }
    GLuint texLinea = subirTextura(pix, TX, TY, false);
    GLuint texSRGB  = subirTextura(pix, TX, TY, true);

    f21::MallaGPU gPlano = f21::subirMalla(f21::cubo(1.0f));
    f21::Vec3 ojo{0.0f, 1.4f, 3.0f};
    f21::Mat4 vista = f21::lookAt(ojo, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f});
    f21::Mat4 proj = f21::perspectiva(55.0f,
        (float)c.ancho / (float)c.alto, 0.1f, 100.0f);
    f21::Vec3 luzPos{2.0f, 2.0f, 2.0f};

    while (glu::eventosOk()) {
        glClearColor(0.06f, 0.06f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(prog);
        f21::subirMat(uView.lugar, vista);
        f21::subirVec(uPosCamara.lugar, ojo);

        struct Modo { GLuint tex; float dec, cod; };
        Modo modos[3] = {
            {texLinea, 0.0f, 0.0f},   // izq: sRGB tratado como lineal
            {texSRGB,  0.0f, 1.0f},   // centro: sRGB input, sin salida
            {texSRGB,  1.0f, 1.0f},   // der: input lineal + salida sRGB
        };
        for (int i = 0; i < 3; ++i) {
            glViewport(c.ancho / 3 * i, 0, c.ancho / 3, c.alto);
            f21::subirMat(uModel.lugar,
                          f21::trs({0.0f, -0.1f, 0.0f}, 1.0f, 0.0f));
            f21::subirMat(uProj.lugar, proj);
            f21::subirVec(uLuzPos.lugar, luzPos);
            glBindTexture(GL_TEXTURE_2D, modos[i].tex);
            glUniform1f(uDec.lugar, modos[i].dec);
            glUniform1f(uCod.lugar, modos[i].cod);
            f21::dibujar(gPlano);
        }
        SDL_GL_SwapWindow(c.ventana);
    }

    glDeleteProgram(prog);
    glDeleteTextures(1, &texLinea);
    glDeleteTextures(1, &texSRGB);
    f21::borrarMalla(gPlano);
    glu::cerrar(c);
    std::cout << "Listo: arriba quedó la tabla sRGB<->lineal; en pantalla "
                 "comparaste los 3 modos de manejo gamma.\n";
    return 0;
}
