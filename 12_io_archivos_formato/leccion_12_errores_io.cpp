/* =============================================================================
   MÓDULO 12 - LECCIÓN 12: errores de E/S: fail / bad / eof y validación manual
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
       L01/L02 presentaron los bits de estado. Esta lección los convierte
       en PATRÓN: cada operación de E/S de tu programa se valida a sí
       misma, con la guía de los tres estados:

         eofbit  "llegaste al final". NO es un error en sí mismo: es
                 la base del "leer todo" idiomatico. ALGO importante:
                 eofbit casi siempre APARECE JUNTO a failbit, porque la
                 lectura que cruzó el final también falló (pediste algo
                 que ya no estaba). Chequear fail sigue siendo la vara:
                 eof solo explica POR QUÉ falló.
         failbit "la operación lógica falló": parse fallido, archivo que
                 no abrió, read incompleto. ES RECUPERABLE: clear() +
                 (si hay basura) ignore(), y el stream vuelve a servir.
         badbit  "el stream se rompió de forma cruda" (el disco se
                 desconectó, el descriptor murió). NO es recuperable:
                 cerrá, recreá el stream, reiniciá el intento.

       La validación manual tiene CUATRO instante fijos:
         1) APERTURA: if (stream) tras el constructor.
         2) OPERACIÓN: tras cada write/read/getline: if (stream).
         3) CONSOLIDACIÓN al cierre: flush() hace el write REAL (el
            buffer entra al kernel); si el disco está lleno, falla AHÍ
            y no en el `<<`. Por eso el flush y el close explícito se
            chequean como operaciones: el dtor RAII hace flush pero NO
            te deja ver el resultado: es la desventaja de dejarlo solo.
         4) CON READS CRUDAS: gcount() o completitud (L08/L13): solo
            "tomó exactamente los bytes esperados" pasa.
       En la práctica: la función de save "segura" es la que devuelve
       false en el PRIMER chequeo fallido, con la razón loggeada.

       ¿Y las excepciones? filesystem_error y format_error SÍ lanzan
       (L11, L04): los STREAMS de siempre NO: en streams la señal es
       el estado, en filesystem la señal es la excepción o el
       std::error_code: elegir por familia y NO dejar el "silencio de
       no validar" (el módulo 13 abre la caja completa, con códigos y
       expected).

   2. ¿QUÉ PASA POR DEBAJO?
       El flush VACÍA el buffer con una syscall write() al kernel; el
       dtor de ofstream también hace flush; el close suelta el fd.
       El "disco lleno" o el "sistema read-only" NO se manifiesta en
       el `<<`: se manifiesta en flush (o close): por eso "cuando
       grabo no falla" es un patrón engañoso: el error de disco aparece
       DESPUÉS, y el check de cierre es lo que lo ve. Este es EL BUG
       de los saves que se "guardaron" y a la mañana están rotos.
       El failbit en la apertura viene del errno del open del sistema
       (no existe, sin permiso, ruta no encontrada): es ESTADO, no
       excepción: si hay que describir el error, strerror(errno) o
       (mejor, en filesystem) ec.message().

   3. BUENAS PRÁCTICAS:
       - Validar AMBOS lados: apertura y operación: el patrón de la
         función guardarDuro de abajo: 4 chequeos en 6 líneas.
       - Devolver bool y REPORTAR: el save "falló" y no se le dice a
         nadie es el bug más caro del juego: ruta + qué estado quedó.
       - Ante badbit: cerrá el stream, esperá un retry sensible (el
         jugador no puye el disco): NO quedes iterando sobre un stream
         muerto: el estado malo no se repara solo.
       - close() explícito cuando el archivo es CRÍTICO: el RAII no te
         salva de un flush que falla en el close: hay que mirarlo.
   ============================================================================= */
#include <cstdint>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>

namespace errores_io {
    // (receta) guardar COMPLETO validando CADA paso (la guía de la
    // partie 1): 4 chequeos en 6 líneas: es el "save duro" del save real.
    bool guardarDuro(const std::string& ruta, const std::string& datos) {
        std::ofstream f(ruta, std::ios::binary | std::ios::trunc);
        if (!f) return false;                       // 1) apertura
        f.write(datos.data(), static_cast<std::streamsize>(datos.size()));
        if (!f) return false;                       // 2) operacion
        f.flush();
        if (!f) return false;                       // 2 bis: flush
        f.close();                                  // 3) cierre explicito
        return !f.fail();                           // 3 bis: close no miente
    }
} // namespace errores_io

int main() {
    std::cout << "=== 1) apertura fallida: el bit mas comun ===\n";
    std::ofstream imposible("/raiz/permiso/denegado.txt");
    std::cout << "abrir en carpeta prohibida: bool="
              << (imposible ? "true" : "false")
              << " (validar cuesta 1 linea: `if (!f) ...`)\n";

    std::cout << "\n=== 2) fail tras un parse malo: recoverable ===\n";
    std::istringstream fuente("no es numero");
    int n = 0;
    fuente >> n;
    std::cout << "estado tras pedir un int: fail="
              << (fuente.fail() ? "true" : "false")
              << ", eof=" << (fuente.eof() ? "true" : "false")
              << ", bad=" << (fuente.bad() ? "true" : "false") << "\n";
    fuente.clear();
    fuente.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cout << "clear+ignore: el stream VOLVIO (fail recupera)\n";

    std::cout << "\n=== 3) eofbit vs failbit en el mismo while ===\n";
    std::istringstream corta("3");
    n = 0;
    int suma = 0;
    while (corta >> n) suma += n;    // el canon: corta en fail
    std::cout << "suma de '3': " << suma << "; tras cortar: eof="
              << (corta.eof() ? "true" : "false")
              << ", fail=" << (corta.fail() ? "true" : "false") << ": "
                 "eofbit y failbit llegaron JUNTOS: el check canon es fail\n";
    std::cout << "eof 'todo bien y nada mas' existe en getline de linea final "
                 "sin \\n (L05): en `>>` casi siempre viene con fail\n";

    std::cout << "\n=== 4) el save 'duro': validacion de TODOS los pasos ===\n";
    std::cout << "guardarDuro ok: "
              << (errores_io::guardarDuro("/tmp/opencode/curso12_duro.bin",
                                          "datos") ? "true" : "false") << "\n";
    std::cout << "guardarDuro en ruta invalida: "
              << (errores_io::guardarDuro("/raiz/no.txt", "datos") ? "true" : "false")
              << " <- falla en 1) y devuelve false temprano\n";

    std::cout << "\n=== 5) badbit: el stream roto (concepto) ===\n";
    std::cout << "badbit llega de abajo (kernel/descriptor roto), no del "
                 "parse: no se repara con clear: se CERRÁ y recrea el "
                 "stream\n";

    std::cout << "\n=== 6) REPORTAR: la mitad que casi siempre falta ===\n";
    std::cout << "un save fallido SIN mensaje = save corrupto invisible\n";
    std::cout << "regla: si !guardar(...): mostrar ruta + estado del stream "
                 "(fail/bad/eof) en stderr\n";
    return 0;
}
