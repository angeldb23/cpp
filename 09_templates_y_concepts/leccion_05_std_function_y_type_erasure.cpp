/* =============================================================================
   MÓDULO 09 - LECCIÓN 05: std::function y el costo del type erasure
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
      En la lección 04 un template recibía "cualquier callable" y quedaba una
      versión del algoritmo POR CADA tipo de callable: rápido, pero el tipo
      concreto tenía que conocerse en compilación.

      std::function<int(int)> es otra cosa: una CAJA que guarda a cualquier
      callable con esa firma. El tipo concreto se "borra" (type erasure):
      por fuera solo se ve la firma, y por dentro hay una interfaz común
      anónima. En el momento de la llamada no importa si adentro hay una
      lambda con captura, un functor o un puntero a función.

      Para qué sirve: cuando el callable se tiene que GUARDAR (miembro de
      clase, elemento de un contenedor, callback registrado en una API que
      no puede ser un template) y su tipo se decide en tiempo de
      ejecución. El costo que se paga:
        - Llamada indirecta: el compilador normalmente NO ve el cuerpo, así
          que no lo puede meter inline (la lección 04 mostró el caso opuesto
          con un template).
        - Puede alocar memoria: si el callable entra en un buffer interno no
          hay heap; si es grande o captura mucho, se hace un new.
        - Copiar un std::function copia lo que hay adentro.
      Regla de amigo: si el callable llega a un template, dejalo como F
      genérico; usá std::function solo donde el tipo se GUARDA o se decide
      en runtime.

   2. ¿QUÉ PASA POR DEBAJO?
      std::function guarda el callable y un puntero a una función interna
      que sabe ejecutarlo. Ese puntero es la bisagra del type erasure: la
      llamada real atraviesa una indirección. Por eso un template con F
      puede ser más rápido (llamada directa, inlineable) y por eso
      std::function se usa donde hay que almacenar sin conocer el tipo.

   3. ERRORES COMUNES Y BUENAS PRÁCTICAS:
      - Llamar a un std::function vacío lanza std::bad_function_call:
        chequeá con `if (f)` (se convierte en bool) antes de llamarlo.
      - std::bind es reemplazable por lambdas en C++20: preferí las lambdas.
      - Un miembro std::function pierde type safety respecto a un template:
        no sabes si lo que guardaste era una lambda o un functor, solo la
        firma. Está bien si eso es lo que querés: documentos "una acción".

   4. PARA PROFUNDIZAR:
      https://en.cppreference.com/w/cpp/utility/functional/function
      https://en.cppreference.com/w/cpp/utility/functional/bad_function_call
   ============================================================================= */
#include <functional>
#include <iostream>
#include <string>
#include <type_traits>
#include <vector>

// --- 1. Una firma, tres orígenes ---
int doble(int x) { return x * 2; }                                  // función común

struct Multiplicador {                                              // functor
    int factor;
    int operator()(int x) const { return x * factor; }
};

std::function<int(int)> caja;           // vacío al nacer: no lances nada acá

void correrTarea(const std::function<void()>& tarea) {
    if (!tarea) {                                                   // invocable? preguntalo
        std::cout << "  no hay tarea (std::function vacio)\n";
        return;
    }
    tarea();
}

// Un callable puede cambiar de TIPO a lo largo de la vida de la caja:
std::function<int(int)> cambiarDeCallable(bool conCaptura) {
    if (conCaptura) {
        int paso = 10;
        return [paso](int x) { return x + paso; };                  // lambda CON captura
    }
    return doble;                                                   // función común
}

// --- 2. Guardar el callable en un miembro ---
// En un miembro no hay template: el tipo se borra en la declaración y por
// eso el callable puede venir de cualgún lado y decidirse en runtime.
class Calculadora {
public:
    std::function<int(int)> operacion;

    int aplicar(int x) const {
        if (!operacion) throw std::bad_function_call();
        return operacion(x);
    }
};

int main() {
    std::cout << "=== 1. LA MISMA FIRMA, TRES ORIGENES ===\n";
    caja = doble;                                                   // función común
    std::cout << "funcion:  caja(5) = " << caja(5) << "\n";
    caja = Multiplicador{100};                                      // functor
    std::cout << "functor:  caja(5) = " << caja(5) << "\n";
    int fuera = 3;
    caja = [&fuera](int x) { return x * fuera; };                   // lambda con captura
    std::cout << "lambda:   caja(5) = " << caja(5) << "\n";

    std::cout << "\n=== 2. VACIA NO SE LLAMA ===\n";
    std::function<void()> pendiente;                                // sin callable
    correrTarea(pendiente);
    pendiente = [] { std::cout << "  tarea lista\n"; };
    correrTarea(pendiente);

    std::cout << "\n=== 3. UN VECTOR DE TAREAS QUE CAMBIAN DE TIPO ===\n";
    std::vector<std::function<int(int)>> pasos;
    pasos.push_back(doble);                                         // función
    pasos.push_back(Multiplicador{100});                            // functor
    pasos.push_back([](int x) { return x / 2; });                   // sin captura
    int valor = 10;
    for (const auto& paso : pasos) valor = paso(valor);
    std::cout << "pipeline 10 -> " << valor << " (doble, *100, /2)\n";

    std::cout << "\n=== 4. EL COSTO DEL TYPE ERASURE ===\n";
    auto conCaptura = cambiarDeCallable(true);
    auto comun = cambiarDeCallable(false);
    std::cout << "con captura: " << conCaptura(5) << "\n";
    std::cout << "sin captura: " << comun(5) << "\n";
    std::cout << "sizeof(std::function<int(int)>) = "
              << sizeof(caja) << " bytes en este compilador: el callable\n"
              << "puede alocar memoria si no entra en el buffer interno.\n";
    // Con un template (lección 04) la llamada es directa; con std::function
    // atraviesa un puntero interno: mismo resultado, costo distinto.

    std::cout << "\n=== 5. CALCULADORA: callable en un MEMBER ===\n";
    Calculadora calc;
    try {
        calc.aplicar(12);                                           // operacion sin asignar
    } catch (const std::bad_function_call& e) {
        std::cout << "operacion no asignada -> bad_function_call\n";
    }
    calc.operacion = [](int x) { return x * x; };
    std::cout << "operacion al cuadrado:  " << calc.aplicar(12) << "\n";
    calc.operacion = doble;
    std::cout << "operacion reemplazada:  " << calc.aplicar(12) << "\n";

    return 0;
}
