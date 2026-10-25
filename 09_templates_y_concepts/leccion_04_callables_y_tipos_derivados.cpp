/* =============================================================================
   MÓDULO 09 - LECCIÓN 04: Callables y tipos derivados
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
      Un algoritmo genérico (como los de <algorithm>) no sabe qué función le van
      a pasar. Recibe un parámetro de template que puede ser:
        - una lambda:                 [](int x) { return x > 3; }
        - un puntero a función:       &esPar
        - un functor (objeto con operator()): struct Mayor { bool operator()... }
      El template no se fija cuál es: solo necesita poder escribir f(x).

      Otras dos herramientas de esta lección:
        - typename Contenedor::value_type: el tipo de los elementos de un
          contenedor estándar (vector<int>::value_type es int).
        - std::invoke_result_t<F, Args...>: el tipo que devuelve F al llamarla
          con esos argumentos. Con eso el tipo de retorno de un template se
          calcula solo y puede cambiar (int -> std::string, por ejemplo).
        - std::remove_cvref_t<T>: saca const, volatile y referencia de T.

   2. ¿QUÉ PASA POR DEBAJO?
      Cada lambda es un tipo ÚNICO generado por el compilador (una clase con
      operator()). Por eso pasar una lambda a un template instancia una versión
      nueva del algoritmo, y el compilador puede "meter inline" la llamada: suele
      ser tan rápido como escribir el bucle a mano, y más rápido que usar
      std::function, que borra el tipo y llama por indirección.

   3. ERRORES COMUNES Y BUENAS PRÁCTICAS:
      - typename es obligatorio en typename C::value_type: le avisa al compilador
        que value_type es un TIPO y no un valor.
      - Si f devuelve una referencia, invoke_result_t también: sacale la
        referencia con remove_cvref_t antes de usarlo como tipo de elemento.
      - No pases lambdas con captura por referencia a algo que muere antes de
        que se use la lambda.
      - Recibí los callables por valor (F f): son livianos y es lo que hace la
        biblioteca estándar.

   4. PARA PROFUNDIZAR:
      https://en.cppreference.com/w/cpp/types/result_of
      https://en.cppreference.com/w/cpp/types/remove_cvref
      https://en.cppreference.com/w/cpp/language/lambda
   ============================================================================= */
#include <array>
#include <cstddef>
#include <iostream>
#include <numeric>
#include <string>
#include <type_traits>
#include <vector>

// --- 1. Recibir "algo llamable" ---
template <typename F, typename X>
auto aplicarDosVeces(F f, X x) {
    return f(f(x));
}

// --- 2. Los tres tipos de callable ---
bool esMultiploDe3(int x) { return x % 3 == 0; }       // función común

struct MayorQue {                                      // functor
    int limite;
    bool operator()(int x) const { return x > limite; }
};

// --- 3. Un algoritmo genérico sobre cualquier contenedor ---
template <typename Contenedor, typename Pred>
bool existe(const Contenedor& c, Pred pred) {
    for (const auto& x : c) {
        if (pred(x)) return true;
    }
    return false;
}

// --- 4. Alias para el tipo de los elementos ---
template <typename Contenedor>
using Elemento = typename Contenedor::value_type;

// --- 5. Calcular el tipo que devuelve un callable ---
template <typename F, typename X>
using Resultado = std::remove_cvref_t<std::invoke_result_t<F, X>>;

// Una función que fabrica un vector del tipo que devuelva f:
template <typename F, typename X>
std::vector<Resultado<F, X>> repetir(F f, X x, std::size_t veces) {
    std::vector<Resultado<F, X>> r;
    r.reserve(veces);
    for (std::size_t i = 0; i < veces; ++i) r.push_back(f(x));
    return r;
}

int main() {
    std::cout << "=== 1. LAMBDA, FUNCIÓN Y FUNCTOR ===\n";
    auto masUno = [](int x) { return x + 1; };
    std::cout << "aplicarDosVeces(lambda, 5) = " << aplicarDosVeces(masUno, 5) << "\n";
    std::vector<int> v = {1, 2, 3, 4, 5, 6};
    std::cout << "existe(multiplo de 3)  = " << existe(v, esMultiploDe3) << "\n";
    std::cout << "existe(> 10, functor)  = " << existe(v, MayorQue{10}) << "\n";
    int umbral = 4;
    auto conCaptura = [umbral](int x) { return x > umbral; };
    std::cout << "existe(> 4, con captura) = " << existe(v, conCaptura) << "\n";

    std::cout << "\n=== 2. CON CUALQUIER CONTENEDOR ===\n";
    std::array<int, 3> a = {7, 8, 9};
    std::cout << "existe en std::array = " << existe(a, [](int x) { return x == 8; }) << "\n";

    std::cout << "\n=== 3. TIPO DE LOS ELEMENTOS ===\n";
    static_assert(std::is_same_v<Elemento<std::vector<double>>, double>);
    static_assert(std::is_same_v<Elemento<std::array<int, 3>>, int>);
    std::cout << "Elemento<vector<double>> es double: verificado al compilar\n";

    std::cout << "\n=== 4. TIPO DE RETORNO CALCULADO ===\n";
    auto aTexto = [](int x) { return std::to_string(x); };
    static_assert(std::is_same_v<Resultado<decltype(aTexto), int>, std::string>);
    auto r = repetir(aTexto, 7, 3);                 // vector<std::string>
    std::cout << "repetir(aTexto, 7, 3) -> " << r[0] << ", " << r[1] << ", " << r[2] << "\n";
    auto cuadrado = [](int x) { return x * x; };
    auto q = repetir(cuadrado, 4, 2);               // vector<int>
    std::cout << "repetir(cuadrado, 4, 2) -> " << q[0] << ", " << q[1] << "\n";

    std::cout << "\n=== 5. COMPARACIÓN CON LA BIBLIOTECA ESTÁNDAR ===\n";
    // std::accumulate es un "fold" genérico con exactamente esta idea:
    int suma = std::accumulate(v.begin(), v.end(), 0, [](int acc, int x) { return acc + x; });
    std::cout << "accumulate con lambda = " << suma << "\n";

    return 0;
}
