/* =============================================================================
   MÓDULO 12 - LECCIÓN 09: std::filesystem::path, la joya de nombres de archivo
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
       std::filesystem::path es el tipo de C++17 que modela "dónde está
       un archivo". Reemplaza al antipatrón de concatenar strings a mano
       (el "/" que en Windows es "\\", el "cuidado con el separador
       final") y dale nombre, extensión y carpeta padre como PIEZAS del
       tipo, no como substrings a ojo.

       Un path tiene piezas nombradas:
         "/home/ave/juegos/guardado.dat"
          parent_path  = /home/ave/juegos
          filename     = guardado.dat
          stem         = guardado   (sin la extensión)
          extension    = .dat       (con punto: esa es la convención)
       Consultas: parent_path, filename, stem, extension, has_extension,
       is_absolute. Todas son O(1): recortan el string interno, no hay
       parseo caro ni caché que invalidar.

       La manera de construir es operator/= y operator+=:
         path p = "/tmp";
         p /= "demos";    -> /tmp/demos      (el /= agrega el separador)
         p += ".txt";     -> /tmp/demos.txt  (pega chars crudos, sin /)
       lexically_normal() limpia los caminos feos: "./a/../b" -> "b".
       lexically_relative(base) entrega el relativo respecto de base.

       La plusvalía del tipo es la PORTABILIDAD: dentro guarda la forma
       nativa del sistema (en Linux: char UTF-8 y '/'; en Windows:
       wchar_t y '\\'), pero el API con el que vos escribís es el mismo
       en ambos: los detalles de plataforma viven dentro del tipo, no
       en tu código.

       Idea que cierra la lección: path NO consulta el sistema: es solo
       un nombre. exists(), is_directory(), etc. son consultas de L10 y
       L11. Esa separación "nombre vs sistema" es la pieza conceptual
       del tema.
   2. ¿QUÉ PASA POR DEBAJO?
       En Linux un path es chars UTF-8 con '/' (el separador de UNIX);
       en Windows son wchar_t y/backslash (que acepta '/' igual):
       el API es el mismo, el dato interno no.
       Los paths RELATIVOS se resuelven contra el directorio de trabajo
       del proceso EN RUNTIME (no contra el .cpp): current_path() da
       ese cwd. Por eso el mismo binario corrido desde dos carpetas
       distintas puede encontrar (o no) el mismo archivo relativo: en
       los tests el cwd es controlado (yo uso temp dirs absolutos en
       los ejercicios por ese mismo motivo).
       path::preferred_separator es el separador del sistema: no lo
       hardcodees a mano nunca.
   3. BUENAS PRÁCTICAS:
       - TODO path declarado filesystem::path: NO concatenar strings
         a mano.
       - CONSTANTES de base: `base_dir /= "assets" /= "sprites.png"`.
       - Filtrar por extension() (L10): cuidado ".DAT" vs ".dat" en
         Linux (son archivos distintos): en sistemas sensibles al case
         toca comparar en minúscula.
       - RELATIVOS para el save del usuario; ABSOLUTOS (o temp dir)
         para tests.
       - TRAMPA del ejecutable: cwd NO es "dónde está el binario": si
         corrés ./build/juego desde la raíz, cwd = la raíz. Si te
         interesa el binario, mirá argv[0] (o canonical/weakly_canonical
         sobre él).
   ============================================================================= */


#include <filesystem>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

int main() {
    std::cout << "=== 1) piezas de un path ===\n";
    const fs::path ruta = "/home/ave/juegos/guardado.dat";
    std::cout << "root_path    : " << ruta.root_path() << "\n";
    std::cout << "parent_path  : " << ruta.parent_path() << "\n";
    std::cout << "filename     : " << ruta.filename() << "\n";
    std::cout << "stem         : " << ruta.stem() << "\n";
    std::cout << "extension    : " << ruta.extension() << "\n";

    std::cout << "\n=== 2) construir con /= y += ===\n";
    fs::path p = "/tmp";
    p /= "demos";            // agrega separador
    p += ".txt";             // pega el string hard: sin separador
    std::cout << "construido: " << p << "\n";
    std::cout << "has_extension: " << (p.has_extension() ? "true" : "false") << "\n";

    p.replace_extension(".bin");
    std::cout << "replace_extension: " << p << " (calto de .txt a .bin)\n";

    std::cout << "\n=== 3) piezas de path relativo ===\n";
    const fs::path rel = "assets/sprites/hero.png";
    std::cout << "parent: " << rel.parent_path() << ", stem: " << rel.stem()
              << ", ext: " << rel.extension()
              << ", absoluta? " << (rel.is_absolute() ? "sí" : "no") << "\n";

    std::cout << "=== 4) lexically_normal y current_path ===\n";
    const fs::path feo = "/tmp/./basura/../demos";
    std::cout << "normal: " << feo.lexically_normal() << "\n";
    const fs::path base = "/tmp/demos";
    std::cout << "relativo a base: " << feo.lexically_normal().lexically_relative(base) << "\n";
    std::cout << "working dir (cwd): " << fs::current_path().filename() << "\n";
    std::cout << "(nota: cwd = de vista correr el binario: no 'dónde está el exe')\n";

    std::cout << "\n=== 5) el path NO verifica si existe: naming only ===\n";
    const fs::path fantasma = "path_inexistente_mapa.txt";
    std::cout << "el path existe? " << (fs::exists(fantasma) ? "true" : "false")
              << " <- la pregunta es del OS (L10-L11 profundizan)\n";
    std::cout << "(un path vacío existe? any path: exists() es CONOCIDO "
                 "por el sistema: building: NO llamarla por constante)\n";
    return 0;
}
