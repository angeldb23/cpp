/* =============================================================================
   MÓDULO 07 - LECCIÓN 05: unique_ptr en APIs: paso por referencia y observadores
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      La librería estándar tiene un patrón claro para hablar de propiedad en
      funciones, y se resume en una frase:

        "El unique_ptr sigue en manos de su dueño; a las funciones se les
         pasa por REFERENCIA (outras veces como puntero crudo a lo que
         guarda). Nunca se copia, porque no tiene copia."

      Los tres contratos, de más restrictivo a más libre:

      a) const std::unique_ptr<T>&     la función SOLO LEE el objeto. No puede
         reemplazarlo (el operador= de un const unique_ptr no existe) y por
         lo tanto no puede romper el estado del dueño. Es el contrato típico
         de "dibujá este sprite".

      b) std::unique_ptr<T>&           la función PUEDE REEMPLAZAR el recurso
         (con reset o =). Ejemplo típico: un cargador que reemplaza la
         textura por un placeholder si el puntero llegó nulo. El DUEÑO sigue
         siendo el llamador; el calleado solo tiene permiso de escritura.

      c) const T* / T* (crudo)         el contrato no-depende del tipo de
         dueño: sirve igual para unique_ptr, shared_ptr, elementos de vector,
         objetos de stack: la función no sabe ni le importa quién es dueño.
         Es la firma de las APIs de C y, en C++ moderno, la de "uso durante
         la llamada". Puede recibir nullptr (por eso se chequea).

      d) ¿Y cuando la función debe GUARDAR el recurso para después (un caché
         que guarda el sprite que le pasás)? Ahí la API pide propiedad, y
         normalmente se resuelve así:
           - la función recibe `std::unique_ptr<T>&` y le hace swap/release
             al dueño, Ó
           - la API se construye en base al módulo 08 (unique_ptr por valor +
             std::move = transferencia).
         La señal de diseño: ¿quién controla la VIDA del recurso? Si el
         calleado va a usarlo más tiempo que la llamada, necesita ser dueño
         o guardar una referencia viva con un dueño garantizado encima.

      e) Regla general: `raw T*` (o `T&`) significa SIEMPRE "no dueño". Que
         una función reciba T* y adentro haga delete p es el peor bug posible
         (el llamador no lo esperaba). delete solo aparece a mano dentro de
         clases RAII.

   2. ¿QUÉ PASA POR DEBAJO?
      Pasar const unique_ptr<T>& NO copia nada: es una referencia a ese
      struct de 8 bytes del dueño. Pasar const T* es directamente el puntero
      interior. En ambos casos: una indirección, cero constructor, cero
      contador, cero costo de llamada esta función no produce recursos.

   3. ERRORES COMUNES
      - Pasar unique_ptr por valor cuando solo se lee: el parámetro por
        valor de unique_ptr existe para ADQUIRIR propiedad; si la función
        nunca la adopta, es diseño confuso (y con move-only tipa, un llamado
        más caro de escribir que de ejecutar).
      - Recibir `T* p` y liberar: viola el contrato "crudo = no dueño".
      - Guardar el resultado de get() en una clase/estructura y usarlo
        después de que el dueño fue reset o destruido: colgante. Si vas a
        guardar, guardá el dueño o garantizá que el dueño vive más.
      - Forzar unique_ptr a recursos COMPARTIDOS: si tres escenas usan el
        mismo shader, "un dueño" es a capa de dependencias compartidas y
        copiando eso te vas a pelear con el diseño. Eso es shared_ptr (L06).
      - const_cast para "rescatar" el operador= de un const&: si tu función
        quiere reemplazar el recurso, el contrato correcto es (b), no
        morder la constancia.

   4. DEMO: los tres contratos en un mini-juego
 */
#include <cstddef>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

class Sprite {
private:
    std::string nombre_;
    mutable int usos_ = 0;    // mutable: dato de medición que no rompe const

public:
    explicit Sprite(std::string nombre) : nombre_(std::move(nombre)) {}

    void usar() const {         // const: usar cumple el contrato de lectura
        std::cout << "    uso: " << nombre_ << " (usos " << ++usos_ << ")\n";
    }
    const std::string& nombre() const { return nombre_; }
};

// a) const&: SOLO LEE. El dueño queda intacto.
void dibujar(const std::unique_ptr<Sprite>& sprite) {
    sprite->usar();
}

// b) &: puede REEMPLAZAR el recurso; el dueño sigue siendo el llamador.
void cargarSiHaceFalta(std::unique_ptr<Sprite>& sprite, const char* ruta) {
    if (!sprite) {
        sprite = std::make_unique<Sprite>(ruta);   // carga real: reset/=
    }
}

// c) crudo const T*: contrato no-dueño, indiferente a quién administre.
void loggear(const Sprite* sprite) {
    if (sprite) {
        std::cout << "    log de " << sprite->nombre() << "\n";
    } else {
        std::cout << "    log de (vacio: sprite nulo)\n";
    }
}

int main() {
    std::cout << "=== 1. CONST&: LEE SIN TOCAR LA PROPIEDAD ===\n";
    {
        auto heroe = std::make_unique<Sprite>("heroe.png");
        dibujar(heroe);
        dibujar(heroe);
        std::cout << "    el recurso sigue con vida y su dueño intacto\n";
    }

    std::cout << "\n=== 2. &: PUEDE REEMPLAZAR EL RECURSO ===\n";
    {
        std::unique_ptr<Sprite> sprite;          // nulo a propósito
        cargarSiHaceFalta(sprite, "placeholder.png");
        cargarSiHaceFalta(sprite, "placeholder.png");  // segunda: no recarga
        dibujar(sprite);
    }

    std::cout << "\n=== 3. RAW T*: EL CONTRATO QUE NO DEPENDE DEL DUEÑO ===\n";
    {
        auto cofre = std::make_unique<Sprite>("cofre.png");
        Sprite  temporal("bala_del_frame.png");  // otro dueño: el STACK
        loggear(cofre.get());
        loggear(&temporal);                      // misma firma, dueño distinto
        loggear(nullptr);                        // y tolera nulo
    }

    std::cout << "\n=== 4. RESUMEN DE CONTRATOS (CHEAT SHEET) ===\n";
    std::cout << "  const unique_ptr<T>& : LEE, no puede cambiar el recurso\n";
    std::cout << "       unique_ptr<T>&  : LEE + puede REEMPLAZAR el recurso\n";
    std::cout << "       const T* / T*   : NO-DUENO, tolera nulo, API neutral\n";
    std::cout << "       unique_ptr<T> POR VALOR   : ADQUIERE propiedad\n";
    std::cout << "          (sintaxis de la transferencia: modulo 08)\n";
    return 0;
}
