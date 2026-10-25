/* =============================================================================
   MÓDULO 04 - LECCIÓN 13: void*, reinterpret_cast y alineación
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      void* es un puntero sin tipo: guarda una dirección, pero no dice qué hay
      allí. No se puede desreferenciar directamente; hay que convertirlo al
      tipo correcto primero.

          int x = 42;
          void* v = &x;                 // dirección sin tipo
          int* p = static_cast<int*>(v); // recupera el tipo
          std::cout << *p;

      reinterpret_cast se usa para conversiones entre tipos de puntero no
      relacionados y para pasar una dirección a un entero del tamaño adecuado
      (std::uintptr_t). Es la conversión más peligrosa: el compilador no
      verifica que el resultado sea válido.

      La alineación es la dirección mínima a la que un tipo puede empezar.
      alignof(T) dice cuántos bytes exige T; alignas(N) fuerza una alineación
      mayor. Un int suele exigir 4 bytes: su dirección debe ser múltiplo de 4.
      Acceder a un objeto desalineado es UB en algunos tipos y lento en todos.

   2. ¿QUÉ PASA POR DEBAJO?
      El hardware carga datos en bloques alineados. Un acceso desalineado puede
      requerir dos lecturas o provocar una excepción (en ARM, por ejemplo). Por
      eso el compilador inserta padding en las structs y alinea las variables.

      reinterpret_cast no cambia los bits: solo reinterpreta el tipo. Si hacés
      `reinterpret_cast<double*>(&unInt)` y desreferenciás, violás las reglas
      de aliasing y el resultado es indefinido. Para convertir valores usá
      static_cast; para punteros relacionados, static_cast también alcanza.

   3. ERRORES COMUNES
      - Desreferenciar un void* sin convertirlo.
      - Usar reinterpret_cast para "convertir" un int a double (no convierte
        el valor: reinterpreta los bits).
      - Suponer que cualquier dirección está alineada para cualquier tipo.
      - Hacer aritmética de bytes sobre T* sin pasar por char* o std::byte*.
      - Ignorar el aliasing estricto al leer un objeto por un tipo distinto.

   4. PARA PROFUNDIZAR
      https://en.cppreference.com/w/cpp/language/reinterpret_cast
      https://en.cppreference.com/w/cpp/language/alignof
   ============================================================================= */
#include <cstddef>
#include <cstdint>
#include <iostream>

struct ConPadding {
    char a;    // 1 byte
    int b;     // 4 bytes, con padding antes
    char c;    // 1 byte, con padding al final
};

// Comprueba si una dirección es múltiplo de la alineación pedida.
bool estaAlineado(const void* p, std::size_t alineacion) {
    auto direccion = reinterpret_cast<std::uintptr_t>(p);
    return alineacion != 0 && direccion % alineacion == 0;
}

// Avanza n bytes sobre un bloque sin tipo.
void* avanzarBytes(void* p, std::size_t n) {
    auto* bytes = static_cast<unsigned char*>(p);
    return bytes + n;
}

int main() {
    std::cout << "=== 1. void* GUARDA UNA DIRECCIÓN SIN TIPO ===\n";
    int x = 42;
    void* v = &x;
    std::cout << "v = " << v << "\n";
    // *v = 10;  // ERROR: no se puede desreferenciar void*
    int* p = static_cast<int*>(v);
    std::cout << "*p = " << *p << "\n";

    std::cout << "\n=== 2. reinterpret_cast A ENTERO ===\n";
    auto direccion = reinterpret_cast<std::uintptr_t>(&x);
    std::cout << "dirección de x = 0x" << std::hex << direccion << std::dec << "\n";
    std::cout << "¿alineada a 4? " << (estaAlineado(&x, 4) ? "sí" : "no") << "\n";

    std::cout << "\n=== 3. ALINEACIÓN DE TIPOS ===\n";
    std::cout << "alignof(char)        = " << alignof(char) << "\n";
    std::cout << "alignof(int)         = " << alignof(int) << "\n";
    std::cout << "alignof(double)      = " << alignof(double) << "\n";
    std::cout << "alignof(std::uint64_t) = " << alignof(std::uint64_t) << "\n";

    std::cout << "\n=== 4. PADDING EN UNA STRUCT ===\n";
    std::cout << "sizeof(ConPadding) = " << sizeof(ConPadding) << " bytes\n";
    std::cout << "  char + padding + int + char + padding\n";
    std::cout << "offset de b = " << offsetof(ConPadding, b) << "\n";
    std::cout << "offset de c = " << offsetof(ConPadding, c) << "\n";

    std::cout << "\n=== 5. alignas ===\n";
    alignas(16) unsigned char buffer[32] = {};
    std::cout << "buffer alineado a 16: "
              << (estaAlineado(buffer, 16) ? "sí" : "no") << "\n";
    std::cout << "buffer[0] = " << static_cast<int>(buffer[0]) << "\n";

    std::cout << "\n=== 6. AVANZAR BYTES CON void* ===\n";
    int valores[3] = {10, 20, 30};
    void* base = valores;
    void* segundo = avanzarBytes(base, sizeof(int));
    std::cout << "*static_cast<int*>(segundo) = "
              << *static_cast<int*>(segundo) << "\n";

    std::cout << "\n=== 7. LO QUE NO HAY QUE HACER ===\n";
    // double* malo = reinterpret_cast<double*>(&x);
    // std::cout << *malo;  // UB: aliasing y tamaño distintos
    std::cout << "no reinterpretes un tipo como otro y lo desreferencies\n";

    return 0;
}
