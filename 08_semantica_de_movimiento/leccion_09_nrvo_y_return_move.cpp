/* =============================================================================
   MÓDULO 08 - LECCIÓN 09: NRVO a fondo: cuándo NO mover en el return
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      La L08 presentó la elisión y la NRVO. Esta lección se concentra
      en la pregunta del return que más estilo feo genera:

          ¿Hace falta escribir `std::move` cuando devuelvo un objeto?

      RESPUESTA: NO. En ninguna de las tres situaciones normales:

      A) Devolver un LOCAL:
             Buffer leerNivel() {
                 Buffer nivel(2048);
                 return nivel;               // BIEN tal cual
             }
         El estándar ordena que, para este return exacto, el
         compilador trate `nivel` como RVALUE aunque sea lvalue (es
         la única excepción a la regla "un nombre es lvalue"). Si
         corre NRVO, ni move ni copy; si no corre, mueve. Siempre
         al máximo de gratis posible. El std::move explícito
         PROHÍBE el NRVO (le exige "move sí o sí" al compilador y
         mata la construcción in-place) y llama al constructor de
         movimiento que podía no llamarse: es una pesimización
         clásica, avisada por -Wpessimizing-move en GCC y Clang.

      B) Devolver un PARÁMETRO por valor:
             Buffer passthrough(Buffer p) { return p; }
         `p` es un lvalue con nombre... pero el estándar TAMBIÉN lo
         trata como rvalue en el return (misma excepción). Sale en
         movimiento sin escribir nada.

      C) Tipos move-only (los unique_ptr del módulo 07):
             std::unique_ptr<Nivel> cargar() {
                 auto nivel = std::make_unique<Nivel>();   // L04
                 return nivel;                // BIEN: move implícito
             }
         Con `return std::move(nivel);` compila igual, pero es
         redundante: -Wredundant-move lo marca.

      Escribir std::move acá le quita la decisión al compilador: es
      menos claro y nunca más rápido. El return simple es la línea
      que gana.

    2. ¿QUÉ PASA POR DEBAJO?
      Regla del estándar (breve): en un `return expr;` donde expr es
      el NOMBRE de una variable local (o parámetro) con el mismo tipo
      (sin contar const/volatile) que el tipo de retorno, el
      compilador PRIMERO intenta la elisión: construir el objeto
      directamente en el espacio del resultado. Si la elisión no fue
      posible (ramas que devuelven objetos distintos, etc.), la
      resolución de sobrecargas trata a `expr` como rvalue: elige el
      constructor de movimiento antes que el de copia, si existe;
      si no hay movimiento, copia.

      Esa "mirada como rvalue" ES std::move, aplicado por el
      estándar SOLO en este caso del return. Es la única excepción
      de la regla del lvalue de la L01, y existe por una experiencia
      histórica: devolver por valor es la forma natural de las
      fábricas en C++, y había que hacerla gratis.

    3. ERRORES COMUNES
      - `return std::move(local);` por querer parecer experto: pessimiza
        (impide NRVO) y hace parecer "experto" al autor cuando hace
        lo contrario. Con la línea de compilación del curso
        (`-Wall -Wextra`) el aviso no está activo por defecto, pero
        en el módulo 15 se arma el set de advertencias completo
        (incluido -Wpessimizing-move/-Wredundant-move).
      - "Si el tipo no tiene movimiento, ¿qué pasa con el return de
        un local?": si la NRVO corre, nada (no hay ninguna llamada);
        si no, aplica la copia. El std::move no "desatasca" nada; el
        cast a rvalue no fabrica un constructor de movimiento que no
        existe.
      - Devolver referencias "para ahorrar": NO sustituye al return
        por valor: una referencia a un local es colgante (la L02 lo
        avisó con el `T&&` devuelto a un local).

    4. DEMO: el return con y sin std::move, medido con contadores
   ============================================================================
 */
#include <cstddef>
#include <iostream>
#include <memory>
#include <utility>

struct Combo {
    static inline int copias_ = 0;
    static inline int movimientos_ = 0;
    int valor_ = 0;
    explicit Combo(int v) : valor_(v) {}
    Combo(const Combo& otro) : valor_(otro.valor_) { ++copias_; }
    Combo(Combo&& otro) noexcept : valor_(otro.valor_) { ++movimientos_; }
};

// Versión CORRECTA: el return tal cual -> NRVO o move implícito
Combo correcta() {
    Combo local(3);
    return local;
}

// Versión PESSIMIZADA: std::move mata el NRVO -> 1 move de más
Combo pesimista() {
    Combo local(3);
    return std::move(local);   // -Wpessimizing-move lo avisa (gcc/clang)
}

// Tipos move-only: el return simple también mueve.
std::unique_ptr<Combo> fabricarPunterito() {
    auto ptr = std::make_unique<Combo>(5);
    return ptr;                // move implícito de unique_ptr
}

int main() {
    std::cout << "== return local sin std::move ==\n";
    Combo bien = correcta();
    std::cout << "  copias=" << Combo::copias_
              << " movimientos=" << Combo::movimientos_
              << "  (NRVO: 'local' se construyo directo en 'bien')\n";
    std::cout << "  bien.valor_=" << bien.valor_ << "\n";

    std::cout << "\n== return std::move(local) ==\n";
    Combo mal = pesimista();
    std::cout << "  copias=" << Combo::copias_
              << " movimientos=" << Combo::movimientos_
              << "  (+1 move: la elision ya no estaba permitida)\n";
    std::cout << "  mal.valor_=" << mal.valor_ << "\n";

    std::cout << "\n== move-only: return tal cual = move implicito ==\n";
    auto resultado = fabricarPunterito();
    std::cout << "  resultado->valor_=" << resultado->valor_
              << " (return simple > std::move explicito: siempre)\n";
    return 0;
}
