/* =============================================================================
   MÓDULO 21 - LECCIÓN 13: Compute shaders (concepto) y lo que el 3.3 SÍ puede
   =============================================================================
   TEMA 7 de TEMAS.md: "compute shaders: concepto y por qué no en 3.3
   (requeriría GL 4.3/ARB_compute_shader); migración de cargas con instancing".

   --- EL CONCEPTO (aunque el hardware de la notebook no lo corra) -----------
   Hasta acá TODO shader se cuelga de la línea de dibujo: hay vértices,
   hay primitivas, hay fragmentos, hay un framebuffer. Un COMPUTE SHADER
   rompe eso: es un programa que el GPU corre sobre cuántos "grupos de
   trabajo" (workgroups) le pidas, SIN geometría y SIN framebuffer:
       glDispatchCompute(16, 9, 1);    // 16x9x1 workgroups, core en GL 4.3
       // dentro: uint idx = gl_GlobalInvocationID.x;
       // escribo lo que quiero en un SSBO (Shader Storage Buffer Object)
   Las cargas típicas de un juego que se pasan a compute:
     - CULLING: descartar qué meshes entran en la vista (CPU manda un
       plan de draw que el GPU consume: multi-draw indirecto, GL 4.x).
     - PARTÍCULAS/FÍSICA: posiciones y velocidades en un SSBO, sin eje.
     - GENERACIÓN PROCEDURAL: ruido, terreno, texturas comprimidas.
   La estructura de datos: buffers escritos por el shader (SSBOs, GL 4.x;
   en 4.3 es ARB_shader_storage_buffer_object) + memoria coherente entre
   dispatches con glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT).

   --- ¿POR QUÉ NO EN 3.3? (la respuesta arranca acá) ----------------------
   GL 3.3 (2010) NO trae: glDispatchCompute (core 4.3), SSBO (4.3), ni
   imagenLoad/imagenStore (4.2). Es más: en la notebook acá ni el perfil
   del driver viejito garantiza ARB_compute_shader. "Pero puedo comprar la
   extensión"? NO: la base del módulo es LO QUE HAY, no lo que se anhela.
   El método honesto: en runtime PREGUNTAR con
       glGetIntegerv(GL_NUM_EXTENSIONS, &n);      // core desde 3.0
       for (i < n) glGetStringi(GL_EXTENSIONS, i); // core desde 3.0
   y decidir según lo que SALGA: ese chequeo es la mitad de esta lección.

   --- Lo que 3.3 SÍ da para "cuál es el trabajo": INSTANCING ----------------
   Para 1000-4000 copias DE UNA MISMA malla (espadas, hierba, árboles), el
   compute no es la única vía: glDrawArraysInstanced / glDrawElementsInstanced
   (core en 3.1-3.3) dibuja N copias en UNA sola llamada, y el VS sabe cuál
   corriendo por gl_InstanceID (core en 3.3). Cada copia arma su model desde
   una UBO/array de transforms. Para "muchos objetos pequeños" es el 90%
   de la ganancia, SIN compute. (Los datos por instancia en 3.3 VAN EN UN
   UBO al que el shader lo indexa: hasta 4.3 no hay atributos de instancia
   divisor (llegan recién con esos perfiles; buen detalle para el ejercicio 08).

   --- ESTA DEMO -------------------------------------------------------------
   1. ESCANEÁ LAS EXTENSIONES presentes en el contexto 3.3 de TU marca de
      drivers: imprime cuántas hay y si están las que a compute le
      falta/necesita (ARB_compute_shader, ARB_shader_storage... casi
      seguro que NO: el punto es verificar con datos, no con dudas).
   2. DEMO instanced: UNA sola malla de esferita, 3000 copias del grid con
      glDrawElementsInstanced + gl_InstanceID: 3000 modelos en 1 draw, CERO
      calls extra de CPU entre cuadros.
   ============================================================================ */

#include "include/l21.hpp"

#include <iostream>
#include <string>
#include <vector>

namespace {

/* Extensiones "que compute necesita" (por nombre exacto de ARB). */
std::vector<std::string> SI_COMPUTE = {
    "GL_ARB_compute_shader",
    "GL_ARB_shader_storage_buffer_object",
    "GL_ARB_shader_image_load_store",
    "GL_ARB_shader_image_size",
    "GL_ARB_explicit_uniform_location",};

const char* VS_INST = R"GLSL(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUV;
#define MAXI 3000
uniform mat4 uMatVistaProj;          // cámara
uniform vec3 uPosiciones[MAXI];      // por instancia (UBO del mundo real)
uniform float uRadio;
out vec3 vNormal;
out vec2 vUV;
void main() {
    // gl_InstanceID: core en 3.3. La copia #i se arma SU model acá mismo.
    vec3 pos = uPosiciones[gl_InstanceID];
    vec3 local = vec3(aPos.xy, aPos.z * 0.6f) * uRadio;
    vec3 mundo = pos + local;
    vNormal = aNormal;
    vUV = aUV;
    gl_Position = uMatVistaProj * vec4(mundo, 1.0);
}
)GLSL";

const char* FS_INST = R"GLSL(
#version 330 core
in vec3 vNormal;
in vec2 vUV;
out vec4 FragColor;
void main() {
    // gradiente por UV + leve top-light: suficiente para ver las 3000
    vec3 a = vec3(0.22f, 0.46f, 0.38f);
    vec3 b = vec3(0.09f, 0.18f, 0.24f);
    vec3 c = mix(a, b, fract(vUV.x * 3.0f));
    c *= 0.6f + 0.4f * max(vNormal.y, 0.0f);
    FragColor = vec4(c, 1.0);
}
)GLSL";

}  // namespace

int main() {
    std::cout << "M21 L13: compute (concepto) vs lo que 3.3 SÍ puede "
                 "(instancing).\n\n";

    glu::Contexto c = glu::abrir("M21 L13: compute inspect + instancing",
                                 800, 550);
    if (c.ventana == nullptr) return 1;
    glEnable(GL_DEPTH_TEST);

    /* --- 1) ESCANEO DE EXTENSIONES (glGetStringi + NUM_EXTENSIONS) ------ */
    GLint numExt = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &numExt);
    std::cout << "== Extensiones del contexto (glGetStringi, core 3.0) ==\n";
    std::cout << "   " << numExt << " extensiones.\n";
    std::vector<std::string> encontradas;
    for (GLint i = 0; i < numExt; ++i) {
        const char* ext = (const char*)glGetStringi(GL_EXTENSIONS, (GLuint)i);
        if (ext != nullptr) encontradas.emplace_back(ext);
    }
    auto tiene = [&](std::string const& n) {
        for (auto const& e : encontradas) if (e == n) return true;
        return false;
    };
    const unsigned char* verGL = glGetString(GL_VERSION);
    const unsigned char* verShl = glGetString(GL_SHADING_LANGUAGE_VERSION);
    std::cout << "   GL: " << (verGL ? (const char*)verGL : "?") << "\n";
    std::cout << "   GLSL: " << (verShl ? (const char*)verShl : "?") << "\n";
    std::cout << "   Las piezas de compute:\n";
    for (auto const& n : SI_COMPUTE) {
        bool si = tiene(n);
        std::cout << "   " << (si ? "[X] SI" : "[ ] NO") << "  " << n << "\n";
    }
    bool computable = tiene("GL_ARB_compute_shader");
    std::cout << "   VEREDICTO: glDispatchCompute en 3.3 core es "
              << (computable ? "POSIBLE (si la extensión está)"
                             : "IMPOSIBLE en este driver (lo esperado)") << "\n\n";

    /* --- 2) DEMO instanced: N esferitas en UN draw ----------------------- */
    constexpr int N_INST = 3000;
    GLuint prog = glu::enlazar(VS_INST, FS_INST);
    if (prog == 0) { glu::cerrar(c); return 1; }

    glu::Uniform uMat, uRadio;
    uMat.buscar(prog, "uMatVistaProj");
    uRadio.buscar(prog, "uRadio");

    f21::Vec3 ojo{0.0f, 6.0f, 14.0f};
    f21::Mat4 vistaCam = f21::lookAt(ojo, {0.0f, 0.0f, 0.0f},
                                     {0.0f, 1.0f, 0.0f});
    f21::Mat4 projCam = f21::perspectiva(55.0f,
        (float)c.ancho / (float)c.alto, 0.1f, 100.0f);

    f21::MallaGPU esferita = f21::subirMalla(f21::esfera(1.0f, 8, 4));

    /* grid 25x25x... no: dispersas en disco con radios suave? grid simple */
    std::vector<f21::Vec3> posiciones(N_INST);
    int grilla = 55;                      // 55x55 = 3025 >= 3000
    float paso = 0.45f;
    for (int i = 0; i < N_INST; ++i) {
        int gx = i % grilla, gz = i / grilla;
        posiciones[i] = { (float(gx) - grilla * 0.5f) * paso,
                          0.15f,
                          (float(gz) - N_INST / grilla * 0.5f) * paso};
    }
    GLuint posUbo = 0;
    glGenBuffers(1, &posUbo);
    glBindBuffer(GL_UNIFORM_BUFFER, posUbo);
    glBufferData(GL_UNIFORM_BUFFER,
                 (GLsizeiptr)(posiciones.size() * sizeof(f21::Vec3)),
                 posiciones.data(), GL_STATIC_DRAW);

    const bool* teclas = SDL_GetKeyboardState(nullptr);
    bool teclaIPrev = false;
    int instanciasVisibles = N_INST;
    std::cout << "   3000 esferas en grid: UN call de glDrawElementsInstanced"
              << " (tecla I saca/repone la mitad para el scope del culling).\n\n";
    while (glu::eventosOk()) {
        bool teclaI = teclas[SDL_SCANCODE_I];
        if (teclaI && !teclaIPrev) {
            instanciasVisibles = (instanciasVisibles == N_INST)
                                     ? N_INST / 2 : N_INST;
            std::cout << "instancias: " << instanciasVisibles
                      << " (mismo 1 draw del GPU)\n";
        }
        teclaIPrev = teclaI;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(prog);
        f21::subirMat(uMat.lugar, projCam * vistaCam);
        glUniform1f(uRadio.lugar, 0.18f);
        glUniform3fv(glGetUniformLocation(prog, "uPosiciones"),
                     N_INST, (const GLfloat*)posiciones.data());
        /* LO IMPORTANTE: 1 sola llamada dibuja N copias. */
        glDrawElementsInstanced(GL_TRIANGLES, esferita.indices,
                                GL_UNSIGNED_INT, nullptr, instanciasVisibles);
        SDL_GL_SwapWindow(c.ventana);
    }

    glDeleteBuffers(1, &posUbo);
    glDeleteProgram(prog);
    f21::borrarMalla(esferita);
    glu::cerrar(c);
    std::cout << "Listo: sin compute, 3000 modelos en 1 draw con "
                 "glDrawElementsInstanced + gl_InstanceID (core 3.3);\n"
                 "el scan imprimió la verdad del driver: si ARB_compute_"
                 "shader no está, NADA de dispatch.\n";
    return 0;
}
