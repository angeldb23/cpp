/* =============================================================================
   MÓDULO 08 - LECCIÓN 10: Mover dentro de contenedores: noexcept manda
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      Hay un patrón que se dispara MILLONES de veces con un vector:
      crecer. Cuando un vector llega a capacity y reserva memoria
      nueva, transporta cada elemento de la reserva vieja a la
      nueva. ¿Copia o mueve? La decisión NO es tuya NI la del
      compilador: la firma de la STL:

        - Si el constructor de movimiento del elemento es `noexcept`:
          el vector MUEVE cada elemento (rápido, O(1) por elemento).
        - Si NO es noexcept: el vector COPIA cada elemento (lento,
          O(n) por elemento).

      ¿Por qué tan paranoico? Porque la realocación es la única
      operación de vector que NO puede deshacerse: si el vector
      empieza a mover elementos y el constructor de movimiento lanza
      una excepción a mitad de camino, la reserva vieja ya se
      liberó, la nueva quedó a medio llenar y NO HAY ESTADO
      coherente. Con copia es distinto: si la copia falla, la
      original sigue entera (garantía fuerte; el tema completo es
      del módulo 13).

      El compilador SABE si el movimiento lanza: eso es noexcept (la
      L06). Con std::move_if_noexcept los contenedores consultan ese
      resultado y eligen: mover solo si es noexcept. Si un elemento
      es "movible pero lanza", el vector lo copia por seguridad.
      UNA PALABRA (noexcept) decide si tu constructor de movimiento,
      que tanto te costó escribir, sirve para algo dentro de los
      contenedores.

      Los contenedores y el movimiento tienen OTRO punto de
      encuentro: acumulación a mano.
         vec.push_back(std::move(objeto));           // roba al temporal/lvalue
         vec.emplace_back(n + 2, piel);              // construye EN el lugar
      Y el movimiento en ERASE: degradar (erase) tiene que COMPRIMIR:
      los elementos de la derecha se deslizan a la izquierda, y ese
      deslize es incremental con move (L03: move-assign): por eso
      erase de un vector está lleno de asignación de movimiento
      (y por eso después existe std::remove/erase en el módulo 10).

    2. ¿QUÉ PASA POR DEBAJO?
      El crecimiento es: reservar memoria nueva (duplicar capacity),
      mover o copiar los n elementos viejos a la nueva, liberar la
      vieja. El costo total es O(n) por crecimiento, amortizado O(1)
      por push. La diferencia entre alternativas: por cada elemento,
      un MOVE mueve unas pocas palabras (puntero+tamaño); una COPIA
      recorre el contenido completo de cada elemento. Con noexcept,
      la realocación NO copia nada: solo roba internos.

      (Los traits que consultan los contenedores son invasores al
      detalle de plantillas; lo que tenés que recordar es el que
      importa: el vector decide con noexcept. Con eso alcanza.)

    3. ERRORES COMUNES
      - Escribir un constructor de movimiento optimizado SIN
        noexcept: el vector lo ignora y copia. El ejercicio 04 de
        este módulo te deja verlo con contadores.
      - Confundir: `noexcept` también se puede poner en la COPIA
        (es válido y las librerías lo aprovechan), pero la pregunta clave
        de los contenedores es sobre el MOVIMIENTO.
      - El erase de un vector COMPRIME: los elementos de la derecha
        se deslizan a la izquierda con asignación de movimiento:
        por eso borrar "a candela" en medio del vector cuesta O(n)
        (los algoritmos más finos de borrado vienen en el módulo 10).

    4. DEMO: dos clases gemelas, una con noexcept y otra sin, y los
      contadores del vector decidiendo todo.
   ============================================================================
 */
#include <cstddef>
#include <iostream>
#include <utility>
#include <vector>

// Primera: movimiento noexcept: el vector MUEVE al crecer
class PasajeroNoexcept {
public:
    explicit PasajeroNoexcept(int id) : id_(id) {}
    PasajeroNoexcept(const PasajeroNoexcept& otro) : id_(otro.id_) { ++copias_; }
    PasajeroNoexcept(PasajeroNoexcept&& otro) noexcept : id_(otro.id_) { ++movimientos_; }
    PasajeroNoexcept& operator=(const PasajeroNoexcept& otro) {      // L05: copia
        id_ = otro.id_;
        ++asignaciones_;
        return *this;
    }
    PasajeroNoexcept& operator=(PasajeroNoexcept&& otro) noexcept {  // L06: move
        id_ = otro.id_;
        ++asignaciones_;
        return *this;
    }
    static int copias() { return copias_; }
    static int movimientos() { return movimientos_; }
    static int asignaciones() { return asignaciones_; }
private:
    int id_;
    inline static int copias_ = 0;
    inline static int movimientos_ = 0;
    inline static int asignaciones_ = 0;
};

// Gemela pero SIN noexcept: el vector COPIA al crecer
class PasajeroLanzador {
public:
    explicit PasajeroLanzador(int id) : id_(id) {}
    PasajeroLanzador(const PasajeroLanzador& otro) : id_(otro.id_) { ++copias_; }
    PasajeroLanzador(PasajeroLanzador&& otro) : id_(otro.id_) { ++movimientos_; }   // sin noexcept!
    static int copias() { return copias_; }
    static int movimientos() { return movimientos_; }
private:
    int id_;
    inline static int copias_ = 0;
    inline static int movimientos_ = 0;
};

int main() {
    std::cout << "== vector de un tipo con move noexcept: crece y MUEVE ==\n";
    std::vector<PasajeroNoexcept> cajas;
    for (int i = 0; i < 16; ++i)
        cajas.push_back(PasajeroNoexcept{i});        // temporales: moves
    std::cout << "  copias=" << PasajeroNoexcept::copias()
              << " movimientos=" << PasajeroNoexcept::movimientos()
              << " (moves de realocación + moves de carga)\n";

    std::cout << "\n== vector del gemelo SIN noexcept: crece y COPIA ==\n";
    std::vector<PasajeroLanzador> lanzadores;
    for (int i = 0; i < 16; ++i)
        lanzadores.push_back(PasajeroLanzador{i});   // MISMOS temporales
    std::cout << "  copias=" << PasajeroLanzador::copias()
              << " movimientos=" << PasajeroLanzador::movimientos() << "\n";

    std::cout << "\n== la palabra noexcept decidió el comportamiento completo ==\n";
    return 0;
}
