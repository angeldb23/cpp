/* =============================================================================
   MÓDULO 08 - LECCIÓN 02: Referencias rvalue (T&&) y elección de sobrecarga
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      La lección anterior dijo: "un rvalue está muriendo: robémosle los
      internos". Pero C++ no mueve nada automágicamente: hace falta un
      MECANISMO que distinga "esto puede copiarse porque sigue vivo" de
      "esto puede moverse porque está por morir". Ese mecanismo es la
      REFERENCIA RVALUE: `T&&`.

      En el módulo 02 conocimos las dos referencias de siempre:
        - `T&`: referencia no const. SOLO se liga a lvalues no const.
        - `const T&`: se liga a CUALQUIER cosa (lvalues y rvalues, const
          o no). Por eso fue, hasta ahora, el tipo universal para pasar
          argumentos pesados.
      Con `T&&` entra el cuarto integrante:
        - `T&&`: referencia rvalue: se liga SOLO a rvalues (temporales,
          resultados de std::move, literales). Su contrato: "lo que me
          pasás ya no lo vas a necesitar; tengo permiso de vaciarte".

      Tabla de ligadura (apréndela: resuelve el 90% de este módulo):

                                    T&      const T&    T&&
        lvalue modificable          SÍ      SÍ          no
        lvalue const                no      SÍ          no
        rvalue (temporal/move)      no      SÍ          SÍ

      ¿Para qué sirve? Para SOBRECARGAR. Si una clase define el
      constructor de copia `Buffer(const Buffer&)` Y el constructor de
      movimiento `Buffer(Buffer&&)`, el compilador elige mirando la
      CATEGORÍA del argumento (que es el mismo tipo en ambos):
        Buffer a(lleno);       // lvalue -> COPIA: a sigue vivo
        Buffer b(crear());     // rvalue -> MUEVE: el temporal muere
      Es el mismo truco que declaramos con la regla de 5 en el módulo
      05. ACÁ entendemos por qué funciona: por las valor-categorías de
      la L01 y esta tabla de ligadura.

      Vocabulario fino (C++11): con identidad = glvalue; con permiso de
      moverse = rvalue; el punto de encuentro es el XVALUE: un objeto
      con identidad PERO marcado para morir. Es exactamente lo que
      produce `std::move(unLvalue)`. No hace falta recitar más
      teoría: con saber "std::move convierte un lvalue en un rvalue
      (xvalue)" alcanza para todo el módulo.

      Y el vicio a evitar de entrada: una referencia rvalue CON NOMBRE
      se usa como lvalue SIEMPRE. Aunque la variable esté declarada
      `T&&`, el nombre tiene dirección, así que cada USO de `ref` es
      lvalue (regla 5 de la L01: `&ref` compila). Eso es justo lo que
      `std::move` viene a arreglar: si querés que un `T&&` nombrado
      vuelva a comportarse como rvalue, tenés que envolverlo con
      `std::move(ref)` (L03).

    2. ¿QUÉ PASA POR DEBAJO?
      `T&&` circula EXACTAMENTE igual que `T&`: una dirección. No hay
      "flujo rvalue" ninguno: la diferencia es puramente semántica, la
      usa el compilador para ELEGIR SOBRECARGA. Cuando conviven
      `f(const T&)` y `f(T&&)`, un lvalue elige la primera (a igual
      calidad de partido, la referencia no const no compite para
      lvalues const y para no const gana la exacta), y un rvalue elige
      `T&&`. El mismo mecanismo de "elegir por categoría" es el que
      funciona detrás del trío copia/movimiento en constructores y
      asignaciones, y es el que va a complicarse con deducción de
      plantillas en el módulo 09 (perfect forwarding). Acá TODO es
      explícito: se elige por el tipo de la expresión tal cual la
      escribiste.

    3. ERRORES COMUNES
      - Intentar ligar `T&&` a un lvalue: error de compilación (y bien
        que exista: nos avisa). La confusión típica es la inversa:
        creer que `Buffer&& r = crear();` "extiende la vida del
        temporal" es la parte VÁLIDA: el temporal ligado a una
        referencia vive mientras viva la referencia; pero si de ese
        objeto ya se movió el contenido, asumir que sigue lleno es un
        error (la L07 trata el estado exacto).
      - Devolver `T&&` hacia un local: NUNCA. `return
        std::move(local);` con `T local` devuelve referencia a un
        muerto: colgante instantáneo. Devolver POR VALOR se ve en
        L08-L09 (y ahí vas a ver que el std::move hasta es
        contraproducente).
      - Usar `T&&` como tipo de parámetro "porque es el moderno": para
        lvalues basta `const T&` y es más flexible (acepta todo). `T&&`
        es para SOBRECARGAS de categoría y transferencia de propiedad;
        no es un reemplazo de const T&.

    4. DEMO: el conjunto de sobrecargas se elige por valor-categoría
   ============================================================================
 */
#include <iostream>
#include <string>
#include <utility>

// Clase "ruidosa": anuncia si la construyeron copiando o moviendo.
// Ruidosa a propósito: acá adivinar a ojo es exactamente lo que NO queremos.
class Buffer {
public:
    explicit Buffer(std::string datos) : datos_(std::move(datos)) {
        std::cout << "  [ctor normal]            guardo \"" << datos_ << "\"\n";
    }

    Buffer(const Buffer& otro) : datos_(otro.datos_) {
        std::cout << "  [CONSTRUCTOR COPIA]      copio \"" << datos_ << "\" (O(n))\n";
    }

    Buffer(Buffer&& otro) noexcept : datos_(std::move(otro.datos_)) {
        std::cout << "  [CONSTRUCTOR MOVIMIENTO] robo  \"" << datos_ << "\" (O(1))\n";
    }

    const std::string& datos() const { return datos_; }

private:
    std::string datos_;
};

// Tres firmas que reciben el mismo tipo y se reparten por categoría:
void diagnosticar(const Buffer&) {      // se liga a lvalues (y const)
    std::cout << "  -> sobrecarga const T& corre (el lvalue SIGUE viviendo)\n";
}
void diagnosticar(Buffer&&) {           // se liga a rvalues
    std::cout << "  -> sobrecarga T&& corre (el rvalue estaba muriendo)\n";
}
// Un rvalue NO se liga a `Buffer&` sin const: la tabla lo prohíbe.

Buffer fabricar() {
    return Buffer{"abc"};               // rvalue al return: mueve o elisiona (L08/L09)
}

int main() {
    std::cout << "== Tabla de ligadura en acción ==\n";

    Buffer lleno{".sprite"};
    std::cout << "\n-- paso un LVALUE ->\n";
    diagnosticar(lleno);                // lvalue  -> const T&
    Buffer copia{lleno};                // lvalue  -> constructor COPIA

    std::cout << "\n-- paso un RVALUE ->\n";
    diagnosticar(Buffer{"temporal"});   // temporal -> T&&
    Buffer robado{std::move(lleno)};    // std::move -> rvalue -> MOVIMIENTO

    std::cout << "\n-- rvalue que viene del return ->\n";
    Buffer delReturn = fabricar();      // rvalue -> movimiento (o elisiona)

    // Y la prueba final: una referencia rvalue CON NOMBRE es lvalue:
    Buffer&& ref = std::move(robado);
    std::cout << "\n-- `ref` es T&&, PERO el nombre es un lvalue ->\n";
    diagnosticar(ref);                  // ! corre const T& OTRA VEZ
    // Si quiero volver a robar el contenido: std::move(ref) (L03).

    // (Todas quedaron usadas: con prints alcanzan para la demo.)
    std::cout << "\nresiduos: ref=\"" << ref.datos() << "\" (estado moved-from, L07)\n";
    (void)copia;
    (void)delReturn;
    return 0;
}
