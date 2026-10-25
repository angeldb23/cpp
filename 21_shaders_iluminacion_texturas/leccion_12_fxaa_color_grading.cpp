/* =============================================================================
   MÓDULO 21 - LECCIÓN 12: Post-proceso 2: antialiasing (FXAA) + color grading
   =============================================================================
   TEMA 6 de TEMAS.md (segunda de 2 lecciones).

   --- EL DENTADO (aliasing) y sus curas -------------------------------------
   El dentado nace de PARTIR la escena en celdas de píxeles: los bordes
   geométricos quedan escalonados. Curas, en orden de clásico a moderno:
     MSAA: más SAMPLES por píxel del depth/color (hardware; GL 3.3 core lo
       apoya con glTexImage2DMultisample + resolve por blitFramebuffer; el
       módulo lo nombra pero NO lo usa: es escribir FBOs multisample y la
       resolución sin muestrear en shader).
     FXAA (NVIDIA, 2011): 1 pase a pantalla completa, SIN samples extra del
       depth: analiza la LUMINANCIA del pixel vs sus vecinos, detecta el
       eje del borde (horizontal o vertical) y lo "barrerito" mezclándose a
       un lado y al otro. Barato a morir, algo borroso: EL de consolas.
     TAA/DLSS: ya son otra liga (historia de frames más jitter); fuera de
       alcance del 3.3.
   Acá armamos FXAA "de cátedra": la idea (luma de adjacencia => eje => mezcla)
   completa pero compacta; el FXAA oficial tiene ~20 modos y años de taa.
       float lumaNW = luma(pix + (-1,-1))  ...  lumaNW/lumaNE (eje X)
       if (luma corto) mezcla en la dirección del borde
   La contraindicación: FXAA mezcla COLORES, no geometría: líneas de 1px
   tipo "cortacésped" las suaviza igual (el trade-off honesto).

   --- COLOR GRADING: la identidad visual del juego -------------------------
   Aplicar una CURVA/color por píxel de forma global: sepia shooter retro,
   azules fríos de noche, teal-and-orange de cualquier cover de película. La forma
   de motor: una LUT 3D: un GL_TEXTURE_3D (GL_TEXTURE_3D 0x806F, glTexImage3D,
   núcleo desde GL 3.0) donde el RGB del píxel entra como COORDENADA y sale
   el color ajustado; con sampler3D la interpolación trilinear es gratis.
   Guardá toda la curva en UNA textura (8x8x8 basta de média calidad y 32x32
   es la de los .cube reales). EL que "sube" la LUT arma los cambios: en la
   demo generamos 3 LUTs por CPU (identidad, cálido, nocturno) con las
   MISMAS coords.

   --- ESTA DEMO -------------------------------------------------------------
   Misma escena de la 11 PERO ya en LDR (GL_RGBA8): el post es el ÚNICO
   paso. Teclas:
     [A] FXAA on/off (mirá el borde de la esfera y del piso al fondo)
     [G] cicla grading: identidad / cálido / nocturno (cómo la LUT re-arma)
   ============================================================================ */

#include "include/l21.hpp"

#include <iostream>

namespace {

const char* VS_PANTALLA = R"GLSL(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUV;
out vec2 vUV;
void main() {
    vUV = aUV;
    gl_Position = vec4(aPos, 1.0);
}
)GLSL";

/* PASE ESCENA básico (Phong L01) volcado a un RGB8 (LDR: la L11 explicó HDR). */
const char* FS_ESCENA = R"GLSL(
#version 330 core
in vec3 vPos;
in vec3 vNormal;
out vec4 FragColor;
uniform vec3 uPosCamara;
uniform vec3 uPosLuz;
void main() {
    vec3 N = normalize(vNormal);
    vec3 L = normalize(uPosLuz - vPos);
    vec3 V = normalize(uPosCamara - vPos);
    vec3 albedo;
    if (vPos.y > 0.9f) albedo = vec3(0.85f, 0.60f, 0.20f);
    else               albedo = vec3(0.30f, 0.35f, 0.45f);
    vec3 color = albedo * 0.12f;
    color += albedo * max(dot(N, L), 0.0) * 0.55f;
    vec3 R = reflect(-L, N);
    color += vec3(0.9f) * pow(max(dot(R, V), 0.0), 40.0f) * 0.35f;
    FragColor = vec4(color, 1.0);
}
)GLSL";

const char* VS_ESCENA = R"GLSL(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;
out vec3 vPos;
out vec3 vNormal;
void main() {
    vec4 mundo = uModel * vec4(aPos, 1.0);
    vPos = mundo.xyz;
    vNormal = mat3(uModel) * aNormal;
    gl_Position = uProj * uView * mundo;
}
)GLSL";

/* FXAA DE CÁTEDRA: luma alrededor del punto, eje del borde (X o Y),
   mezcla simétrica del pixel con sus 2 vecinos del lado opuesto.
   El real además sube el REACH del borde: no hay que formalizarlo
   todo para ver el efecto. */
const char* FS_FXAA_GRADE = R"GLSL(
#version 330 core
in vec2 vUV;
out vec4 FragColor;
uniform sampler2D uEscena;
uniform sampler3D uLut;
uniform int   uFxaaActivo;
uniform vec2  uPix;          // 1.0/ancho, 1.0/alto
float luma(vec3 c) { return dot(c, vec3(0.299, 0.587, 0.114)); }
void main() {
    vec3 centro = texture(uEscena, vUV).rgb;
    vec3 c = centro;

    if (uFxaaActivo == 1) {
        float lc = luma(centro);
        float lN = luma(texture(uEscena, vUV + vec2(0.0,  uPix.y)).rgb);
        float lS = luma(texture(uEscena, vUV + vec2(0.0, -uPix.y)).rgb);
        float lE = luma(texture(uEscena, vUV + vec2( uPix.x, 0.0)).rgb);
        float lO = luma(texture(uEscena, vUV + vec2(-uPix.x, 0.0)).rgb);
        float minL = min(lc, min(min(lN, lS), min(lE, lO)));
        float maxL = max(lc, max(max(lN, lS), max(lE, lO)));
        // 1) ¿hay borde? contraste local alto => sí
        float delta = maxL - minL;
        if (delta > 0.08f) {
            // 2) ¿el borde corre HORIZONTAL o VERTICAL?
            //    suma de las luminancias "transversales" por eje: la más
            //    contrastada delata la dirección en la que hay que alisar.
            float sumVert = lN + lS;      // vecinos de arriba/abajo
            float sumHor  = lE + lO;      // vecinos de los costados
            vec3 arribaAbajo = (texture(uEscena, vUV + vec2(0.0,  uPix.y)).rgb
                             + texture(uEscena, vUV + vec2(0.0, -uPix.y)).rgb) * 0.5f;
            vec3 costados    = (texture(uEscena, vUV + vec2( uPix.x, 0.0)).rgb
                             + texture(uEscena, vUV + vec2(-uPix.x, 0.0)).rgb) * 0.5f;
            // borde vertical => lo alisamos en X; borde horizontal => en Y
            c = (sumVert < sumHor) ? mix(centro, arribaAbajo, 0.5f)
                                   : mix(centro, costados,    0.5f);
        }
    }

    // COLOR GRADING: el RGB entra como COORDENADA de la LUT 3D (la media
    //  resolución S del cubo corrige el muestreo al centro de la célula:
    //  uv*(S-1)/S + 0.5/S)
    const float S = 8.0f;
    vec3 coord = c * ((S - 1.0f) / S) + (0.5f / S);
    c = texture(uLut, coord).rgb;

    FragColor = vec4(c, 1.0);
}
)GLSL";

f21::Malla cuadrado() {
    f21::Malla g;
    auto pon = [&](float x, float y, float u, float v) {
        g.vertices.insert(g.vertices.end(),
                          {x, y, 0.0f, 0.0f, 0.0f, 0.0f, u, v});
    };
    pon(-1, -1, 0, 0); pon(1, -1, 1, 0);
    pon(1, 1, 1, 1);   pon(-1, 1, 0, 1);
    g.indices = {0, 1, 2, 0, 2, 3};
    return g;
}

/* LUT 3D 8x8x8 GL_RGBA8: la identidad devuelve 1:1 por célula. El "grade"
   aplica por separado sombras (con lift frío) y luces (calor/desaturación)
   SOBRE el mismo cuadrilátero; el motor de las 3 LUTs es el mismo upload. */
std::vector<unsigned char> construirLut(int modo) {
    constexpr int N = 8;
    std::vector<unsigned char> d(N * N * N * 4);
    auto idx = [&](int r, int g, int b) {
        return (r * N * N + g * N + b) * 4;
    };
    for (int r = 0; r < N; ++r)
    for (int g = 0; g < N; ++g)
    for (int b = 0; b < N; ++b) {
        float fr = (float)r / (N - 1), fg = (float)g / (N - 1),
              fb = (float)b / (N - 1);
        float orr = fr, og = fg, ob = fb;
        if (modo == 1) {  // cálido: levanta rojos, baja azules en luces
            float brillo = (fr + fg + fb) / 3.0f;
            orr = fr * 1.10f + 0.05f * brillo;
            og  = fg * 1.02f;
            ob  = fb * 0.85f;
        } else if (modo == 2) {  // nocturno: sombras teal, luces violetas
            float fac = fb * 0.35f + 0.25f;  // sombras más afectadas
            orr = fr * 0.80f;
            og  = fg * 0.95f + 0.05f * (1.0f - fac);
            ob  = fb * 1.15f + 0.10f * fac;
        }
        auto clamp8 = [](float x) {
            // re-mapear a 0..255 sin saturar más allá de los bordes
            return (unsigned char)(std::min(1.0f, std::max(0.0f, x)) * 255.0f);
        };
        d[idx(r, g, b) + 0] = clamp8(orr);
        d[idx(r, g, b) + 1] = clamp8(og);
        d[idx(r, g, b) + 2] = clamp8(ob);
        d[idx(r, g, b) + 3] = 255;
    }
    return d;
}

void crearLut(GLuint& tex, int modo) {
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_3D, tex);
    auto d = construirLut(modo);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, 8, 8, 8, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, d.data());
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

void crearTexturaColor(GLuint& tex, int w, int h) {
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

}  // namespace

int main() {
    std::cout << "FXAA de cátedra + color grading por LUT 3D (8x8x8 GL_RGBA8). "
              << "Teclas: A fxaa, G grading (3 LUTs).\n";

    glu::Contexto c = glu::abrir("M21 L12: FXAA + color grading", 800, 550);
    if (c.ventana == nullptr) return 1;
    glEnable(GL_DEPTH_TEST);

    GLuint progEscena = glu::enlazar(VS_ESCENA, FS_ESCENA);
    GLuint progPost   = glu::enlazar(VS_PANTALLA, FS_FXAA_GRADE);
    if (progEscena == 0 || progPost == 0) { glu::cerrar(c); return 1; }

    glu::Uniform uModel, uView, uProj;
    uModel.buscar(progEscena, "uModel");
    uView.buscar(progEscena, "uView");
    uProj.buscar(progEscena, "uProj");

    f21::Vec3 ojo{0.0f, 0.6f, 4.2f};
    f21::Mat4 vistaCam = f21::lookAt(ojo, {0.0f, 0.6f, 0.0f},
                                     {0.0f, 1.0f, 0.0f});
    f21::Mat4 projCam = f21::perspectiva(55.0f,
        (float)c.ancho / (float)c.alto, 0.1f, 100.0f);
    f21::Vec3 posLuz{2.0f, 3.0f, 2.5f};

    f21::MallaGPU piso     = f21::subirMalla(f21::cubo(0.5f));
    f21::MallaGPU esfera   = f21::subirMalla(f21::esfera(0.50f, 32, 16));
    f21::MallaGPU poste    = f21::subirMalla(f21::cubo(0.30f));
    f21::MallaGPU quad     = f21::subirMalla(cuadrado());
    f21::Mat4 modeloPiso     = f21::trs({0.0f, -0.5f, 0.0f}, 1.0f, 0.0f);
    f21::Mat4 modeloEsfera   = f21::trs({-1.1f, 0.5f, 0.3f}, 1.0f, 0.0f);
    f21::Mat4 modeloPoste    = f21::trs({1.4f, 0.3f, -0.8f}, 1.0f, 12.0f);

    /* Pase de escena en un LDR al window-size. */
    GLuint texEscena = 0, fboEscena = 0, rbDeph = 0;
    crearTexturaColor(texEscena, c.ancho, c.alto);
    glGenFramebuffers(1, &fboEscena);
    glBindFramebuffer(GL_FRAMEBUFFER, fboEscena);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, texEscena, 0);
    glGenRenderbuffers(1, &rbDeph);
    glBindRenderbuffer(GL_RENDERBUFFER, rbDeph);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, c.ancho,
                          c.alto);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                              GL_RENDERBUFFER, rbDeph);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "FBO de la escena incompleto\n";
        glu::cerrar(c);
        return 1;
    }

    /* Las 3 LUTs: identidad, cálido, nocturno. */
    GLuint luts[3] = {0, 0, 0};
    for (int m = 0; m < 3; ++m) crearLut(luts[m], m);

    const bool* teclas = SDL_GetKeyboardState(nullptr);
    bool teclaAPrev = false, teclaGPrev = false;
    bool fxaaActivo = true;
    int grading = 0;
    while (glu::eventosOk()) {
        bool teclaA = teclas[SDL_SCANCODE_A];
        if (teclaA && !teclaAPrev) {
            fxaaActivo = !fxaaActivo;
            std::cout << "FXAA: " << (fxaaActivo ? "ON" : "OFF") << "\n";
        }
        teclaAPrev = teclaA;
        bool teclaG = teclas[SDL_SCANCODE_G];
        if (teclaG && !teclaGPrev) {
            grading = (grading + 1) % 3;
            std::cout << "grading: "
                      << (grading == 0 ? "identidad" :
                          grading == 1 ? "cálido" : "nocturno") << "\n";
        }
        teclaGPrev = teclaG;

        /* PASE ESCENA -> FBO. */
        glBindFramebuffer(GL_FRAMEBUFFER, fboEscena);
        glViewport(0, 0, c.ancho, c.alto);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(progEscena);
        f21::subirMat(uView.lugar, vistaCam);
        f21::subirMat(uProj.lugar, projCam);
        f21::subirVec(glGetUniformLocation(progEscena, "uPosLuz"), posLuz);
        f21::subirVec(glGetUniformLocation(progEscena, "uPosCamara"), ojo);
        f21::subirMat(uModel.lugar, modeloPiso);
        f21::dibujar(piso);
        f21::subirMat(uModel.lugar, modeloEsfera);
        f21::dibujar(esfera);
        f21::subirMat(uModel.lugar, modeloPoste);
        f21::dibujar(poste);

        /* POST: FXAA + grading a la pantalla. */
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, c.ancho, c.alto);
        glDisable(GL_DEPTH_TEST);
        glUseProgram(progPost);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texEscena);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_3D, luts[grading]);
        glUniform1i(glGetUniformLocation(progPost, "uEscena"), 0);
        glUniform1i(glGetUniformLocation(progPost, "uLut"), 1);
        glUniform1i(glGetUniformLocation(progPost, "uFxaaActivo"),
                    fxaaActivo ? 1 : 0);
        glUniform2f(glGetUniformLocation(progPost, "uPix"),
                    1.0f / (float)c.ancho, 1.0f / (float)c.alto);
        f21::dibujar(quad);
        glBindVertexArray(0);

        glEnable(GL_DEPTH_TEST);
        SDL_GL_SwapWindow(c.ventana);
    }

    glDeleteFramebuffers(1, &fboEscena);
    glDeleteRenderbuffers(1, &rbDeph);
    glDeleteTextures(1, &texEscena);
    glDeleteTextures(3, luts);
    glDeleteProgram(progEscena);
    glDeleteProgram(progPost);
    f21::borrarMalla(piso);
    f21::borrarMalla(esfera);
    f21::borrarMalla(poste);
    f21::borrarMalla(quad);
    glu::cerrar(c);
    std::cout << "Listo: FXAA suaviza con luma (1 pase), la LUT 3D pinta el "
                 "humor del juego con un solo sample trilinear.\n";
    return 0;
}
