/* =============================================================================
   MÓDULO 05 - LECCIÓN 01: Organización en varios archivos: headers y .cpp
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      Cuando empezás a escribir clases, romper el código en varios archivos
      deja de ser opcional. El esquema clásico de C++:

          nombre.h  (header): QUÉ se ofrece. Declaraciones de la interfaz.
          nombre.cpp (implementación): CÓMO se hace. Las definiciones.

          // cuenta.h
          #pragma once            // evita incluir el header dos veces
          class Cuenta {
          public:
              void depositar(double monto);   // solo se declara
          private:
              double saldo = 0.0;
          };

          // cuenta.cpp
          #include "cuenta.h"
          void Cuenta::depositar(double monto) {   // se define acá
              if (monto > 0) saldo += monto;
          }

      Reglas básicas:
      a) Un header se "#incluye"; un .cpp se compila aparte y luego se
         linkea. El compilador trabaja archivo por archivo; el linker une.
      b) Include guards o #pragma once: sin ellos, incluir el mismo header
         dos veces (directa o indirectamente) duplica sus declaraciones y
         el compilador se queja de redefinición.

          // guard clásico (funciona con cualquier compilador)
          #ifndef CUENTA_H
          #define CUENTA_H
          // ... contenido ...
          #endif

      c) Declaración vs definición: una declaración anuncia que algo existe
         ("int sumar(int, int);"). Una definición da el cuerpo o el valor.
         Podés declarar el mismo nombre muchas veces; definirlo UNA sola
         en todo el programa. Esa regla es la ODR (One Definition Rule).

      d) El include del .cpp siempre va primero ("este archivo par"). La
         clase principal de cada .cpp se declara en su propio header, y
         por eso el header debe compiler solo: si cuenta.h olvida un
         #include, vas a descubrirlo compilando cuenta.cpp y no antes.

   2. ¿QUÉ PASA POR DEBAJO?
      #include hace pegado de texto: el preprocesador reemplaza la línea
      por el contenido completo del archivo incluido. "Compilar un .cpp"
      en realidad compila una "unidad de traducción": tu archivo más todo
      lo que arrastraron sus #include. Por eso compilar crece con los
      headers: cada unidad de traducción paga el costo de leerlos.

      El compilador genera un archivo objeto (.o) por cada .cpp, con las
      definiciones etiquetadas. El linker junta todos los .o, resuelve las
      referencias cruzadas (el código del main.cpp llama a algo definido
      en cuenta.o) y falla con "undefined reference" si nunca encontró
      una definición.

   3. ERRORES COMUNES
      - Definir funciones normales (no inline) en un header: si el header
        lo incluyen dos .cpp distintos, cada unidad de traducción tiene su
        propia definición y el linker protesta. Las definiciones van al
        .cpp (las excepciones: funciones inline, templates, constexpr).
      - Olvidar el include guard y ver errores raros de redefinición.
      - #include "cuenta.h" dentro de cuenta.cpp escribido mal o con
        <cuenta.h>: los <...> son para la librería estándar y rutas del
        sistema; las comillas son tus archivos.
      - Creer que el .cpp "ya incluye lo que usa el header": si en el
        .cpp usás std::string, incluís <string> vos, aunque el header
        ya lo traiga de regalo.

   4. PARA PROFUNDIZAR
      https://en.cppreference.com/w/cpp/preprocessor
      https://en.cppreference.com/w/cpp/language/definition (ODR)
   ============================================================================= */
#include <iostream>
#include <string>

// =========================================================================
// En una sola lección no hay multi-archivo real, así que simulamos las dos
// mitades en un mismo archivo para ver la mecánica de la ODR y el linker.
// Cuando armes el proyecto, cortá la parte "HEADER" a un .h real y la
// "IMPLEMENTACIÓN" a su .cpp: el contenido queda igual.
// =========================================================================

// ------------------------- PARTE A: lo que iría al header ----------------
// Declaración pura: anuncia la existencia sin dar el cuerpo.
namespace banco {
class Cuenta;
}  // forward declaration: apuntadores/referencias a clase incompleta OK

namespace banco {
class Cuenta {
public:
    void depositar(double monto);       // DECLARADA: se define en la "Parte B"
    double saldo() const { return m_saldo; }  // definida acá: una línea, inline
private:
    double m_saldo = 0.0;
};
}  // namespace banco

// ------------------- PARTE B: lo que iría al .cpp correspondiente --------
// DEFINICIÓN (única en todo el programa: ODR).
void banco::Cuenta::depositar(double monto) {
    if (monto > 0) {
        m_saldo += monto;
    }
}

// =========================================================================
// Demostración de la ODR: una función se declara tantas veces como quiera,
// pero se define UNA sola vez.
// =========================================================================
// Declaraciones repetidas (todas válidas):
int sumar(int a, int b);
int sumar(int a, int b);
int sumar(int a, int b);
// ... y ahora sí, la única definición:
int sumar(int a, int b) { return a + b; }

int main() {
    std::cout << "=== 1. DECLARADA EN 'EL HEADER', DEFINIDA EN 'EL .CPP' ===\n";
    banco::Cuenta c;
    c.depositar(250.0);
    c.depositar(-100.0);  // el método ignora depósitos inválidos
    std::cout << "saldo final: " << c.saldo() << "\n";

    std::cout << "\n=== 2. ODR: TRES DECLARACIONES, UNA DEFINICIÓN ===\n";
    std::cout << "sumar(2, 3) = " << sumar(2, 3) << "\n";

    std::cout << "\n(OJO: el include es pegado de texto. Si incluyeras este\n"
                 "archivo dos veces sin guard, tendrías DOS main y el linker\n"
                 "fallaría: multiple definition of 'main'.)\n";
    return 0;
}
