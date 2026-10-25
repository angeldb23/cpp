/* =============================================================================
   MÓDULO 21 - LECCIÓN 11: Post-proceso 1: bloom + tone mapping
   =============================================================================
   TEMA 6 de TEMAS.md (primera de 2 lecciones): "post-procesado: bloom,
   tone mapping (ACES/Reinhard), FXAA, color grading". FXAA + grading en la
   LECCIÓN 12.

   --- HASTA ACÁ EL DESTINO ERA LA PANTALLA; ahora hay FBOs en el medio ------
   El módulo 20 L16 ya salió "por un renderbuffer" para el pase de picking.
   Bloom necesita lo mismo pero al revés: la ESCENA entera vuela a una
   TEXTURA de color, después pasa por 2-3 programas de pantalla-completa y
   recién el último dibuja a la ventana. Ese pattern es "post-processing":
       [pase escena] -> texEscena
       [bright pass] -> texBloom      (downsampled + solo lo brillante)
       [pingpong x N] -> texBloom     (bloom gaussiano horizontal+vertical)
       [composición]  -> PANTALLA     (escena + bloom, tonemap, gamma)

   --- ¿POR QUÉ HDR (RGBA16F) ANTES DE TONE MAP? ----------------------------
   Un GL_RGBA8 CLAMPEA al 0..1: el sol de la esfera que apunta a 3.0
   se guarda como 1.0 y el bloom ya no distingue "muy brillante" de "blanco
   quemado". GL_RGBA16F (0x881A, núcleo desde GL 3.0, color-renderable con
   el FBO core) guarda valores POR ENCIMA de 1.0 sin recorte: hacia el tone
   mapping le llega la ENERGÍA real y el mapeo decide qué tan blanco
   renderiza cada cosa. El tone mapping comprime la energía HDR a lo que
   un monitor puede mostrar:
     Reinhard:   c / (1 + c)                       (el más barato; lavado)
     ACES fit (Stephen Hill): aprieta en 4 zonas
       out = clamp( (c*(2.51*c + 0.03)) /
                    (c*(2.43*c + 0.59) + 0.14) )     (negros densos, caída
                                                     en altas luces: el
                                                     formato del moderno)
   Y por ÚLTIMO gamma: tonemap trabaja en lineal; la pantalla muestra
   sRGB, así que el último pase aplica pow(c, 1/2.2) (exacto la L02).

   --- EL BLUR SEPARABLE: 2 pasadas, NO una matríz 2D -----------------------
   Un Gaussian 2D 5x5 son 25 samples por fragmento. PERO es separable:
   kernel [1,4,6,4,1]/16 moviéndose en X (pase 1), luego en Y (pase 2):
   MISMA apariencia con 10 samples. Y más: ping-pong entre DOS FBOs
   (blur A->B y B->A) varias veces = blur más grande sin texturas extra.

   --- ESTA DEMO --------------------------------------------------------------
   Escena simple + un SOL emissivo (una esfera de albedo 10.0, way past 1.0
   en el buffer HDR). Teclas:
     [B] bloom on/off     [T] cicla tonemap: ACES / Reinhard / sin tonemap
     [LEFT/RIGHT] mueve el umbral del bright pass (qué cuenta como "brillo")
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
    gl_Position = vec4(aPos, 1.0);      // quad ya está en clip space
}
)GLSL";

/* PASE ESCENA: Phong de la L01, pero SALIENDO A un FBO RGBA16F: los
   valores de spec y del sol pueden pasar de 1.0 sin quemarse. */
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
    if (vPos.y > 2.4f) albedo = vec3(8.0f, 7.0f, 2.0f);  // sol HDR (13x!)
    else               albedo = vec3(0.30f, 0.35f, 0.45f);
    vec3 color = albedo * 0.06f;                        // ambient chico
    color += albedo * max(dot(N, L), 0.0) * 0.45f;      // difusa
    vec3 R = reflect(-L, N);
    color += albedo * pow(max(dot(R, V), 0.0), 48.0f);  // spec HDR
    FragColor = vec4(color, 1.0);
}
)GLSL";

/* BRIGHT PASS: retiene solo lo que pasa el umbral (con soft-knee corto:
   la arranca suave alrededor del umbral, __no__ de golpe). Down-sample:
   el pase corre en una textura de la CUARTA parte. */
const char* FS_BRIGHT = R"GLSL(
#version 330 core
in vec2 vUV;
out vec4 FragColor;
uniform sampler2D uEscena;
uniform float uUmbral;
void main() {
    vec3 c = texture(uEscena, vUV).rgb;
    float brillo = dot(c, vec3(0.2126, 0.7152, 0.0722));  // luma BT.709
    float articule = max(brillo - uUmbral, 0.0f);
    articule = articule / (1.0f + articule);              // knee suave 0..1
    FragColor = vec4(c * articule, 1.0f);
}
)GLSL";

/* BLUR SEPARABLE: kernel [1,4,6,4,1]/16, dirección por uniform
   (1,0)=horizontal / (0,1)=vertical. 10 samples en vez de 25. */
const char* FS_BLUR = R"GLSL(
#version 330 core
in vec2 vUV;
out vec4 FragColor;
uniform sampler2D uEscena;
uniform vec2 uDir;      // dirección del pase (texel * pasito)
void main() {
    float w[5] = float[](1.0/16.0, 4.0/16.0, 6.0/16.0, 4.0/16.0, 1.0/16.0);
    vec3 suma = texture(uEscena, vUV).rgb * w[0];
    for (int i = 1; i < 5; ++i) {
        suma += texture(uEscena, vUV + uDir * float(i)).rgb * w[i];
        suma += texture(uEscena, vUV - uDir * float(i)).rgb * w[i];
    }
    FragColor = vec4(suma, 1.0);
}
)GLSL";

/* COMPOSICIÓN FINAL: escena + bloom, tone map y gamma (a la pantalla). */
const char* FS_COMPO = R"GLSL(
#version 330 core
in vec2 vUV;
out vec4 FragColor;
uniform sampler2D uEscena;
uniform sampler2D uBloom;
uniform float uFuerzaBloom;
uniform int   uTonemap;    // 0 ACES, 1 Reinhard, 2 sin tonemap (quemado)
vec3 aces(vec3 c) {
    return clamp((c * (2.51f * c + 0.03f)) / (c * (2.43f * c + 0.59f) + 0.14f),
                 0.0f, 1.0f);
}
void main() {
    vec3 c = texture(uEscena, vUV).rgb;
    c += texture(uBloom, vUV).rgb * uFuerzaBloom;
    if (uTonemap == 0)      c = aces(c);
    else if (uTonemap == 1) c = c / (1.0f + c);
    c = pow(c, vec3(1.0f / 2.2f));   // lineal->sRGB del pipeline (la L02)
    FragColor = vec4(c, 1.0);
}
)GLSL";

/* Cuá de pantalla completa con el MISMO layout pos3|normal3|uv2 de f21. */
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

/* Textura de color RGBA16F para un FBO (el canal HDR del pipeline). */
void crearTexturaHDR(GLuint& tex, int w, int h) {
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0,
                 GL_RGBA, GL_HALF_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void crearFBO(GLuint& fbo, GLuint tex) {
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, tex, 0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
}

void dibujarQuad(GLuint prog, f21::MallaGPU& quad) {
    glUseProgram(prog);
    glDisable(GL_DEPTH_TEST);
    f21::dibujar(quad);
    glBindVertexArray(0);
}

}  // namespace

int main() {
    std::cout << "Bloom HDR (RGBA16F) + tonemap ACES/Reinhard. Teclas: "
              << "B bloom, T tonemap, izq/der umbral.\n";

    glu::Contexto c = glu::abrir("M21 L11: bloom + tone mapping", 800, 550);
    if (c.ventana == nullptr) return 1;
    glEnable(GL_DEPTH_TEST);

    GLuint progEscena = glu::enlazar([] {
        return R"GLSL(
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
    }(), FS_ESCENA);
    GLuint progBright = glu::enlazar(VS_PANTALLA, FS_BRIGHT);
    GLuint progBlur   = glu::enlazar(VS_PANTALLA, FS_BLUR);
    GLuint progCompo  = glu::enlazar(VS_PANTALLA, FS_COMPO);
    if (progEscena == 0 || progBright == 0 || progBlur == 0
        || progCompo == 0) {
        glu::cerrar(c);
        return 1;
    }

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

    f21::MallaGPU piso   = f21::subirMalla(f21::cubo(0.5f));
    f21::MallaGPU esfera = f21::subirMalla(f21::esfera(0.50f, 32, 16));
    f21::MallaGPU sol    = f21::subirMalla(f21::esfera(0.35f, 32, 16));
    f21::MallaGPU quad   = f21::subirMalla(cuadrado());
    f21::Mat4 modeloPiso = f21::trs({0.0f, -0.5f, 0.0f}, 1.0f, 0.0f);
    f21::Mat4 modeloEsfera = f21::trs({-1.1f, 0.5f, 0.3f}, 1.0f, 0.0f);
    f21::Mat4 modeloSol  = f21::trs({1.6f, 2.8f, -1.5f}, 1.0f, 0.0f);

    /* --- Los FBOs del pipeline --- */
    // 1) escena HDR a tamaño de ventana + depth renderbuffer
    GLuint texEscena = 0, fboEscena = 0, rbDeph = 0;
    crearTexturaHDR(texEscena, c.ancho, c.alto);
    crearFBO(fboEscena, texEscena);
    glGenRenderbuffers(1, &rbDeph);
    glBindRenderbuffer(GL_RENDERBUFFER, rbDeph);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, c.ancho,
                          c.alto);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                              GL_RENDERBUFFER, rbDeph);

    // 2) bloom al cuarto + ping-pong b (ambos completos de un canal HDR)
    constexpr int BW = 200, BH = 138;      // ~ ventana/4 (acá no hace falta potencia de dos)
    GLuint texBloomA = 0, texBloomB = 0, fboA = 0, fboB = 0;
    crearTexturaHDR(texBloomA, BW, BH);
    crearTexturaHDR(texBloomB, BW, BH);
    crearFBO(fboA, texBloomA);
    crearFBO(fboB, texBloomB);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "FBO del post-proceso incompleto\n";
        glu::cerrar(c);
        return 1;
    }

    const bool* teclas = SDL_GetKeyboardState(nullptr);
    bool teclaBPrev = false, teclaTPrev = false;
    bool bloomActivo = true;
    int  tonemap = 0;      // 0 ACES, 1 Reinhard, 2 sin tonemap
    float umbral = 1.0f;
    while (glu::eventosOk()) {
        float dt = 1.0f / 60.0f;
        if (teclas[SDL_SCANCODE_LEFT])  umbral -= 1.0f * dt;
        if (teclas[SDL_SCANCODE_RIGHT]) umbral += 1.0f * dt;
        if (umbral < 0.0f) umbral = 0.0f;
        bool teclaB = teclas[SDL_SCANCODE_B];
        if (teclaB && !teclaBPrev) {
            bloomActivo = !bloomActivo;
            std::cout << "bloom: " << (bloomActivo ? "ON" : "OFF") << "\n";
        }
        teclaBPrev = teclaB;
        bool teclaT = teclas[SDL_SCANCODE_T];
        if (teclaT && !teclaTPrev) {
            tonemap = (tonemap + 1) % 3;
            std::cout << "tonemap: "
                      << (tonemap == 0 ? "ACES" :
                          tonemap == 1 ? "Reinhard" : "sin tonemap") << "\n";
        }
        teclaTPrev = teclaT;

        /* PASE ESCENA -> FBO HDR. */
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
        f21::subirMat(uModel.lugar, modeloSol);
        f21::dibujar(sol);

        /* BRIGHT PASS: escena -> texBloomA (downsampled al cuarto). */
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texEscena);
        glViewport(0, 0, BW, BH);
        glBindFramebuffer(GL_FRAMEBUFFER, fboA);
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(progBright);
        glUniform1f(glGetUniformLocation(progBright, "uUmbral"), umbral);
        glUniform1i(glGetUniformLocation(progBright, "uEscena"), 0);
        dibujarQuad(progBright, quad);

        /* PING-PONG: horizontal A->B, vertical B->A, x 4 (blur amplio). */
        glUseProgram(progBlur);
        glUniform1i(glGetUniformLocation(progBlur, "uEscena"), 0);
        const float ti = 1.0f / (float)BW, tj = 1.0f / (float)BH;
        for (int i = 0; i < 4; ++i) {
            glBindFramebuffer(GL_FRAMEBUFFER, fboB);
            glBindTexture(GL_TEXTURE_2D, texBloomA);
            glUniform2f(glGetUniformLocation(progBlur, "uDir"),
                        ti * 1.0f, 0.0f);
            dibujarQuad(progBlur, quad);
            glBindFramebuffer(GL_FRAMEBUFFER, fboA);
            glBindTexture(GL_TEXTURE_2D, texBloomB);
            glUniform2f(glGetUniformLocation(progBlur, "uDir"),
                        0.0f, tj * 1.0f);
            dibujarQuad(progBlur, quad);
        }

        /* COMPOSICIÓN: escena + bloom, tonemap, gamma, a la PANTALLA. */
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, c.ancho, c.alto);
        glClearColor(0.0f, 0.0f, 0.02f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(progCompo);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texEscena);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texBloomA);
        glUniform1i(glGetUniformLocation(progCompo, "uEscena"), 0);
        glUniform1i(glGetUniformLocation(progCompo, "uBloom"), 1);
        glUniform1f(glGetUniformLocation(progCompo, "uFuerzaBloom"),
                    bloomActivo ? 1.0f : 0.0f);
        glUniform1i(glGetUniformLocation(progCompo, "uTonemap"), tonemap);
        dibujarQuad(progCompo, quad);
        glEnable(GL_DEPTH_TEST);

        SDL_GL_SwapWindow(c.ventana);
    }

    glDeleteFramebuffers(1, &fboEscena);
    glDeleteFramebuffers(1, &fboA);
    glDeleteFramebuffers(1, &fboB);
    glDeleteTextures(1, &texEscena);
    glDeleteTextures(1, &texBloomA);
    glDeleteTextures(1, &texBloomB);
    glDeleteRenderbuffers(1, &rbDeph);
    glDeleteProgram(progEscena);
    glDeleteProgram(progBright);
    glDeleteProgram(progBlur);
    glDeleteProgram(progCompo);
    f21::borrarMalla(piso);
    f21::borrarMalla(esfera);
    f21::borrarMalla(sol);
    f21::borrarMalla(quad);
    glu::cerrar(c);
    std::cout << "Listo: en HDR el sol rinde energía, el bloom difunde esa "
                 "energía y el tonemap comprime a lo monitorable.\n";
    return 0;
}
