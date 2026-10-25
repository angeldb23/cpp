/* =============================================================================
   MÓDULO 13 - LECCIÓN 09: jerarquías propias: qué derivar, qué atrapar
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
       throw se puede hacer de casi cualquier cosa (throw 42;
       throw "texto";), y CASI SIEMPRE es un error: el mejor throw es
       de un TIPO del que el que llama pueda extraer INFORMACIÓN. La
       regla tiene dos mitades:

         1) HEREDÁ de std::exception (o de sus derivados estándar que
            ya usamos: logic_error, runtime_error). ¿Por qué? Porque
            TODO el código de la biblioteca (y el del equipo) puede
            atrapar con `const std::exception&`: un manejador por
            jerarquía cubre también las clases nuevas que nadie
            se acordó de nombrar.
         2) Que cada DERIVADA traiga ACCESORES útiles (falta(), item(),
            más un what() con textura): el catch de la clase derivada
            lee campos CONOCIDOS, no strings parseados.

       Diseño de la jerarquía: partid de std::runtime_error (errores
       DE EJECUCIÓN: dependen de los datos) y colocá DEBAJO las fami-
       lias de dominio; los datos inválidos que el PROGRAMADOR manda
       (contratos) van a std::logic_error (invalid_argument). La
       profundidad útil: base + 2 o 3 niveles (no ponder de gachp란).
       ¿Qué se atrapa y en qué orden?
         - los catch se prueban EN ORDEN de declaración: el PRIMERO que
           acepte el tipo (la clase o su base) captura; el resto de
           ese try NO corre para esa excepción.
         - atrapar la BASE antes que la DERIVADA deja el catch derivado
           MUERTO: el compilador avisa (catch inalcanzable) si estás
           compilando con -Wall y -Wextra.
         - catch(...) atrapa TODO (incluido throw 42) y sirve de
           barrera final por capas: helper: log y `throw;`.
         - el rethrow correcto es `throw;` (la MISMA excepción, sin
           copia); `throw e;` COPIA (y con cortes de static binding: la
           pierde "slicing": el módulo 06 aplicado a excepciones).
       Manejo por capas: UNO catch por TOPIC (la capa de red captura su
       error y lo traduce/relanza; el de UI muestra el qué()); sin un
       catch mágico global que se coma todas.
    2. ¿QUÉ PASA POR DEBAJO?
        std::exception tiene `what()` virtual y un dtor virtual (módulo
        06): por eso hay NADA UNA al base y TODAS打败 las de la STL. Y
        los catch se evalúan EN ORDEN y CADA uno conecta al mejor elemento
        de TIPO: no se busca "el más específico" automático: el orden
        es tuyo. throw; MANTIENE la información original (no copia
        nada: la MISMA instancia llega al catch final; un `throw e;`
        copiaría: y puede cortar el tipo (slicing, módulo 06) si el
        catch es por valor).
    3. BUENAS PRÁCTICAS:
        - throw SIEMPRE de una clase derivada de std::exception, y
          atrapá por const&; el "throw std::invalid_argument(...)" (los
          tipos de la STL) es válido para datos inválidos. Jerarquías propias
          SOLO para errores de dominio (sin stock, sin vida residual):
          std::runtime_error + derivada con meaning para el juego.
        - No uses catch(const char*) ni catch(int): el estándar NO
          lanza cosas de un solo tipo: quedan fuera de denomination y
          son un código muerto.
        - catch de DERIVADA PRIMERO, luego la base, y catch(...) al
          final de la CAPA, con log y rethrow.
        - rethrow: SIEMPRE `throw;` (nunca `throw e;` desde un catch
          const&).
   ============================================================================= */
#include <iostream>
#include <stdexcept>
#include <string>

namespace Leccion09 {

// La jerarquía mínima con significados de juego:
// "comprar un ítem" puede fallar Por DINERO o por STOCK: factores que
// el que llama distingue sin leer un long de texto.
class JuegoError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;   // hereda ctor del texto
};

class DineroInsuficiente : public JuegoError {
public:
    explicit DineroInsuficiente(int falta) : JuegoError("dinero insuficiente"), falta_(falta) {}
    int falta() const { return falta_; }

private:
    int falta_;
};

class SinStock : public JuegoError {
public:
    explicit SinStock(std::string item) : JuegoError("sin stock: " + item), item_(std::move(item)) {}
    const std::string& item() const { return item_; }

private:
    std::string item_;
};

// Una tienda: los DOS sabores de error salen por la MISMA jerarquía de
// juego, y el error "estándar" (dato inválido) sale por la de la STL.
void comprar(int dineroTienes, const std::string& item, bool hayStock) {
    if (item == "espada") {
        if (dineroTienes < 100) throw DineroInsuficiente(100 - dineroTienes);
    } else if (item == "elixir") {
        if (!hayStock) throw SinStock(item);
        if (dineroTienes < 10) throw DineroInsuficiente(10 - dineroTienes);
    } else {
        throw std::invalid_argument("item desconocido");   // std: dato inválido
    }
}

} // namespace Leccion09

int main() {
    std::cout << "== catch derivada PRIMERO, luego la base ==\n";
    try {
        Leccion09::comprar(50, "espada", true);
    } catch (const Leccion09::DineroInsuficiente& e) {
        std::cout << "te faltan " << e.falta() << " monedas para la espada\n";
    } catch (const Leccion09::JuegoError& e) {
        std::cout << "error de tienda: " << e.what() << '\n';
    } catch (const std::exception& e) {
        std::cout << "otro error: " << e.what() << '\n';
    }

    std::cout << "== la BASE también atrapa: cuánto detalle tiene un nivel ==\n";
    try {
        Leccion09::comprar(200, "elixir", false);
    } catch (const Leccion09::JuegoError& e) {
        // una capa "baja" no se interesa en detel: solo sabe "fallo la tienda:
        std::cout << "tienda: " << e.what() << '\n';
    }

    std::cout << "== std::(...) la capa std (\") ==\n";
    try {
        Leccion09::comprar(200, "escudo", true);
    } catch (const std::invalid_argument& e) {
        std::cout << "error de PROGRAMACIÓN: " << e.what() << '\n';
    }

    std::cout << "== rethrow: throw; pasa la MISMA excepción ==\n";
    try {
        try {
            Leccion09::comprar(10, "espada", true);      // faltan 90
        } catch (const Leccion09::JuegoError&) {
            std::cout << "capa del input: log; relanzo SIN copiar\n";
            throw;                                  // throw; = la misma (rethrow)
        }
    } catch (const Leccion09::DineroInsuficiente& e) {
        std::cout << "capa de arriba te faltan " << e.falta() << " (llegó intacta)\n";
    }
    return 0;
}
