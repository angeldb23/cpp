/* =============================================================================
   MÓDULO 05 - LECCIÓN 19: enum y enum class: alcance, tipo y conversiones
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      Un enum define un TIPO con un conjunto finito de valores con nombre.
      Para un juego: direcciones, estados de personaje, códigos de error.

      enum "clásico" (C-style, sin clase):
          enum Estado { VIVO, MUERTO, HERIDO };
          // VIVO == 0, MUERTO == 1, HERIDO == 2
          // los nombres viven en el SCOPE de alrededor (sin calificar)
      Problemas:
      - VIVO/HERIDO se definen en el ámbito de alrededor (global): un
        segundo enum en el mismo archivo con un nombre igual colisiona.
      - Se convierte a int SILENCIAMENTE: Estado e = 3; y comparaciones
        `Estado == int` pasan sin aviso (bug clásico de "usó el número".

      enum class (C++11, el que vas a usar SIEMPRE):
          enum class Estado { Vivo, Muerto, Herido };
          Estado e = Estado::Vivo;    // calificado con ::
          // e == 0;                     ERROR de compilación: sin
          //                             conversión implícita a int
          int n = static_cast<int>(e);  // conversión explícita, con aviso
      Ventajas:
      - Alcance del enum: Estado::Vivo; el nombre no ensucia afuera.
      - Tipo fuerte: no es int disfrazado; no se mezcla con otros enums
        ni con ints.
      - Tipo subyacente elegible:
          enum class Id : unsigned char { Jugador, Enemigo, Prop };  // 1 byte

      El tipo subyacente controla el storage (sizeof) y el rango. Para
      enumeraciones que viajan por red o a disco, explicitarlo es una
      herramienta (evita interpretaciones cambiantes entre compiladores).

   2. ¿QUÉ PASA POR DEBAJO?
      Internamente un enum es un entero con nombre: cada enumerator
      obtiene un valor (0, 1, 2... o el que vos le asignes:
      enum class Nivel { Facil=1, Normal=5, Duro=10 }; o `= 0` para el
      siguiente). El tamaño: sin especificar, sizeof(Estado) depende del
      compilador (usualmente int); con `: unsigned char` garantizás
      1 byte.

      Las conversiones:
      - enum class → entero: SOLO explícito (static_cast). Predecible.
      - entero → enum class: static_cast también, sin validación: un
        valor fuera del rango queda en un valor indeterminado (por eso
        conviene validar antes de convertir).
      - enum clásico → entero: implícito (es la fuente de los ac pitfalls).

      En el juego: los switch de enums son el flujo universal de las
      máquinas de estados (estado de sprites, input, IA). En switch el
      compilador avisa si te olvidaste de un caso (parcialidad -
      -Wswitch) si no hay default.

   3. ERRORES COMUNES
      - Usar ints "mágicos" (if (estado == 1)) en vez de enumeraciones:
        el enum es el fix documentado.
      - Comparar enum clásico con int y que pase por silencio.
      - Poner el mismo nombre de enum member en dos enums clásicos:
        colisión de nombres en el scope global.
      - Calificar con :: un enum clásico: no existe (solo enum class
        tiene alcance propio).
      - Olvidar que los valores siguen al último asignado:
          enum class Nivel { Facil = 1, Normal = 5, Duro = 10 };
          // y sin igualdad explícita: enum class X { A = 10, B };  // B == 11
      - Guardar enums en archivos como int sin definir el tipo
        subyacente: cambia tamaño y fracciona tus datos.

   4. PARA PROFUNDIZAR
      https://en.cppreference.com/w/cpp/language/enum
   ============================================================================= */
#include <iostream>

// =========================================================================
// enum clásico: los nombres se filtran al scope de alrededor (evitalo).
// =========================================================================
enum EstadoViejo { VIVO, MUERTO, HERIDO };   // VIVO == 0 en el scope global

// =========================================================================
// enum class: alcance de clase y tipo fuerte.
// =========================================================================
enum class Estado { Vivo, Muerto, Herido };

enum class IdEntidad : unsigned char {   // subyacente: 1 byte
    Jugador,
    Enemigo,
    Prop
};

const char* nombreEstado(Estado e) {
    switch (e) {
        case Estado::Vivo:   return "vivo";
        case Estado::Muerto: return "muerto";
        case Estado::Herido: return "herido";
    }
    return "desconocido";    // inalcanzable: sin default, -Wswitch vigila
}

int main() {
    std::cout << "=== 1. ENUM CLASS: VALORES CALIFICADOS ===\n";
    Estado e = Estado::Herido;
    std::cout << "  estado: " << nombreEstado(e) << "\n";
    std::cout << "  sizeof(Estado) = " << sizeof(e) << "\n";
    std::cout << "  sizeof(IdEntidad) = " << sizeof(IdEntidad::Prop)
              << " (unsigned char subyacente)\n";

    std::cout << "\n=== 2. CONVERTIR CON static_cast (EXPLÍCITO) ===\n";
    int comoInt = static_cast<int>(e);
    std::cout << "  Estado::Herido como int: " << comoInt << "\n";
    // if (e == 2) {}       // ERROR de compilación: no hay conversión

    std::cout << "\n=== 3. ENUM CLÁSICO: LA FUGA DE NOMBRES ===\n";
    EstadoViejo v = HERIDO;
    std::cout << "  VIVO (sin calificar) = " << VIVO << "\n";
    std::cout << "  (enum clásico se convierte a int implícitamente:"
                 " por eso se usa enum class)\n";
    std::cout << "  sizeof(EstadoViejo): " << sizeof(v) << "\n";
    return 0;
}
