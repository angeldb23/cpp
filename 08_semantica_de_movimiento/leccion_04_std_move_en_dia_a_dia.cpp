/* =============================================================================
   MÓDULO 08 - LECCIÓN 04: std::move en el día a día: dónde se usa de verdad
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      La L03 explicó QUÉ es el movimiento. Esta lección contesta la
      pregunta práctica: ¿DÓNDE pongo std::move en el código de un
      juego de verdad? Los tres sitios donde aparece una y otra vez:

      A) CARGAR contenedores. El vector es el dueño de la colección:
           std::vector<std::string> nombres;
           nombres.push_back(std::move(fuente));        // roba el string
           nombres.push_back(otro);                     // copia (otro sigue vivo)
           nombres.emplace_back(std::move(armado));     // es igual: roba
         Si "fuente" era un temporal que pediste por valor a una
         fábrica, el move te ahorra la copia completa del texto.

      B) ASIGNACIÓN entre objetos ya existentes:
           destino = std::move(origen);
         El operador de movimiento "reinstala" las tres palabras: no
         hay recorridos ni reservas. (La L06 lo escribe a mano.)

      C) PASAR un argumento "que termina su vida": las funciones que
         se quedan con el contenido (funciones "sink") se llaman así:
           guardarComo(std::move(nombreArchivo));
         La firma es `void f(T x)` por valor, y ADENTRO de f el
         parámetro se mueve al lugar final (miembro, vector, cache):
         la regla completa de elección de firma está en la L11.

      Reglas de pulgar que lo resumen:
        - std::move SOLO significa algo cuando lo que queda del lado
          izquierdo es algo pesado QUE NO VAS A VOLVER A USAR.
        - NO ponerlo sobre algo que el siguiente lector va a
          necesita: mover mal y después usar = bug silencioso.
        - NO ponerlo en el return de un local (la L09 lo explica:
          ahí estorba).
        - NO es "cast gratuito": cada move es un contrato que escribís
          y que el que lee el código firma también.

    2. ¿QUÉ PASA POR DEBAJO?
      Cada `std::move(x)` compila a casi NINGUNA instrucción: el cast
      cambia la categoría de la expresión en el IR intermedio, no al
      objeto. El costo del movimiento REAL aparece cuando corre el
      constructor o el operador de movimiento del destino: en
      std::string y std::vector son dos-swaps de punteros y un
      deallocate del estado viejo. Por eso mover una colección de
      100.000 elementos es gratis y copiarla cuesta una recorrida por
      todos los datos. La comparación con contadores en vivo la hacen
      la L06 y la L10.

    3. ERRORES COMUNES
      - Mover un bool/int/float: no hace daño pero es ruido: para
        tipos chicos el movimiento ES copiar (mismo O(1)); no lo pongas.
      - Mover y SEGUIR usando lo movido: el error clásico más común:
           auto copia = std::move(miCola);
           usar(miCola);          // <- compila pero NO hay garantías
        La L07 demuestra el estado exacto; hasta allí: tapá la
        variable movida.
      - Mover el código de abajo: si hay que robarle a un destino
        CONST... el constructor de movimiento NO puede escribir: cae
        a copia silenciosa (la misma trampa de la L03, mostrada en el
        ejemplo de abajo).

    4. DEMO: los tres lugares del día a día, con contador de piezas
   ============================================================================
 */
#include <iostream>
#include <string>
#include <utility>
#include <vector>

class Cron {
public:
    explicit Cron(std::string eventos) : eventos_(std::move(eventos)) {}

    // Copia que TARDE (el string se copia):
    Cron(const Cron& otro) : eventos_(otro.eventos_) { ++copias_; }
    // Movimiento que NO tarda (el string se roba):
    Cron(Cron&& otro) noexcept : eventos_(std::move(otro.eventos_)) { ++movidos_; }

    // Asignación de movimiento: la variante B del día a día (L06 la
    // escribe a mano para un buffer crudo).
    Cron& operator=(Cron&& otro) noexcept {
        eventos_ = std::move(otro.eventos_);
        ++asignaciones_;
        return *this;
    }

    const std::string& eventos() const { return eventos_; }
    std::string& eventos() { return eventos_; }
    static int copias() { return copias_; }
    static int movidos() { return movidos_; }
    static int asignaciones() { return asignaciones_; }

private:
    std::string eventos_;
    inline static int copias_ = 0;
    inline static int movidos_ = 0;
    inline static int asignaciones_ = 0;
};

// Función "sink": se queda con el contenido (recibe por valor).
// El criterio completo de elección de firma está en la L11.
void guardarComo(Cron apunte) {
    std::cout << "  guardo un apunte con " << apunte.eventos().size()
              << " caracteres de log\n";
}

int main() {
    std::cout << "== A) cargar contenedor ==\n";
    std::vector<Cron> caja;
    caja.push_back(Cron{"nivel1/nivel2/acos;10 ms"});        // temporal: se mueve solo
    std::cout << "  (push temporal: 0 copias extra, movimientos: "
              << Cron::movidos() << ")\n";

    Cron nota{"frame 42: update 8 ms / render 12 ms"};
    caja.push_back(std::move(nota));                         // lvalue: mover EXPLICITO
    std::cout << "  (push con std::move: movidos=" << Cron::movidos()
              << ", copias=" << Cron::copias() << ")\n";
    std::cout << "  la 'nota' queda valida pero moved-from: "
              << nota.eventos() << " (L07 explica el estado)\n";

    std::cout << "\n== B) asignacion entre objetos ==\n";
    Cron destino{"vacio"};
    destino = std::move(caja[1]);                            // asignacion movimiento
    std::cout << "  destino=\"" << destino.eventos() << "\", asignaciones: "
              << Cron::asignaciones() << "\n";

    std::cout << "\n== C) funcion sink: el argumento termina su vida ==\n";
    Cron log{"sesion completa"};
    guardarComo(std::move(log));                             // lo soltamos a proposito
    std::cout << "  tras la llamada, 'log' ya no se toca (moved-from)\n";

    std::cout << "\ntotal dia a dia: copias=" << Cron::copias()
              << ", movimientos=" << Cron::movidos()
              << ", asignaciones=" << Cron::asignaciones() << "\n";
    return 0;
}
