/* =============================================================================
   MÓDULO 21 - LECCIÓN 01: Phong y Blinn-Phong: ambient + diffuse + specular
   =============================================================================
   TEMA 1 de TEMAS.md (primera de 2 lecciones): "Modelo de iluminación:
   ambient/diffuse/specular (Phong), Blinn-Phong, gamma correction y sRGB".
   La parte de gamma/sRGB completa es la LECCIÓN 02.

   CÓMO CORRERLA A MANO (verificar.sh NO la corre: abre una ventana):
     g++ -std=c++20 -Wall -Wextra leccion_01_phong_blinn_phong.cpp \
         $(pkg-config --cflags --libs sdl3) -lOpenGL -o lec01 && ./lec01

   --- EL PROBLEMA QUE RESUELVE PHONG (1975) ----------------------------------
   En el módulo 20 dibujaste la malla con su color plano: cada fragmento era
   `vec4(0.8, 0.2, 0.2, 1.0)` sin importar de dónde venía la luz. Resultado: la
   esfera parecía un círculo pegado, el cubo se veía con caras planas y faltaba
   TODO el brillo (el brillo es lo que la mente lee como "material").

   Phong descompone el color de UN fragmento en la suma de tres términos:
     1. AMBIENT  : luz "de ambiente" que llega igual a todos lados. En la
                   realidad es rebote indirecto; el modelo lo clava como una
                   constante chica. Su trabajo pedagógico: que las sombras
                   nunca sean negro TOTAL (eso se llama "apagón" y se percibe
                   como un error, no como oscuridad).
     2. DIFFUSE  : luz directa que llega y rebota igual en todos los ángulos
                   (material mate): proporcional a dot(N, L). El ángulo entre
                   la normal y la luz decide CUÁNTA luz golpea la cara: una
                   cara muy de lado recibe menos (el coseno hace su trabajo).
     3. SPECULAR : el REFLEJO de la luz en la cara (el brillo). Aparece cuando
                   la cara REFLEJA la luz hacia la cámara. En Phong:
                   refl = reflect(-L, N) y se toma dot(refl, V) con V la
                   dirección hacia la cámara, elevado al exponente de brillo.
                   Exponente ALTO: mancha de spec PEQUEÑA y definida (metal
                   pulido); exponente BAJO: mancha ancha y porosa (metal opaco).

     color = ambient + diffuse + specular, TODO multiplicado por el color
     base (albedo). SIN el max(..., 0.0) del diffuse, las caras de ESPALDA a
     la luz reciben un dot negativo y "restan luz" iluminándose: puro error.

   --- BLINN-PHONG (1977): MISMA idea, cálculo distinto ----------------------
   Phong computa reflect + un dot extra. Blinn-PHONG cambia SOLO el specular:
       spec = pow(max(dot(N, H), 0.0), brillo), con H = normalize(L + V)
   (half-vector: la bisectriz entre luz y cámara). H == N es el centro exacto
   del brillo. La ventaja histórica (y por qué los motores clásicos la usan):
   no llama reflect y el resultado es casi igual (la mancha es un poco más
   ancha; para comparar specs de igual TAMAÑO se DOBLA el exponente: esta
   lección lo hace). En la GPU moderna la diferencia de costo es mínima, pero
   sigue siendo el default por SU look clásico (el "vibe retrogaming").

   --- QUÉ MANDA EL CPU Y QUÉ CALCULA EL GPU ---------------------------------
   El CPU manda por uniform: las POSICIONES (luz y cámara, en MUNDO), el
   exponente brillo y las 3 matrices (model/view/proj, módulo 19). El GPU
   calcula por fragmento: N, L, V, los tres términos y el color final.
   Detalle crítico: la NORMAL interpolada ya NO es unitaria (recorre los
   vértices y el promedio de dos unitarias NO es unitaria): SIEMPRE
   normalize() en el fragment.

   --- APIs USADAS (todas verificadas contra glcorearb.h local) ---------------
   - glu::abrir / glu::enlazar: ya del módulo 20 (incluye SDL3 3.4.18).
   - glEnable(GL_DEPTH_TEST): sin depth test el fragmento se pinta por orden
     de envío y el frente queda tapado por lo que se envíe de último.
   - glUniformMatrix4fv / glUniform3f / glUniform1f / glUniform1i: helpers
     f21::subirMat / subirVec (transpose=GL_FALSE: la Mat4 de mate3d ya es
     column-major, misma convención que GLM y que GLSL).
   - glTexImage2D con internalformat GL_RGBA8 (0x8058... en glcorearb) +
     GL_RGBA + GL_UNSIGNED_BYTE (mecánica completa en la lección 09 del 20).
   DE MENCION: el ajedrez procedural de esta lección reemplaza stb_image
   (20 L09); la lección 05 (materiales) sube texturas reales por unidades.

   ============================================================================
 */

#include "include/l21.hpp"

#include <iostream>

namespace {

/* VÉRTICES compartido por los DOS programas: pasamos al fragment el PUNTO EN
   MUNDO y la NORMAL EN MUNDO (la normal se rota por el bloque 3x3 del model:
   traslación NO mueve direcciones; escalar UNIFORME se corrige normalizando
   luego del fragment). */
const char* VS_FUENTE = R"GLSL(
#version 330 core
layout (location = 0) in vec3 aPos;      // espacio MODELO
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUV;

uniform mat4 uModel;   // local -> mundo
uniform mat4 uView;    // mundo -> cámara (lección 19 L5-L6)
uniform mat4 uProj;    // cámara -> clip

out vec3  vPos;        // punto EN MUNDO (la luz vive en mundo)
out vec3  vNormal;     // normal en mundo (NO recorra traslación)
out vec2  vUV;

void main() {
    vec4 mundo = uModel * vec4(aPos, 1.0);
    vPos = mundo.xyz;
    vNormal = mat3(uModel) * aNormal;
    vUV = aUV;
    gl_Position = uProj * uView * mundo;
}
)GLSL";

/* FRAGMENTO PHONG: las tres funciones de encima; comentadas para ver QUÉ
   aporta cada una. Para ENTENDER el efecto, mostrá el resultado final con
   términos COMUNES (apagar uno: 0.0). */
const char* FS_PHONG = R"GLSL(
#version 330 core
in vec3 vPos;
in vec3 vNormal;
in vec2 vUV;
out vec4 FragColor;

uniform vec3  uLuzPos;      // posición de la LUZ en MUNDO
uniform vec3  uLuzColor;
uniform vec3  uPosCamara;   // posición del ojo en MUNDO
uniform float uBrillo;      // exponente specular
uniform sampler2D uAlbedo;  // textura base (lección 05 profundiza uniformes)

vec3 ambient(vec3 albedo, vec3 N) {
    // medio-lambert: mezcla la normal con algo de orientación (luz de relleno
    // de artistas: barata, "falso rebote", sin dirección física real)
    float suave = 0.5 + 0.5 * N.y;
    return (0.08 + 0.06 * suave) * albedo;
}

vec3 diffuse(vec3 albedo, vec3 N, vec3 L) {
    float dif = max(dot(N, L), 0.0);   // el max descarta la "luz de espalda"
    return uLuzColor * dif * albedo;
}

// PHONG clásico: reflejo + cámara
vec3 specularPhong(vec3 N, vec3 L, vec3 V) {
    vec3 refl = reflect(-L, N);
    float spec = pow(max(dot(refl, V), 0.0), uBrillo);
    return uLuzColor * 0.35 * spec;        // 0.35 = intensidad especular
}

void main() {
    vec3 N = normalize(vNormal);           // SIEMPRE: interpolada no es unit
    vec3 L = normalize(uLuzPos - vPos);    // fragmento -> luz
    vec3 V = normalize(uPosCamara - vPos); // fragmento -> cámara

    vec3 albedo = texture(uAlbedo, vUV).rgb;
    vec3 color = ambient(albedo, N)
               + diffuse(albedo, N, L)
               + specularPhong(N, L, V);
    FragColor = vec4(color, 1.0);
}
)GLSL";

/* FRAGMENTO BLINN-PHONG: SOLO cambia la función del specular. Todo lo demás
   es el GLSL de arriba: eso es el punto pedagógico (el CHANGE es 3 líneas). */
const char* FS_BLINN = R"GLSL(
#version 330 core
in vec3 vPos;
in vec3 vNormal;
in vec2 vUV;
out vec4 FragColor;

uniform vec3  uLuzPos;
uniform vec3  uLuzColor;
uniform vec3  uPosCamara;
uniform float uBrillo;
uniform sampler2D uAlbedo;

vec3 ambient(vec3 albedo, vec3 N) {
    float suave = 0.5 + 0.5 * N.y;
    return (0.08 + 0.06 * suave) * albedo;
}

vec3 diffuse(vec3 albedo, vec3 N, vec3 L) {
    float dif = max(dot(N, L), 0.0);
    return uLuzColor * dif * albedo;
}

// BLINN: H = bisectriz de L y V. Si N == H, el reflejo de la luz cae exacto
// sobre la cámara: centro del brillo. Con max() se evita el spec "invertido".
vec3 specularBlinn(vec3 N, vec3 L, vec3 V) {
    vec3 H = normalize(L + V);
    float spec = pow(max(dot(N, H), 0.0), uBrillo);
    return uLuzColor * 0.35 * spec;
}

void main() {
    vec3 N = normalize(vNormal);
    vec3 L = normalize(uLuzPos - vPos);
    vec3 V = normalize(uPosCamara - vPos);
    vec3 albedo = texture(uAlbedo, vUV).rgb;
    vec3 color = ambient(albedo, N)
               + diffuse(albedo, N, L)
               + specularBlinn(N, L, V);
    FragColor = vec4(color, 1.0);
}
)GLSL";

}  // namespace

int main() {
    glu::Contexto c = glu::abrir("M21 L01: Phong (izq) vs Blinn-Phong (der)",
                                 800, 500);
    if (c.ventana == nullptr) return 1;

    glEnable(GL_DEPTH_TEST);

    GLuint progPhong = glu::enlazar(VS_FUENTE, FS_PHONG);
    GLuint progBlinn = glu::enlazar(VS_FUENTE, FS_BLINN);
    if (progPhong == 0 || progBlinn == 0) { glu::cerrar(c); return 1; }

    // Textura PROCEDURAL 64x64 (sin stb_image; esa mecánica es del 20 L09):
    // ajedrez para que el spec MUEVA sobre superficie con variación.
    constexpr int TX = 64, TY = 64;
    std::vector<unsigned char> pix(TX * TY * 4);
    for (int y = 0; y < TY; ++y)
        for (int x = 0; x < TX; ++x) {
            bool par = ((x / 8) + (y / 8)) % 2 == 0;
            float v = par ? 0.8f : 0.35f;
            std::size_t i = (std::size_t(y) * TX + std::size_t(x)) * 4;
            pix[i] = pix[i + 1] = pix[i + 2] =
                static_cast<unsigned char>(v * 255.0f + 0.5f);
            pix[i + 3] = 255;
        }

    GLuint texAlbedo = 0;
    glGenTextures(1, &texAlbedo);
    glBindTexture(GL_TEXTURE_2D, texAlbedo);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, TX, TY, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, pix.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    f21::Malla cubo = f21::cubo(0.5f);
    f21::Malla esf  = f21::esfera(0.6f);
    f21::MallaGPU gCubo = f21::subirMalla(cubo);
    f21::MallaGPU gEsfera = f21::subirMalla(esf);

    // Cada programa CACHÉ sus lugares UNA vez (20 L03: cada GET llama al
    // driver: en el update del frame sería un despilfarro).
    struct Luces {
        glu::Uniform model, view, proj, luzPos, luzColor, posCamara, brillo;
        void buscarEn(GLuint prog) {
            model.buscar(prog, "uModel");     view.buscar(prog, "uView");
            proj.buscar(prog, "uProj");       luzPos.buscar(prog, "uLuzPos");
            luzColor.buscar(prog, "uLuzColor");
            posCamara.buscar(prog, "uPosCamara");
            brillo.buscar(prog, "uBrillo");
        }
    } les[2];
    les[0].buscarEn(progPhong);
    les[1].buscarEn(progBlinn);
    GLint albedoP = glGetUniformLocation(progPhong, "uAlbedo");
    GLint albedoB = glGetUniformLocation(progBlinn, "uAlbedo");

    f21::Mat4 vista = f21::lookAt({0.0f, 1.3f, 3.0f}, {0.0f, 0.1f, 0.0f},
                                  {0.0f, 1.0f, 0.0f});
    f21::Mat4 proj = f21::perspectiva(
        60.0f, (float)c.ancho / (float)c.alto, 0.1f, 100.0f);
    f21::Vec3 ojo{0.0f, 1.3f, 3.0f};
    const bool* teclas = SDL_GetKeyboardState(nullptr);

    float angulo = 0.0f;   // órbita de la LUZ: verás el brillo caminar
    float brillo = 32.0f;  // exponente specular ajustable
    while (glu::eventosOk()) {
        float dt = 1.0f / 60.0f;   // timestep fijo simple del módulo 16
        if (teclas[SDL_SCANCODE_LEFT])  angulo += 40.0f * dt;
        if (teclas[SDL_SCANCODE_RIGHT]) angulo -= 40.0f * dt;
        if (teclas[SDL_SCANCODE_UP])    brillo *= 1.02f;
        if (teclas[SDL_SCANCODE_DOWN])  brillo /= 1.02f;
        angulo += 15.0f * dt;        // la luz por sí sola da una vuelta lenta

        // Luz orbitando en XZ a altura 1.5 (el brillo "pasea" por la esfera).
        float rad = f21::aRad(angulo);
        f21::Vec3 luzPos{ std::cos(rad) * 1.5f, 1.5f, std::sin(rad) * 1.5f };
        f21::Vec3 luzColor{1.0f, 0.95f, 0.85f};

        glClearColor(0.08f, 0.08f, 0.10f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // MITAD IZQUIERDA: cubo con PHONG
        glViewport(0, 0, c.ancho / 2, c.alto);
        glUseProgram(progPhong);
        f21::subirMat(les[0].model.lugar, f21::trs({0.0f, 0.0f, 0.0f}, 1.0f, 0.0f));
        f21::subirMat(les[0].view.lugar,  vista);
        f21::subirMat(les[0].proj.lugar,  proj);
        f21::subirVec(les[0].luzPos.lugar,  luzPos);
        f21::subirVec(les[0].luzColor.lugar, luzColor);
        f21::subirVec(les[0].posCamara.lugar, ojo);
        glUniform1f(les[0].brillo.lugar, brillo);
        glUniform1i(albedoP, 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texAlbedo);
        f21::dibujar(gCubo);

        // MITAD DERECHA: esfera con BLINN. Exponente DOBLE: la mancha Blinn
        // es más ancha; con 2x queda el mismo radio aprox. que en Phong.
        glViewport(c.ancho / 2, 0, c.ancho - c.ancho / 2, c.alto);
        glUseProgram(progBlinn);
        f21::subirMat(les[1].model.lugar, f21::trs({0.0f, 0.0f, 0.0f}, 1.0f, 0.0f));
        f21::subirMat(les[1].view.lugar,  vista);
        f21::subirMat(les[1].proj.lugar,  proj);
        f21::subirVec(les[1].luzPos.lugar,  luzPos);
        f21::subirVec(les[1].luzColor.lugar, luzColor);
        f21::subirVec(les[1].posCamara.lugar, ojo);
        glUniform1f(les[1].brillo.lugar, brillo * 2.0f);
        glUniform1i(albedoB, 0);
        f21::dibujar(gEsfera);

        SDL_GL_SwapWindow(c.ventana);
    }

    glDeleteProgram(progPhong);
    glDeleteProgram(progBlinn);
    glDeleteTextures(1, &texAlbedo);
    f21::borrarMalla(gCubo);
    f21::borrarMalla(gEsfera);
    glu::cerrar(c);
    std::cout << "Listo: comparaste specular de Phong (izquierda, cubo) contra "
                 "Blinn (derecha, esfera). La luz orbita sola; flechas la "
                 "mueven más rápido.\n";
    return 0;
}
