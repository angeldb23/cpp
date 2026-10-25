/* =============================================================================
   MÓDULO 13 - LECCIÓN 08: excepciones en destructores
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
       Regla de oro, sin rincón: un destructor NO LANZA. Dos motivaciones
       técnicas:

         1) Los destructores son noexcept POR DEFECTO. Está en el
            estándar: ~X() tiene el efecto de ~X() noexcept. Para
            "permitirle" lanzar hay que declararlo a mano
            (~X() noexcept(false)), que es una decisión rara y casi
            siempre un error de diseño.
         2) Que LANCE es pelearse con la propagación: si el dtor corre
            DURANTE el unwinding (ya había una excepción volando) y
            lanza la suya, hay dos excepciones activas a la vez y el
            resultado es std::terminate: el programa muere sin catch,
            sin log, sin confirmación.

       ¿Y si "cerrar el archivo" puede FALLAR? Ese error existe y hay
       que reportarlo a alguien: el diseño en C++ es separar dos
       caminos por la MISMA clase:
         - el método explícito cerrar()/flush()/commit(): lanzable, y
           ahí el que llama puede REACCIONAR (reintentar, guardar,
           mostrar el error): es el camino de "repáralo ahora".
         - el dtor SIEMPRE cierra con "mejor esfuerzo": no lanza; si la
           operación de cierre falla, lo anota (log) y el error no
           sale. Si te olvidaste de llamar cerrar(), el dtor te cubre
           en el último caso (módulo 07: el RAII es el cerrojo de la
           operación).
       El detalle fino: si el dtor corre DURANTE el unwinding (un throw
       anda en marcha) y lanza OTRA, se pierde todo: std::terminate.
       La forma estándar de hacer la limpieza de un dtor es un
       try/catch ADENTRO del dtor que ATRAPA lo lanzable y NO RELANZA:
       el catch de un dtor es el "no lanzar" hecho explícito.
    2. ¿QUÉ PASA POR DEBAJO?
        El estándar lo determina: un dtor de usuario es noexcept(true)
        (igual cualquier dtor llamado internamente, incluido el de cada
        miembro y el de la base). Si un throw sale de un dtor:
            - si el unwinding no estaba en marcha: el error se propaga
              igual que cualquier otro throw (y el catch de la capa
              sigue su camino);
            - si el unwinding YA estaba en marcha: el resultado es
              std::terminate() (no hay un manejador para "dos errores
              a la vez").
        noexcept(false) en un dtor no cambia la regla de unwinding: por
        eso es una decisión "rarísima" y se escribe DECLARE-do arriba
        (L03 dejó esa marca y acá la justifica).
    3. BUENAS PRÁCTICAS:
        - El dtor no lanza SIEMPRE: en el peor caso try/catch adentro
          que atrapa y loguea (sin relanzar).
        - Si cerrar puede fallar y eso TIENE que poder repararse: ofré
          cerrar()/flush()/commit() lanzable y descuelga tu RAII del
          camino: el que_quiere reparar llama el método y maneja el
          error con la L02.
        - No "confíes" en lanzar desde el dtor: el resultado en un
          unwinding es std::terminate: ni log ni catch de la capa lo
          puede capturar para nada.
        - noexcept(false) en un dtor: solo como último recurso y con
          justificación escrita.
   ============================================================================= */
#include <iostream>
#include <stdexcept>
#include <string>

namespace Leccion08 {

// Sincronizador de log: el caso clásico de dtor que no lanza.
class EscritorLog {
public:
    explicit EscritorLog(std::string archivo) : archivo_(std::move(archivo)) {}

    // El método LANZABLE: el camino donde el fallo se puede reparar ya.
    void cerrar() {                        // puede lanzar: ese es el punto
        if (falla_) throw std::runtime_error("el flush falló: " + archivo_);
        estado_ = "cerrado";
        cerrado_ = true;
    }

    ~EscritorLog() {
        try {
            if (!cerrado_) {
                // mejor esfuerzo: sin lanzar; si el "cerrar real" (con
                // disco de por medio) fallara, alguien lo atraparía acá
                estado_ = "cerrado (por dtor)";
            }
        } catch (...) {
            // catch en un dtor: ATRAPAR y NO RELANZAR. Es la forma
            // estándar de garantizar el "no lanzar" de este dtor: el
            // fallo queda en el log de la clase.
            std::cout << "  dtor de " << archivo_ << ": fallo silencioso (log)\n";
        }
    }

    void setFalla(bool f) { falla_ = f; }   // simula el disco en contra
    const std::string& estado() const { return estado_; }

private:
    std::string archivo_;
    std::string estado_ = "abierto";
    bool falla_ = false;
    bool cerrado_ = false;
};

// El costo exacto de lanzar DESDE un dtor DURANTE la propagación:
// este demo es conceptual: si se ejecuta con unwinding arriba,
// termina en std::terminate. La regla: NUNCA hacer esto de verdad.
struct Peligroso {
    ~Peligroso() noexcept(false) {         // explícito: casi nunca conviene
        throw std::runtime_error("dtor lanza");
    }
};

} // namespace Leccion08

int main() {
    std::cout << "== el dtor silencioso (la pieza del RAII) ==\n";
    {
        Leccion08::EscritorLog log("juego.log");
        std::cout << "estado en uso: " << log.estado() << '\n';
    }                                     // el dtor cierra sin lanzar
    std::cout << "saliendo del scope; el RAII cerró solo\n";

    std::cout << "== el camino explícito (cerrar() lanzable) ==\n";
    Leccion08::EscritorLog log2("config.log");
    log2.setFalla(true);
    try {
        log2.cerrar();                       // el error sale y se puede REACCIONAR
    } catch (const std::exception& e) {
        std::cout << "cerrar() lanzó: " << e.what() << " (el que llama decide)\n";
    }
    std::cout << "estado final: " << log2.estado() << '\n';
    return 0;
}
