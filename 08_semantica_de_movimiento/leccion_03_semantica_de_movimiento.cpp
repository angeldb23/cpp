/* =============================================================================
   MÓDULO 08 - LECCIÓN 03: Semántica de movimiento y std::move
   (migrada y ampliada desde el módulo 02)
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      Copiar un objeto pesado (un vector con 10.000 vértices, una
      textura, un string largo) cuesta O(n): hay que reservar nueva
      memoria y duplicar cada elemento. Pero muchas veces el objeto
      de origen YA NO IMPORTA: es un temporal, o un lvalue que de acá
      en más descartamos. Duplicar un mueble que igual se va al
      contenedor es derrochar.

      La SEMÁNTICA DE MOVIMIENTO es el trato al revés: en vez de
      copiar el contenido, se TRANSFIEREN los internos (el puntero al
      buffer, el tamaño, la capacidad) de un objeto hacia otro, en
      O(1): unas pocas palabras copiadas sin recorrer nada.

      ¿Cómo le avisamos al compilador "este ya no lo necesito"? Con
      `std::move`, que tiene un único objetivo: convertir un lvalue en
      un rvalue. (Qué es exactamente y qué queda del origen: la L07 lo
      desmenuza; por ahora tomalo como "el permiso para robar".)

      Movimiento en tres lugares de uso diario:
        1. Transferir al contenedor:
             vector<Buffer> saco; saco.push_back(std::move(miBuffer));
           sin std::move, el vector copia los 10.000 datos.
        2. Asignación:
             destino = std::move(origen);   // roba, no copia
        3. Dejar ir el argumento al final de su vida útil:
             consumir(std::move(buffer));   // buffer ya no me sirve
      Y la regla de oro que hay que interiorizar YA: después de mover,
      el ORIGEN queda en un estado válido pero inespecificado (la L07
      lo muestra con números). O la línea siguiente lo destruye (un
      temporal), o le asignás algo nuevo antes de volver a leerlo.

    2. ¿QUÉ PASA POR DEBAJO?
      Un vector son TRES palabras: puntero a los datos, tamaño y
      capacidad. Copiarlo es: reservar de nuevo + copiar n datos.
      MOVERLO es: copiar esas tres palabras al destino y vaciar el
      original (puntero nulo, tamaños en 0). El contenido NO se toca:
      ya está físicamente donde estaba; lo único que cambia es quién
      tiene el puntero. Por eso "mover es gratis": es la copia literal
      de unas pocas palabras, con nada que recorrer.

      En el flujo del programa: `std::move(x)` no ejecuta nada por sí
      (es un cast: L07). El que realmente mueve es el CONSTRUCTOR de
      movimiento o la ASIGNACIÓN de movimiento del tipo de destino,
      elegidos por la sobrecarga sobre la referencia rvalue de la L02.
      Si un tipo no tiene constructor de movimiento, un rvalue igual
      termina en la copia silenciosa (const T&): compatibilidad, no
      error. Y en una asignación con vectores entra en juego noexcept:
      qué pasa ahí lo dejan claro la L06 y la L10.

    3. ERRORES COMUNES
      - std::move de algo que VAS A VOLVER A USAR: el objeto no se
        rompe en el sentido de UB, pero queda vacío de garantías. La
        convención de lectura vale doble: si veo `std::move(x)`,
        mentalmente tapo a x: no se vuelve a leer hasta que alguien
        le escriba algo nuevo.
      - Mover el contenido de un objeto const: `std::move(constObj)`
        compila, pero el cast produce const Buffer&& y NO hay
        constructor de movimiento para const: silenciosamente copia.
        El movimiento funciona con objetos (y sus datos const fuera
        de eso). Detallado en la L07.
      - Mover tipos chicos por la gloria: mover un int no ahorra nada
        (ya era O(1) copiarlo). El movimiento vale oro para recursos
        dinámicos: buffers, contenedores, strings largos, handles.
        En tipos livianos solo ensucia el código.

    4. DEMO: copia O(n) contra movimiento O(1), contadas a mano
   ============================================================================
 */
#include <cstddef>
#include <iostream>
#include <utility>
#include <vector>

// Clase ruidosa: cuenta construcciones por copia y por movimiento
// para VER cuánto sale cada camino (la usamos todo el módulo).
class BufferSenal {
public:
    explicit BufferSenal(std::vector<int> muestras)
        : muestras_(std::move(muestras)) {}      // muestras es local: mover es gratis

    // COPIA: O(n) — recorre y duplica cada muestra
    BufferSenal(const BufferSenal& otro)
        : muestras_(otro.muestras_) {            // copia del vector: O(n)
        ++copias_;
    }

    // MOVIMIENTO: O(1) — las palabras del origen pasan al destino
    BufferSenal(BufferSenal&& otro) noexcept
        : muestras_(std::move(otro.muestras_)) { // el vector se mueve a sí mismo
        ++movimientos_;
    }

    std::size_t tamanio() const { return muestras_.size(); }

    static int copias() { return copias_; }
    static int movimientos() { return movimientos_; }

private:
    std::vector<int> muestras_;
    inline static int copias_ = 0;
    inline static int movimientos_ = 0;
};

// Por valor: si llega un rvalue se MUEVE (L02); si llega un lvalue,
// se COPIA.
void consumir(BufferSenal s) {
    std::cout << "  consumi un buffer con " << s.tamanio() << " muestras\n";
}

int main() {
    BufferSenal original{std::vector<int>{4, 8, 15, 16, 23, 42}};
    std::cout << "original de " << original.tamanio()
              << " muestras (copias: " << BufferSenal::copias()
              << ", movimientos: " << BufferSenal::movimientos() << ")\n";

    std::cout << "\n-- COPIA explicita: duplica los datos (O(n)) --\n";
    BufferSenal copia{original};                 // constructor de copia
    std::cout << "  copia tiene " << copia.tamanio()
              << " y la original conserva sus " << original.tamanio() << "\n";

    std::cout << "\n-- MOVIMIENTO explicito: std::move roba (O(1)) --\n";
    BufferSenal robado{std::move(copia)};        // constructor de movimiento
    std::cout << "  robado tiene " << robado.tamanio()
              << " y copia quedo con size " << copia.tamanio() << "\n";

    std::cout << "\n-- al llamar por valor: copia o move, segun el argumento --\n";
    std::cout << "(paso un lvalue -> copia)\n";
    consumir(robado);                            // robado sigue vivo: copia
    std::cout << "(paso std::move(robado) -> movimiento)\n";
    consumir(std::move(robado));                 // nos despedimos: se mueve

    std::cout << "\nconteo final: copias=" << BufferSenal::copias()
              << ", movimientos=" << BufferSenal::movimientos() << "\n";
    // El origen ultimo ya no se usa: quedo "moved-from" (L07 lo demuestra).
    return 0;
}
