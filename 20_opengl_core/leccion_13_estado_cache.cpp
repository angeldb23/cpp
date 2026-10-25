/* =============================================================================
   MÓDULO 20 - LECCIÓN 13: Estado global (2/2) — state caching para reducir
   llamadas gl (el núcleo testeable del ejercicio 06)
   =============================================================================
   TEMA 8 de TEMAS.md (2 de 2): state caching para reducir draw calls de
   cambios de estado.

   CÓMO CORRERLA A MANO (desde la carpeta del módulo):
     g++ -std=c++20 -Wall -Wextra leccion_13_estado_cache.cpp \
         $(pkg-config --cflags --libs sdl3) -lOpenGL -o lec13 && ./lec13

   --- EL PROBLEMA: CAMBIAR de estado es más caro que el draw mismo ----------
   glDrawArrays de 6 vértices es casi gratis; el costo real de una lista
   de draws son los CAMBIOS de estado (glEnable/glDisable/glBlendFunc/
   glBind*) que obligan al driver a re-programar las unidades del GPU.
   El profiling del módulo 15 confirma la regla: el estado mata el frame
   time antes que los triángulos.

   --- LA SOLUCIÓN: primero Organizar + Cachear, después DIBUJAR ---------------------
   1) CACHE eu: el estado VIGENTE en el lado C++ (struct GLState).
      Antes de cualquier glCall COMPARABLE: "¿es igual a lo vigente?"
      -> si SÍ NO LLAMO NADA (el GL lo sabe: el estado es el persistent).
   2) ORDENAR los draws por CLAVE de estado (shader, textura, blend mode...):
      el sort del batch (17 batching manual / L14 instancing) reduce
      cambios agrupando draws que comparten clave (shader, textura, blend).

   La mecánica: mantengo en C++ el estado vigente; cuando alguien pide un
   estado nuevo, calculo la DIFFERENCIA y devuelvo SOLO los comandos que
   aplican los bits cambiados. Es el mismo patrón que un render graph
   real arma para el módulo 22.
   Esta estructura es el núcleo del ejercicio 06 (tests que cuentan
   comandos). La versión acá corre C++ puro: ni GPU ni OpenGL necesarios  */

#include "include/gl_util.hpp"
#include <iostream>
#include <string>
#include <vector>

/* ---------------------------------------------------------------------------
   NÚCLEO TESTEABLE: el gestor de estado con cache de diff (C++ puro).
 ------------------------------------------------------------------------- */
enum class BlendModo { Inactivo, Alfa, Aditivo };

struct EstadoGL {                       // SOLO los campos que usamos (sub-set)
    int    shaderPrograma   = -1;       // -1 = "ninguno aún"
    int    texturaEnlazada  = -1;
    int    viewportLetra[4] = { -1, -1, -1, -1 };   // x, y, w, h
    bool   blendActivo      = false;
    BlendModo blend          = BlendModo::Inactivo;
    bool   depthActivo      = false;
};

class GestorEstado {
public:
    explicit GestorEstado(const EstadoGL& vigente) : cache(vigente) {}

    static std::string modoStr(BlendModo m) {
        switch (m) {
            case BlendModo::Inactivo: return "";
            case BlendModo::Alfa:     return "glBlendFunc(SRC_ALPHA, ONE_MINUS_SRC_ALPHA)";
            case BlendModo::Aditivo:  return "glBlendFunc(ONE, ONE)";
        }
        return "";
    }

    /* Recibe el estado QUE QUERÉS y devuelve SOLO los comandos GL que
       aplicarlo a la cache. Idempotente: con el mismo estado -> vacío. */
    std::vector<std::string> aplicar(const EstadoGL& nuevo) {
        std::vector<std::string> cmds;
        cmds.reserve(6);

        if (nuevo.blend != cache.blend) {
            if (nuevo.blend == BlendModo::Inactivo)
                cmds.push_back("glDisable(GL_BLEND)");
            else {
                if (!cache.blendActivo)
                    cmds.push_back("glEnable(GL_BLEND)");
                std::string s = modoStr(nuevo.blend);
                if (!s.empty()) cmds.push_back(s);
            }
        }
        if (nuevo.depthActivo != cache.depthActivo)
            cmds.push_back(nuevo.depthActivo
                ? "glEnable(GL_DEPTH_TEST)" : "glDisable(GL_DEPTH_TEST)");
        if (nuevo.shaderPrograma != cache.shaderPrograma)
            cmds.push_back("glUseProgram(" + std::to_string(nuevo.shaderPrograma) + ")");
        if (nuevo.texturaEnlazada != cache.texturaEnlazada)
            cmds.push_back("glBindTexture(GL_TEXTURE_2D, "
                           + std::to_string(nuevo.texturaEnlazada) + ")");
        if (nuevo.viewportLetra[2] != cache.viewportLetra[2]
            || nuevo.viewportLetra[3] != cache.viewportLetra[3])
            cmds.push_back("glViewport(" + std::to_string(nuevo.viewportLetra[0])
                         + "," + std::to_string(nuevo.viewportLetra[1])
                         + "," + std::to_string(nuevo.viewportLetra[2])
                         + "," + std::to_string(nuevo.viewportLetra[3]) + ")");

        cache = nuevo;    // tras aplicar: el GL ya conoce este estado
        return cmds;
    }

    const EstadoGL& cacheVigente() const { return cache; }

private:
    EstadoGL cache;
};

/* --- Demo: 3 draws con estados repetidos -> pocas llamadas gl -------------- */
int main() {
    GestorEstado g(EstadoGL{});    // arranca "not yet": nada enlazado

    // Draw 1: el estado inicial completo
    EstadoGL a;
    a.shaderPrograma = 7; a.texturaEnlazada = 3; a.blend = BlendModo::Alfa;
    a.blendActivo = true; a.viewportLetra[2] = 800; a.viewportLetra[3] = 600;
    auto cs = g.aplicar(a);
    for (const std::string& c : cs) std::cout << c << "\n";
    std::cout << "--(draw 1: " << cs.size() << " comandos)\n";

    // Draw 2: el MISMO estado -> 0 comandos (el cache paga!)
    auto cs2 = g.aplicar(a);
    std::cout << "--(draw 2 con el mismo estado: " << cs2.size() << " comandos)\n";
    std::cout << "draw repetido debe ser 0: " << (cs2.empty() ? "OK" : "BUG") << "\n";

    // Draw 3: SOLO blend aditivo
    EstadoGL b = a;
    b.blend = BlendModo::Aditivo;
    auto cs3 = g.aplicar(b);
    std::cout << "--(draw 3 delta: " << cs3.size() << " comando(s))\n";
    for (const std::string& c : cs3) std::cout << "   " << c << "\n";

    std::cout << "(los comandos son strings para leer el delta con ojos: el render\n"
              << " graph del módulo 22 usa este MISMO patrón; el ejercicio 06 prueba\n"
              << " la corrección de los deltas con tests).\n";
    return 0;
}
