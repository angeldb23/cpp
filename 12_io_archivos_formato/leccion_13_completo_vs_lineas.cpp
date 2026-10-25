/* =============================================================================
   MÓDULO 12 - LECCIÓN 13: leer el archivo COMPLETO vs línea por línea; buffers
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
       La semana que viene vas a escribir un parser de saves, un lector
       de logs o un cargador de assets: todos arrancan con la misma
       decisión: ¿leo el archivo entero a memoria o lo proceso línea a
       línea mientras viene? Las dos estrategias y sus costos:

       A) TODO DE UN GOLPE (a memoria):
            std::ifstream f(...);
            std::ostringstream ss;
            ss << f.rdbuf();            // el truco oficial: filebuf -> stream
            std::string todo = ss.str();
          A favor: POCÍSIMOS syscalls (el buffer grande manda), y un parser
          que corre sobre `todo` es TESTEABLE con istringstream (L05) sin
          tocar disco. En contra: el archivo ENTERO vive en RAM (el pico
          de memoria es O(archivo)): para un log de gigas no es opción.
          Variante A2: medir el tamaño con seekg(0, end) + tellg() y hacer
          un read() del tamaño exacto (la forma "con medida previa").

       B) LÍNEA POR LÍNEA (getline en loop):
          A favor: memoria O(UNA línea); es la forma natural del streaming:
          el programa empieza a procesar desde la PRIMERA línea sin cargar
          todo (logs en marcha, CSV enormes). En contra: hay una llamada
          al stream POR LÍNEA (no syscall por línea, ojo): para millones
          de líneas chiquitas el costo administrativo sube.

       BUFFERS: todo stream tiene un buffer interno (filebuf). El costo
       real NO es una syscall por getline: es una syscall por buffer
       vaciado: filebuf pide al kernel un lote (el st_blksize del FS
       suele ser 4096) y getline cobra del buffer de RAM hasta que se
       seca. El tamaño del buffer se ajusta con pubsetbuf() ANTES del
       primer read (un detalle avanzado; la lección se queda con el
       modelo: wholesale = menos syscalls).

       STREAMING REAL: para STDIN / PIPE / RED no hay "seek al final";
       son arroyos en marcha: solo aplica B. Por eso un buen parser se
       escribe contra `std::istream&`: sirve para archivo Y para stdin
       sin cambiar una línea (L05 ya lo usa).
   2. ¿QUÉ PASA POR DEBAJO?
       rdbuf() devuelve un PUNTERO al filebuf del archivo: `ss <<
       f.rdbuf()` vacía ese buffer directo a ostringstream (sin copias
       intermedias por línea). El A2 con seekg/tellg son llamadas
       lseek() del sistema: O(1) para archivos regulares, INVÁLIDAS en
       pipes (más arriba: por qué). Un read() con el tamaño medido es
       una sola syscall del kernel si el archivo está en un slurp.
       Para MEDIR los tiempos está steady_clock (módulo 11): los
       chrono::duration_cast que se usan abajo ya son conocidos.
   3. BUENAS PRÁCTICAS:
       - Archivos chiquitos que se re-lean (configs, mini-datasets y
         mapas de partida): A) a memoria, una vez.
       - LOGS y CSV grandes en producción: B) streaming SIEMPRE: el
         pico de memoria manda sobre la micro-optimización de calls.
       - MEDIR con steady_clock antes de optimizar (lo hacemos abajo):
         es el mismo músculo del benchmarking del módulo 11.
       - reserve() del string destino si el tamaño se conoce (módulo
         03: menos realocaciones).
       - Escribir contra std::istream& es lo que hace testable el
         pipeline completo (L05-L06).
   ============================================================================= */
#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

int main() {
    const std::string ruta = "/tmp/opencode/curso12_leccion13.txt";
    {
        std::ofstream f(ruta);
        for (int i = 1; i <= 10000; ++i) {
            f << "linea " << i << "\n";
        }
    }
    auto inicio = std::chrono::steady_clock::now();

    std::cout << "=== 1) A: lectura completa con rdbuf() ===\n";
    {
        std::ifstream f(ruta);
        std::ostringstream ss;
        ss << f.rdbuf();              // vacia el filebuf del archivo
        std::string todo = ss.str();
        std::cout << "bytes: " << todo.size() << ", lineas: "
                  << std::count(todo.begin(), todo.end(), '\n') << "\n";

        auto medio = std::chrono::steady_clock::now();
        std::cout << "rdbuf(): duracion "
                  << std::chrono::duration_cast<std::chrono::microseconds>(
                         medio - inicio).count() << " us\n";
        inicio = medio;
    }

    std::cout << "\n=== 2) A2: medida con seekg/tellg + read directo ===\n";
    {
        std::ifstream f(ruta, std::ios::binary);
        f.seekg(0, std::ios::end);
        const std::streamsize tamano = f.tellg();
        std::string bytes(static_cast<size_t>(tamano), '\0');
        f.seekg(0);
        f.read(bytes.data(), tamano);
        std::cout << "medido " << tamano << " bytes y leidos en un read()\n";
        auto medio = std::chrono::steady_clock::now();
        std::cout << "seek+read: duracion "
                  << std::chrono::duration_cast<std::chrono::microseconds>(
                         medio - inicio).count() << " us\n";
        inicio = medio;
    }

    std::cout << "\n=== 3) B: linea por linea con getline ===\n";
    {
        std::ifstream f(ruta);
        std::string linea;
        long lineas = 0;
        while (std::getline(f, linea)) {
            if (!linea.empty()) ++lineas;
        }
        std::cout << "lineas procesadas: " << lineas
                  << " (memoria: una sola linea a la vez)\n";
        auto fin = std::chrono::steady_clock::now();
        std::cout << "getline loop: duracion "
                  << std::chrono::duration_cast<std::chrono::microseconds>(
                         fin - inicio).count() << " us\n";
    }

    std::cout << "\n=== 4) streaming con parse por linea (el patron canon) ===\n";
    std::ifstream f(ruta);
    std::string linea;
    int suma = 0;
    while (std::getline(f, linea)) {
        std::istringstream fila(linea);
        std::string palabra; int num = 0;
        fila >> palabra >> num;
        if (fila) suma += num;
    }
    std::cout << "suma de los numeros: " << suma << "\n";

    std::cout << "\n=== 5) el techo: donde A no puede ===\n";
    std::cout << "para STDIN/PIPE/RED no hay seek: solo streaming (B) "
                 "es opcion; por eso el parser se escribe contra "
                 "std::istream&\n";
    return 0;
}
