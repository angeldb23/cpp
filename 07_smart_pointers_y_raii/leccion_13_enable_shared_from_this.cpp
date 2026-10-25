/* =============================================================================
   MÓDULO 07 - LECCIÓN 13: enable_shared_from_this y ciclos de vida sutiles
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      Un problema que aparece SIEMPRE al combinar clases RAII con callbacks:
      un método de la clase necesita entregar "a sí misma" a otro sistema
      SIN que ese otro sistema sea dueño.

      a) El enunciado concreto:
            class Tarea {
                void enviarAlPlanificador() {
                    // necesito un shared_ptr<A Tarea> DE ESTA TAREA
                    // (el planificador aguarda y luego la ejecuta).
                }
            };
         ¿Qué NO funciona?
            - `shared_ptr<Tarea> yo(this);` : crea un control block NUEVO
              para el mismo objeto: dos contadores independientes para una
              misma memoria: doble delete cuando ambos llegan a 0 (L06):
              el error clásico número 1 con shared_ptr.
            - devolver *this por copia: el objeto no se copia: los recursos
              del objeto mueren con la copia, no con el original.

      b) La solución: heredamos de std::enable_shared_from_this<T>:
            class Tarea : public std::enable_shared_from_this<Tarea> {
                std::shared_ptr<Tarea> yo() { return shared_from_this(); }
            };
         shared_from_this() NO CREA un nuevo bloque: busca el control
         block EXISTENTE del objeto (la clase guarda, a bajo nivel, un
         weak_ptr a su propio bloque) y devuelve un shared_ptr del mismo
         bloque, sumando +1 al MISMO contador: perfect: mismo bloque,
         mismos dueños.

      c) LA REGLA DURA: shared_from_this() es válido únicamente si el
         objeto YA está administrado por algún shared_ptr (fuera de este
         idioma, el bloque de control ni siquiera existe aún).
            - dentro de funciones llamadas DESPUÉS de la construcción
              de un shared_ptr: ok.
            - en el CONSTRUCTOR (el objeto aún no tiene bloque): lanza
              std::bad_weak_ptr (C++17 y después) o es UB (C++14).
            - en una variable de STACK: `Tarea t; t.enviar()`: UB: no
              hay bloque ni lo puede haber.

      d) Cuándo hace falta de verdad:
            - registrar SELF en callbacks asíncronos: el sistema guarda su
              propia copia (weak o strong) mientras la tarea está en vuelo
              y el llamador original puede haber muerto;
            - objetos que se SUSCRIBEN a sí mismos a un sistema (escucha
              de eventos, colas de trabajo) y luego se auto-avisan;
            - observadores de la L09 que necesitan auto-referencia para
              registrarse.
          Y siempre la MISMA decisión de la L10: ¿el receptor MANTIENE
          VIVO o SOLO SEÑALA? Esa respuesta define lo que se entrega:
            - shared_from_this(): entrega un DUEÑO del mismo bloque;
            - weak_from_this() (C++17): entrega el weak interno, ideal
              para callbacks que observan sin sostener.

      e) El auto-ciclo: si el objeto se SOSTIENE a sí mismo guardando un
         shared_from_this() en un miembro permanente, generó el ciclo más
         corto posible: el objeto es su propio dueño y nunca muere (L08).
         La regla: shared_from_this() se entrega HACIA AFUERA y, si el
         receptor lo guarda, que lo guarde como weak (sostiene quien usa,
         no quien observa: patrón de las cachés de la L09).

   2. ¿QUÉ PASA POR DEBAJO?
      enable_shared_from_this<T> es una clase base que inyecta UN miembro
      débil interno (el _M_weak_this de libstdc++); ese weak se RELLENA
      cuando un shared_ptr adopta al objeto por PRIMERA VEZ. ¿Cómo lo
      sabe el bloque de control? El constructor de shared_ptr (y de
      make_shared) DETECTA en compile-time que T hereda de
      enable_shared_from_this<T> (mediante traits: la técnica del
      módulo 09) y enlaza el bloque con ese weak.

      Consecuencia directa: shared_from_this() es, de hecho, el lock()
      del weak interno: la MISMA técnica de la L09 aplicada al propio
      objeto. Y el weak interno muere con el bloque: si murió el último
      shared, no hay "agarrarse" de nada.

      Detalle de herencia: la base (o la clase donde se usa
      shared_from_this()) debe SER la que hereda. Si viajás compartiendo
      shared_ptr<Base> (polimorfismo del módulo 06) y la BASE no heredó
      de enable_shared_from_this<Base>, entonces desde la base no existe
      el método correcto; el truco sano es que la CLASE BASE de la
      jerarquía herede (y las derivadas lo heredan gratis, como los
      métodos virtuales).

   3. ERRORES COMUNES
      - shared_ptr<T>(this) hecho a mano: DOS bloques de control para el
        mismo objeto: doble delete al final: EL CLÁSICO.
      - Llamar shared_from_this() en el CONSTRUCTOR: el weak interno aún
        no existe (nadie te compartió aún): std::bad_weak_ptr.
      - Llamar shared_from_this() en un objeto del STACK: nunca hay
        bloque (y no puede haberlo): bad_weak_ptr también. Si un tipo
        puede vivir "en stack o compartido", su API de auto-referencia
        es señal de un diseño que mezcla maneras de vida (L10).
      - Heredar enable_shared_from_this en la DERIVADA cuando el método
        que lo usa está en la BASE: hay que heredar en la clase donde
        se llama (o en la base común de la jerarquía).
      - Entregar shared_from_this() cuando el receptor era simple
        OBSERVADOR: suma un dueño innecesario (+1) y, si lo guarda
        permanente, ciclo. Para observar: weak_from_this() (C++17), el
        método pensado exactamente para eso.

   4. DEMO: tarea que se entrega sola al planificador y su deuda de vida
 */
#include <iostream>
#include <memory>
#include <string>
#include <vector>

class Planificador;   // se define después

class Tarea : public std::enable_shared_from_this<Tarea> {
private:
    std::string nombre_;

public:
    explicit Tarea(std::string n) : nombre_(std::move(n)) {
        // OJO: acá AÚN NO se puede shared_from_this(): ningún shared_ptr
        // adoptó al objeto todavía: no hay bloque de control que enlazar.
        std::cout << "    [ctor ] " << nombre_ << "\n";
    }
    ~Tarea() { std::cout << "    [dtor ] " << nombre_ << "\n"; }

    const std::string& nombre() const { return nombre_; }

    // Se da a sí misma al planificador con el MISMO bloque de control.
    void registrarEn(Planificador& p);
};

class Planificador {
private:
    std::vector<std::weak_ptr<Tarea>> pendientes_;

public:
    void agregar(std::weak_ptr<Tarea> t) {
        pendientes_.push_back(std::move(t));
    }

    void ejecutarPendientes() {
        for (auto& w : pendientes_) {
            if (auto viva = w.lock()) {
                std::cout << "    ejecutando " << viva->nombre() << "\n";
            } else {
                std::cout << "    (una tarea pendiente ya no está)\n";
            }
        }
        pendientes_.clear();
    }
};

void Tarea::registrarEn(Planificador& p) {
    // shared_from_this(): el MISMO bloque de control, no uno nuevo.
    // Acá lo entregamos como DUENO (el +1 lo decidiría el receptor: si
    // guardara shared_ptr sería +1; guarda weak: se convierte y listo).
    p.agregar(shared_from_this());
}

int main() {
    std::cout << "=== 1. EL PATRÓN CORRECTO: SE ENTREGA POR shared_from_this ===\n";
    Planificador p;
    {
        auto t = std::make_shared<Tarea>("cargar_mapa");
        std::cout << "    strong tras crear:      " << t.use_count() << "\n";
        t->registrarEn(p);
        std::cout << "    strong tras registrar:  " << t.use_count()
                  << " (el planificador guardó WEAK: no suma dueños)\n";
    }   // 't' muere: strong 0: la tarea se destruye COMPLETO...
        // ...y el planificador no la frena (era observador).
    p.ejecutarPendientes();

    std::cout << "\n=== 2. SI EL RECEPTOR FUERA DUEÑO (shared en vez de weak) ===\n";
    std::cout << "  el mismo shared_from_this() con un receptor que guarda\n";
    std::cout << "  shared_ptr sostendría la tarea: strong no bajaría a 0 y\n";
    std::cout << "  el [dtor] correría solo cuando el PLANIFICADOR se vacíe.\n";
    std::cout << "  (Esa es la decisión L10: ¿guardás dueño o observador?)\n";

    std::cout << "\n=== 3. LA TRAMPA: shared_ptr<Tarea>(this) ===\n";
    std::cout << "  NUNCA: dos bloques de control para la misma memoria\n";
    std::cout << "  => doble delete. La solución única: enable_shared_from_this.\n";

    std::cout << "\n=== 4. LA REGLA ===\n";
    std::cout << "  heredá de enable_shared_from_this<T>\n";
    std::cout << "  usá shared_from_this() SOLO cuando ya esté compartido,\n";
    std::cout << "  y weak_from_this() si el receptor debe solo OBSERVAR.\n";
    return 0;
}
