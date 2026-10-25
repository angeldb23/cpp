/* =============================================================================
   MÓDULO 05 - LECCIÓN 13: Regla de 5 (y el nexo con el módulo 08)
   =============================================================================
   RECORDATORIO: ya definiste las 3 de la regla de tres (lección 12). La
   regla de 5 agrega las DOS restantes cuando tu clase administra un
   recurso crudo:

   1. EXPLICACIÓN CONCEPTUAL
      Operaciones especiales completas (una clase con recurso crudo):
      1) destructor
      2) constructor de copia
      3) operator= de copia
      4) constructor de movimiento   ← apareció en C++11
      5) operator= de movimiento     ← idem

      ¿Qué es MOVER? Transferir la propiedad del recurso de un objeto a
      otro, sin copiar el contenido. Ejemplo: copiar un buffer de 1 GB
      copia 1 GB; moverlo transfiere el puntero y queda gratis.

      La separación didáctica de este curso:
      - MÓDULO 05 (este): la regla de CINCO como mapa + las tres que ya
        sabés escribir. El movimiento se MENCIONA y se DECLARA, no se
        implementa con std::move todavía.
      - MÓDULO 08: valor-categorías (lvalue/rvalue), std::move, cómo se
        escribe un constructor de movimiento en serio y cuándo el
        compilador usa cada operación.

      Analogía clave: "Copiar es fotocopiar el contrato. Mover es firmar
      la transferencia del título de propiedad."
      Después de mover, el emisor queda en un estado "válido pero
      indeterminado" (por ejemplo, con puntero nulo y tamaño 0): algo se
      hará con él, y el destructor igual se va a llamar sobre ese
      estado, así que tiene que seguir siendo un objeto sano.

   2. ¿QUÉ PASA POR DEBAJO?
      ¿Por qué no basta la regla de 3 en el recursos?
      - Contenedores (std::vector al agrandarse: lecciones del módulo 03)
        necesitan mover elementos: si la clase SOLO tiene copia, cada
        realojamiento copia todos los objetos (lento). Cuando un tipo
        tiene constructor de movimiento noexcept, el vector lo usa.
      - devolución por valor, swap, std::sort con elementos grandes:
        el costo se toca de inmediato.
      Con sólo copia (recursos caros) el compilador no puede "mover"
      porque no sabe; C++11 te deja declararlo.

      Las dos operaciones de movimiento se reciben como `T&&` (referencia
      rvalue). Por eso se pueden llamar sobre temporales o con
      std::move(existente) (detalle del módulo 08).

   3. ERRORES COMUNES
      - Definir el constructor de movimiento destruyendo el estado del
        emisor: el emisor debe quedar VALIDO (dtor será llamado).
      - Decir "con la regla de 3 alcanza": para tipos con recursos caros
        solo dice que las copias van a ser el doble de caras de lo que
        podían ser.
      - Mezclar las 5 operaciones al revés: mover caro y copiar barato.
        Al contrario: mueve lo barato (transferir punteros), copia
        lo necesario.
      - Escalar la regla de 5 a tipos sin recursos (Vector2, Punto):
        como regla de 0 se escribe TODO implícito y vale igual.

   4. PARA PROFUNDIZAR
      C++ Core Guidelines: C.20 (regla de 5), C.21 (regla de 0)
      https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#-c20-if-you-might
   ============================================================================= */
#include <iostream>

// =========================================================================
// La clase de lección 11/12 complementada: con el mapa de 5 operaciones.
// El movimiento se DECLARA con &&, pero la implementación completa llega
// en el módulo 08 (acá marcamos el esqueleto didáctico).
// =========================================================================
class Metal {
public:
    explicit Metal(const char* tipo, int gramos = 1000)
        : m_lingotes(new int[gramos] {}), m_gramos(gramos), m_tipo(tipo) {
        std::cout << "  ctor: " << m_tipo << " (" << m_gramos << " g)\n";
    }

    // 1) Destructor: libera el recurso crudo.
    ~Metal() { delete[] m_lingotes; }

    // 2) Constructor de copia: recurso nuevo + contenido.
    Metal(const Metal& otra)
        : m_lingotes(new int[otra.m_gramos] {}),
          m_gramos(otra.m_gramos), m_tipo("copia-de-" + otra.m_tipo) {
        for (int i = 0; i < m_gramos; ++i) m_lingotes[i] = otra.m_lingotes[i];
        std::cout << "  ctor copia: doble del material\n";
    }

    // 3) operator= de copia.
    Metal& operator=(const Metal& otra) {
        if (this == &otra) return *this;
        int* nuevos = new int[otra.m_gramos] {};
        for (int i = 0; i < otra.m_gramos; ++i) {
            nuevos[i] = otra.m_lingotes[i];
        }
        delete[] m_lingotes;
        m_lingotes = nuevos;
        m_gramos = otra.m_gramos;
        std::cout << "  operator= (copia) de '" << m_tipo << " <- "
                  << otra.m_tipo << "'\n";
        return *this;
    }

    // 4) Constructor de movimiento (MENCIONADO, implementado en 08):
    //    recibe referencia rvalue (T&&): "que viene derecho a moverse".
    //    Su cuerpo definitivo transfiere el puntero y vacía el emisor.
    Metal(Metal&& otra);

    // 5) operator= de movimiento (idem; cuerpo completo en 08).
    Metal& operator=(Metal&& otra);

    int gramos() const { return m_gramos; }
    const std::string& tipo() const { return m_tipo; }

private:
    int* m_lingotes;
    int m_gramos;
    std::string m_tipo;
};

int main() {
    std::cout << "=== 1. LAS 5 OPERACIONES SOBRE UN TIPO CON RECURSO ===\n";
    Metal bronce("bronce");
    {
        Metal copia = bronce;    // 2) constructor de copia
        std::cout << "  dentro del bloque: '" << copia.tipo() << "' ("
                  << copia.gramos() << " g)\n";
    }                            // 1) destructor del local
    Metal replica("replica");
    replica = bronce;            // 3) operator= de copia

    std::cout << "\n=== 2. LO QUE FALTA (MÓDULO 08) ===\n";
    std::cout << "  Este módulo deja declaradas 4 y 5 (movimiento):\n"
              << "  - transferir el puntero del emisor al receptor,\n"
              << "  - dejar al emisor en un estado válido (nullptr, 0).\n"
              << "  Hoy las declaración sin cuerpo: si se usaran, el\n"
              << "  linker las buscaría y no las encontraría.\n";
    return 0;
}
