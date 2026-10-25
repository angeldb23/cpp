/* =============================================================================
   MÓDULO 21 - LECCIÓN 10: PCF, bias refinado y cascaded shadow maps (intro)
   =============================================================================
   TEMA 5 de TEMAS.md (segunda de 2 lecciones). La 09 armó el canal completo
   de sombras (depth map + compare sampler + bias slope). Esta lección ata el
   último nudo de calidad:

    1) PCF (Percentage-Closer Filtering): UN sample de shadow map devuelve
       bordes duros de escalones (cada texel del mapa es un "todo o nada").
       Promediar VARIAS muestras alrededor del punto suaviza el borde de
       la sombra. En GL 3.3 lo hacemos a mano: un loop 3x3 de
       texture(sampler2DShadow, ...) con offset por tamaño de texel.
       (Las variantes fijas textureOffset() y el sampler PCF "gratis" con
       hardware llegan en GL posteriores: acá, loop a mano, sí o sí.)
    2) Bias refinado: el punto clave de la 09 sigue en pie
       (bias = max(0.05*(1-dot(N,L)), 0.005)) pero ahora lo ajustamos CON
       TECLAS en vivo: mirar el shadow acne con bias insuficiente y el
       peter-panning con bias excesivo. El equilibrado es un dial, no una
       fórmula mágica.
    3) CSM intro (CONCEPTUAL, sin código de cascadas): una sola mapa
       ortográfica tiene un dilema: caja grande => pocos texels por metro
       cerca de la cámara (sombras borrosas y con acne); caja chica => el
       suelo lejano queda afuera. La idea de Cascaded Shadow Maps: partir
       el frustum de la vista en 2-4 "cascadas" a distintas distancias, y
       renderizar UN mapa más fino por cada una; al iluminar, cada fragmento
       elige su cascada por su distancia a la cámara. Todos los AAA de mundo
       abierto viven de esto. Acá solo lo charlamos (la demo imprime el
       desglose de texels/metro para ver por qué hacen falta cascadas).

   --- PCF EN PRÁCTICA -------------------------------------------------------
       float texel = 1.0 / float(TAM_MAPA);      // tamaño de un texel en UV
       float suma = 0.0;
       for (int x = -1; x <= 1; ++x)
       for (int y = -1; y <= 1; ++y)
           suma += texture(uMapaSombra, vec3(coord.xy + vec2(x, y) * texel,
                                             coord.z - bias));
       float sombra = suma / 9.0;               // kernel 3x3 => 9 muestras
   Nota: la vec3 entra al compare sampler con (uv, ref): cada uno de los 9
   samples YA SE COMPARA contra el depth guardado (compare mode de la 09).

   --- APIs (verificadas en glcorearb.h local) -------------------------------
   - GL_TEXTURE_COMPARE_MODE (0x884C), GL_TEXTURE_COMPARE_FUNC (0x884D),
     GL_COMPARE_REF_TO_TEXTURE (0x884E), GL_LEQUAL: mismo setup de la 09.
   - GL_DEPTH_COMPONENT32F (0x8CAC), GL_CLAMP_TO_BORDER (0x812D).
   - glDrawBuffer(GL_NONE) + glCheckFramebufferStatus GL_FRAMEBUFFER_COMPLETE
     (0x8CD5): el FBO depth-only del pase de luz.

   CÓMO CORRERLA A MANO:
     g++ -std=c++20 -Wall -Wextra leccion_10_pcf_bias_cascadas.cpp \
         $(pkg-config --cflags --libs sdl3) -lOpenGL -o lec10 && ./lec10

   DEMO: misma escena piso + cubo de la 09. Teclas:
     [P] alterna PCF 3x3 on/off (amanecer de bordes duros => suaves)
     [J]/[K] suben/bajan el multiplicador de bias (mirar el piso de cerca
        para el acne; el contacto cubo-piso para el peter-panning)
     [C] imprime el análisis texels/metro de la caja única (por qué cascadas)
     [FLECHAS] mueven la luz para ver la sombra deslizarse.
   ============================================================================ */

#include "include/l21.hpp"

#include <iostream>

namespace {

/* Pase de luz: VS que ubica geometría en el espacio de la luz. */
const char* VS_LUZ = R"GLSL(
#version 330 core
layout (location = 0) in vec3 aPos;
uniform mat4 uMatLuz;
uniform mat4 uModel;
void main() { gl_Position = uMatLuz * uModel * vec4(aPos, 1.0); }
)GLSL";

/* Pase de luz: fragment mínimo (el FBO no liga color: glDrawBuffer(GL_NONE),
   pero el lienzo exige una etapa fragment para enlazar el programa). */
const char* FS_LUZ_VACIO = R"GLSL(
#version 330 core
void main() {}
)GLSL";

/* Pase de vista: además de la posición mundial y la normal, lleva la
   coordenada del punto VISTA POR LA LUZ (la matLuz del pase 1). */
const char* VS_VISTA = R"GLSL(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
uniform mat4 uModel;
uniform mat4 uMatVista;
uniform mat4 uMatLuz;
out vec3 vPos;
out vec3 vNormal;
out vec4 vPosLuz;
void main() {
    vec4 mundo = uModel * vec4(aPos, 1.0);
    vPos = mundo.xyz;
    vNormal = mat3(uModel) * aNormal;
    vPosLuz = uMatLuz * mundo;
    gl_Position = uMatVista * mundo;
}
)GLSL";

/* Pase de vista: PCF 3x3 + bias de slope ajustable. */
const char* FS_SOMBRA_PCF = R"GLSL(
#version 330 core
in vec3 vPos;
in vec3 vNormal;
in vec4 vPosLuz;
out vec4 FragColor;

uniform sampler2DShadow uMapaSombra;
uniform vec3  uDirLuz;
uniform vec3  uColorLuz;
uniform vec3  uPosCamara;
uniform float uMultipBias;
uniform int   uPcfActivo;
uniform float uTexel;        // 1.0 / tamaño del mapa, en UV

float factorSombra() {
    // 1) NDC [-1,1] -> UV + depth [0,1] (igual que la 09)
    vec3 coord = vPosLuz.xyz / vPosLuz.w;
    coord = coord * 0.5 + 0.5;
    if (coord.z > 1.0) return 1.0;

    // 2) bias de slope-scale: cara rasante al rayo pide más bias
    vec3 N = normalize(vNormal);
    vec3 L = normalize(-uDirLuz);
    float bias = max(0.05 * (1.0 - dot(N, L)), 0.005) * uMultipBias;

    // 3) MUESTREO: sin PCF un sample seco (borde duro); con PCF kernel 3x3
    //    de compares suaviza la frontera de la sombra.
    float ref = coord.z - bias;
    if (uPcfActivo == 0) {
        return texture(uMapaSombra, vec3(coord.xy, ref));
    }
    float suma = 0.0;
    for (int x = -1; x <= 1; ++x)
    for (int y = -1; y <= 1; ++y)
        suma += texture(uMapaSombra, vec3(coord.xy + vec2(x, y) * uTexel, ref));
    return suma / 9.0;
}

void main() {
    vec3 N = normalize(vNormal);
    vec3 L = normalize(-uDirLuz);
    vec3 V = normalize(uPosCamara - vPos);

    float sombra = factorSombra();
    vec3 albedo = vec3(0.75, 0.75, 0.78);
    vec3 color = albedo * 0.10;                                  // ambient
    color += albedo * max(dot(N, L), 0.0) * uColorLuz * sombra;  // difusa
    vec3 reflejo = reflect(-L, N);
    color += vec3(0.2) * pow(max(dot(reflejo, V), 0.0), 24.0)
           * uColorLuz * sombra;                                 // spec
    FragColor = vec4(color, 1.0);
}
)GLSL";

/* La textura de depth de la luz, con compare mode LEQUAL (setup de la 09). */
void subirDepthTexture(GLuint& tex, int w, int h) {
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, w, h, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borde[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borde);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE,
                    GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
}

/* Análisis conceptual de CSM: cuántos texels por metro le toca a la caja
   única vs a una cascada de mitad de caja (misma resolución). */
void reporteCascadas(int tamMapa, float halfCaja) {
    std::cout << "\n== CSM intro (conceptual): texels por metro ==\n";
    float porMetroCaja = (float)tamMapa / (2.0f * halfCaja);
    std::cout << "  caja única " << halfCaja << "m -> " << porMetroCaja
              << " texels/m en TODO el suelo\n";
    float halfNear = halfCaja * 0.25f;
    float porMetroCasc = (float)tamMapa / (2.0f * halfNear);
    std::cout << "  cascada cercana " << halfNear << "m -> " << porMetroCasc
              << " texels/m: 4x de detalle justamente donde se ve\n";
    std::cout << "  (cada fragmento elige cascada por distancia a la cámara)\n\n";
}

}  // namespace

int main() {
    constexpr int TAM_MAPA = 1024;
    std::cout << "M21 L10: PCF " << 3 << "x" << 3 << " (tecla P), bias con "
              << "J/K, cascadas con C. Mapa " << TAM_MAPA << "x" << TAM_MAPA
              << "\n";

    glu::Contexto c = glu::abrir("M21 L10: PCF + bias + CSM intro", 800, 550);
    if (c.ventana == nullptr) return 1;
    glEnable(GL_DEPTH_TEST);

    GLuint progLuz   = glu::enlazar(VS_LUZ, FS_LUZ_VACIO);
    GLuint progVista = glu::enlazar(VS_VISTA, FS_SOMBRA_PCF);
    if (progLuz == 0 || progVista == 0) { glu::cerrar(c); return 1; }

    glu::Uniform uMatLuzL, uModelL, uModelV, uMatVista, uMatLuzV;
    uMatLuzL.buscar(progLuz, "uMatLuz");
    uModelL.buscar(progLuz, "uModel");
    uModelV.buscar(progVista, "uModel");
    uMatVista.buscar(progVista, "uMatVista");
    uMatLuzV.buscar(progVista, "uMatLuz");

    f21::Vec3 ojo{0.0f, 2.4f, 5.0f};
    f21::Mat4 vistaCam = f21::lookAt(ojo, {0.0f, 0.0f, 0.0f},
                                     {0.0f, 1.0f, 0.0f});
    f21::Mat4 projCam = f21::perspectiva(55.0f,
        (float)c.ancho / (float)c.alto, 0.1f, 100.0f);

    /* La caja de la luz: ortográfica de half 6x6, near 0.5, far 25. */
    f21::Vec3 posLuz{3.0f, 5.0f, 2.0f};
    constexpr float HALF_CJA = 6.0f;
    f21::Mat4 projLuz = f21::orto(HALF_CJA, HALF_CJA, 0.5f, 25.0f);
    f21::Mat4 matLuz = f21::Mat4::identidad();

    f21::MallaGPU cubo = f21::subirMalla(f21::cubo(0.6f));
    f21::MallaGPU piso = f21::subirMalla(f21::cubo(1.0f));
    f21::Mat4 chapa = f21::Mat4::identidad();
    chapa.set(0, 0, 6.0f); chapa.set(1, 1, 0.1f); chapa.set(2, 2, 6.0f);
    f21::Mat4 modeloPiso = f21::trs({0.0f, -0.6f, 0.0f}, 1.0f, 0.0f) * chapa;
    f21::Mat4 modeloCubo = f21::trs({0.0f, 0.4f, 0.0f}, 1.0f, 0.0f);
    f21::Vec3 dirLuz = f21::normalizar(f21::Vec3{-3.0f, -5.0f, -2.0f});

    /* Depth texture + FBO depth-only del pase de luz. */
    GLuint texSombra = 0;
    subirDepthTexture(texSombra, TAM_MAPA, TAM_MAPA);
    GLuint fbo = 0;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                           GL_TEXTURE_2D, texSombra, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "FBO de sombras incompleto\n";
        glu::cerrar(c);
        return 1;
    }

    const bool* teclas = SDL_GetKeyboardState(nullptr);
    bool teclaPPrev = false, teclaCPrev = false;
    bool pcfActivo = true;
    float multipBias = 1.0f;
    while (glu::eventosOk()) {
        float dt = 1.0f / 60.0f;
        if (teclas[SDL_SCANCODE_LEFT])  posLuz.x -= 1.5f * dt;
        if (teclas[SDL_SCANCODE_RIGHT]) posLuz.x += 1.5f * dt;
        if (teclas[SDL_SCANCODE_UP])    posLuz.y += 1.0f * dt;
        if (teclas[SDL_SCANCODE_DOWN])  posLuz.y -= 1.0f * dt;

        bool teclaP = teclas[SDL_SCANCODE_P];
        if (teclaP && !teclaPPrev) {
            pcfActivo = !pcfActivo;
            std::cout << "PCF 3x3: " << (pcfActivo ? "ON" : "OFF")
                      << " (mirá el borde de la sombra)\n";
        }
        teclaPPrev = teclaP;

        if (teclas[SDL_SCANCODE_J]) multipBias -= 1.0f * dt;  // menos bias
        if (teclas[SDL_SCANCODE_K]) multipBias += 1.0f * dt;  // más bias
        if (multipBias < 0.0f) multipBias = 0.0f;

        bool teclaC = teclas[SDL_SCANCODE_C];
        if (teclaC && !teclaCPrev) reporteCascadas(TAM_MAPA, HALF_CJA);
        teclaCPrev = teclaC;

        /* Cada frame: recalcular matLuz, pase de depth, pase de color. */
        f21::Mat4 vistaLuz = f21::lookAt(posLuz, {0.0f, 0.0f, 0.0f},
                                         {0.0f, 1.0f, 0.0f});
        matLuz = projLuz * vistaLuz;

        /* PASE 1: depth desde la luz. */
        glViewport(0, 0, TAM_MAPA, TAM_MAPA);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glClear(GL_DEPTH_BUFFER_BIT);
        glUseProgram(progLuz);
        f21::subirMat(uMatLuzL.lugar, matLuz);
        f21::subirMat(uModelL.lugar, modeloPiso);
        f21::dibujar(piso);
        f21::subirMat(uModelL.lugar, modeloCubo);
        f21::dibujar(cubo);

        /* PASE 2: iluminación con el shadow test + PCF. */
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, c.ancho, c.alto);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(progVista);
        f21::subirMat(uMatVista.lugar, projCam * vistaCam);
        f21::subirMat(uMatLuzV.lugar, matLuz);
        GLint samU = glGetUniformLocation(progVista, "uMapaSombra");
        glUniform1i(samU, 0);
        GLint dirU = glGetUniformLocation(progVista, "uDirLuz");
        f21::subirVec(dirU, dirLuz);
        GLint colU = glGetUniformLocation(progVista, "uColorLuz");
        f21::subirVec(colU, {1.0f, 0.95f, 0.85f});
        GLint camU = glGetUniformLocation(progVista, "uPosCamara");
        f21::subirVec(camU, ojo);
        GLint multU = glGetUniformLocation(progVista, "uMultipBias");
        glUniform1f(multU, multipBias);
        GLint pcfU = glGetUniformLocation(progVista, "uPcfActivo");
        glUniform1i(pcfU, pcfActivo ? 1 : 0);
        GLint texelU = glGetUniformLocation(progVista, "uTexel");
        glUniform1f(texelU, 1.0f / (float)TAM_MAPA);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texSombra);
        f21::subirMat(uModelV.lugar, modeloPiso);
        f21::dibujar(piso);
        f21::subirMat(uModelV.lugar, modeloCubo);
        f21::dibujar(cubo);

        SDL_GL_SwapWindow(c.ventana);
    }

    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &texSombra);
    glDeleteProgram(progLuz);
    glDeleteProgram(progVista);
    f21::borrarMalla(cubo);
    f21::borrarMalla(piso);
    glu::cerrar(c);
    std::cout << "Listo: PCF suaviza la frontera, el bias es un dial y las "
                 "cascadas reparten texels donde la cámara mira.\n";
    return 0;
}
