/* =============================================================================
   MÓDULO 20 - LECCIÓN 09: Carga de imágenes con stb_image (header-only) y
   su subida a una textura OpenGL
   =============================================================================
   TEMA 6 de TEMAS.md: stb_image (header-only), formatos, canales, HDR y
   subida a textura OpenGL.

   --- DÓNDE ESTÁ stb_image ----------------------------------------------------
   Repo oficial: github.com/nothings/stb (Sean Barrett). El archivo que
   usamos es stb_image.h EN LA RAIZ del repo. Verificado en la sesión de
   construcción:
     - stb NO publica versiones ni tags oficial: LA MANERA REAL de fijarlo
       es un COMMIT CONCRETO. Tag usado (GIT_TAG):
         a7d4c2175630e1ea5b6f8dd00ab96fed0b97b96d
       (resuelto con git ls-remote HEAD; un commit fijo de GitHub es
       (un commit es inmutable: un "master" congelado a la fecha).
     - El CMakeLists del módulo lo sirve con FetchContent + include dir
       ${stb_SOURCE_DIR} (basta con eso: header-only).
   ¿POR QUÉ stb_image y no una lib con .so? Porque es UNA CABECERA: no
   linkeo, no dependencia de sistema, y el dueño puede compilar todo con
   "g++ leccion_09 ... ": ¡sin dancing con pacman! (El curso C++ puro: es el
   objetivo de 12 y FS ("árbiles de assets reales, no me pasen por uloc").)

   --- COMO FUNCIONA (y sus 3 llamadas god ones) ------------------------------
   #define STB_IMAGE_IMPLEMENTATION
   #include "stb_image.h"
   (la implementación se compila UNA vez, en el .cpp del main)

     stbi_load(ruta, &ancho, &alto, &canales, cargaCanales)
       -> unsigned char* (memoria del heap C), o nullptr si no cargó.
         - cargaCanales 0: lo que trae el archivo (JPG=3, PNG=3/4...).
         - cargaCanales 4: FORZAR 4 bytes/pixel (RGBA): SI es el clásico y
           la recomendación del curso (driver feliz con alfa siempre).
         - los datos siguen ROW-MAJOR de arriba a abajo (GL texturas
           manejan el orden "bottom-left": refactor del estado de la
           levección del stb: stbi_set_flip_vertically_on_load(1) y listo.
       stbi_image_free(void*) es el free COMPLEMENTARIO de stbi_load: esa
       memoria se libera con stbi_image_free, NUNCA con delete[] (es una
       alloc de C). Para RAII real: unique_ptr con deleter custom del
       módulo 07.

   --- SUBIDA A TEXTURA (lección 10 entra a fondo en wrap/filter/mip) ---------
   glGenTextures(1, &tex); glBindTexture(GL_TEXTURE_2D, tex);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, ancho, alto, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, datos);
     - internalformat vs format: primero ESCÓNDIDO cómo se guarda en la
       GPU de la textura (canales por pixel + tipo, juegos con "4 bytes" es
       "una textura ancha"), segundo decimos cómo venirlamos el array C++.
       (Con stb forzado a 4 canales: internal GL_RGBA8 + format GL_RGBA ✔.)
     - El borde (0) es un legacy del GL 1.x: SIEMPRE 0 en core 3.3.
     - internal GL_SRGB8_ALPHA8 (sRGB, gamma-aware): va en la lección 11.
       Acá GL_RGBA8 "lineal" con el color discreto en pantalla, suficiente
       para todos los ejemplos del módulo.

   --- FORMATOS EXÓTICOS (menciones) -------------------------------------------
   - JPG/PNG/BMP/TGA/PSD/PIC/GIF... que stb abre (lista en el header).
   - HDR: stbi_loadf (floats) para SHDR/HDR de .hdr/.exr; core 3.3 soporta
     internal GL_RGBA16F (con GL_FLOAT en el ancho/16f). Mención, no uso.
   - KTX/DDS COMPRESIONADO (GL_COMPRESSED_RGBA_S3TC...): no lo soporta
     stb_image acá; texto mención en la L10 y tema del module 25.
   --------------------------------------------------------------------------- */

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"            // traído por FetchContent (ver arriba)

#define STB_IMAGE_WRITE_IMPLEMENTATION
// stb headers disparan avisos -Wmissing-field-initializers dentro de su
// propio código (terceros): los silenciamos solo alrededor del include.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#include "stb_image_write.h"      // para GENERAR el png de demo (mismo repo)
#pragma GCC diagnostic pop

#include "include/gl_util.hpp"
#include <filesystem>
#include <iostream>

int main() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cout << "SDL_Init fallo: " << SDL_GetError() << "\n";
        return 1;
    }

    glu::Contexto c = glu::abrir("Modulo 20 - L09 stb_image -> textura", 640, 480);
    if (c.contexto == nullptr) { SDL_Quit(); return 1; }

    /* Generamos UNA imagen RGBA 2x2 de prueba (el curso ya la usa para
       moverse: es el floor del texto RAW): 4 píxeles, uno por color.    */
    const std::string ruta = "test_img_2x2.png";
    if (!std::filesystem::exists(ruta)) {
        // Nota: escribir un PNG a mano es trabajo (códigos stbi_write_png
        // sí existen y sí los muestra aclarado en la lección); acá usamos
        // stbi_write_png para CHEQUEAR: la naturaleza de la imagen es la
        // CARGA (stbi_load) y su subida a GL (glTexImage2D).
        unsigned char px[2][2][4] = {   // RGBA
            { {255,   0,   0, 255}, {  0, 255,   0, 255} },   // fila 0
            { {  0,   0, 255, 255}, {255, 255,   0, 255} },   // fila 1
        };
        stbi_write_png(ruta.c_str(), 2, 2, 4, px, 2 * 4);
    }

    // ---- CARGA ------------------------------------------------------------
    stbi_set_flip_vertically_on_load(1);   // flip Y: origen abajo (GL)
    int ancho = 0, alto = 0, canalesArchivo = 0;
    unsigned char* datos = stbi_load(ruta.c_str(), &ancho, &alto,
                                     &canalesArchivo, 4);   // forzar RGBA
    if (datos == nullptr) {
        std::cout << "stbi_load fallo: " << stbi_failure_reason() << "\n";
        glu::cerrar(c);
        SDL_Quit();
        return 1;
    }
    std::cout << "cargado " << ruta << ": " << ancho << "x" << alto
              << " (file=" << canalesArchivo << " canales, pedimos 4)\n";
    std::cout << "primer pixel RGBA: " << (int)datos[0] << " " << (int)datos[1]
              << " " << (int)datos[2] << " " << (int)datos[3] << "\n";   // ¡flip!

    // ---- SUBIDA ------------------------------------------------------------ 
    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, ancho, alto, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, datos);
    stbi_image_free(datos);      // ¡SIEMPRE! (RAII custom si querés: 07)

    // (wrap/filter obligatorios: "la textura incompleta" no se dibuja, el
    //  detalle completo de wrap/filter es la lección 10:)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    std::cout << "textura " << tex << " creada desde stb_image (2x2 RGBA8)\n";
    std::cout << "NOTA: la lección 10 dibuja quad + esta textura: la de esta "
              << "lección es solo cargar y subir.\n";

    glDeleteTextures(1, &tex);
    glu::cerrar(c);
    SDL_Quit();
    return 0;
}
