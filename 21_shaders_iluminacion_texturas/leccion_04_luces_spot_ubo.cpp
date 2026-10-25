/* =============================================================================
   MÓDULO 21 - LECCIÓN 04: Tipos de luz 2: spot, atenuación fina y UBO de luces
   =============================================================================
   TEMA 2 de TEMAS.md (segunda de 2 lecciones): "Luces: direccional, puntual,
   spot; atenuación (constante/lineal/cuadrática) y uniforms arrays vs UBO".

   CÓMO CORRERLA A MANO:
     g++ -std=c++20 -Wall -Wextra leccion_04_luces_spot_ubo.cpp \
         $(pkg-config --cflags --libs sdl3) -lOpenGL -o lec04 && ./lec04
   El programa IMPRIME además el plan de bytes std140 del UBO (lo comparás
   con el ejercicio 03).

   --- SPOT: la linterna -----------------------------------------------------
   Una puntual + un CONO de relleno: solo "ve" (y solo "baña") lo que cae
   dentro de su ángulo de apertura. Dos ángulos:
       interno  (corte-nítido central): hasta acá luz PLENA
       externo  (punta del cono):       a partir de acá luz CERO
   y un margen suave entre ambos (sin él, el borde es una "escalera" antinatural:
   hard edge). La matemática OCURRE EN COSENO (dot de direcciones; el coseno
   es MÁS GRANDE cuando el ángulo es más CHICO: cos(30°) > cos(60°)):
       cosTheta = dot(normalize(pos->luz), -dirSpot)   // cuánto "queda dentro"
       factor    = smoothstep(cosExterno, cosInterno, cosTheta)
   OJO con el orden de los parámetros: en coseno, PRIMERO el MENOR
   (externo), DESPUÉS el interno: invertirlos produce un cono INVERTIDO
   que arranca de cero en el centro. smoothstep de GLSL 330 integra con la
   curvita del easing (el módulo 19 L14 la describió).

   --- ATENUACIÓN + SPOT: dos máscaras que se multiplican ---------------------
   El spot comparte la atenuación 3-term de la puntual (caída con la
   distancia) y ENCIMA multiplica por el cono. La decisión artística de la
   lección 03 (¿se atenúa el spec?) acá se resuelve simple: cono y caída
   se aplican a TODO (spec incluído): si no, el brillo de la linterna rebota
   más allá del cono y la ilusión se rompe.

       aporte = (diffuse + specular) * atenuacion * conoFactor

   --- UNIFORMS ARRAY vs UBO: el diseño de la luz en 3.3 ---------------------
   La lección 03 manda las luces con glUniform3fv por FRAME:
       pro: simple, "setear y olvidar";
       contra: N llamadas glUniform3fv + un glUniform1i: comandos CPU->driver
       por cada bloque de datos (y si NO se modifican igual van).
   El UBO (uniform buffer object, visto en la 20 L05) lo repite con otro plan:
       pro: UN glBufferSubData para actualizar TODAS las luces de TODOS los
            programas a la vez (comparten el binding point);
            el layout es std140 y el sizeof se saca con reglas fijas:
            vec3 alinea en 16 (¡como un vec4 del hardware!), floats libres
            se apilan sin hueco, arrays de structs van con stride múltiplo
            de 16. VENTAJA real: el driver NO re-parsea uniforms: es solo
            memoria y un update de un solo rango.
       contra: layout fijo std140 (padding a mano) y no podés indexarlo con
            un uniform variable (array inline sí; el acceso "dinámico por
            buffer pointer" es del GL 4.x).
   Regla del curso (en tu máquina modesta): hasta 8 luces, uniform arrays;
   de ahí en adelante, UBO único de la escena que todos los shaders leen.

   --- ESTA DEMO --------------------------------------------------------------
   UN spot SOLO (sin otras luces: es la estrella) con esquinas suavizadas con
   el smoothstep del cono. Flechas IZQ/DHCA barren; ARRIBA/ABAJO abre el cono
   interno (el externo queda a +15). El plan de bytes std140 del bloque de
   luces se IMPRIME al arranque: es la tabla que reproduce el ejercicio 03
   en su UBO CPU-side.

   --- APIs usadas (verificadas en glcorearb.h local) -----------------------
   - glGetUniformBlockIndex + glUniformBlockBinding (núcleo GL 2.0/3.0,
     nombre intacto en GL 3.3 core/glvnd).
   - glBindBufferBase(GL_UNIFORM_BUFFER, binding, buffer) + glBufferSubData
     para actualizar el bloque (la 20 L07 describió los sub-rangos).
   ============================================================================--- APIs usadas (verificadas en glcorearb.h local) -----------------------
   - glGetUniformBlockIndex + glUniformBlockBinding (núcleo GL 2.0/3.0, con
     nombre que sigue igual en SDL3/GL 3.3).
   - glBindBufferBase(GL_UNIFORM_BUFFER, binding, buffer).
   - glBufferSubData para actualizar el bloque (20 L07 ya lo desnudó).
   ============================================================================
 */

#include "include/l21.hpp"

#include <cstring>
#include <iostream>

namespace {

/* --- Layout std140 del bloque (mismo diseño del ejercicio 03) -------------
   std140: un vec3 CONSUME 16 bytes (alineación de vec4); un float pegado a un
   vec3 NO entra en el relleno del vec3: la próxima alineación de struct es
   16, el array de structs debe un STRIDE múltiplo de 16. */
struct alignas(16) LuzStd140 {  // 32 bytes de stride (múltiplo de 16: OK)
    float color3[3];         // offset 0  (vec3: alinea 16, ocupa 12)
    float intensidad;        // offset 12 (float: entra al final del "bloque")
    float pos3[3];           // offset 16 (el siguiente vec3 abre un bloque nuevo)
    float alcance;           // offset 28 (float libre)
};  // stride: 32 (size redondeado a la alineación del struct: 16)
struct alignas(16) BloqueLuces {
    int cantidad;            // offset 0
    float relleno[3];        // offsets 4..15 (a mano; GLSL lo alinea solo)
    LuzStd140 luces[8];      // offset 16, stride 32
};  // sizeof = 16 + 8*32 = 272
static_assert(sizeof(BloqueLuces) == 272, "std140: revisar el padding");
static_assert(sizeof(LuzStd140) == 32, "std140: el struct luz mide 32");

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

/* FRAGMENTO SPOT standalone: SOLO el factor de cono cambia la 03. El resto
   de las luces queda igual (están en el UBO, que en esta demo no se lee:
   el spot ES la estrella). Como guía: PON LA COTA en el coseno si no entendés
   uno de las fracciones: smoothstep lo resuelve. */
const char* FS_SPOT = R"GLSL(
#version 330 core
in vec3 vPos;
in vec3 vNormal;
in vec2 vUV;
out vec4 FragColor;

uniform vec3  uPosCamara;
uniform float uBrillo;
uniform vec3  uAlbedoColor;

// SPOT
uniform vec3  uPosSpot;
uniform vec3  uDirSpot;         // ENTRES la cae (sin normalizar en chip)
uniform vec3  uColorSpot;
uniform vec3  uAtenSpot;        // (kc, kl, kq)
uniform float uCosInterno;      // coseno del borde INTERNO: luz PLENA
uniform float uCosExterno;      // coseno del borde EXTERNO: luz CERO

void main() {
    vec3 N = normalize(vNormal);
    vec3 V = normalize(uPosCamara - vPos);
    vec3 L = normalize(uPosSpot - vPos);

    // CONO: cuánto queda "dentro" (el coseno es mayor = ángulo más chico)
    // dirSpot señala HACIA el piso: apuntar desde la luz y desde el fragmento
    vec3 dirSpot = -normalize(uDirSpot);
    float cosTheta = dot(dirSpot, L);
    // smoothstep(edge0=cosExterno, edge1=cosInterno, x=cosTheta):
    // 0 fuera del cono externo, 1 dentro del interno, mix suave entremedio.
    float cono = smoothstep(uCosExterno, uCosInterno, cosTheta);

    // PHONG + atenuación 3-term, TODO multiplicado por el cono
    vec3 sobre = uColorSpot * max(dot(N, L), 0.0) * uAlbedoColor;
    vec3 reflejo = reflect(-L, N);
    vec3 spec = uColorSpot * pow(max(dot(reflejo, V), 0.0), uBrillo);
    float d = length(uPosSpot - vPos);
    float caida = 1.0 / (uAtenSpot.x + uAtenSpot.y * d
                         + uAtenSpot.z * d * d);
    vec3 color = (sobre + spec) * caida * cono;
    color += 0.08 * uAlbedoColor;          // ambient para no sumergir
    FragColor = vec4(color, 1.0);
}
)GLSL";

}  // namespace

/* Mando por teclado del CONO (en grados, se convierten en CPU a coseno). */
static float aCos(float grados) {
    return std::cos(grados * 3.14159265f / 180.0f);
}

int main() {
    // El layout std140 se imprime AL ARRANQUE: es la MISMA tabla que debe
    // reproducir el ejercicio 03.
    std::cout << "std140 del bloque Luces:\n";
    std::cout << "  LuzStd140: color[3]@0  intensidad@12  pos[3]@16  "
                 "alcance@28  | stride " << sizeof(LuzStd140) << "\n";
    std::cout << "  BloqueLuces: cantidad@0  relleno@4  luces[8]@16 "
              << "| TOTAL " << sizeof(BloqueLuces) << " bytes\n\n";

    glu::Contexto c = glu::abrir("M21 L04: spot con esquinas suaves", 800, 600);
    if (c.ventana == nullptr) return 1;
    glEnable(GL_DEPTH_TEST);

    GLuint prog = glu::enlazar(VS_FUENTE, FS_SPOT);
    if (prog == 0) { glu::cerrar(c); return 1; }

    glu::Uniform uModel, uView, uProj, uPosCamara, uBrillo, uAlbedo,
                 uPosSpot, uDirSpot, uColorSpot, uAtenSpot, uCosIn, uCosOut;
    uModel.buscar(prog, "uModel");    uView.buscar(prog, "uView");
    uProj.buscar(prog, "uProj");      uPosCamara.buscar(prog, "uPosCamara");
    uBrillo.buscar(prog, "uBrillo");  uAlbedo.buscar(prog, "uAlbedoColor");
    uPosSpot.buscar(prog, "uPosSpot"); uDirSpot.buscar(prog, "uDirSpot");
    uColorSpot.buscar(prog, "uColorSpot");
    uAtenSpot.buscar(prog, "uAtenSpot");
    uCosIn.buscar(prog, "uCosInterno"); uCosOut.buscar(prog, "uCosExterno");

    f21::Vec3 ojo{0.0f, 2.0f, 6.0f};
    f21::Mat4 vista = f21::lookAt(ojo, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f});
    f21::Mat4 proj = f21::perspectiva(55.0f,
        (float)c.ancho / (float)c.alto, 0.1f, 100.0f);

    f21::MallaGPU piso = f21::subirMalla(f21::cubo(1.0f));
    f21::Mat4 chapa = f21::Mat4::identidad();
    chapa.set(0, 0, 6.0f); chapa.set(1, 1, 0.15f); chapa.set(2, 2, 6.0f);
    f21::Mat4 modeloPiso = f21::trs({0.0f, -1.0f, 0.0f}, 1.0f, 0.0f) * chapa;

    f21::Vec3 posSpot{0.0f, 3.5f, 1.0f};

    float aperturaInterna = 20.0f;    // grados: luz PLENA hasta acá
    float ping = 0.0f;                // el spot barre con esta rotación

    const bool* teclas = SDL_GetKeyboardState(nullptr);
    while (glu::eventosOk()) {
        float dt = 1.0f / 60.0f;
        if (teclas[SDL_SCANCODE_LEFT])  ping -= 30.0f * dt;
        if (teclas[SDL_SCANCODE_RIGHT]) ping += 30.0f * dt;
        if (teclas[SDL_SCANCODE_UP])    aperturaInterna += 10.0f * dt;
        if (teclas[SDL_SCANCODE_DOWN])  aperturaInterna -= 10.0f * dt;
        if (aperturaInterna < 5.0f)  aperturaInterna = 5.0f;
        if (aperturaInterna > 70.0f) aperturaInterna = 70.0f;
        // el EXTERNO queda siempre 15° más abierto que el interno: el
        // "duro" que puede cambiar en el ejercicio 04e
        float aperturaExterna = aperturaInterna + 15.0f;
        if (aperturaExterna > 89.0f) aperturaExterna = 89.0f;

        // la linterna barre con la rotación por teclado
        f21::Vec3 d{std::sin(ping) * 0.5f, -1.0f, -0.3f};
        f21::Vec3 dirNorm = f21::normalizar(d);

        glClearColor(0.03f, 0.03f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(prog);
        f21::subirMat(uView.lugar, vista);
        f21::subirMat(uProj.lugar, proj);
        f21::subirMat(uModel.lugar, modeloPiso);
        f21::subirVec(uPosCamara.lugar, ojo);
        f21::subirVec(uPosSpot.lugar, posSpot);
        f21::subirVec(uDirSpot.lugar, dirNorm);
        f21::subirVec(uColorSpot.lugar, {1.0f, 0.85f, 0.55f});
        f21::subirVec(uAtenSpot.lugar, {1.0f, 0.14f, 0.07f});
        glUniform1f(uCosIn.lugar, aCos(aperturaInterna));
        glUniform1f(uCosOut.lugar, aCos(aperturaExterna));
        glUniform1f(uBrillo.lugar, 24.0f);
        f21::subirVec(uAlbedo.lugar, {0.75f, 0.75f, 0.78f});
        f21::dibujar(piso);
        SDL_GL_SwapWindow(c.ventana);
    }

    glDeleteProgram(prog);
    f21::borrarMalla(piso);
    glu::cerrar(c);
    std::cout << "Listo: el spot barre el piso. IZQ/DCHA rota, ARRIBA/ABAJO "
                 "abre y cierra el cono interno/externo.\n";
    return 0;
}
