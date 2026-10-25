/* =============================================================================
   MÓDULO 04 - LECCIÓN 16: throw, try/catch y std::exception
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      Una excepción es una forma de abortar el flujo normal cuando algo no se
      puede completar. Se lanza con throw y se atrapa con try/catch:

          try {
              int r = dividir(10, 0);
          } catch (const std::invalid_argument& e) {
              std::cerr << "error: " << e.what() << "\n";
          }

      Reglas básicas:
        - Atrapá por const&: evita copias y conserva el tipo dinámico.
        - Atrapá de lo más específico a lo más general.
        - std::exception es la base de la jerarquía estándar; what() devuelve
          una descripción.
        - catch (...) atrapa cualquier cosa, pero no te da información.
        - Si nadie atrapa, el programa llama a std::terminate.

      La biblioteca estándar ya lanza excepciones: std::vector::at fuera de
      rango lanza std::out_of_range, std::stoi con texto inválido lanza
      std::invalid_argument, new sin memoria lanza std::bad_alloc.

   2. ¿QUÉ PASA POR DEBAJO?
      Cuando se lanza una excepción, el runtime busca un handler en la pila de
      llamadas. Al desenrollar (stack unwinding), destruye los objetos locales
      de cada frame. Por eso RAII funciona: los destructores liberan recursos
      aunque el camino normal se corte.

      Si un destructor lanza durante el desenrollado, el programa termina. Los
      destructores no deberían dejar escapar excepciones.

   3. ERRORES COMUNES
      - Atrapar por valor (slicing) en lugar de const&.
      - Atrapar std::exception antes que una excepción derivada.
      - Usar excepciones para el flujo normal (caro y confuso).
      - Olvidar que un recurso reservado antes del throw se libera solo si está
        dentro de un objeto con destructor (RAII), no si es un puntero crudo.
      - Lanzar desde un destructor.

   4. PARA PROFUNDIZAR
      https://en.cppreference.com/w/cpp/error/exception
   ============================================================================= */
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

class ErrorRango : public std::runtime_error {
public:
    ErrorRango(int valor, int minimo, int maximo)
        : std::runtime_error("valor " + std::to_string(valor) +
                             " fuera de [" + std::to_string(minimo) + ", " +
                             std::to_string(maximo) + "]") {}
};

int dividir(int a, int b) {
    if (b == 0) throw std::invalid_argument("división por cero");
    return a / b;
}

int validarRango(int valor, int minimo, int maximo) {
    if (valor < minimo || valor > maximo) throw ErrorRango(valor, minimo, maximo);
    return valor;
}

// Muestra el desenrollado: el destructor corre aunque haya throw.
struct Rastreador {
    std::string nombre;
    explicit Rastreador(std::string n) : nombre(n) {
        std::cout << "  + " << nombre << "\n";
    }
    ~Rastreador() { std::cout << "  - " << nombre << "\n"; }
};

void funcionQueFalla() {
    Rastreador local("recurso local");
    throw std::runtime_error("falló algo adentro");
}

int main() {
    std::cout << "=== 1. throw Y catch ESPECÍFICO ===\n";
    try {
        std::cout << dividir(10, 2) << "\n";
        std::cout << dividir(10, 0) << "\n";  // lanza
        std::cout << "esto no se imprime\n";
    } catch (const std::invalid_argument& e) {
        std::cout << "  atrapado: " << e.what() << "\n";
    }

    std::cout << "\n=== 2. EXCEPCIÓN PERSONALIZADA ===\n";
    try {
        validarRango(15, 0, 10);
    } catch (const ErrorRango& e) {
        std::cout << "  ErrorRango: " << e.what() << "\n";
    }

    std::cout << "\n=== 3. std::exception COMO BASE ===\n";
    try {
        std::vector<int> v = {1, 2, 3};
        std::cout << v.at(10) << "\n";  // out_of_range
    } catch (const std::exception& e) {
        std::cout << "  std::exception: " << e.what() << "\n";
    }

    std::cout << "\n=== 4. DESENROLLADO Y DESTRUCTORES ===\n";
    try {
        funcionQueFalla();
    } catch (const std::exception& e) {
        std::cout << "  atrapado después del unwind: " << e.what() << "\n";
    }

    std::cout << "\n=== 5. PARSEAR TEXTO ===\n";
    try {
        int n = std::stoi("no es un número");
        std::cout << n << "\n";
    } catch (const std::invalid_argument& e) {
        std::cout << "  stoi: " << e.what() << "\n";
    }

    std::cout << "\n=== 6. RETHROW ===\n";
    try {
        try {
            throw std::runtime_error("original");
        } catch (const std::exception& e) {
            std::cout << "  interno: " << e.what() << "\n";
            throw;  // relanza la misma excepción
        }
    } catch (const std::exception& e) {
        std::cout << "  externo: " << e.what() << "\n";
    }

    std::cout << "\n=== 7. catch(...) ===\n";
    try {
        throw 42;  // no es una excepción estándar
    } catch (...) {
        std::cout << "  atrapado cualquier tipo\n";
    }

    return 0;
}
