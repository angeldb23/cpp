/* =============================================================================
   MÓDULO 07 - LECCIÓN 09: weak_ptr (2/2): lock(), observador y caché viva
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      La L08 mostró weak_ptr como LA herramienta anti-ciclo. Su segunda vida
      es más general: es el CONTRATO DE OBSERVADOR del curso:

        "Puedo señalar tu recurso, NO puedo exigir que esté."

      a) Tres operaciones y ya:
           - w.expired():  ¿el objeto está muerto? (bool, sin lock)
           - w.lock():     me da un shared_ptr vivo o vacío (el rito de uso).
           - w.reset():    suelta mi señalamiento (raramente necesario).
         Eso ES el API completa. Todo lo demás son patrones encima.

      b) El patrón CACHÉ: el caso de uso estrella en juegos. Un gestor de
         recursos carga una textura POR NOMBRE y la guarda. ¿Qué guarda?
         Opciones:
           - shared_ptr: mantiene la textura viva aunque ningún sprite la
             esté usando: RAM desperdiciada ("el mapa entero siempre vivo").
           - weak_ptr: el gestor solo RASTREA la textura; los sprites (los
             que la usan) la sostienen con sus shared_ptr.
         La segunda es la caché "auto-vaciante":
           1) el gestor guarda weak_ptr en su tabla;
           2) quien usa la textura la sostiene (shared) mientras dura;
           3) cuando el último usuario muere, la textura se libera SOLA
              (el weak no la sostiene);
           4) si otro la pide más tarde, lock() falla, el gestor la recarga
              y actualiza su weak.
         Sin contador manual de usuarios, sin fuga, y la RAM libera lo que
         nadie mira. En juegos de mapas grandes es el patrón rey.

      c) El patrón NOTIFICACIÓN: una lista de suscriptores que NO debe
         impedir que un suscriptor muera. La lista guarda weak_ptr; al
         notificar, lock()'ea a cada uno: los expirados se caen solos en la
         próxima pasada. (La versión polimórfica era el observador de POO II;
         la vida segura de este patrón es weak_ptr.)

      d) El PRECIO (que ya sabés de la L07-L08):
           - lock() es una operación atómica sobre el bloque de control:
             barata, pero no gratis;
           - recibir/devolver weak_ptr implica que CADA USO ya es "¿estás?"
             + usar: nunca acceso directo.
         Y a favor: weak_ptr nunca impide la liberación: no puede fugar.

      e) Conclusión: la tabla de decisión del curso (se completa en la L10):
           propiedad exclusiva   -> unique_ptr  (+ make_unique)
           propiedad compartida  -> shared_ptr  (+ make_shared)
           observación           -> weak_ptr (quedará vivo variable) o
                                    raw T* (uso seguro al corto plazo: L10)

   2. ¿QUÉ PASA POR DEBAJO?
      El weak_ptr guarda el puntero al BLOQUE DE CONTROL (que conoce dónde
      está el objeto) y participa del contador DÉBIL. lock() es una
      operación atómica: "intentá sumarme al grupo de dueños":
        - si strong > 0 en ese instante: +1 (efectivo) y devuelve el shared:
          el objeto queda GARANTIZADO vivo hasta que tu cargador muera;
        - si strong == 0: devuelve vacío: no intenta resucitar.
      Por eso lock() directo y único es el camino seguro: la combinación
      expired()-seguido-de-lock() tiene una VENTANA: otro hilo puede matar
      al último dueño entre las dos llamadas: expired() solo sirve para
      diagnóstico o para limpiar tablas (no para decidir el uso).

   3. ERRORES COMUNES
      - expired() y después lock() esperando coherencia entre hilos: raza.
        lock() es "decide y sostiene" en un paso atómico: usalo directo.
      - Guardar weak y asumir que lock() eterno va a dar resultado: si el
        único dueño ya murió, lock() da vacío EN CUALQUIER MOMENTO: el
        código SIEMPRE chekea el resultado del lock().
      - "Solo usaré expired() para no cargar la textura": la señal del
        objeto puede cambiar entre tu chequeo y tu uso: ATOMICO, o nada.
      - Devolver weak_ptr de una factoría SIN dueño en el sitio de creación:
        el recurso nace sin sostenedor: muerte inmediata. La factoría
        devuelve shared_ptr (dueño) y quien quiera observación, crea weak
        desde ese shared.
      - Lista de notificación que NUNCA se limpia de expireds: el rastro
        del weak crece: barato, pero sucio: limpiá en la pasada de
        notificación (patrón estándar: es una línea).

   4. DEMO: cache de texturas auto-vaciante (el patrón rey)
 */
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>

class Textura {
private:
    std::string nombre_;
    int tamano_;
public:
    Textura(std::string n, int t) : nombre_(std::move(n)), tamano_(t) {
        std::cout << "    [ctor ] " << nombre_ << " (" << tamano_ << " KB)\n";
    }
    ~Textura() { std::cout << "    [dtor ] " << nombre_ << "\n"; }
    const std::string& nombre() const { return nombre_; }
};

// El GESTOR de recursos: caché de weak_ptr auto-vaciante.
class GestorTexturas {
private:
    std::unordered_map<std::string, std::weak_ptr<Textura>> cache_;

public:
    std::shared_ptr<Textura> pedir(const std::string& nombre) {
        std::weak_ptr<Textura>& slot = cache_[nombre];
        if (auto vivo = slot.lock()) {           // ¿sigue en RAM?
            std::cout << "    cache HIT\n";
            return vivo;
        }
        // El weak expiró (o es nuevo): recarga y actualiza el slot.
        std::cout << "    cache MISS -> cargo y guardo un weak\n";
        auto fresca = std::make_shared<Textura>(nombre, 512);
        slot = fresca;                           // weak: NO sostiene
        return fresca;                           // el caller sostiene
    }
    std::size_t entradas() const { return cache_.size(); }
};

int main() {
    std::cout << "=== 1. LA CACHÉ QUE SE SECA SOLA ===\n";
    GestorTexturas gestor;
    {
        auto dragon = gestor.pedir("dragon.png");       // miss -> carga
        auto dragon2 = gestor.pedir("dragon.png");      // hit -> la misma
        std::cout << "    ¿misma textura? "
                  << (dragon == dragon2 ? "sí" : "no") << "\n";
        // al cerrar el bloque, dragon y dragon2 mueren: strong 0: dtor.
        // El weak de la caché NO lo impide.
    }
    std::cout << "    la textura ya no está en RAM (el [dtor] lo dice).\n";
    std::cout << "    entradas en la caché: " << gestor.entradas()
              << " (es el RASTRO del weak, no el recurso)\n";

    std::cout << "\n=== 2. LA CACHÉ RECONSTRUYE LO PEDIDO ===\n";
    {
        auto dragon = gestor.pedir("dragon.png");       // miss de nuevo
        (void)dragon;
        std::cout << "    entradas sigue: " << gestor.entradas()
                  << " (mismo slot, recargado)\n";
    }

    std::cout << "\n=== 3. EL RITUAL DE lock() ===\n";
    {
        auto tex = std::make_shared<Textura>("barril.png", 64);
        std::weak_ptr<Textura> w = tex;                 // observador
        std::cout << "    weak pendiente, sin dueño nuevo: strong = "
                  << tex.use_count() << "\n";
        if (auto vivo = w.lock()) {                     // decide y sostiene
            std::cout << "    uso vía lock(): " << vivo->nombre() << "\n";
        }
        std::cout << "    ¿expired? " << (w.expired() ? "sí" : "no") << "\n";
    }

    std::cout << "\n=== 4. LA REGLA SIMPLE ===\n";
    std::cout << "  lock() si tenés un weak y querés USAR;\n";
    std::cout << "  expired() solo para diagnóstico o limpieza de tablas;\n";
    std::cout << "  y el resultado de lock() SIEMPRE se chequea.\n";
    return 0;
}
