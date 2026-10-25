/* =============================================================================
   MÓDULO 07 - LECCIÓN 11: Crudos y referencias que NO son dueños (uso diario)
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      La L10 separó DUEÑO / OBSERVADOR / TRANSITORIO. Esta lección es la
      práctica diaria de los dos últimos con las herramientas más antiguas
      (¡y más usadas!) del lenguaje: puntero crudo T* y referencia T&.

      a) El raw T* es UN OBSERVADOR de relación flexible; se usa para dos
         situaciones concretas:
            1) transitorio (durante la llamada): función que pide "un
               objeto" sin importar quién es dueño: const T* / T* (y
               nullptr expresa "sin objeto este frame").
            2) observador garantizado: miembro crudo SOLO cuando el dueño
               vive ESTRUCTURALMENTE más que él (por ejemplo: un objeto
               que apunta a un servicio global, o a su propiedad encadenada
               arriba: dueños estáticos). La única condición: ¿podés
               PROBAR que el dueño sobrevive? Si no podés: weak_ptr.

      b) La referencia T&: "hay objeto, garantizado, no nulo".
         - transitoria por excelencia: las funciones const T& de todo el
           curso;
         - el observador "estático" por excelencia: cuando un objeto SIEMPRE
           tiene a ese otro y NUNCA reasigna (módulo 05: referencias no se
           reasignan después de la construcción). Relación necesaria:
           T&; relación opcional: T* o weak.
         T& no puede expresar "sin objeto": ese caso lo resuelve T*, y el
         estado "opcional que se SOSTIENE" se resuelve con weak_ptr.
         (El módulo 11 agrega std::optional para valores no-puntero.)

      c) El patrón "dueño + vistas" (el estándar del curso): una escena
         DUEÑA sus entidades (vector<unique_ptr>) y sus sistemas de juego
         reciben `const Entidad&` para trabajar: no saben quién es dueño
         ni dónde vive la memoria, y NO DEBEN saberlo.

      d) Cuando el raw T* vence: llamadas a APIs de C (no entienden de
         weak ni de unique), hot paths de un frame (crear un cargador
         weak por cada toque sería ruido) y relaciones de observación a
         dueños garantizados. El precio: el compilador NO protege al raw
         del colgante (ASan sí: en debug). Por eso: raw = transitorio o
         garantizado; weak = vida dudosa.

      e) De qué NO se trata: no es "raw = más rápido". El weak_ptr no
         cuesta más en almacenamiento (dos punteros, 16B) y lock() es un
         chequeo atómico pequeño. La elección es SEMÁNTICA: ¿es un DATO
         de la llamada (raw/ref) o RELACIÓN entera (owner/weak)?

      f) Regla "dueño únicamente": nunca crear dueños encima de crudos:
         delete a mano. delete/delete[] manual queda permitido ÚNICAMENTE
         dentro de clases RAII (módulo 04) y, en el día a día, ni eso:
         make_unique/make_shared ya cubren todo (L04-L07).

   2. ¿QUÉ PASA POR DEBAJO?
      El puntero crudo es un entero con dirección (módulo 04):
        - admite nullptr (a diferencia de T&);
        - reasignable (a diferencia de T&);
        - NO sabe de dueños: ni RAII ni contadores: el compilador no te
          protege del colgante (ASan sí, en debug). El weak_ptr en cambio
          se protege solo: lock() consulta al dueño.

      La referencia T& es un puntero sin operadores de puntero: el
      compilador garantiza NO-NULL (por eso desreferenciar es "libre"),
      pero NO garantiza VIDA: devolver T& de un local es colgante (módulo
      04). La referencia no reemplaza al dueño: la acompaña. El dueño es
      RAII: la referencia es solo una VISTA encima.

   3. ERRORES COMUNES
      - Raw como MIEMBRO de clase con vida no garantizada: gestión manual
        de "quién muere primero", reordenar construcción... La respuesta:
        weak_ptr, o rediseñar para que el dueño garantizado exista de
        verdad. El raw de miembro exige poder PROBAR que el dueño vive
        siempre encima de él (relación estructural del módulo 05/06: la
        L10 lo llama "observador permanente").
      - Referencia colgante: devolver T& de un local (módulo 04). La
        solución sigue siendo RAII: la fábrica devuelve dueño (unique) o
        shared; no devuelvas vistas que nacen muertas.
      - Medir en crudo == performance: el dueño RAII es igual de rápido
        (el unique_ptr ES un raw + dtor). La decisión es de CONTRATO, no
        de métricas: el raw no "vence" a RAII: es la vista del dueño.
      - Firmas que piden unique_ptr cuando solo quieren USAR: bloquean al
        llamador a ser dueño exclusivo (un shared_ptr, un objeto de
        stack o un elemento de pool no pueden pasar). Contrato neutral:
        T& / const T*; propiedad: unique por valor (L10).

   4. DEMO: patrón dueño + observadores en una escena de juego
 */
#include <cstddef>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

class Enemigo {
private:
    std::string nombre_;
    int vida_ = 100;

public:
    explicit Enemigo(std::string n) : nombre_(std::move(n)) {}
    ~Enemigo() { std::cout << "    [dtor ] " << nombre_ << "\n"; }

    void recibirDano(int d) {
        vida_ -= d;
        std::cout << "    " << nombre_ << " tiene " << vida_ << " HP\n";
    }
    int vida() const { return vida_; }
    const std::string& nombre() const { return nombre_; }
};

// OBSERVADORES: crudo y referencia del RECURSO que tiene dueño aparte.

// a) Transitoria por referencia (const&, null imposible).
void curar(const Enemigo& e, int cantidad) {
    std::cout << "    (curado " << e.nombre() << ")\n";
    (void)cantidad;   // (la cura real es del dueño: acá es solo demo)
}

// b) Transitoria por crudo (admite nullptr; estado de producto).
bool golpear(Enemigo* e, int d) {
    if (e == nullptr) return false;    // sin target: no es error
    e->recibirDano(d);
    return true;
}

// c) Observador PERMANENTE: guardado en un objeto de vida CORTA (frame).
class Disparo {
private:
    Enemigo* objetivo_;      // apunta a un enemigo que el dueño garantiza
    const char* balas_;

public:
    Disparo(Enemigo* objetivo, const char* balas)
        : objetivo_(objetivo), balas_(balas) {
        std::cout << "    " << balas_ << " apunta a " << objetivo->nombre() << "\n";
    }
    void disparar() {
        if (objetivo_ != nullptr && objetivo_->vida() > 0) {
            std::cout << "    boom de " << balas_ << ": ";
            objetivo_->recibirDano(10);
        }
    }
};

// La escena: DUEÑO del enemigo en unique_ptr. (Vector dueño.)
int main() {
    std::cout << "=== 1. DUEÑO + OBSERVADORES EN EL MISMO FRAME ===\n";
    {
        auto jefe = std::make_unique<Enemigo>("jefe_orco");

        curar(*jefe, 20);              // referncia: siempre objetivo vivo
        golpear(jefe.get(), 30);       // crudo: maneja "sin target"
        Enemigo* sinTarget = nullptr;
        golpear(sinTarget, 30);        // y ni siquiera es ERROR.

        Disparo primera(&*jefe, "flecha");
        primera.disparar();
        primera.disparar();
        std::cout << "    ¿vida del jefe garantizada DURANTE Disparo?: sí\n";
        std::cout << "    (el dueño del jefe es el scope: Disparo muere antes)\n";
    }

    std::cout << "\n=== 2. DUEÑO ES VECTOR: LA VIDA ES DEL CONTENEDOR ===\n";
    {
        std::vector<std::unique_ptr<Enemigo>> escuadron;
        escuadron.push_back(std::make_unique<Enemigo>("abeja_1"));
        escuadron.push_back(std::make_unique<Enemigo>("abeja_2"));

        for (const auto& e : escuadron) {
            std::cout << "    en la lista: " << e->nombre() << "\n";
        }
        escuadron.clear();             // la escena libera TODO acá
        std::cout << "    (después de clear, no quedan enemigos vivos)\n";
    }

    std::cout << "\n=== 3. LA REGLA FINAL DEL MÓDULO ===\n";
    std::cout << "  mem. dinámica       : SIEMPRE con dueño RAII (L04-L07)\n";
    std::cout << "  crudo T*/T&         : transitorio u observador garantizado\n";
    std::cout << "  vida dudosa         : weak_ptr + lock() (L08-L09)\n";
    std::cout << "  delete a mano       : ÚNICO en clases RAII (módulo 04)\n";
    return 0;
}
