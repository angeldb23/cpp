/* =============================================================================
   MÓDULO 21 - LECCIÓN 15: Cámara 3D libre: yaw + pitch, mouse y WASD
   =============================================================================
   TEMA 9 de TEMAS.md: "cámara 3D FPS: yaw/pitch, mouse capture y WASD".

   --- DE "dibujar" A "habitar" la escena -----------------------------------
   Todas las demos de módulo/lecciones previas usaban UN lookAt estático
   (ojo + centro fijados a mano). Una CÁMARA LIBRE necesita que UN PAR de
   números describa la DIRECCIÓN en la que mirás:
       yaw  (giro en el piso, alrededor de +Y, en grados)
       pitch (subir/bajar la vista, limitado, en grados)
   Con la CONVENCIÓN DEL CURSO (mundo +X derecha, +Y arriba, cámara por
   defecto mirando -Z) el vector "adelante" sale con senos/cosenos:
       ala = aRad(yaw), elp = aRad(pitch)
       adelantero = ( cos(pitch)*sin(yaw),  sin(pitch),
                     -cos(pitch)*cos(yaw) )    <- normalizado POR DISEÑO
     yaw=0, pitch=0 -> (0,0,-1): mirando -Z. yaw=90 -> +X (derecha).
     pitch=+90 -> (0,1,0): mirá al cielo (por eso se CLAMPEA antes).
   Y de ahí HACIA lookAt (de l21.hpp): la derecha es cross(adelante, arriba
   mundo) y el movimiento por teclado es suma en el plano PISO (adelante
   proyectado a Y=0, así caminar no te volás):
       adelantePiso = (sin(yaw), 0, -cos(yaw))
       derechaPiso  = (cos(yaw), 0,  sin(yaw))
   W/S: + adelantePiso  A/D: - derechaPiso  (el "strafe" del FPS).

   --- EL MOUSE: captura y delta por cuadro ------------------------------------
   SDL_SetWindowRelativeMouseMode(ventana, true) (SDL3: LA API de captura;
   en SDL2 era SDL_SetRelativeMouseMode): el cursor desaparece y
       SDL_GetRelativeMouseState(&mx, &my)
   da DELTAS desde el cuadro anterior (NO xy de pantalla: na de límites).
   Convertir: deltas crudos a grados:
       yaw   += (float)mx * SENSIB;      pitch -= (float)my * SENSIB;
   (mx=+ mueve el cursor a la derecha => mirar a la derecha => yaw sube;
    my=+ va hacia abajo => pitch BAJA: signo negado.)
   El CLAMP DEL PITCH es obligatorio: a ±89.9° lookAt con "arriba=(0,1,0)"
   DEGENERA (delante paralelo a arriba: cross de vectores colineales = 0:
   la matriz de cámara pierde rango y la vista se gorra: nada de render
   raro, el espacio de la proyección). Conceptual: es UNA línea de clamp.

   --- ESTA DEMO --------------------------------------------------------------
   La escena de pilares de SIEMPRE + cámara libre:
       MOUSE: girar      WASD: mover (relative al yaw)  [Q]/[E]: subir/bajar
       [SHIFT]: correr   [R]: toggle "modo teclas" (sin mouse, con flechas
       para máquinas sin buen mouse o trackpad angosto de notebook).
   ============================================================================ */

#include "include/l21.hpp"

#include <iostream>

namespace {

const char* VS_MUNDO = R"GLSL(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
uniform mat4 uModel;
uniform mat4 uMatVistaProj;
out vec3 vPos;
out vec3 vNormal;
void main() {
    vec4 mundo = uModel * vec4(aPos, 1.0);
    vPos = mundo.xyz;
    vNormal = mat3(uModel) * aNormal;
    gl_Position = uMatVistaProj * mundo;
}
)GLSL";

const char* FS_MUNDO = R"GLSL(
#version 330 core
in vec3 vPos;
in vec3 vNormal;
out vec4 FragColor;
void main() {
    // NADA de luz global: pintura plana (lo importante es la CÁMARA);
    float luz = 0.35f + 0.65f * max(normalize(vNormal).y, 0.0f);
    vec3 base = vec3(0.35f, 0.42f, 0.50f);
    // teñir los pilares según su x para VER la distancia/gira
    base = mix(base, vec3(0.55f, 0.30f, 0.20f),
               fract(vPos.x * 0.25f));
    FragColor = vec4(base * luz, 1.0);
}
)GLSL";

}  // namespace

int main() {
    std::cout << "Cámara libre: MOUSE girar, WASD mover, Q/E altura, "
              << "SHIFT correr, R flechas sin mouse.\n";

    glu::Contexto c = glu::abrir("M21 L15: cámara yaw/pitch + WASD",
                                 800, 550);
    if (c.ventana == nullptr) return 1;
    glEnable(GL_DEPTH_TEST);

    GLuint prog = glu::enlazar(VS_MUNDO, FS_MUNDO);
    if (prog == 0) { glu::cerrar(c); return 1; }

    glu::Uniform uModel, uMat;
    uModel.buscar(prog, "uModel");
    uMat.buscar(prog, "uMatVistaProj");

    /* --- El estado de la CÁMARA (lo mínimo que un FPS necesita) ------------- */
    float yawDeg = 0.0f, pitchDeg = 0.0f;
    f21::Vec3 posCam{0.0f, 1.8f, 8.0f};      // ojo de persona parada
    constexpr float SENSIB = 0.12f;          // grados por px del mouse
    constexpr float VEL = 3.5f;              // m/s al caminar

    /* La escena: piso + cantera de pilares al azar (grid con huecos). */
    f21::MallaGPU piso = f21::subirMalla(f21::cubo(0.5f));
    f21::MallaGPU pilar = f21::subirMalla(f21::cubo(0.5f));
    f21::Mat4 modeloPiso = f21::Mat4::identidad();
    modeloPiso.set(0, 0, 24.0f);
    modeloPiso.set(1, 1, 0.2f);
    modeloPiso.set(2, 2, 24.0f);
    modeloPiso.set(3, 1, -0.1f);      // piso al nivel = -0.1
    struct Pilar { float x, z, alto, yaw; };
    std::vector<Pilar> pilares;
    {
        for (int gx = -5; gx <= 5; ++gx)
        for (int gz = -5; gz <= 5; ++gz) {
            if (gx == 0 && gz >= -1 && gz <= 4) continue; // pasillo inicial
            if ((gx + gz * 3) % 5 != 0) continue;         // disperso
            pilares.push_back({gx * 2.2f, gz * 2.2f,
                               2.0f + 0.4f * (float)((gx * 7 + gz * 13) % 5),
                               (float)((gx * 5 + gz * 3) % 4) * 22.5f});
        }
    }
    std::cout << "pilares: " << pilares.size() << "\n";

    if (!SDL_SetWindowRelativeMouseMode(c.ventana, true)) {
        std::cout << "(sin captura de mouse: tipear R para el modo flechas)\n";
    }

    const bool* teclas = SDL_GetKeyboardState(nullptr);
    bool teclaRPrev = false;
    bool mouseCapturado = true;
    while (glu::eventosOk()) {
        float dt = 1.0f / 60.0f;

        /* MODO: mouse capturado vs teclado "sin mouse". */
        bool teclaR = teclas[SDL_SCANCODE_R];
        if (teclaR && !teclaRPrev) {
            mouseCapturado = !mouseCapturado;
            SDL_SetWindowRelativeMouseMode(c.ventana, mouseCapturado);
            std::cout << "modo: " << (mouseCapturado ? "mouse" : "flechas")
                      << "\n";
        }
        teclaRPrev = teclaR;

        if (mouseCapturado) {
            /* DELTA por cuadro (no posición): portátiles con DPI chico
               también sirven. */
            float mx = 0.0f, my = 0.0f;
            SDL_GetRelativeMouseState(&mx, &my);
            yawDeg  += mx * SENSIB;
            pitchDeg -= my * SENSIB;      // cursor abajo => mirar abajo
        } else {
            if (teclas[SDL_SCANCODE_LEFT])  yawDeg -= 120.0f * dt;
            if (teclas[SDL_SCANCODE_RIGHT]) yawDeg += 120.0f * dt;
            if (teclas[SDL_SCANCODE_UP])    pitchDeg += 90.0f * dt;
            if (teclas[SDL_SCANCODE_DOWN])  pitchDeg -= 90.0f * dt;
        }
        /* CLAMP DEL PITCH: sin esto lookAt degenera al mirar arriba. */
        if (pitchDeg >  89.0f) pitchDeg =  89.0f;
        if (pitchDeg < -89.0f) pitchDeg = -89.0f;

        /* VECTORES del movimiento en el PISO (sin el pitch: caminar y
           mirar son asuntos separados en un FPS). */
        float sy = std::sin(mate::aRadianes(yawDeg));
        float cy = std::cos(mate::aRadianes(yawDeg));
        f21::Vec3 adelantePiso{sy, 0.0f, -cy};
        f21::Vec3 derechaPiso{cy, 0.0f, sy};
        float vel = teclas[SDL_SCANCODE_LSHIFT] ? 2.2f * VEL : VEL;

        if (teclas[SDL_SCANCODE_W]) posCam += adelantePiso * (vel * dt);
        if (teclas[SDL_SCANCODE_S]) posCam -= adelantePiso * (vel * dt);
        if (teclas[SDL_SCANCODE_A]) posCam -= derechaPiso * (vel * dt);
        if (teclas[SDL_SCANCODE_D]) posCam += derechaPiso * (vel * dt);
        if (teclas[SDL_SCANCODE_Q]) posCam.y -= 2.0f * dt;
        if (teclas[SDL_SCANCODE_E]) posCam.y += 2.0f * dt;

        /* ADELANTE 3D con pitch+yaw (la fórmula del encabezado:
           cos(pitch) achata SOLO X y Z; el y puro sale de sin(pitch)). */
        float sp = std::sin(mate::aRadianes(pitchDeg));
        float cp = std::cos(mate::aRadianes(pitchDeg));
        f21::Vec3 delante{cp * sy, sp, -cp * cy};

        f21::Mat4 vista = f21::lookAt(posCam, posCam + delante,
                                      {0.0f, 1.0f, 0.0f});
        f21::Mat4 projCam = f21::perspectiva(55.0f,
            (float)c.ancho / (float)c.alto, 0.1f, 100.0f);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(prog);
        f21::subirMat(uMat.lugar, projCam * vista);
        f21::subirMat(uModel.lugar, modeloPiso);
        f21::dibujar(piso);
        for (auto const& p : pilares) {
            f21::Mat4 m = f21::trs({p.x, p.alto * 0.5f - 0.1f, p.z},
                                   1.0f, p.yaw);
            f21::Mat4 escala = f21::Mat4::identidad();
            escala.set(0, 0, 0.6f);
            escala.set(1, 1, p.alto);
            escala.set(2, 2, 0.6f);
            f21::subirMat(uModel.lugar, m * escala);
            f21::dibujar(pilar);
        }
        SDL_GL_SwapWindow(c.ventana);
    }

    glDeleteProgram(prog);
    f21::borrarMalla(piso);
    f21::borrarMalla(pilar);
    glu::cerrar(c);
    std::cout << "Listo: yaw/pitch -> delante -> lookAt; el movimiento usa"
                 " el plano piso para no mezclar con el pitch.\n";
    return 0;
}
