/* =============================================================================
   MÓDULO 09 - LECCIÓN 01: Templates de función
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
      Un template es una "receta" que el compilador usa para generar código a
      medida. Escribís UNA vez maximo<T>(a, b) y el compilador fabrica
      maximo<int>, maximo<double>, maximo<std::string>... según lo que uses.
      Eso se llama INSTANCIACIÓN, y pasa en tiempo de compilación: no hay costo
      en tiempo de ejecución (a diferencia de la herencia con funciones virtuales).

      Cosas que vas a ver en esta lección:
        - Deducción de tipos: el compilador adivina T mirando los argumentos.
        - Especificación explícita: maximo<double>(3, 2.5) fuerza T = double.
        - Parámetros NO tipo: template <typename T, std::size_t N>.
        - Parameter packs y fold expressions: funciones con cantidad variable
          de argumentos.
        - auto en parámetros (C++20): sintaxis corta para templates.
        - Folds con otros operadores (|| y &&), sizeof... y armar texto con
          std::ostringstream a partir de argumentos de cualquier tipo.

   2. ¿QUÉ PASA POR DEBAJO?
      El compilador NO compila el template "en abstracto" hasta el final: lo
      chequea por arriba y recién cuando lo usás con un tipo concreto genera una
      función real. Si usás maximo con int y con double, el binario contiene DOS
      funciones distintas. Por eso los templates viven en headers (el compilador
      necesita ver el cuerpo para instanciarlo) y por eso abusar de ellos puede
      inflar el tamaño del ejecutable ("code bloat") y los tiempos de compilación.

   3. ERRORES COMUNES Y BUENAS PRÁCTICAS:
      - maximo(3, 2.5) NO compila: T se deduce como int por un argumento y
        como double por el otro. Solución: maximo<double>(3, 2.5).
      - Pedirle de más al tipo: si solo usás operator<, no exijas operator>.
        Así tu función sirve para más tipos.
      - Los mensajes de error de templates son larguísimos: leé la PRIMERA
        línea que dice "error" y la que dice "required from here".
      - Buena práctica moderna: preferí concepts (lección 03) para documentar
        qué exige cada template.

   4. PARA PROFUNDIZAR:
      https://en.cppreference.com/w/cpp/language/function_template
      https://en.cppreference.com/w/cpp/language/fold
      https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#t-templates-and-generic-programming
   ============================================================================= */
#include <cstddef>
#include <iostream>
#include <sstream>
#include <string>

// --- 1. El template más simple: solo exige operator< ---
template <typename T>
T maximo(T a, T b) {
    return (a < b) ? b : a;
}

// --- 2. Dos tipos distintos: el tipo de retorno se deduce con auto ---
template <typename T, typename U>
auto sumar(T a, U b) {
    return a + b;
}

// --- 3. Parámetro NO tipo: N es un valor conocido en compilación ---
template <typename T, std::size_t N>
constexpr std::size_t largo(const T (&)[N]) {
    return N;
}

// --- 4. Parameter pack + fold expression (C++17) ---
// (args + ... + 0) expande a: ((a1 + a2) + a3) + ... + 0
template <typename... Args>
auto sumaTodo(Args... args) {
    return (args + ... + 0);
}

template <typename... Args>
void imprimirTodo(const Args&... args) {
    ((std::cout << args << ' '), ...);   // fold sobre el operador coma
    std::cout << '\n';
}

// --- 5. auto en parámetros (C++20): es un template "disfrazado" ---
void mostrar(const auto& valor) {
    std::cout << "  valor: " << valor << "\n";
}

// --- 6. Más folds: || y sizeof... ---
// Una fold unaria con || vale false si el pack está vacío (con && vale true).
template <typename... Args>
bool alguno(Args... args) {
    return (... || args);
}

template <typename... Args>
constexpr std::size_t contar(Args...) {
    return sizeof...(Args);          // cuenta los elementos del pack, sin recorrerlos
}

// --- 7. Armar texto con argumentos de CUALQUIER tipo ---
// std::ostringstream acepta con << todo lo que acepta std::cout.
template <typename... Args>
std::string aTexto(const Args&... args) {
    std::ostringstream os;
    ((os << args << ';'), ...);      // fold sobre el operador coma
    return os.str();
}

int main() {
    std::cout << "=== 1. DEDUCCIÓN DE TIPOS ===\n";
    std::cout << "maximo(3, 7)         = " << maximo(3, 7) << "\n";
    std::cout << "maximo(2.5, 1.5)     = " << maximo(2.5, 1.5) << "\n";
    std::cout << "maximo(\"pera\", \"manzana\") con std::string = "
              << maximo(std::string("pera"), std::string("manzana")) << "\n";

    std::cout << "\n=== 2. ESPECIFICACIÓN EXPLÍCITA ===\n";
    // maximo(3, 2.5) no compilaría: T sería int y double a la vez.
    std::cout << "maximo<double>(3, 2.5) = " << maximo<double>(3, 2.5) << "\n";

    std::cout << "\n=== 3. DOS PARÁMETROS DE TIPO ===\n";
    std::cout << "sumar(1, 2.5)   = " << sumar(1, 2.5) << "  (int + double -> double)\n";
    std::cout << "sumar(2, 3)     = " << sumar(2, 3) << "  (int + int -> int)\n";

    std::cout << "\n=== 4. PARÁMETRO NO TIPO ===\n";
    int numeros[] = {10, 20, 30, 40, 50};
    double notas[] = {7.5, 9.0};
    std::cout << "largo(numeros) = " << largo(numeros) << "\n";
    std::cout << "largo(notas)   = " << largo(notas) << "\n";
    static_assert(largo(numeros) == 5, "se calcula en tiempo de compilación");

    std::cout << "\n=== 5. CANTIDAD VARIABLE DE ARGUMENTOS ===\n";
    std::cout << "sumaTodo()            = " << sumaTodo() << "\n";
    std::cout << "sumaTodo(1, 2, 3, 4)  = " << sumaTodo(1, 2, 3, 4) << "\n";
    std::cout << "sumaTodo(1, 2.5)      = " << sumaTodo(1, 2.5) << "\n";
    std::cout << "imprimirTodo: ";
    imprimirTodo("vida:", 3, "puntos:", 150.5);

    std::cout << "\n=== 6. AUTO EN PARÁMETROS (C++20) ===\n";
    mostrar(42);
    mostrar(std::string("hola"));
    mostrar(3.14);

    std::cout << "\n=== 7. FOLDS CON || Y SIZEOF... ===\n";
    std::cout << "alguno(false, false, true) = " << alguno(false, false, true) << "\n";
    std::cout << "alguno()                   = " << alguno() << "  (pack vacío)\n";
    std::cout << "contar(1, 'a', 2.5)        = " << contar(1, 'a', 2.5) << "\n";
    static_assert(contar(1, 2, 3, 4) == 4, "sizeof... se evalúa en compilación");

    std::cout << "\n=== 8. ARMAR TEXTO CON OSTRINGSTREAM ===\n";
    std::cout << "aTexto(1, \"dos\", 3.5) = " << aTexto(1, "dos", 3.5) << "\n";

    return 0;
}
