/* =============================================================================
   MÓDULO 07 - LECCIÓN 10: Propiedad en APIs: dueño, observador y transitorio
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      Cada parámetro (y cada miembro, y cada retorno) declara UNA de tres
      relaciones con el recurso:

        - DUEÑO:       vive y muere conmigo.      -> unique_ptr / shared_ptr
        - OBSERVADOR:  apunto, no sostengo.       -> weak_ptr
        - TRANSITORIO: lo uso SOLAMENTE en la llamada a mano. -> T& / T* crudo

      a) La tabla de decisión completa (cheat sheet del módulo, es importa
         que la tengas de memoria):

           ¿QUIÉN LIBERA / QUÉ RELACIÓN?          -> ¿QUÉ FIRMA?
           uso corto de garantía de vida          -> const T& (o const T*)
           uso corto con estado "sin dato"        -> const T* (nullptr ok)
           observador permanente de vida dudosa   -> weak_ptr + lock()
           UN solo dueño (el común)               -> unique_ptr
           varios dueños legítimos                -> shared_ptr
           la función CREA y entrega un recurso   -> que devuelve dueño

      b) FUNCIONES QUE CREA Y ENTREGA (fábrica): devuelve unique_ptr<T> (la
         opción por defecto) o shared_ptr<T> si el diseño es compartido.
         El llamador RECIBE el dueño: es quien decide el destino final.
         No devuelvas crudo: un T* "de factoría" deja el destino sin dueño
         (¿quién lo libera? ¿cómo? ¿con qué? la respuesta empieza por RAII).

      c) FUNCIONES QUE RECIBEN PARA USAR: const T& si garantías que existe;
         const T* si "puede no estar" (nullptr es un estado SEMÁNTICO:
         "sin dato en este frame", no un error). NUNCA unique_ptr por valor
         cuando la función no va a tomar propiedad: es una firma mentirosa.

      d) DEVOLVER UN OBSERVADOR: cuidado con la vida. "me das un puntero a
         tu recurso" puede apuntar a algo que está POR LIBERARSE. Las
         firmas seguras: devolver weak_ptr (el observador lo muy suave: no
         puede usarla a la fuerza sin lock()) o devolver raw T* solo cuando
         el dueño (la llamada que devuelve) garantía que vive mientras el
         observador está en uso (por ejemplo, otro miembro de la misma clase).

      e) La palabra clave del diseño: ¿QUIÉN LIBERA? es la pregunta que
         convierte diseños difusos en código sin fugas: toda liberación
         tiene UN dueño determinado y ese dueño ES RAII (o destroy control).

      f) Nota sobre stack: los punteros CRUDOS a un objeto local (del
         frame) son contratos VÁLIDOS como transitorios: se utilizan solo
         durante la llamada. El problema es otro: un raw que SOBREVIVE AL
         FRAME (así: observador permanente): para el observador permanente,
         es weak_ptr (o carga la vida del dueño garantizada).

   2. ¿QUÉ PASA POR DEBAJO?
      No hay costo nuevo: este es un CONTRATO de tipo que paga en
      legibilidad y en checks de compilación: el unique_ptr no copia (el
      error es de compilador, no una fuga posterior), el shared_ptr copia
      contando, y el weak te OBLIGA a pasar por lock() antes de usar.
      El compilador se vuelve tu documentador vivo: si la firma dice cuál
      es la relación, ninguna parte del programa puede romperla en silencio.

      El retorno de make_unique/make_shared se construye EN el lugar del
      return sin copia innecesaria (copy elision garantizada de C++17; el
      detalle fino, módulo 08). El costo de la fábrica que devuelve dueño
      es exactamente cero adicional.

   3. ERRORES COMUNES
      - Contrato mentiroso: función que "devuelve un dueño" de un recurso
        que YA TENÍA dueño en otro lado: queda dos dueños independientes.
        Si la función no CREA, no devuelve propiedad.
      - "Te paso el raw y vos las responsables": el T* que sale de una
        factoría es diseño roto: la regla es que la FÁBRICA entrega el RAII.
      - Confundir "no puede ser null" (T&) con "puede no estar" (T*): son
        dos semánticas DISTINTAS y cada una tiene su firma. Elegí y
        respetá: coherencia entre la demo y la documentación.
      - const unique_ptr<T>& en TODAS las piezas: innecesario: la const T&
        (desreferenciada) hace lo mismo con menos fricción, salvo que
        además tengas que hablar del PUNTERO (por ejemplo: si puede null).

   4. DEMO: la misma escena escrita con la tabla a la vista
 */
#include <iostream>
#include <memory>
#include <string>

class Sonido {
private:
    std::string nombre_;

public:
    explicit Sonido(std::string n) : nombre_(std::move(n)) {
        std::cout << "    [ctor ] " << nombre_ << "\n";
    }
    ~Sonido() { std::cout << "    [dtor ] " << nombre_ << "\n"; }
    void reproducir() const {
        std::cout << "    suena " << nombre_ << "\n";
    }
    const std::string& nombre() const { return nombre_; }
};

// FÁBRICA: CREA y ENTREGA el dueño. (Solo crea: nunca recicla recursos.)
std::unique_ptr<Sonido> crearEfecto(const std::string& nombre) {
    return std::make_unique<Sonido>(nombre);
}

// TRANSITORIA: usa DURANTE la llamada. Referencia: garantiza existencia.
void reproducir(const Sonido& s) {
    std::cout << "    (uso transitorio de " << s.nombre() << ")\n";
    s.reproducir();
}

// TRANSITORIA OPCIONAL: nullptr es un estado ("sin sonido este frame").
void marcar(const Sonido* s) {
    if (s) {
        std::cout << "    hit: " << s->nombre() << "\n";
    } else {
        std::cout << "    sin sonido (nullptr = sin dato, no bug)\n";
    }
}

// OBSERVADOR de vida dudosa: weak_ptr + lock() (ritual de la L09).
void chequear(const std::weak_ptr<Sonido>& w) {
    if (auto vivo = w.lock()) {
        std::cout << "    visible: " << vivo->nombre() << "\n";
    } else {
        std::cout << "    se fue (el weak no sostiene)\n";
    }
}

int main() {
    std::cout << "=== 1. FÁBRICA DEVUELVE DUEÑO ===\n";
    auto pasos = crearEfecto("paso.wav");      // dueño: main
    pasos->reproducir();

    std::cout << "\n=== 2. TRANSITORIO (referencia, no admite null) ===\n";
    reproducir(*pasos);

    std::cout << "\n=== 3. TRANSITORIO OPCIONAL (raw, admite null) ===\n";
    marcar(pasos.get());
    marcar(nullptr);

    std::cout << "\n=== 4. OBSERVADOR CON VIDA DUDOSA (weak) ===\n";
    {
        std::shared_ptr<Sonido> mensaje = std::make_shared<Sonido>("ping.wav");
        std::weak_ptr<Sonido> w = mensaje;
        chequear(w);
        mensaje.reset();                       // el dueño suelta
        chequear(w);                           // ahora ya no está
    }

    std::cout << "\n=== LA TABLA, UNA VEZ MÁS ===\n";
    std::cout << "  transitorio seguro        : const T& / const T*\n";
    std::cout << "  transitorio opcional      : const T* (nullptr ok)\n";
    std::cout << "  observador de vida dudosa : weak_ptr + lock()\n";
    std::cout << "  dueño exclusivo           : unique_ptr + make_unique\n";
    std::cout << "  dueños legítimos          : shared_ptr + make_shared\n";
    return 0;
}

