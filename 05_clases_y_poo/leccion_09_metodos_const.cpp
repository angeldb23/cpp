/* =============================================================================
   MÓDULO 05 - LECCIÓN 09: Métodos const y const-correctness
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      Un método const promete NO MODIFICAR el objeto:

          int vida() const;         // solo lee
          void danificar(int d);     // muta (sin const)

      El sufijo const va DESPUÉS de los paréntesis, en la declaración Y en
      la definición. Los efectos:
      - Dentro del método, this pasa de Cuenta* a const Cuenta*: no podés
        escribir miembros (ni burlarlo con `const_cast` para "arreglarlo":
        es una mentira).
      - Sólo métodos const pueden llamarse sobre objetos const:
          const Cuenta c = ...;
          c.saldo();     bien: método const
          c.extraer(5);  ERROR: método no-const sobre objeto const

      La const-correctness es un contrato que se propaga: los getters son
      const, los parámetros const& no copian y prometen no tocar (módulo
      02), los métodos const no mutan. Un getter const permite pasar el
      objeto a código de solo lectura con confianza.

      Métodos const que deben tocar algo: el mutable. Si un método const
      necesita cachear un calculo o contar llamadas (detalle interno,
      no invariantes), el atributo se marca mutable:

          mutable int cuentaConsultas = 0;    // escribible desde const

   2. ¿QUÉ PASA POR DEBAJO?
      `void f() const` se compila como una función que recibe `const T*
      this`. El compilador se queja si escribís un atributo dentro (error
      de "assignment of member in read-only"). Para std::string, un getter
      por valor copia (reserva memoria, copia bytes); por const& no copia.
      Por eso la granularidad del const importa para el costo:
      - int/float/pointer chico: por valor.
      - string/vector/estructuras pesadas: const& para lectura.

      mutable NO quita el const del objeto: sólo marca un atributo
      específico como "detalle de administración, no estado observable".
      El compilador te pide la palabra para que quede explícito.

   3. ERRORES COMUNES
      - Olvidarla en la definición (solo ponerla en la declaración):
        el compilador las trata como funciones distintas y no linka:
        "undefined reference to Cuenta::saldo()".
      - Marcar const un método que en realidad muta: error de compilación
        útil, no intentes saltearlo con const_cast.
      - Devolver datos internos por referencia no-const desde un getter:
        rompe el contrato.
      - Creer que const se propaga a punteros: un método const que
        devuelve un puntero a memoria dinámica permite modificar el
        contenido apuntado.

   4. PARA PROFUNDIZAR
      https://en.cppreference.com/w/cpp/language/member_functions
      https://en.cppreference.com/w/cpp/language/cv
   ============================================================================= */
#include <iostream>
#include <sstream>

class Inventario {
public:
    Inventario() = default;

    void agregar(int id) {
        ++m_tamano;
        (void)id;
    }
    void quitarTodo() { m_tamano = 0; }

    // Métodos const: observan, no mutan.
    bool vacio() const { return m_tamano == 0; }
    int tamano() const { return m_tamano; }

    // mutable: detalle de administración que un const puede escribir.
    int consultas() const { return m_consultas; }
    int tamanoMedido() const {
        ++m_consultas;          // mutable: permitido en const
        return m_tamano;
    }

private:
    int m_tamano = 0;
    mutable int m_consultas = 0;
};

bool tieneTresItems(const Inventario& inv) {   // solo lectura (módulo 02)
    return inv.tamano() == 3;
}

int main() {
    std::cout << "=== 1. MÉTODOS CONST SE PUEDEN LLAMAR SOBRE OBJETOS CONST ===\n";
    Inventario bolsa;
    bolsa.agregar(1);
    bolsa.agregar(2);
    bolsa.agregar(3);
    std::cout << "  bolsa: " << bolsa.tamano() << " ítems\n";

    const Inventario solaLectura = bolsa;
    std::cout << "  copia const: " << solaLectura.tamano()
              << " ítems (vacio(): " << (solaLectura.vacio() ? "sí" : "no")
              << ")\n";
    // solaLectura.agregar(4);  // ERROR de compilación: agregar no es const

    std::cout << "\n=== 2. PARÁMETRO CONST& USA SOLO MÉTODOS CONST ===\n";
    std::cout << "  tieneTresItems(bolsa) = "
              << (tieneTresItems(bolsa) ? "true" : "false") << "\n";

    std::cout << "\n=== 3. MUTABLE PARA ADMINISTRACIÓN ===\n";
    std::cout << "  tamanoMedido(): " << bolsa.tamanoMedido() << " ("
              << bolsa.consultas() << " consulta)\n";
    std::cout << "  tamanoMedido(): " << bolsa.tamanoMedido() << " ("
              << bolsa.consultas() << " consultas)\n";
    std::cout << "  (el contador suma aunque esté en métodos const)\n";
    return 0;
}
