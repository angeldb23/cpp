/* =============================================================================
   MÓDULO 07 - LECCIÓN 08: weak_ptr (1/2): los ciclos de propiedad
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      shared_ptr resuelve "varios dueños": el recurso vive mientras alguno lo
      necesite. Pero hay un diseño que shared_ptr SOLO no puede resolver:
      cuando dos objetos se referencian MUTUAMENTE.

      a) El ciclo: si A tiene un shared_ptr a B, y B tiene un shared_ptr a A,
         los contadores nunca llegan a 0:
            - muere el shared_ptr externo de A: strong(A) baja de 2 a 1
              (porque adentro B todavía lo tiene);
            - muere el externo de B: strong(B) baja de 2 a 1 (A lo tiene).
         Cada uno sostiene la vida del otro: NINGÚN destructor corre. Es una
         fuga de memoria que ni falta hace descubrir con herramientas: son
         dos objetos vivos para siempre en el heap.

      b) La solución conceptual: en un ciclo real, las dos flechas NO son
         igual de propiedad. Uno es el dueño (el "padre"), el otro apenas
         OBSERVA (el "hijo" que necesita señalar a su padre, pero no puede
         exigir que la escena viva). La flecha del observador se escribe con
         weak_ptr: apunta al recurso SIN sostenerlo.

      c) weak_ptr<T>: "puede que el recurso ya no esté". No incrementa el
         contador fuerte: es invisible para la duración del recurso. Si los
         dueños mueren, el weak queda EXPIRADO.

      d) weak_ptr NO se desreferencia: no hay *w ni w->. Esa es la característica,
         no una limitación: no podés tocar el recurso sin pasar por lock(),
         que devuelve un shared_ptr temporal:
            - recurso vivo: lock() te da un "cargador" (dueño temporal) para
              usar el recurso con RAII hasta el fin de la expresión;
            - recurso muerto: lock() devuelve un shared_ptr vacío.
         El patrón constante: `auto fuerte = w.lock(); if (fuerte) ...`

      e) El ejemplo más común del curso (y de juego): el árbol de escena.
         Escena es dueña de sus hijos (vector<shared_ptr>). Cada hijo quiere
         saber quiénes son su padre o los sistemas (cámara, audio): flechas
         HACIA ATRÁS: weak_ptr. Hacia atrás, strong forzaría que el hijo
         mantenga viva la escena que lo contiene: el ciclo de nuevo.

      f) Aun así, el mejor consejo: el ciclo casi SIEMPRE se rompe mejor con
         DISEÑO que con herramientas: si A y B se apuntan mutuamente con
         strong, preguntate cuál de las dos flechas NO es propiedad. Dejar
         una sola flecha strong y hacer las otras débiles es el patrón de
         referencia unidireccional sano.

   2. ¿QUÉ PASA POR DEBAJO?
      El bloque de control tiene DOS contadores: fuerte (dueños) y débil
      (weak_ptr pendientes). Las reglas: strong en 0 ejecuta el destructor
      del OBJETO; strong y weak juntos en 0 liberan el BLOQUE (la memoria
      administrativa y, con make_shared, la del objeto también: L07).

      lock() hace un intento atómico de "strong+1": si en ese instante hay
      dueños, te devuelve un shared_ptr (y el objeto queda garantizado vivo
      mientras ese cargador exista); si strong era 0, devuelve vacío. La
      decisión es atómica: dos hilos haciendo lock() y destruyendo dueños
      simultáneamente no corrompen nada (esa sincronización sale gratis:
      módulo 14 para compartir UN shared_ptr entre hilos).

      expired() es el atajo de lock() cuando no querés el recurso: solo
      considera si el objeto sigue vivo (strong > 0).

   3. ERRORES COMUNES
      - Debilitar TODOS los punteros "por prudencia": sin ciclos reales, no
        hace falta: cada lock() que aparece en toda tu lógica es código
        defensivo gratuito, y el `auto f = w.lock(); if (f)` es dos veces
        el código de `p->usar()`.
      - "Guardar el resultado de lock()": ya no es débil: lock() te dio un
        DUEÑO (por eso compila y mantiene el objeto). El patrón débil es
        lock() en el punto de uso, no almacenar el cargador.
      - CUIDADO: usar expired() y después lock() "porque acá seguro sigue
        vivo": entre las dos llamadas otro hilo pudo matar al último dueño.
        Usa lock() directamente: entrega un shared o vacío, no adivine.
      - Debilitar por compulsión: si la referencia mutua es en realidad
        jerarquía (dueño de un lado, observador del otro), una sola flecha
        strong + la otra weak es el diseño correcto, NO meter weak a ambos
        lados del ciclo sin analizarlos.
      - lock() defensivo a cada paso: el weak no dueño es para observadores
        concretos (cámara, UI, caché). Si llamás lock() mil veces por frame
        para lo mismo, guardá el resultado en un shared_ptr local (cargo
        local de ese frame es legal; guardarlo en un miembro permanente
        ya no es débil: sumó un dueño).

   4. DEMO: el ciclo fugado a propósito y la versión que sí se limpia
      (La parte 1 fuga memoria REAL, a propósito, para que veas que no hay
      destructores: con ASan el binario reportaría la fuga del ciclo.)
 */
#include <iostream>
#include <memory>
#include <string>

// El ciclo ROTO: flechas fuertes mutuas.
class Caballero;
class Monstruo;

class Caballero {
public:
    std::shared_ptr<Monstruo> enemigo;   // strong hacia el otro: MAL
    const char* nombre;
    explicit Caballero(const char* n) : nombre(n) {
        std::cout << "    [ctor ] " << nombre << "\n";
    }
    ~Caballero() {
        std::cout << "    [dtor ] " << nombre << "\n";
    }
};

class Monstruo {
public:
    std::shared_ptr<Caballero> enemigo;  // strong de vuelta: CYCLE
    const char* nombre;
    explicit Monstruo(const char* n) : nombre(n) {
        std::cout << "    [ctor ] " << nombre << "\n";
    }
    ~Monstruo() {
        std::cout << "    [dtor ] " << nombre << "\n";
    }
};

// La versión CORRECTA: una flecha strong, la de vuelta, weak.
class Heroe;
class Bicho;

class Heroe {
public:
    std::shared_ptr<Bicho> enemigo;      // strong: EXACTAMENTE una
    const char* nombre;
    explicit Heroe(const char* n) : nombre(n) {
        std::cout << "    [ctor ] " << nombre << "\n";
    }
    ~Heroe() { std::cout << "    [dtor ] " << nombre << "\n"; }
};

class Bicho {
public:
    std::weak_ptr<Heroe> mirando_a;      // weak: no sostiene la vida
    const char* nombre;
    explicit Bicho(const char* n) : nombre(n) {
        std::cout << "    [ctor ] " << nombre << "\n";
    }
    ~Bicho() { std::cout << "    [dtor ] " << nombre << "\n"; }
};

// El rito de uso del weak: lock() + chequeo.
std::string estadoDe(const std::weak_ptr<Caballero>& w) {
    if (auto fuerte = w.lock()) {        // ¿está vivo? -> cargador RAII
        return std::string("vivo: ") + fuerte->nombre;
    }
    return "expirado";                   // sin dueños: nada que mirar
}

void print_rationale();                  // declarado: definido después de main

int main() {
    std::cout << "=== 1. EL CICLO: NINGUN DTOR CORRE (FUGA REAL) ===\n";
    {
        auto a = std::make_shared<Caballero>("helmo");
        auto b = std::make_shared<Monstruo>("goblin");
        a->enemigo = b;                  // strong(b) = 2
        b->enemigo = a;                  // strong(a) = 2
        std::cout << "    strong helmo : " << a.use_count() << "\n";
        std::cout << "    strong goblin: " << b.use_count() << "\n";
    }   // los dos shared_ptr externos mueren: strong queda 1 y 1: nadie llega
        // a 0. ACÁ NO APARECE NINGÚN [dtor]. Fuga pedagógica y oficiosa.
    std::cout << "    (al salir del bloque NO se imprime ningún [dtor])\n\n";

    std::cout << "=== 2. LA CORRECCIÓN: UNA SOLA FLECHA STRONG ===\n";
    {
        auto heroe = std::make_shared<Heroe>("aldric");
        auto bicho = std::make_shared<Bicho>("slime");
        heroe->enemigo  = bicho;         // strong hacia slime
        bicho->mirando_a = heroe;        // weak hacia aldric
        std::cout << "    strong aldric: " << heroe.use_count()
                  << " (1: solo main; el weak NO suma)\n";
        std::cout << "    strong slime : " << bicho.use_count() << "\n";
    }   // strong llega a 0 en ambos: dtors de ALDRIC y SLIME corren.
    std::cout << "    (los [dtor] sí aparecieron: el ciclo no existe)\n\n";

    std::cout << "=== 3. EL RITO DE lock()/expired() ===\n";
    {
        std::weak_ptr<Caballero> w;
        {
            auto k = std::make_shared<Caballero>("sansoon");
            w = k;                       // weak: no sube el contador
            std::cout << "    estado: " << estadoDe(w) << "\n";
        }   // 'k' muere: strong 0: dtor corre: el weak no puede frenar.
        std::cout << "    tras morir: " << estadoDe(w) << "\n";
    }

    std::cout << "\n=== 4. PALABRA FINAL QUE LANZA HACIA LA L09 ===\n";
    std::cout << "  weak_ptr: un rótulo 'puede que no esté' para observadores\n";
    print_rationale();
    return 0;
}

void print_rationale() {
    std::cout << "  que muestran las flechas NO-owner: cámara, UI, caches,\n";
    std::cout << "  suscripciones: lo típico es lock() en el punto de uso.\n";
}
