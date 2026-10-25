/* =============================================================================
   MÓDULO 12 - LECCIÓN 07: archivos de texto, modos de apertura y el patrón RAII
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
       Llegó la hora de tocar disco. Las tres clases (los tres son RAII:
       el módulo 07 presentó la idea y el destructor cierra el stream
       si o sí):
         std::ifstream  solo LECTURA (in)  -> datos desde el disco
         std::ofstream  solo ESCRITURA(out)-> log, save, config, CSV
         std::fstream   ambos (in|out)     -> leer y tocar in-situ

       Constructor con path y MODO (los modos se combinan con |):
         std::ios::in      lectura (default de ifstream)
         std::ios::out     escritura (default de ofstream)
         std::ios::app     cada write va al FINAL del archivo (append):
                           el modo de logs: nunca pisa lo viejo
         std::ios::trunc   borra el contenido si el archivo existe (es el
                           default de ofstream cuando abre para escribir)
         std::ios::binary  sin traducción de fines de línea (L08)
       El error común de la vida: `ofstream f("save.txt")` es TRUNC:
       pisó el save de ayer. Para logs: app. Para saves que se
       regeneran enteros: trunc va bien, PERO hay que saber que es el
       default y no un accidente.

       El patrón canon de leer un archivo de texto completo es la
       apertura validada + getline en loop:
         std::ifstream f(path);
         if (!f) return ...;          // SIEMPRE validar apertura
         std::string linea;
         while (std::getline(f, linea)) { ... }   // canon de la L05
       La apertura fallida viene en el bool del stream: no lanzan: si
       (!f) vos te encargás del mensaje.

       En TODO esto entra la RAII: al salir del ámbito, el destructor
       hace flush + close solo. El close() explícito vale solo si vos
       querés VALIDAR su resultado (un flush puede fallar en el close:
       L12 lo documenta).

       Sobre el path: std::ifstream acepta std::string, C-string y
       std::filesystem::path (L09): lo que NO acepta es inventarte el
       separador a mano ("/" vs "\\"): para eso es el tipo path.
   2. ¿QUÉ PASA POR DEBAJO?
       El buffer de ofstream es el MISMO cache que cout: el '\n' entra
       a memoria y el disco recibe en lotes. En juegos, un crash sin
       flush es un save corto (la mitad de bytes: L12 con el check).
       El modo app obliga a posicionar al final antes de cada write:
       el seekp manual sobre un stream en app no lo logerase (el modo
       lo manda).
       El error de disco: el error de disco (full/read-only)
       llega en el flush o el close, no en el `<<`.
   3. BUENAS PRÁCTICAS:
       - Abrir como RECURSO RAII y en el scope más corto posible.
       - `app` para logs, `trunc` para saves regenerados de cero: la
         decisión explícala con un comentario en la apertura.
       - CHEQUEAR la apertura SIEMPRE (el (!f) con un mensaje de por
         qué está bien aceptar allí).
       - NO tests en disco real para el parseo: istringstream (L05):
         el disco solo en tests de file paths (los ejercicios de este
         módulo usan un dir temporal limpio).
   ============================================================================= */

#include <fstream>
#include <iostream>
#include <string>

namespace archivos {
    // (receta) leer TODO el archivo de texto línea por línea;
    // devuelve false si la apertura falló (y el caller decide).
    bool guardarLineas(const std::string& ruta, const std::string& contenido,
                       bool append = false) {
        std::ios::openmode modo = std::ios::out;
        if (append) modo |= std::ios::app;
        std::ofstream salida(ruta, modo);
        if (!salida) return false;          // ruta inexistente'permiso'etc
        salida << contenido;
        return true;                        // el close() dado RAII hace flush
    }

    std::string cargarLineas(const std::string& ruta, bool* ok) {
        std::ifstream entrada(ruta);
        if (!entrada) {                     // validar apertura ANTES de nada
            if (ok) *ok = false;
            return {};
        }
        std::string contenido, linea;
        while (std::getline(entrada, linea)) {
            contenido += linea;
            contenido += '\n';
        }
        if (ok) *ok = true;
        return contenido;
    }
} // namespace archivos

int main() {
    const std::string ruta = "/tmp/opencode/curso12_leccion07_demo.txt";

    std::cout << "=== 1) escribir (trunc: regenera) ===\n";
    if (archivos::guardarLineas(ruta, "partida 1\npos 3 7\n")) {
        std::cout << "estado escrito en '" << ruta << "': trunc regenera\n";
    }

    std::cout << "\n=== 2) leer de vuelta ===\n";
    bool ok = false;
    const std::string leido = archivos::cargarLineas(ruta, &ok);
    std::cout << "lectura=" << (ok ? "ok" : "fallo") << ", contenido:\n"
              << leido;

    std::cout << "=== 3) append: el log que nunca pisa ===\n";
    archivos::guardarLineas(ruta, "evento: goblin derrotado\n", true);
    std::cout << archivos::cargarLineas(ruta, &ok);

    std::cout << "=== 4) abrir archivo que NO existe ===\n";
    bool archivo_ok = true;
    archivos::cargarLineas("no_existe_seguro.txt", &archivo_ok);
    std::cout << "lectura de inexistente: ok=" << (archivo_ok ? "true" : "false")
              << " <- (validado: SIEMPRE chequear (!f) antes)\n";

    std::cout << "\n=== 5) RAII: close implicito y flush garantizado ===\n";
    {
        std::ofstream f("/tmp/opencode/curso12_leccion07_raii.txt");
        f << "texto en el buffer al salir del scope";
        std::cout << "se cierra al salir (dtor): flush + close\n";
    } // <- ACA se ejecuta ~ofstream
    std::cout << "(el archivo ya esta completo EN DISCO sin close() manual)\n";
    return 0;
}
