/* =============================================================================
   MÓDULO 12 - LECCIÓN 11: filesystem en acción: crear, borrar, renombrar, status
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
       Todo lo que MUTA el sistema de archivos pasa por un puñado de
       funciones libres:
         fs::create_directory(p)    -> crea UNA carpeta (false si ya existe)
         fs::create_directories(p)  -> crea TODAS las carpetas del path
                                       que falten (mkdir -p)
         fs::remove(p)              -> borra archivo o carpeta VACÍA
                                       (no falla si no existía)
         fs::remove_all(p)          -> borra archivo o carpeta con TODO
                                       su contenido (¡cuidado con esto!)
         fs::rename(viejo, nuevo)   -> mueve/renombrar: O(1) en la misma
                                       FS (no copia bytes: re-indexa)
         fs::copy_file(orig, dst, opts) -> copia datos (con opciones, p.ej.
                                       overwrite_existing)
         fs::exists / fs::status(p) -> preguntar el estado actual

       status vs symlink_status: status SIGUE los symlinks (la entrada
       que está del otro lado del link); symlink_status mira EL LINK en
       sí (is_symlink() da true solo ahí).

       ERRORES, dos estilos:
         a) EXCEPCIONES (el default): fs::rename(la, la) lanza
            fs::filesystem_error (con e.what(), e.path1(), e.path2()).
         b) std::error_code overload: fs::rename(a, b, ec): NO lanza:
            el resultado se lee en ec (ec: L12).
       La elección es política del módulo: en runtime de juego el estilo
       "no lanza" con ec está bien para cada llamada aislada: "falló,
       pero sigo vivo". Los dos son validación manual:
       ec != 0 <=> hubo error.

       EL PATRÓN "ESCUDO DE SAVE" de los motores reales:
         1) escribir save_nuevo.tmp (ofstream, trunc)
         2) fs::rename(tmp, save_real)   // atómico en POSIX
       Si el programa crashea entre 1 y 2, el save viejo queda intacto:
       el rename es EL paso atómico de toda la familia.
   2. ¿QUÉ PASA POR DEBAJO?
       rename = la syscall rename(): cambia la entrada de la carpeta a
       otro nombre SIN copiar bytes (el inode se queda): microsegundos
       frente a copy_file (que abre, lee, escribe y cierra).
       create_directories = mkdir -p en cascada: crear lo que ya existe
       NO es error (la función devuelve false y se termina).
       create_directory devuelve false si la carpeta YA EXISTÍA: es el
       caso normal (no un error): mirar el bool sin curarse es la
       trampa de novicio.
       TODAS las funciones tienen DOS versiones: la que lanza
       (filesystem_error) y la que devuelve ec: es EXACTO el mismo
       parásito de ec del módulo 13: acá lo viste: la firma
       que elegís determina el flujo ("try/catch" vs "if (ec)").
       SIEMPRE pedís copy_options para sobrescribir: la copia de un
       archivo que YA EXISTE sin las opciones lanza (o pone ec):
       overwrite_existing es un permiso declarado.
   3. BUENAS PRÁCTICAS:
       - remove_all es UNA EXPLOSIÓN: solo con paths devenidos de
         constantes del programa; STOP paths del usuario SIN validar
         (un "/..." absoluto del user con remove_all = día fatal).
       - copy_file SIEMPRE con opciones explícitas: el
         overwrite_existing si querés pisar; sin opción, un destino ya
         existente es error.
       - exists() tiene un hueco (TOCTOU): entre la llamada y el
         siguiente uso el archivo puede aparecer o desaparecer: para
         lo que importa, la operación correcta es hacer-y-observar-ec
         (hacer commit y ver qué pasa), no exists-curar.
       - Renombrar en el mismo filesystem es O(1); entre filesystems
         distinto el rename FALLA (y toca copiar + borrar de verdad).
   ============================================================================= */

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

int main() {
    const fs::path base = "/tmp/opencode/curso12_leccion11_demo";
    std::error_code ec;
    fs::remove_all(base, ec);     // limpiar el demo previo (no-lanza estilo)
    ec.clear();
    fs::create_directories(base, ec); // la raiz del demo tambien se crea

    std::cout << "=== 1) crear: una vs todas las carpetas ===\n";
    std::cout << "create_directory sola : "
              << (fs::create_directory(base / "saves") ? "creo" : "ya estaba") << "\n";
    ec.clear();
    fs::create_directories(base / "assets/animaciones/hero", ec);
    std::cout << "create_directories anidadas: ec simple="
              << (ec ? "........" : "ok") << "\n";

    std::cout << "\n=== 2) escudo de save: tmp + rename ===\n";
    {
        const fs::path tmp = base / "saves/slot1.tmp";
        const fs::path final_ = base / "saves/slot1.sav";
        std::ofstream f(tmp, std::ios::binary);
        f << "estado del jugador\n";
        fs::rename(tmp, final_, ec);   // atómico: nunca queda semi-saved
        std::cout << "rename tmp->slot1.sav: "
                  << (ec ? ec.message() : "ok") << "\n";
        std::cout << "tmp sigue existiendo? "
                  << (fs::exists(base / "saves/slot1.tmp") ? "true" : "false")
                  << " <- rename vira el inode\n";
    }

    std::cout << "\n=== 3) copiar y sobrescribir (con opcion explicita) ===\n";
    ec.clear();
    fs::copy_file(base / "saves/slot1.sav", base / "saves/backup.sav", ec);
    std::cout << "copy 1ra vez (dst no existia): "
              << (!ec ? "ok" : ec.message()) << "\n";
    ec.clear();
    fs::copy_file(base / "saves/slot1.sav", base / "saves/backup.sav", ec);
    std::cout << "copy 2da vez sin opción:        "
              << (ec ? "falla: " + ec.message() : "ok") << "\n";
    ec.clear();
    fs::copy_file(base / "saves/slot1.sav", base / "saves/backup.sav",
                  fs::copy_options::overwrite_existing, ec);
    std::cout << "copy con overwrite_existing:    "
              << (!ec ? "ok" : ec.message()) << "\n";

    std::cout << "\n=== 4) renombrar un archivo existente: el caso colision ===\n";
    ec.clear();
    // en POSIX rename SOLAPA el destino sin error: en el API C++ es igual
    // (el estandar lo permite, la ic llamada): es DATA LOSS por diseño:
    fs::rename(base / "saves/backup.sav", base / "saves/slot1.sav", ec);
    std::cout << "rename sobre slot1 existente: "
              << (!ec ? "silenciosamente lo PISA (POSIX)" : ec.message()) << "\n";

    std::cout << "\n=== 5) borrar y errores como filesystem_error ===\n";
    ec.clear();
    fs::remove_all(base / "assets", ec);
    std::cout << "remove_all assets: " << (!ec ? "borrada" : ec.message())
              << " (los directioies enteros pueden status algún)\n";
    ec.clear();
    fs::remove(base / "saves/ghost.sav", ec);     // no-instancia: NO es error
    std::cout << "remove de inexistente: ec vacia? "
              << (!ec ? "true" : "false")
              << " (remove no falla si no existia; TRY tense en load_n)\n";

    std::cout << "\n=== 6) el estilo EXCEPCION (compara tú el onus) ===\n";
    try {
        fs::rename(base / "fantasma", base / "otro");
        std::cout << "no llega acá\n";
    } catch (const fs::filesystem_error& e) {
        std::cout << "filesystem_error: " << e.what() << "\n";
        std::cout << "  path1: " << e.path1() << " | path2: " << e.path2() << "\n";
    }

    std::cout << "\n=== 7) limpiar (y fuzz del remove_all con path propio) ===\n";
    fs::remove_all(base, ec);
    std::cout << "remove_all del demo completo: " << (!ec ? "limpio" : ec.message())
              << "\n";
    return 0;
}
