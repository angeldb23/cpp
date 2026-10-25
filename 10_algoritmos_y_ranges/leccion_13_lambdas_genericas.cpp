/* =============================================================================
   MÓDULO 10 - LECCIÓN 13: lambdas genéricas (auto params y lambda-templates)
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
       Una lambda con parámetros `auto` es GENÉRICA: sirve para el tipo que
       llegue (la lambda-template del módulo 09 aplicada al día a día). Con
       los algoritmos STL suma legibilidad: un COMPARADOR sirve para
       cualquier tipo que tenga el campo que te interesa:
           std::sort(v.begin(), v.end(), [](const auto& a, const auto& b) {
               return a.hp < b.hp; });    // crew, torreta, lo que venga
       El compilador genera una VERSIÓN por cada tipo concreto que usa la
       lambda, igual que se instancia un template de función. Y además
       C++20 trae la sintaxis EXPLÍCITA con lista de parámetros:
           []<typename T>(T x) { ... }               // el tipo único
           []<typename T>(const std::vector<T>& v)   // el T del contenedor
       útil cuando el cuerpo necesita hablar del tipo con precisión (por
       ejemplo, para saber de qué contenedor es el elemento).

       El punto de decisión vs std::function (la L05 del módulo 09): la
       lambda genérica se llama DIRECTAMENTE (el algoritmo conoce el tipo
       concreto y la puede inlinear); std::function borra el tipo y paga
       una indirección en cada llamada. Mismo veredicto de siempre:
       callable concreto para computar, std::function para GUARDAR.

       Otro uso central de este módulo: la PROYECCIÓN A MANO: el algoritmo
       no sabe qué campo te importa; tu lambda decide cómo MIRAR cada
       elemento (max por vida, orden por nombre, etc.). Las versiones
       ranges con proyección integrada (L17) vuelven esto todavía más
       directo, pero el lambda genérico te sirve para la STL clásica.
   2. ¿QUÉ PASA POR DEBAJO?
       El tipo de una lambda es ÚNICO y no tiene nombre. La genérica con
       `auto` es un operator() TEMPLATE: al llamarse con un tipo concreto,
       el compilador instancia esa versión, con la misma deducción que
       viste en la L01 del módulo 09. Por eso usar la MISMA lambda
       genérica sobre dos tipos distintos da DOS instancias: mismo
         código fuente, dos especializaciones (más binario, cero indirección).
   3. ERRORES COMUNES Y BUENAS PRÁCTICAS:
       - [](auto a, auto b) cuando el tipo era CONOCIDO: la genérica
         acepta demasiado y el compilador no avisa si entra un tipo que
         no esperabas. Preferí el tipo concreto cuando lo conocés.
       - La lista de parámetros template va ANTES del paréntesis de
         parámetros: []<typename T>(T x). Ni confundirla con los
         corchetes de captura, que van primero y no declaran tipos.
       - `auto&` es un PARÁMETRO, no una captura: mutable no tiene nada
         que ver con él. Si además mutás capturas por valor, sigue haciendo
         falta mutable (L12).
       - Varios tipos en la misma lambda genérica = varias
         especializaciones: más código objeto, no un único callable.
   4. PARA PROFUNDIZAR:
       https://en.cppreference.com/w/cpp/language/lambda
       https://en.cppreference.com/w/cpp/language/lambda#Generic_lambdas
   ============================================================================= */
#include <algorithm>
#include <iostream>
#include <numeric>
#include <string>
#include <typeinfo>
#include <vector>

struct Crew {
    std::string nombre;
    int hp;
};

struct Torreta {
    std::string nombre;
    int hp;
};

// Un comparador genérico por el campo hp: sirve para Crew, Torreta y para
// cualquier tipo futuro que tenga .hp comparable:
constexpr auto menosHp = [](const auto& a, const auto& b) {
    return a.hp < b.hp;
};

int main() {
    std::vector<Crew> equipo{
        {"Quesito", 90}, {"Mordel", 60}, {"Turbo", 120},
    };
    std::vector<Torreta> defensas{
        {"Pistola", 40}, {"Cañón", 210},
    };

    // --- 1. el MISMO comparador genérico para dos tipos distintos ------------
    std::sort(equipo.begin(), equipo.end(), menosHp);
    std::sort(defensas.begin(), defensas.end(), menosHp);
    std::cout << "equipo por hp: ";
    for (const Crew& c : equipo) std::cout << c.nombre << '=' << c.hp << ' ';
    std::cout << "\ndefensas por hp: ";
    for (const Torreta& t : defensas) std::cout << t.nombre << '=' << t.hp << ' ';
    std::cout << '\n';

    // --- 2. la genérica reusada en otro algoritmo (accumulate con proyección)
    const int hpTotal = std::accumulate(
        equipo.begin(), equipo.end(), 0,
        [](int acc, const auto& unidad) { return acc + unidad.hp; });
    std::cout << "hp total del equipo: " << hpTotal << '\n';

    // --- 3. sintaxis explícita: el cuerpo sabe de qué tipo se trata ----------
    const auto describe = []<typename T>(const T& unidad) {
        std::cout << typeid(unidad).name() << ": " << unidad.nombre << '\n';
    };
    describe(equipo.front());
    describe(defensas.front());

    // --- 4. el T deducido del CONTENEDOR completo ----------------------------
    const auto tamanoDe = []<typename T>(const std::vector<T>& v) {
        return v.size();
    };
    std::cout << "equipo: " << tamanoDe(equipo) << " unidades | defensas: "
              << tamanoDe(defensas) << " torretas\n";

    return 0;
}
