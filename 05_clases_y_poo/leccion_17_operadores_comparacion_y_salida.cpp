/* =============================================================================
   MÓDULO 05 - LECCIÓN 17: Operadores de comparación (==) y de salida (<<)
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      Dos operadores que TODA clase de datos seria termina teniendo:

      == (igualdad): regla de oro: funciones libres que no mutan.
      Preferencia canónica: función libre con dos const&:

          bool operator==(const Salida& a, const Salida& b) {
              return a.m_id == b.m_id && a.m_minutos == b.m_minutos;
          }
          bool operator!=(const Salida& a, const Salida& b) {
              return !(a == b);       // reutiliza: nunca se desincroniza
          }

      (En C++20, con `bool operator==(...) = default;` el compilador
      compara miembro- por-miembro y generate `!=` gratis. Es el
      shortcut de esta lección.)

      << (salida a stream): el operador que usás desde el módulo 01 sin
      saber que era sobrecargado: `std::cout << x` llama a
      operator<<(cout, x). Para la clase, se escribe como función libre
      (porque el izquierdo, el stream, no es de tu clase):

          std::ostream& operator<<(std::ostream& os, const Salida& s) {
              std::cout << "  " << s.m...
              os << "[" << s.m_id << " " << s.m_minutos << " min]";
              return os;   // ¡CRUCIAL! permite 'cout << a << b'
          }
      Devuelve la referencia del stream para poder encadenar:
      `std::cout << a << " " << b` es ((cout << a) << " ") << b.

      NO lo escribas como miembro: en `cout << perro` el receptor
      izquierdo es el stream (no un Perro), así que la función tiene
      que ser libre y recibir el stream como primer parámetro.

   2. ¿QUÉ PASA POR DEBAJO?
      `operator<<` es la misma maquinaria que los operadores
      aritméticos: una función que el compilador convierte a… las
      mismas llamadas. El stream es un BUFFER: cada `<<` escribe bytes
      (int→caracteres via to_chars, doubles via formatter) y se vacía
      (flush) al terminal en endl o al final del programa (con \n no
      el flush).

      `bool operator==(...) = default;` compara los miembros en orden
      de declaración y devuelve bool. Es el patrón C++20 más
      modernos. Los "defaulted comparisons" restan texto: el == queda
      igual al == de los miembros; los `<`/`>` modernos llegan en el
      módulo 11 con el operador `<=>`.

   3. ERRORES COMUNES
      - Olvidar `return os;` en operator<<: el encadenado deja de
        compilar (cout << a << b necesita que cout << a devuelva el
        stream, y si devolvió void no hay receiver para el segundo <<).
      - Escribir << como método miembro de tu clase: `cout << perro`
        no compila, porque el receptor (cout) no es de tu clase.
      - == que compara MÁS de lo que importa (comparar un cache) o
        menos (comparar id y devolver true aunque difieran): la
        igualdad debe ser coherente con != y con el estado visible.
      - Marcar operator== con lógica sobre métodos no-const: si llama
        a getters, esos getters deben ser const (lección 09).

   4. PARA PROFUNDIZAR
      https://en.cppreference.com/w/cpp/language/overload_resolution
      https://en.cppreference.com/w/cpp/language/default_comparisons
   ============================================================================= */
#include <iostream>
#include <string>

class Perro {
public:
    Perro(const std::string& nombre, int edad) : m_nombre(nombre), m_edad(edad) {}

    // == de C++20 con = default: compara miembros en orden.
    bool operator==(const Perro& otra) const = default;
    // != sale gratis en C++20 (generado por el compilador).

    const std::string& nombre() const { return m_nombre; }
    int edad() const { return m_edad; }

private:
    std::string m_nombre;
    int m_edad;
};

// << como función libre: el stream va a la IZQUIERDA (no es nuestro).
std::ostream& operator<<(std::ostream& os, const Perro& p) {
    os << p.nombre() << " (" << p.edad() << " años)";
    return os;      // encadenable: (cout << p) << algo más
}

int main() {
    std::cout << "=== 1. operator== CON = default (C++20) ===\n";
    Perro firulais("Firulais", 3);
    Perro firu_2("Firulais", 3);
    Perro mature("Mature", 7);
    std::cout << "  firulais == firu_2: " << (firulais == firu_2) << "\n";
    std::cout << "  firulais != mature:  " << (firulais != mature) << "\n";
    std::cout << "  (el default compara campo a campo, sin escribir nada)\n";

    std::cout << "\n=== 2. operator<< COMO FUNCIÓN LIBRE ===\n";
    std::cout << "  perro: " << firulais << "\n";
    std::cout << "  apoyo: " << firulais << " y " << mature << "\n";
    std::cout << "  (el encadenado usa que << devolvió el stream)\n";
    return 0;
}
