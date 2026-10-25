/* =============================================================================
   MÓDULO 08 - LECCIÓN 08: Copy elision, RVO y prvalues garantizadas (C++17)
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      Todo este módulo se apoya en una capacidad del compilador que
      mencionamos de pasada en la L03: cuando una función devuelve un
      objeto por valor, el ESTÁNDAR le permite ELIMINAR (elidir) las
      copias y movimientos... y en casos clave, desde C++17, hacerlo
      es una GARANTÍA, no un permiso.

      Los tres escenarios, con nombre:

      1) TEMPORAL devuelto (copy elision GARANTIZADA desde C++17):
             Buffer fabricar() { return Buffer{16}; }
             Buffer b = fabricar();
         El return devuelve un prvalue (un temporal "sin nombre");
         el estándar garantiza que NO se llama ni copia ni movimiento:
         el objeto se construye DIRECTO en `b`. Y es obligatorio:
         aunque el constructor de copia/movimiento tenga un cout que
         los "anuncie", no corren. La expresión `Buffer{16}` se
         materializa directamente en `b`.

      2) VARIABLE NOMBRADA devuelta (NRVO, permitida):
             Buffer leerNivel() {
                 Buffer nivel(2048);
                 ... // se carga el nivel
                 return nivel;   // nombrado: NRVO permitida, no garantizada
             }
         NRVO = "named return value optimization": el compilador
         construye `nivel` directamente en la memoria del resultado y
         evita cualquier movimiento. NO está garantizado por el
         estándar (los compiladores reales lo hacen casi siempre);
         si no se puede, el return de un local es tratado como
         rvalue: primero intenta mover, después copiar. SIN escribir
         std::move: eso lo explica la L09.

      3) DEVOLVER UN PARÁMETRO por valor:
             Buffer passthrough(Buffer p) { return p; }
         El parámetro es un local: NRVO casi nunca es aplicable (el
         caller ya tenía su objeto fuera) y el return termina en
         movimiento (el estándar lo trata como rvalue explícitamente
         desde C++11).

      Y la regla que cruza los tres: para devolver, escribí
      `return nombre;` NUNCA `return std::move(nombre);`. El
      std::move MATA el NRVO (le dice al compilador "quiero el
      constructor de movimiento sí o sí") y no mejora nada.

    2. ¿QUÉ PASA POR DEBAJO?
      El concepto clave es la MATERIALIZACIÓN. Antes de C++17, devolver
      un prvalue creaba un temporal que "viajaba" (con move o copy)
      hasta el destino. Desde C++17 un prvalue es solamente una RECETA
      de construcción: el objeto no existe hasta que hará falta, y al
      devolverlo la receta se ejecuta DIRECTO en el espacio del
      resultado (el clásico "caller allocates, callee constructs": el
      caller reserva el lugar y el callee construye ahí). Por eso el
      constructor de movimiento no entra en escena: no hay objeto
      intermedio que mover.

      NRVO es una optimización que el compilador JUZGA: necesita
      comprobar que todas las salidas devuelven LA MISMA variable
      nombrada (o que no hay ninguna). Si las ramas devuelven objetos
      distintos, o devuelves un parámetro, no hay un único lugar donde
      construir: se cae al movimiento normal.

    3. ERRORES COMUNES
      - Escribir `return std::move(variable)`: mata el NRVO, corre el
        constructor de movimiento que podía no correr, y confunde al
        lector. Cero beneficios: L09 lo mide.
      - "Devolver por valor es caro": era cierto en 1998. Hoy el
        return por valor con elisión es LO MÁS RÁPIDO y la forma
        preferida de fábrica: menos punteros crudos, menos colgantes.
      - Depender de "cuántas construcciones corrí" como invariante:
        la elisión puede SALTEARSE constructores y destructores
        completos (no hay objeto intermedio). Contá en la demo, pero
        no hagas lógica que dependa de la cuenta exacta salvo con
        elisión garantizada.

    4. DEMO: contador de construcciones, copias y moves, caso por caso
   ============================================================================
 */
#include <cstddef>
#include <iostream>
#include <utility>

struct Combo {
    static inline int construcciones_ = 0;
    static inline int copias_ = 0;
    static inline int movimientos_ = 0;
    int valor_ = 0;

    explicit Combo(int v) : valor_(v) { ++construcciones_; }
    Combo(const Combo& otro) : valor_(otro.valor_) { ++copias_; }
    Combo(Combo&& otro) noexcept : valor_(otro.valor_) { ++movimientos_; }
};

// CASO 1: devuelve prvalue -> elisión GARANTIZADA (C++17): ni copia ni move
Combo fabricar() {
    return Combo{7};
}

// CASO 2: devuelve variable nombrada -> NRVO (permitida; los compiladores
// reales la aplican; si no: move implícito del local)
Combo preparar() {
    Combo nivel(2048);
    return nivel;
}

// CONTRACASO: dos caminos devuelven OBJETOS DISTINTOS: no hay un solo
// lugar donde construir -> el movimiento (o copia) es inevitable.
Combo bifurcada(bool querer) {
    Combo caminoFacil(1);
    Combo caminoDificil(9);
    if (querer) return caminoFacil;    // rama A: devuelvo un objeto
    return caminoDificil;              // rama B: devuelvo OTRO
}

int main() {
    std::cout << "== prvalue: elisión GARANTIZADA (ni copia ni move) ==\n";
    Combo b = fabricar();
    std::cout << "  construcciones=" << Combo::construcciones_
              << " copias=" << Combo::copias_
              << " movimientos=" << Combo::movimientos_ << "\n";
    std::cout << "  (1 construc; en C++17 es GARANTIA del estandar)\n";
    std::cout << "  b.valor_=" << b.valor_ << "\n";

    std::cout << "\n== variable nombrada: NRVO (caso normal: cero moves) ==\n";
    Combo p = preparar();
    std::cout << "  construcciones=" << Combo::construcciones_
              << " copias=" << Combo::copias_
              << " movimientos=" << Combo::movimientos_
              << "  (con NRVO: cero moves; sin NRVO: 1 move)\n";
    std::cout << "  p.valor_=" << p.valor_ << "\n";

    std::cout << "\n== bifurcada: objetos distintos, el NRVO NO aplica ==\n";
    Combo q = bifurcada(true);
    std::cout << "  movimientos=" << Combo::movimientos_
              << " (aqui el move es inevitable: no hay un solo candidato)\n";
    std::cout << "  q.valor_=" << q.valor_ << "\n";
    return 0;
}
