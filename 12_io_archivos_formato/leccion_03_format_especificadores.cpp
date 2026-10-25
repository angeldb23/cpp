/* =============================================================================
   MÓDULO 12 - LECCIÓN 03: std::format, especificadores, alineación y precisión
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
       std::format es la respuesta moderna a printf y al baile de
       manipuladores de iostream: una función que recibe una CADENA DE
       FORMATO (mini-DSL con "{...}" en el medio del texto) y argumentos,
       y produce un std::string.

       La sintaxis base:
         std::format("texto {0} y {1}", a, b)   // indexado (basado en 0)
         std::format("texto {} y {}", a, b)     // automático: en orden
       Las llaves son el MINILENGUAJE de campo:
         {arg:[[fill]align][sign][#][0][width][.precision][tipo]}
       Lo que más usás:
         - {:>8}     ancho 8, alineado a la derecha (default de números)
         - {:<8}     ancho 8, a la izquierda (default de strings)
         - {:^10}    ancho 10, CENTRADO (con {:=^10} rellena con '=')
         - {:06.2f}  ancho total 6, ceros a la izquierda, DOS decimales
         - {:#x}     hexadecimal con prefijo 0x
         - {:e}      notación científica
         - {:*^8}    centrado rellenando con estrellas (banners)
       Las llaves LITERALES se escapan doblando: "{{" y "}}".

       LA VENTAJA GRANDE vs printf: el chequeo es de TIEMPO DE
       COMPILACIÓN: pasar "{2}" con solo 2 argumentos es un ERROR de
       compilación (el número y los TIPOS de args se verifican contra el
       literal: es la misma familia de machinery de concepts y if
       constexpr que viste en el módulo 09: std::formatter<T> es una
       especialización por tipo: los types formateables son los que la
       tienen; para TU struct, o la implementás (L15 toma el camino con
       funciones manual del curso) o abrazos specialization formatter).

       Los tipos formateables por defecto: los primitivos (int, double,
       bool, char...), std::string y string_view. Para TU tipo el
       "default" también existe SI vos ARMÁS la especialización.

       Comparación de las tres generaciones (cultura general del curso):
         printf:  varargs en runtime, sin chequeo de tipos: el molde
                  chico del C de los noventas
         ostream: seguro de tipos, pero el alineado es un baile
                  (`cout << left << setw(8) << x` son 3 tokens por campo)
         std::format: seguro, chequeado a compile time, UNA línea por
                  celda, y devuelve string reusable (no acopla al stream)
   2. ¿QUÉ PASA POR DEBAJO?
       format ajusta los argumentos al FORMAT COMPILE-TIME check via
       concepts y una plantilla del parseo del literal (el "{texto x}"):
       por eso el error "argument not found" sale en el compilador y no
       en runtime: es un consteval-check del literal: el mismo estilo
       del módulo 09 (concept y constexpr).
       El parseo del mini-lenguaje se hace UNA VEZ por llamada; la
       producción efectiva de los chars se hace a lo to_chars y NO por
       stringstream (cero alocaciones interiores): format viene de la
       librería fmt (Victor Zverovich): héroe conocido.
   3. BUENAS PRÁCTICAS:
       - Usá {} automático cuando los args van en orden natural; {0}/{1}
         cuando REPETÍS un argumento (p.ej. "{0} y otra vez {0}").
       - Ancho fijo para tablas de consola; decimales explícitos para
         datos de máquina (los archivos que un parser tra lo mas lea).
       - precision en reales SIEMPRE: el default de 6 cifras
         significativas es un pisapapeles sorprendido para el jugador
         ("3.14159" cuando querías "3.14").
       - El texto creado EN RUNTIME (lo que el usuario tipeó) NO es un
         literal: para eso existe vformat (L04): el chequeo compile-time
         SOLO corre sobre literals.
   ============================================================================= */
#include <format>
#include <iostream>
#include <string>

int main() {
    std::cout << "=== 1) especificadores de alineacion y ancho ===\n";
    std::string tabla = std::format(
        "{:<10}|{:>10}|{:^10}\n{:<10}|{:>10}|{:^10}\n",
        "NOMBRE", "NIVEL", "VESTUARIO",
        "espada", 5, "yelmo");
    std::cout << tabla;

    std::cout << "=== 2) fill con caracter a eleccion ===\n";
    std::cout << std::format("banner: [{:*^30}]\n", "MODULO 12");
    std::cout << std::format("coord : [{:->8}]\n", 42);

    std::cout << "=== 3) numeros: ancho, ceros y signo ===\n";
    std::cout << std::format("[{:5}] [{:05}] [{:+3}] [{:>6}] [{:<6}]\n",
                             42, 42, 42, 42, 42);

    std::cout << "=== 4) reales: precision fija y cientifica ===\n";
    const double pi = 3.14159265358979;
    std::cout << std::format("fija  : {:.2f}\n",     pi);
    std::cout << std::format("cient : {:.2e}\n",     pi);
    std::cout << std::format("gener : {}\n",         pi);
    std::cout << std::format("anchos: {:10.3f} | {:<10.3f} |\n", pi, 2.5);

    std::cout << "=== 5) enteros en otras bases ===\n";
    std::cout << std::format("hex             : {:x}\n", 255);
    std::cout << std::format("hex con prefijo : {:#x}\n", 255);
    std::cout << std::format("octal con prefij: {:#o}\n", 8);
    std::cout << std::format("binario         : {:b}\n", 5);
    std::cout << std::format("binario con 0b  : {:#b}\n", 5);

    std::cout << "=== 6) strings, bools y argumento repetido ===\n";
    std::cout << std::format("string: [{:>12}]\n", "juego");
    std::cout << std::format("bool  : {} (true en texto)\n", true);
    std::cout << std::format("repeat: {0} y otra vez {0}\n", "mapa");
    std::cout << std::format("llaves: {{literal}}\n");

    std::cout << "=== 7) el chequeo de compile-time que printf no tiene ===\n";
    std::cout << "format(constante, arg, arg) rechaza '{2}' con 2 args:\n";
    std::cout << "   -> el error sale en el COMPILADOR, no en runtime\n";
    std::cout << "   == el mismo espíritu que el concepts del modulo 09\n";
    return 0;
}
