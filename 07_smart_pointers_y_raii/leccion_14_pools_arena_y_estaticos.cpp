/* =============================================================================
   MÓDULO 07 - LECCIÓN 14: Cuándo NO usar smart pointers: pools, arena, estáticos
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      La lección que faltaba para cerrar el módulo honesto: los smart
      pointers no son gratis en TODOS los escenarios. Hay tres modelos de
      vida donde gestión a cargo del dueño individual SOBRA:

      a) OBJETOS DE DURACIÓN ESTÁTICA: viven durante TODO el programa
         (config, tablas de niveles, servicios globales). Ahí no hay
         fuga posible: nacen al arrancar y mueren al terminar el
         proceso, sin dtor individual que administre nada. Un unique_ptr
         global da el mismo resultado con una trampa extra: el ORDEN de
         destrucción entre unidades de traducción no está definido
         (módulo 05: el "static destruction order fiasco": un dtor
         global puede tocar un static de otro archivo ya destruido).
         Para servicios globales simples: static plain, sin RAII
         individual: su vida es "toda la app" por definición.

      b) ARENA (gestión en LOTE): en un juego, cada frame nacen cientos
         de objetos temporales (proyectiles, partículas, cuerpos de IA).
         Crear/destruir CADA uno con new/delete es caro: cada operación
         toca el allocator global (con su lock interno: módulo 14: en
         multihilo se vuelve cuello de botella). La arena invierte el
         modelo:
            1. SE RESERVA UN BLOQUE GRANDE (una sola allocación).
            2. TODOS los objetos del frame se construyen DENTRO.
            3. Al cambiar de frame, el bloque entero se libera: la
               "limpieza" es un solo free, no mil deletes.
         En el 90% de esos casos NADIE suelta una partícula a mitad de
         frame, con lo que el RAII por objeto SOBRA: el dueño de todos
         a la vez es la ARENA.

      c) POOL con REUTILIZACIÓN (free-list): proyectiles, balas,
         partículas: en vez de crear/destruir, se PRE-ALOCAN N al
         arrancar y se marcan libres/ocupados. "Spawn" = tomar uno del
         pool; "destruir" = devolverlo. La memoria ya está reservada:
         ni allocator, ni delete, ni erase de vector: los objetos se
         RECICLAN. Ojo: en un pool NO hay RAII por objeto: el dueño es
         el POOL (envolver sus slots en unique_ptr con deleter que
         "devuelva al pool" es posible con lo de la L12, pero casi
         nunca hace falta: el pool gestiona el ciclo completo).

      d) La forma de decidir, una línea por caso:
           ¿vida arbitraria, objeto por objeto?
              -> dueño RAII (unique/shared: L04-L07)
           ¿viven toda la app?
              -> estáticos: sin RAII individual
           ¿nacen y mueren por oleada, cada frame?
              -> arena: bloque grande + reset completo
           ¿nacen y mueren muchísimas veces, se reciclan?
              -> pool: pre-alocación + free-list
         Las dos últimas cargan el peso en motores: la política de
         asignación es la primera optimización de infra que se aprende
         (la retomamos en el módulo 15 y con los ECS del 22).

   2. ¿QUÉ PASA POR DEBAJO?
      malloc/free cruzan biblioteca y, en volumen, al SO: (y se
      sincronizan entre hilos: cuello). Pool y arena evitan TODO eso:
      pool = un par de instrucciones sobre la free-list; arena = sumar
      el tamaño al puntero de avance (ptr += size, redondeado a
      alineación). El reuso, encima, deja cada objeto SIEMPRE en los
      mismos bytes: entra al caché y queda: la brecha se mide en x10 a
      x1000 según el caso, con las herramientas del módulo 15.

      Nota sobre los estáticos: su precio real es de DISEÑO, no de
      performance: el orden de destrucción ENTRE archivos no está
      garantizado (un dtor global puede tocar un static de otro archivo
      ya destruido) y el estado global acopla todo con todo (y las
      pruebas lo sufren). Por eso: estáticos para datos de configuración
      y servicios inmutables, no para estado mutable de juego.

   3. ERRORES COMUNES
      - unique_ptr GLOBALES "por si acaso": el orden de destrucción
        entre archivos es frágil: si un servicio global destruido antes
        usa a otro, UB silencioso. Mejor: un dueño claro de la vida del
        PROGRAMA (main) o static plain sin dtor individual.
      - pool QUE DEVUELVE objetos "vivos" (objetos con estado mutado y
        sin reset en release): la partida más clásica de bugs del mundo
        de motores: el bug de "salte partícula de la partida pasada".
        Acordate de resetéar TODO el estado al devolver al pool (o usar
        RAII chico dentro).
      - Guardar EL OBJETO DEL POOL en unique_ptr: doble dueño: el pool
        lo cataloga como libre y, al destruirse el unique, el dtor corre
        DOS veces (o el pool lo re-usa mientras su unique vive).
      - Confundir ARENA con POOL: arena: no se puede devolver pieza a
        pieza; pool: pieza a pieza, y solo si los objetos son del MISMO
        tamaño (o clase).

   4. DEMO: pool de proyectiles con free-list (el patrón de los motores)
 */
#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

struct Proyectil {
    float x = 0.0f, y = 0.0f;
    float velocidad_x = 0.0f;
    bool  activo = false;
    std::string dueño;
};

class PoolProyectiles {
private:
    std::vector<Proyectil> bloques_;     // la memoria DE UNA VEZ
    std::vector<std::size_t> libres_;    // free-list de índices

public:
    explicit PoolProyectiles(std::size_t capacidad) : bloques_(capacidad) {
        // Ignorar los índices 0..cap-1 iniciales: TODOS al pool de libres.
        for (std::size_t i = capacidad; i > 0; --i) {
            libres_.push_back(i - 1);
        }
        std::cout << "    pool armado: " << capacidad
                  << " proyectiles pre-alocados en un solo vector\n";
    }

    Proyectil* spawn(const std::string& dueño, float x, float y, float vx) {
        if (libres_.empty()) {
            std::cout << "    (pool agotado: ni creado ni fuga posible)\n";
            return nullptr;
        }
        Proyectil& p = bloques_[libres_.back()];
        libres_.pop_back();
        // RESET COMPLETO: un slot reciclado no puede traer el pasado.
        p.x = x; p.y = y; p.velocidad_x = vx; p.activo = true;
        p.dueño = dueño;
        return &p;
    }

    void liberar(Proyectil* p) {
        if (p == nullptr) return;
        p->activo = false;                 // visible para el loop de update
        p->dueño.clear();
        for (std::size_t i = 0; i < bloques_.size(); ++i) {
            if (&bloques_[i] == p) {
                libres_.push_back(i);      // vuelve al pool
                return;
            }
        }
    }

    std::size_t activos() const {
        std::size_t n = 0;
        for (const auto& p : bloques_) if (p.activo) ++n;
        return n;
    }
};

int main() {
    std::cout << "=== 1. POOL: NADA SE CREA NI SE DESTRUYE EN EL FRAME ===\n";
    PoolProyectiles pool(4);

    pool.spawn("torre", 1.f, 2.f, 0.5f);
    Proyectil* b = pool.spawn("torre", 8.f, 2.f, -0.5f);
    Proyectil* c = pool.spawn("torre", 4.f, 0.f, 0.f);
    std::cout << "    activos: " << pool.activos() << "\n";
    (void)c;

    std::cout << "\n=== 2. RECICLADO: EL MISMO BLOQUE PARA OTRO OBJETO ===\n";
    pool.liberar(b);                        // ¿b "murió"? No: volvió al pool
    std::cout << "    activos: " << pool.activos() << "\n";
    Proyectil* d = pool.spawn("player", 0.f, 0.f, 1.f);
    std::cout << "    d==b (reuso exacto del bloque): "
              << (d == b ? "sí" : "no") << "\n";
    std::cout << "    d.dueño: " << d->dueño
              << " (completamente reseteado, sin [dtor] alguno)\n";

    std::cout << "\n=== 3. EL AGOTAMIENTO NO FUGA: NADA NUEVO BAJO LA LLUVIA ===\n";
    while (pool.spawn("hueco", 0.f, 0.f, 0.f) != nullptr) {
        // ...se llena el pool y se acabó. Sin crashes, sin leaks.
    }
    std::cout << "    activos finales: " << pool.activos() << " (de 4 slots)\n";

    std::cout << "\n=== 4. LA TABLA DEL FIN DEL MÓDULO ===\n";
    std::cout << "  vida arbitraria y pocas:  RAII (unique/shared)\n";
    std::cout << "  vida de TODO el programa: statics (sin dtor individual)\n";
    std::cout << "  oleadas por frame:        arena (bloque + reset)\n";
    std::cout << "  nacimiento/muerte masiva: pool (pre-alocación + free-list)\n";
    std::cout << "  (y el RAII individual dentro del pool, cuando el estado\n";
    std::cout << "   del objeto tiene sus propios recursos: L12 deleters)\n";
    return 0;
}
