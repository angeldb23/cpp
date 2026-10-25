/* =============================================================================
   MÓDULO 09 - LECCIÓN 02: Templates de clase y especialización
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
      Igual que con las funciones, una clase puede ser un template:
      std::vector<T>, std::optional<T>, std::unique_ptr<T> son todos templates
      de clase. Cada combinación de parámetros (Caja<int>, Caja<std::string>)
      es un TIPO DIFERENTE y sin relación entre sí.

      Temas de la lección:
        - Template de clase básico (una caja que guarda un valor).
        - Agregados genéricos con CTAD (C++20) y operator== por defecto.
        - Un método que devuelve OTRA instanciación del mismo template.
        - Parámetros por defecto: template <typename T, std::size_t N = 4>.
        - Especialización TOTAL y PARCIAL de clases.
        - Variable templates (también se especializan) y plantillas con DOS
          parámetros, como std::vector<T, Allocator>.
        - Alias templates: template <typename T> using Lista = std::vector<T>.

   2. ¿QUÉ PASA POR DEBAJO?
      Cada instanciación genera una clase completa e independiente. Por eso
      los miembros static se reparten POR INSTANCIACIÓN: Contador<int>::usos
      y Contador<double>::usos son dos variables distintas. Cuando el
      compilador ve Describir<int*>, busca la especialización más específica
      que coincida: primero la total, después las parciales, y al final la
      plantilla primaria.

   3. ERRORES COMUNES Y BUENAS PRÁCTICAS:
      - Las funciones miembro de un template se instancian SOLO si las usás:
        podés tener un método que no compila para cierto T y nadie se entera
        hasta que lo llamen.
      - No se puede especializar parcialmente una FUNCIÓN template (sí se puede
        sobrecargar). Para clases y variables sí.
      - std::vector parece tener un solo parámetro, pero son DOS: el tipo y el
        allocator. Para especializar "todos los vectores" hay que nombrar los dos.
      - Para lanzar errores (std::out_of_range) incluí <stdexcept>.

   4. PARA PROFUNDIZAR:
      https://en.cppreference.com/w/cpp/language/class_template
      https://en.cppreference.com/w/cpp/language/partial_specialization
      https://en.cppreference.com/w/cpp/language/variable_template
      https://en.cppreference.com/w/cpp/language/class_template_argument_deduction
   ============================================================================= */
#include <cstddef>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

// --- 1. Template de clase básico: una caja con 0 o 1 valor ---
template <typename T>
class Caja {
public:
    void guardar(const T& v) { valor_ = v; }
    void vaciar() { valor_.reset(); }
    bool vacia() const { return !valor_.has_value(); }
    const T& leer() const {
        if (!valor_) throw std::out_of_range("caja vacia");
        return *valor_;
    }

private:
    std::optional<T> valor_;
};

// --- 2. Agregado genérico: CTAD y == por defecto (C++20) ---
template <typename T>
struct Rango {
    T desde;
    T hasta;

    bool operator==(const Rango&) const = default;   // compara miembro a miembro

    // Un método template que devuelve OTRA instanciación: Rango<U>
    template <typename U>
    Rango<U> convertir() const {
        return Rango<U>{static_cast<U>(desde), static_cast<U>(hasta)};
    }
};

template <typename T>
Rango<T> hacerRango(T a, T b) { return Rango<T>{a, b}; }

// --- 3. Parámetro por defecto ---
template <typename T, std::size_t N = 4>
struct Buffer {
    T datos[N]{};
    static constexpr std::size_t capacidad() { return N; }
};

// --- 4. Un miembro static por instanciación ---
template <typename T>
struct Contador {
    static inline int usos = 0;
    Contador() { ++usos; }
};

// --- 5. Especialización total y parcial de CLASES ---
template <typename T>
struct Describir {                       // plantilla primaria
    static std::string texto() { return "tipo generico"; }
};

template <>
struct Describir<int> {                  // total: SOLO para int
    static std::string texto() { return "entero con signo"; }
};

template <typename T>
struct Describir<T*> {                   // parcial: para CUALQUIER puntero
    static std::string texto() { return "puntero a " + Describir<T>::texto(); }
};

// --- 6. Dos parámetros: hay que nombrar T y el allocator ---
template <typename T>
struct EsVector : std::false_type {};

template <typename T, typename A>
struct EsVector<std::vector<T, A>> : std::true_type {};

// --- 7. Variable templates: también se especializan ---
template <typename T>
constexpr bool esDecimal = false;                 // primaria

template <>
constexpr bool esDecimal<double> = true;          // total

template <>
constexpr bool esDecimal<float> = true;

template <typename T>
constexpr bool esCaja = false;

template <typename T>
constexpr bool esCaja<Caja<T>> = true;            // parcial

// --- 8. Alias template ---
template <typename T>
using Lista = std::vector<T>;

int main() {
    std::cout << "=== 1. CAJA GENÉRICA ===\n";
    Caja<int> c;
    std::cout << "vacia al inicio: " << c.vacia() << "\n";
    c.guardar(42);
    std::cout << "leer = " << c.leer() << "\n";
    Caja<std::string> t;
    t.guardar("hola");
    std::cout << "leer = " << t.leer() << "\n";
    try {
        Caja<double> vacia;
        vacia.leer();
    } catch (const std::out_of_range& e) {
        std::cout << "excepción esperada: " << e.what() << "\n";
    }

    std::cout << "\n=== 2. CTAD, == POR DEFECTO Y CONVERSIÓN ===\n";
    Rango r{1, 5};                       // deduce Rango<int>
    Rango<int> r2{1, 5};
    std::cout << "r == r2: " << (r == r2) << "\n";
    auto rd = r.convertir<double>();     // Rango<double>
    std::cout << "convertido: " << rd.desde << " .. " << rd.hasta << "\n";
    auto r3 = hacerRango(2.5, 7.5);
    std::cout << "hacerRango: " << r3.desde << " .. " << r3.hasta << "\n";

    std::cout << "\n=== 3. PARÁMETRO POR DEFECTO ===\n";
    Buffer<int> corto;                   // N = 4
    Buffer<int, 16> largo;
    std::cout << "capacidad corto = " << corto.capacidad() << "\n";
    std::cout << "capacidad largo = " << largo.capacidad() << "\n";

    std::cout << "\n=== 4. UN STATIC POR INSTANCIACIÓN ===\n";
    Contador<int> a, b;
    Contador<double> d;
    std::cout << "Contador<int>::usos    = " << Contador<int>::usos << "\n";
    std::cout << "Contador<double>::usos = " << Contador<double>::usos << "\n";
    (void)a; (void)b; (void)d;

    std::cout << "\n=== 5. ESPECIALIZACIÓN DE CLASES ===\n";
    std::cout << "Describir<double>:  " << Describir<double>::texto() << "\n";
    std::cout << "Describir<int>:     " << Describir<int>::texto() << "\n";
    std::cout << "Describir<int*>:    " << Describir<int*>::texto() << "\n";
    std::cout << "Describir<char**>:  " << Describir<char**>::texto() << "\n";

    std::cout << "\n=== 6. DOS PARÁMETROS (vector<T, A>) ===\n";
    std::cout << "EsVector<vector<int>>: " << EsVector<std::vector<int>>::value << "\n";
    std::cout << "EsVector<int>:        " << EsVector<int>::value << "\n";

    std::cout << "\n=== 7. VARIABLE TEMPLATES ===\n";
    std::cout << "esDecimal<double>: " << esDecimal<double> << "\n";
    std::cout << "esDecimal<int>:    " << esDecimal<int> << "\n";
    std::cout << "esCaja<Caja<int>>: " << esCaja<Caja<int>> << "\n";
    std::cout << "esCaja<int>:       " << esCaja<int> << "\n";

    std::cout << "\n=== 8. ALIAS TEMPLATE ===\n";
    Lista<std::string> nombres = {"Ana", "Beto"};
    std::cout << "nombres tiene " << nombres.size() << " elementos\n";

    return 0;
}
