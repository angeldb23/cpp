/* =============================================================================
   MÓDULO 04 - LECCIÓN 12: Lambdas básicas y capturas
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      Una lambda es una función anónima que se escribe en el lugar donde se
      usa. Sintaxis mínima:

          [](int x) { return x * 2; }

      El corchete de captura decide qué variables del entorno puede usar:
          []        no captura nada
          [x]       captura x por valor (una copia)
          [&x]      captura x por referencia (ve los cambios)
          [=]       captura por valor todo lo que use
          [&]       captura por referencia todo lo que use
          [&, x]    por referencia por defecto, x por valor
          [this]    captura el objeto actual (POO, módulo 05)

      Por defecto el operator() de la lambda es const: no puede modificar las
      capturas por valor. Con mutable sí puede (la copia interna cambia, no el
      original).

      Una lambda sin captura se convierte a un puntero a función, así que sirve
      como callback clásico. Una lambda con captura no: necesita guardar
      estado, y su tipo es una clase única generada por el compilador.

   2. ¿QUÉ PASA POR BAJO?
      El compilador genera una clase con un operator() y, si hay capturas, un
      miembro por cada una. [x] guarda una copia; [&x] guarda una referencia.
      Por eso capturar por referencia y usar la lambda después de que la
      variable murió es un puntero colgante disfrazado.

      Las lambdas son perfectas para algoritmos: std::sort, std::count_if,
      std::transform, std::for_each.

   3. ERRORES COMUNES
      - Devolver una lambda que captura por referencia una local.
      - Usar [=] o [&] sin pensar qué se captura (capturas implícitas).
      - Olvidar mutable cuando se quiere modificar una captura por valor.
      - Creer que [x] refleja cambios posteriores de x: es una copia.
      - Capturar por referencia y ejecutar la lambda en otro hilo o más tarde.

   4. PARA PROFUNDIZAR
      https://en.cppreference.com/w/cpp/language/lambda
   ============================================================================= */
#include <algorithm>
#include <iostream>
#include <iterator>
#include <vector>

int main() {
    std::cout << "=== 1. LAMBDA SIN CAPTURA ===\n";
    auto duplicar = [](int x) { return x * 2; };
    std::cout << "duplicar(21) = " << duplicar(21) << "\n";

    // Sin captura se convierte a puntero a función.
    int (*pf)(int) = duplicar;
    std::cout << "como puntero a función: " << pf(5) << "\n";

    std::cout << "\n=== 2. CAPTURA POR VALOR ===\n";
    int base = 10;
    auto sumarBase = [base](int x) { return x + base; };
    base = 100;  // la lambda guardó una copia de 10
    std::cout << "sumarBase(5) = " << sumarBase(5) << " (no ve el 100)\n";

    std::cout << "\n=== 3. CAPTURA POR REFERENCIA ===\n";
    int contador = 0;
    auto incrementar = [&contador]() { ++contador; };
    incrementar();
    incrementar();
    std::cout << "contador = " << contador << "\n";

    std::cout << "\n=== 4. MUTABLE ===\n";
    int total = 0;
    auto acumular = [total](int x) mutable {
        total += x;      // modifica la COPIA interna, no la variable externa
        return total;
    };
    std::cout << "acumular(3) = " << acumular(3) << "\n";
    std::cout << "acumular(4) = " << acumular(4) << " (la copia interna persiste)\n";
    std::cout << "total externo sigue en " << total << "\n";

    std::cout << "\n=== 5. LAMBDA COMO COMPARADOR DE std::sort ===\n";
    std::vector<int> numeros = {5, 2, 9, 1, 7};
    std::sort(numeros.begin(), numeros.end(),
              [](int a, int b) { return a > b; });  // descendente
    std::cout << "ordenados: ";
    for (int n : numeros) std::cout << n << " ";
    std::cout << "\n";

    std::cout << "\n=== 6. CAPTURA EN ALGORITMOS ===\n";
    int umbral = 4;
    auto mayores = std::count_if(numeros.begin(), numeros.end(),
                                 [umbral](int n) { return n > umbral; });
    std::cout << "mayores que " << umbral << ": " << mayores << "\n";

    int factor = 3;
    std::transform(numeros.begin(), numeros.end(), numeros.begin(),
                   [factor](int n) { return n * factor; });
    std::cout << "multiplicados por " << factor << ": ";
    for (int n : numeros) std::cout << n << " ";
    std::cout << "\n";

    std::cout << "\n=== 7. FILTRAR CON copy_if Y back_inserter ===\n";
    std::vector<int> pares;
    std::copy_if(numeros.begin(), numeros.end(), std::back_inserter(pares),
                 [](int n) { return n % 2 == 0; });
    std::cout << "pares: ";
    for (int n : pares) std::cout << n << " ";
    std::cout << "\n";
    std::cout << "back_inserter agrega al final; el vector crece solo\n";

    std::cout << "\n=== 8. CAPTURA POR REFERENCIA Y VIDA ÚTIL ===\n";
    // Peligro: la lambda guarda una referencia, no una copia. Si la variable
    // muere antes de ejecutar la lambda, el acceso es UB. Acá la variable
    // sigue viva en main, así que es seguro.
    std::cout << "capturar por referencia exige que el objeto viva más\n";

    return 0;
}
