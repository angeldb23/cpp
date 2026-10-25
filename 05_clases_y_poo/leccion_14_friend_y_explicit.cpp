/* =============================================================================
   MÓDULO 05 - LECCIÓN 14: friend, explicit y operadores de conversión
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      `friend`: la puerta trasera controlada. Un amigo declarado rompe el
      acceso privado PERO solo para eso que declaraste:

          class Tau {
              friend bool comparar(const Tau& a, const Tau& b);
          };
          bool comparar(const Tau& a, const Tau& b) {
              return a.letra == b.letra;   // accede a privados (amiga)
          }

      Usos legítimos realistas:
      - Pares de funciones soportadas que necesitan dos objetos A LA
        VEZ (operadores binarios simétricos: `2.0 * vector`).
      - Una clase "contenedor" y su clase "iterador" (amistad mutua).
      - El patrón testado: una función de testeo que inspecciona el
        estado interno.
      
      No es un "haz comunidad con todos": friend no se hereda, no es
      transitivo (el amigo de mi amigo no es mi amigo) y se escribe
      DENTRO de la clase, así que se accede exactamente a lo listado.

      `explicit`: corta las conversiones implícitas sorpresa:

          class Dimension {
              explicit Dimension(int px);   // NO deja convertir int → Dimension
          };
          void meter(Dimension d);
          meter(800);           // ERROR si explicit; compila si no lo es
          meter(Dimension(800));  // siempre OK: conversión explícita

      ¿Para qué? Un constructor de un argumento, si no es explicit, define
      una conversión silenciosa "de lo que sea a mi clase". Eso genera
      bugs sutiles: una función que esperaba letras puede aceptar
      un número que significa "otra cosa".

      Operadores de conversión (operator Tipo()): el inverso. Con
      `explicit` en ellos (C++11) también evitan la conversión involun-
      taria: solo actúan en contextos donde "se necesita" un int (if,
      comparaciones, cast explícito).

   2. ¿QUÉ PASA POR DEBAJO?
      friend no es una relación entre CLASES: es un permiso sobre una
      función o clase concreta. No agrega acoplamiento bidireccional de
      clases (eso es una mala práctica diferente).

      `class X { X(int); };` sin explicit significa que `X x = 5;` es
      legal: el compilador llama al constructor con 5 sin que nadie lo
      pida. Con explicit solo acepta construcción directa `X x(5);`
      (o en C++11: `X x{5};`).

   3. ERRORES COMUNES
      - Añadir friend "para arreglar" un error de acceso: primero
        preguntate si la función no debería ser MIEMBRO o si el atributo
        no debería estar expuesto por un getter.
      - No marcar explicit un constructor de un parámetro: perdés la
        conversión implícita controlada y aparecen errores finos.
      - Confundir friend con protector del módulo: friend no comparte
        herencia (las clases hijas NO son amigas).
      - Operadores de conversión a tipos primitivos sin explicit: la
        clase se convierte a cualquier cosa (el bool de una clase
        Archivo era el clásico) y provoca sorpresas en comparaciones.

   4. PARA PROFUNDIZAR
      https://en.cppreference.com/w/cpp/language/friend
      https://en.cppreference.com/w/cpp/language/explicit
      https://en.cppreference.com/w/cpp/language/cast_operator
   ============================================================================= */
#include <iostream>
#include <string>

class Carta {
public:
    explicit Carta(int valor) : m_valor(valor) {}   // explicit: sin magia

    int valor() const { return m_valor; }

    friend bool envidiar(const Carta& a, const Carta& b);

private:
    int m_valor;    // invariante: 1..12
};

// Función amiga: compara el ESTADO INTERNO de dos cartas.
bool envidiar(const Carta& a, const Carta& b) {
    return a.m_valor == b.m_valor;      // accede a privados (es amiga)
}

class CofreOculto {
public:
    explicit CofreOculto(int llaves) : m_llaves(llaves) {}

    // Conversión a bool EXPLÍCITA: útil en if (cofre), imposible en
    // accidentally-of `cofre + 3`.
    explicit operator bool() const { return m_llaves > 0; }

    int llaves() const { return m_llaves; }

private:
    int m_llaves;
};

void servir(const Carta&) { std::cout << "  carta servida\n"; }

int main() {
    std::cout << "=== 1. EXPLICIT: SIN CONVERSION IMPLÍCITA ===\n";
    Carta as(1);
    servir(as);
    servir(Carta(7));       // convertir con intención: siempre OK
    // servir(7);           // ERROR de compilación: int ~ no es una Carta
    std::cout << "  (sin explicit, servir(7) compilaría por sorpresa)\n";

    std::cout << "\n=== 2. FRIEND: FUNCIÓN AMIGA CON DOS OBJETOS ===\n";
    Carta sota(11);
    Carta otraSota(11);
    std::cout << "  envidiar(sota, otraSota): "
              << (envidiar(sota, otraSota) ? "true" : "false") << "\n";
    std::cout << "  (solo esta función accede a los privados; las demás no)\n";

    std::cout << "\n=== 3. OPERADOR DE CONVERSIÓN EXPLÍCITO A BOOL ===\n";
    CofreOculto cerrado(0);
    CofreOculto abierto(2);
    if (cerrado) std::cout << "  cerrado tiene llaves\n";
    else std::cout << "  cerrado está vacío\n";
    if (abierto) std::cout << "  abierto tiene " << abierto.llaves()
                           << " llaves\n";
    return 0;
}
