/* =============================================================================
   MÓDULO 21 - LECCIÓN 03: Tipos de luz 1: direccional y puntual (atenuación)
   =============================================================================
   TEMA 2 de TEMAS.md (primera de 2 lecciones): "Luces: direccional, puntual,
   spot; atenuación (constante/lineal/cuadrática) y uniforms arrays vs UBO".
   El spot y el debate array-vs-UBO completan la LECCIÓN 04.

   CÓMO CORRERLA A MANO:
     g++ -std=c++20 -Wall -Wextra leccion_03_luces_direccional_puntual.cpp \
         $(pkg-config --cflags --libs sdl3) -lOpenGL -o lec03 && ./lec03

   --- DIRECCIONAL: luz SIN posición -----------------------------------------
   El sol real es un punto a 150 millones de km: a la escala de tu escena es
   "infinitamente lejos": TODOS los fragmentos reciben el vector de llegada
   casi IGUAL. El modelo entonces guarda SOLO la dirección de la luz (un vec3
   unitario) y aplica
       L = -normalize(dirLuz)        // invertida: dirección "hacia abajo"
       dif = max(dot(N, L), 0.0);
   Nota de dirección: se guarda la dirección HACIA DONDE CAE la luz (la
   convención común): por eso se invierte al usarla como vector L de llegada.
   Consecuencia del modelo: la difusa no depende ni de la distancia ni de la
   posición (el "lugar" desaparece): una mancha de luz que camine por el piso
   NO puede salir de una direccional pura: para eso, puntual.

   --- PUNTUAL: luz CON posición Y CAÍDA --------------------------------------
   Bombilla real: cuanto más lejos, MENOS intensidad. La atenuación se modela
   con la fórmula "de 3 términos" (el inverso al cuadrado real, más un tramo
   lineal que lo hace menos agresivo):
       atenuacion = 1.0 / (kc + kl * d + kq * d * d)
       kc = constante  (1.0 típico: nunca 0, divisor cero)
       kl = lineal     (caída suave de media distancia)
       kq = cuadrática (caída violenta en distancias cortas)
   d = distancia fragmento-luz. La misma atenuación se aplica al diffuse y al
   specular (la mancha de brillo se apaga con la distancia igual que la luz;
   excluirla del spec es una decisión artística que retoma la lección 04).
   Valores de referencia (tabla clásica de luces puntuales):
       alcance ~7m:   (1.0, 0.70, 1.8)
       alcance ~13m:  (1.0, 0.35, 0.44)
       alcance ~50m:  (1.0, 0.09, 0.032)
   OJO: la caída es pronunciada: con kq alto la luz muere pronto y SIN SALTO
   (a d grande la intensidad tiende suavemente a 0; si el ambient es 0.0 se
   nota como "bolsa de oscuridad" alrededor de cada bombilla).

   --- LAS DOS EN EL MISMO SHADER (y por qué no se pisan) ---------------------
   El fragment es un ACUMULADOR: arranca con el ambient y se SUMAN los
   términos de cada luz. N luces => el shader se comporta igual, solo suma
   más términos (la suma de colores es conmutativa: el orden no importa).
   El costo se paga POR LUZ (position, dot, pow por fragmento): el módulo 16
   midió que cada luz extra consume frame time: con 4-8 ya se nota en una
   GPU integrada.

   --- ESTA DEMO --------------------------------------------------------------
   Un piso + una luz direccional fría (el "sol de noche") + 4 luces puntuales
   orbitando con colores y alcances distintos. Las 4 esferas marcan DÓNDE está
   cada bombilla (y SON material del tipo "con solo ambiente"). Teclas 1-4
   cambian la cantidad de luces activas: mirá cómo el piso pierde vida.
   ============================================================================
 */

#include "include/l21.hpp"

#include <iostream>

namespace {

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

/* FRAGMENTO: direccional + puntuals (array de uniforms GLSL; la versión
   UBO es la lección siguiente). El loop usa #define MAXP fijado en tiempo de
   COMPILACIÓN y un uniform int uCantidad para variable el límite real. */
const char* FS_LUCES = R"GLSL(
#version 330 core
in vec3 vPos;
in vec3 vNormal;
in vec2 vUV;
out vec4 FragColor;

#define MAXP 4

// DIRECCIONAL: solo dirección (el "sol" de la escena)
uniform vec3  uDirLuz;        // dirección HACIA DONDE cae la luz
uniform vec3  uColorDir;

// PUNTUAL (x MAXP): posición + color + 3 términos de atenuación
uniform int   uCantidad;      // cuántas puntuales HAY (<= MAXP)
uniform vec3  uPosP[MAXP];
uniform vec3  uColorP[MAXP];
uniform vec3  uAtenP[MAXP];   // por luz: (kc, kl, kq)

uniform vec3  uPosCamara;
uniform float uBrillo;
uniform vec3  uAlbedoColor;

vec3 contribucionPuntual(int i, vec3 N, vec3 V) {
    vec3 L = normalize(uPosP[i] - vPos);            // fragmento -> luz
    vec3 sobre = uColorP[i] * max(dot(N, L), 0.0);  // diffuse de la luz i
    // el spec SE ATENÚA igual: la mancha de brillo se apaga con d
    vec3 reflejo = reflect(-L, N);
    vec3 spec = uColorP[i] * pow(max(dot(reflejo, V), 0.0), uBrillo);
    float d = length(uPosP[i] - vPos);
    float caida = 1.0 / (uAtenP[i].x + uAtenP[i].y * d
                         + uAtenP[i].z * d * d);
    return (sobre + spec) * caida;
}

void main() {
    vec3 N = normalize(vNormal);
    vec3 V = normalize(uPosCamara - vPos);

    // DIRECCIONAL: L es la dirección INVERTIDA, sin posición
    vec3 Ld = -normalize(uDirLuz);
    vec3 color = uColorDir * max(dot(N, Ld), 0.0) * uAlbedoColor;
    vec3 reflejoD = reflect(-Ld, N);
    color += uColorDir * 0.30
           * pow(max(dot(reflejoD, V), 0.0), uBrillo);

    // PUNTUALES: ambient + suma con atenuación por luz
    color += 0.08 * uAlbedoColor;                 // ambient
    for (int i = 0; i < MAXP; ++i) {
        if (i >= uCantidad) break;                // límite REAL (uniform)
        color += contribucionPuntual(i, N, V);
    }
    FragColor = vec4(color, 1.0);
}
)GLSL";

}  // namespace

int main() {
    glu::Contexto c = glu::abrir("M21 L03: direccional + 4 puntuales", 900, 600);
    if (c.ventana == nullptr) return 1;
    glEnable(GL_DEPTH_TEST);

    GLuint prog = glu::enlazar(VS_FUENTE, FS_LUCES);
    if (prog == 0) { glu::cerrar(c); return 1; }

    glu::Uniform uModel, uView, uProj, uDirLuz, uColorDir, uCantidad,
                 uPosCamara, uBrillo, uAlbedoColor;
    uModel.buscar(prog, "uModel");   uView.buscar(prog, "uView");
    uProj.buscar(prog, "uProj");     uDirLuz.buscar(prog, "uDirLuz");
    uColorDir.buscar(prog, "uColorDir");
    uCantidad.buscar(prog, "uCantidad");
    uPosCamara.buscar(prog, "uPosCamara");
    uBrillo.buscar(prog, "uBrillo");
    uAlbedoColor.buscar(prog, "uAlbedoColor");
    GLint posP = glGetUniformLocation(prog, "uPosP");
    GLint colorP = glGetUniformLocation(prog, "uColorP");
    GLint atenP = glGetUniformLocation(prog, "uAtenP");

    // luces orbitando con colores y alcances distintos. Piso + 4 esferas.
    f21::MallaGPU piso = f21::subirMalla(f21::cubo(1.0f));
    f21::MallaGPU farol = f21::subirMalla(f21::esfera(0.35f));

    f21::Vec3 ojo{0.0f, 2.2f, 7.0f};
    f21::Mat4 vista = f21::lookAt(ojo, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f});
    f21::Mat4 proj = f21::perspectiva(55.0f,
        (float)c.ancho / (float)c.alto, 0.1f, 100.0f);

    // Luces puntuales: 4, cada una con SU combinación de atenuación de la
    // tabla (13m, 20m, 32m, 50m de alcance): mirá el "radio visible" de cada
    // una en el piso y cómo el corto MATA su luz a pocos metros.
    struct Puntual { f21::Vec3 color; float kc, kl, kq; float radio; } luces[] = {
        {{1.0f, 0.25f, 0.30f}, 1.0f, 0.70f, 1.8f,  3.0f},   // corto (~7m)
        {{0.25f, 1.0f, 0.40f}, 1.0f, 0.35f, 0.44f, 5.0f},   // medio (~13m)
        {{0.30f, 0.55f, 1.0f}, 1.0f, 0.14f, 0.07f, 7.0f},   // largo (~32m)
        {{1.0f, 0.90f, 0.35f}, 1.0f, 0.09f, 0.032f, 9.0f},  // muy largo (~50m)
    };
    int cuantasLuces = 4;
    float t = 0.0f;
    f21::Vec3 dirSol{0.45f, -1.0f, 0.25f};   // cae "hacia abajo-derecha"

    const bool* teclas = SDL_GetKeyboardState(nullptr);
    while (glu::eventosOk()) {
        float dt = 1.0f / 60.0f;
        t += dt;
        if (teclas[SDL_SCANCODE_1]) cuantasLuces = 1;
        if (teclas[SDL_SCANCODE_2]) cuantasLuces = 2;
        if (teclas[SDL_SCANCODE_3]) cuantasLuces = 3;
        if (teclas[SDL_SCANCODE_4]) cuantasLuces = 4;

        glClearColor(0.05f, 0.05f, 0.07f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(prog);

        f21::subirMat(uView.lugar, vista);
        f21::subirMat(uProj.lugar, proj);
        f21::subirVec(uPosCamara.lugar, ojo);
        f21::subirVec(uDirLuz.lugar, dirSol);
        f21::subirVec(uColorDir.lugar, {0.55f, 0.55f, 0.62f});
        glUniform1i(uCantidad.lugar, cuantasLuces);
        glUniform1f(uBrillo.lugar, 24.0f);
        f21::subirVec(uAlbedoColor.lugar, {0.7f, 0.72f, 0.75f});

        // SOLO las posiciones (y colores) de luz cambian por frame: mandamos
        // UNIFORM ARRAYS con glUniform3fv(pos, n, puntero) — n luces de una.
        f21::Vec3 posiciones[4], colores[4], atenciones[4];
        for (int i = 0; i < 4; ++i) {
            float rad = t * 0.7f + 1.57079637f * (float)i;
            posiciones[i] = { std::cos(rad) * luces[i].radio, 1.6f,
                              std::sin(rad) * luces[i].radio };
            colores[i] = luces[i].color;
            atenciones[i] = {luces[i].kc, luces[i].kl, luces[i].kq};
        }
        glUniform3fv(posP, 4, &posiciones[0].x);
        glUniform3fv(colorP, 4, &colores[0].x);
        glUniform3fv(atenP, 4, &atenciones[0].x);

        // PISO: cubo "chapa" con escala NO uniforme (matriz aparte a mano:
        // la normal queda débil pero con el normalize del fragment basta
        // porque el piso solo se aplana en Y).
        {
            f21::Mat4 chapa = f21::Mat4::identidad();
            chapa.set(0, 0, 12.0f); chapa.set(1, 1, 0.15f); chapa.set(2, 2, 12.0f);
            f21::Mat4 m = f21::trs({0.0f, -1.1f, 0.0f}, 1.0f, 0.0f) * chapa;
            f21::subirMat(uModel.lugar, m);
            f21::dibujar(piso);
        }
        // 4 ESFERAS "farolitos" marcando la posición de cada bombilla
        for (int i = 0; i < cuantasLuces; ++i) {
            f21::subirMat(uModel.lugar,
                          f21::trs(posiciones[i], 1.0f, 0.0f));
            f21::dibujar(farol);
        }
        SDL_GL_SwapWindow(c.ventana);
    }

    glDeleteProgram(prog);
    f21::borrarMalla(piso);
    f21::borrarMalla(farol);
    glu::cerrar(c);
    std::cout << "Listo: direccional (sol, SIN caída) + 4 puntuales orbitando "
                 "con atenuación 1/(kc+kl*d+kq*d2). Teclas 1-4 apagan luces.\n";
    return 0;
}
