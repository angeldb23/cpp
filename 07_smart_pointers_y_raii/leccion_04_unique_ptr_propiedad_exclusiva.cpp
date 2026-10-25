/* =============================================================================
   MÓDULO 07 - LECCIÓN 04: std::unique_ptr: propiedad exclusiva con cero costo
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      unique_ptr<T> es el wrapper RAII que vas a usar el 95% del tiempo.

      a) Propiedad EXCLUSIVA: en todo momento hay EXACTAMENTE un unique_ptr
         dueño del objeto. No se puede copiar (dos dueños con delete sería
         doble liberación: módulo 04). Podés renunciar a la propiedad para que
         otro la tome: eso es una transferencia de propiedad, y la forma
         idiomática de escribirla (std::move) entra en el módulo 08. En este
         módulo basta con "existe un solo dueño a la vez".

      b) Cero costo: sizeof(unique_ptr<T>) == sizeof(T*). No hay contador de
         referencias ni bloque de control ni indirección extra: es un puntero
         crudo con un destructor que hace delete. Esa es la diferencia clave
         con shared_ptr (L06).

      c) La fábrica canónica: std::make_unique<T>(args...) en vez de
         `unique_ptr<T> p(new T(args...))`. Dos razones: no repetís el tipo
         (auto lo deduce) y no hay ventana de fuga: el `new` queda envuelto
         dentro de make_unique en la MISMA expresión completa (la rareza del
         caso `f(unique_ptr<T>(new T), otraFunciónQueLanza())` se va a ver de
         nuevo en la L07, porque en shared_ptr es exactamente su razón para
         usar make_shared).

      d) La interfaz mínima que necesitás hoy:

           *p  y  p->      usar el objeto, como con un puntero (p nulo = UB,
                           igual que desreferenciar nullptr: módulo 04).
           p.get()         el puntero crudo SIN transferir propiedad: sirve
                           para OBSERVAR nunca para liberar ni para crear
                           otro dueño a mano. Regla: get() sale para APIs que
                           piden T* sin tomar propiedad.
           p.reset(q)      suelta (y LIBERA) el recurso actual; si pasás q,
                           adopta. El que llama reset es el responsable del
                           delete del recurso anterior.
           p.release()     RENUNCIA: te devuelve el recurso y queda vacío.
                           Es el puente legal hacia APIs de C que dicen "yo
                           libero": la propiedad sigue existiendo, solo
                           cambia de transportista (lección 12 lo usa).

         Con contenedores (módulo 03): std::vector<std::unique_ptr<Enemigo>>
         hace que el VECTOR sea dueño de la colección. erase()/clear()
         destruyen a los Enemigos en ese momento, no "al final, si me
         acuerdo". Con vector<Enemigo*> crudo tenías que liberar cada
         Enemigo a mano y después vaciar el vector: dos pasos que se olvidan.

      e) Como miembro de una clase: un miembro unique_ptr hace que la clase
         ya no sea copiable por defecto (el miembro no se copia), que es
         justamente lo correcto para un recurso exclusivo, sin escribir el
         constructor de copia ni = delete a mano. Regla de 0 que sola se
         respeta. (Las clases que SÍ necesitan copiar un recurso van a usar
         copia profunda como en POO I, o shared_ptr según el diseño.)

   2. ¿QUÉ PASA POR DEBAJO?
      Concretamente: unique_ptr<T> es un struct con solo `T* ptr_` y su
      destructor hace `if (ptr_) delete` (o con el deleter configurado, L12).
      make_unique es literalmente `unique_ptr<T>(new T(args...))`.
      No hay registro en runtime ni sweeps: todo se resuelve en compile-time
      + el delete del destructor. Medidas de banda: igual que un puntero
      crudo en costo y exactamente 8 bytes en 64 bits.

      En un vector<unique_ptr<T>>, cada elemento es un puntero (8B): los
      objetos en sí NO están contiguos. Elegí vector<T> por valor cuando los
      objetos tienen vida simple: vector de unique_ptr vale cuando los
      elementos son polimórficos (con herencia de módulo 06) o demasiado
      grandes/móviles como para copiarlos al reacomodar el vector.

   3. ERRORES COMUNES
      - `auto copia = otroUnique;` no compila: es diseño, no defecto.
      - Usar .get() como si fuera un dueño: no libera, no protege.
      - Double-owner casero: `unique_ptr<T> b(a.get());` el a y el b ambos
        creen tener el recurso: doble delete. La única forma correcta de
        pasarle el recurso a otro dueño es release() (o std::move, módulo 08).
      - Desreferenciar después de reset() o después de que el unique_ptr
        salió de ámbito: colgante módulo 04, resuelto por propiedad.
      - Fuição: guardarse el puntero de release() y no liberar: release(),
        al contrario de reset, te DEJA SIN RAII: es un casillero de seguridad
        que hay que devolver (con delete o pasando el recurso a otra capa).

   4. DEMO: creación, uso, get(), reset()/release(), vector dueño y miembro
 */
#include <cstddef>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

class Textura {
private:
    std::string archivo_;

public:
    explicit Textura(std::string archivo) : archivo_(archivo) {
        std::cout << "    [ctor] cargando " << archivo_ << "\n";
    }
    ~Textura() { std::cout << "    [dtor] liberando " << archivo_ << "\n"; }

    void dibujar() const { std::cout << "    dibujando " << archivo_ << "\n"; }
    const std::string& archivo() const { return archivo_; }
};

// API estilo C de mentira para el demo de release(): "yo libero con freeC".
static void entregarAC(const Textura* t, const char* quien) {
    std::cout << "    [" << quien << "] recibio por fuera del RAII: "
              << t->archivo() << "\n";
}

class Motor {
private:
    std::unique_ptr<Textura> fondo_;    // miembro dueño: regla de 0 gratis

public:
    Motor() : fondo_(std::make_unique<Textura>("fondo.png")) {
        std::cout << "    [Motor] armado\n";
    }
    void render() const { fondo_->dibujar(); }
    void cambiarFondo(const std::string& nuevo) {
        // Operador= de unique_ptr: libera el viejo y adopta el nuevo EN ESTA
        // línea (por eso el [dtor] de fondo.png sale antes que el [ctor]).
        fondo_ = std::make_unique<Textura>(nuevo);
    }
};

int main() {
    std::cout << "=== 1. CREACION Y USO BASICO ===\n";
    {
        auto tex = std::make_unique<Textura>("hero.png");
        tex->dibujar();                          // -> como un puntero
        std::cout << "    sizeof(unique_ptr<Textura>) = " << sizeof(tex)
                  << " bytes (idem puntero crudo)\n";
    }                                            // se libera sola al salir

    std::cout << "\n=== 2. get() PARA OBSERVAR ===\n";
    {
        auto tex = std::make_unique<Textura>("lava.png");
        const Textura* vista = tex.get();        // observador: NUNCA liberar
        vista->dibujar();
        std::cout << "    via get(): " << vista->archivo() << "\n";
        // vista sigue válida hasta que 'tex' muera, no hasta que salga un
        // scope ajeno: depende del DUEÑO, no del observador.
    }

    std::cout << "\n=== 3. reset() Y release(): CAMBIO DE MANOS CONTROLADO ===\n";
    {
        auto tex = std::make_unique<Textura>("lava.png");
        // reset(q): libera lo viejo y adopta q. Pasamos un recurso ya dueño
        // vía release() para no tener dos unique_ptr con el mismo puntero.
        std::unique_ptr<Textura> nueva = std::make_unique<Textura>("agua.png");
        tex.reset(nueva.release());              // dos pasos inmediatos
        std::cout << "    tras reset(): " << tex->archivo() << "\n";
        entregarAC(tex.get(), "gestor");         // observar por fuera
    }

    std::cout << "\n=== 4. MIEMBRO UNIQUE_PTR EN UNA CLASE ===\n";
    {
        Motor m;
        m.render();
        m.cambiarFondo("noche.png");             // libera y carga en un paso
        m.render();
    }                                            // el Motor destruye su textura

    std::cout << "\n=== 5. VECTOR DE UNIQUE_PTR: EL VECTOR ES EL DUENO ===\n";
    {
        std::vector<std::unique_ptr<Textura>> wallpapers;
        wallpapers.push_back(std::make_unique<Textura>("bioma_hielo.png"));
        wallpapers.push_back(std::make_unique<Textura>("bioma_desierto.png"));
        std::cout << "    en el vector: " << wallpapers.size() << " texturas\n";
        wallpapers.erase(wallpapers.begin());    // destruye la primera YA
        std::cout << "    tras erase:   " << wallpapers.size()
                  << " texturas (la muerta se liberó recién)\n";
    }                                            // limpio, sin delete manual
    return 0;
}
