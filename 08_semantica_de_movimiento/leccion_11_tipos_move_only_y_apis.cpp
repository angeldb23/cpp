/* =============================================================================
   MÓDULO 08 - LECCIÓN 11: Tipos move-only y diseño de APIs
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      Cerramos el módulo con la regla de oro que faltaba: la primera
      razón para implementar movimiento en un tipo no es "para que
      corra más rápido": es expresar el CONTRATO con la propiedad.
      El ejemplo canónico es std::unique_ptr (módulo 07): propiedad
      EXCLUSIVA, no copiable. El movimiento es SU ÚNICA forma de
      transferencia, y eso define su diseño en APIs.

      Tres firmas posibles para recibir un objeto pesado, cada una
      con su encuadre:

      A) POR VALOR Y MOVER ADENTRO (el parámetro "sink"):
             void brote(std::unique_ptr<Bala> bala) {
                 balas_.push_back(std::move(bala));
             }
         La función VA A GUARDAR o CONSUMIR el objeto; por eso llega
         por valor y ADENTRO se mueve a su lugar final. El robo
         explícito queda EN EL CALLER:
             escena.brote(std::make_unique<Bala>());   // rvalue: se mueve solo
             std::unique_ptr<Bala> m = ...;
             escena.brote(std::move(m));               // aquí se lee la cesión
         Es el patrón de mayor legibilidad: el punto de la cesión
         queda VISIBLE en el call site.

      B) REFERENCIA NO DUEÑA (la "borrowed"):
             void mostrar(const Bala&);   // observa, no posee
             void afectar(Bala&);         // observa y muta
         Sin movimiento acá: la función no es dueña; mover de algo
         que no te pertenece es un robo literal. (La tabla dueño vs
         observador es la lección 10 del módulo 07.)

      C) SOBRECARGA T&& (la de la L02): reservada para casos en que
         la semántica "SOLO acepto rvalues" / "distingo categoría"
         ES EL PUNTO de la API. No es un reemplazo de const T& (la
         L02 lo dejó claro).

      El árbitro práctico: ¿la función GUARDA o CONSUME el objeto?
      Sí -> firmar por valor + mover ADENTRO (A). ¿Solo lo LEE o lo
      muta? -> B. ¿Necesito distinguir "temporal de solo-morir"? -> C.
      Con esas tres preguntas, el 90% de las firmas quedan resueltas.

    2. ¿QUÉ PASA POR DEBAJO?
      unique_ptr se declara move-only borrando la copia (módulo 05:
      "= delete"): no existe constructor de copia; solo constructor
      de movimiento. Trasladado a una clase propia es EL patrón
      mínimo para un recurso exclusivo: borrar copia + habilitar
      movimiento. A la hora de pasar argumentos, SE ELIGE por las
      categorías (L02): un rvalue activa el constructor de
      movimiento del parámetro por valor (o de la referencia &&);
      un lvalue la función no puede robarlo: el caller escribe
      std::move para convertirlo. Con las reglas de este módulo se
      traza la transferencia de propiedad COMPLETA, sin magia.

    3. ERRORES COMUNES
      - Recibir unique_ptr por const& "para verlo" y después querer
        guardarlo: no se puede (la propiedad exclusiva no se copia).
        Las dos vías correctas: por VALOR (cesión de propiedad) o
        por referencia (observador, sin propiedad). Elegí antes de
        firmar.
      - `return std::move(ptr);` devolviendo unique_ptr: redundante
        (la L09: el return de local ya es rvalue).
      - Firmar APIs con T&& "porque es moderno": los casos reales
        de T&& con deducción (perfect forwarding, referencias
        universales) son del módulo 09; acá, la versión SIMPLE de
        sobrecarga por categorías ya cubre transferencias.

    4. DEMO: una escena que posee balas, con las tres firmas
   ============================================================================
 */
#include <cstddef>
#include <iostream>
#include <memory>
#include <vector>

// Bala: move-only por diseño (la copia está borrada: es un recurso
// exclusivo, como unique_ptr).
class Bala {
public:
    explicit Bala(float x, float vx) : x_(x), vx_(vx) { ++vivas_; }
    Bala(const Bala&)            = delete;   // propiedad exclusiva: no se copia
    Bala& operator=(const Bala&) = delete;
    Bala(Bala&&) noexcept            = default;   // el movimiento sí: transferencia
    Bala& operator=(Bala&&) noexcept = default;
    ~Bala() { --vivas_; }

    void mover() { x_ += vx_; }
    float x() const { return x_; }
    static int vivas() { return vivas_; }

private:
    float x_;
    float vx_;
    inline static int vivas_ = 0;
};

// La escena ES DUEÑA del vector de balas (unique_ptr dueño, módulo 07).
class Escena {
public:
    // A) SINK: recibe la propiedad por valor y la mueve a su lugar final.
    void brote(std::unique_ptr<Bala> bala) {
        balas_.push_back(std::move(bala));   // el vector absorbe la propiedad
    }

    // B) BORROWED: la escena actualiza sin apropiarse de nada. (Sin moves.)
    void actualizar() {
        for (auto& bala : balas_) bala->mover();
    }

    // Consumo: sacar UNA bala del vector; el unique_ptr viaja por valor (A).
    std::unique_ptr<Bala> retirar(std::size_t i) {
        std::unique_ptr<Bala> out = std::move(balas_[i]); // roba del vector
        balas_.erase(balas_.begin() + static_cast<std::ptrdiff_t>(i));  // O(n): L10
        return out;                                       // L09: return simple
    }

    std::size_t cantBalas() const { return balas_.size(); }

    Bala& bala(std::size_t i) { return *balas_[i]; }   // acceso NO dueño

private:
    std::vector<std::unique_ptr<Bala>> balas_;
};

int main() {
    std::cout << "== A) sink: la escena recibe por valor y mueve ==\n";
    Escena escena;
    escena.brote(std::make_unique<Bala>(0.f, 1.f));    // rvalue: se mueve solo
    std::unique_ptr<Bala> m = std::make_unique<Bala>(5.f, -1.f);
    escena.brote(std::move(m));                        // cesión explícita DEL CALLER
    std::cout << "  balas en escena: " << escena.cantBalas()
              << " (m ya no tiene propiedad: se la entregó)\n";
    std::cout << "  balas vivas: " << Bala::vivas() << "\n";

    std::cout << "\n== B) borrowed: actualizamos sin tocar propiedad ==\n";
    escena.actualizar();
    std::cout << "  bala[0].x=" << escena.bala(0).x()
              << ", bala[1].x=" << escena.bala(1).x()
              << " (se mutó contenido: la propiedad no cambió de dueño)\n";

    std::cout << "\n== consumo: retirar() devuelve la propiedad ==\n";
    std::unique_ptr<Bala> retirada = escena.retirar(0);
    std::cout << "  escena quedó con " << escena.cantBalas()
              << " balas; la retirada sigue viva (x=" << retirada->x() << ")\n";
    std::cout << "  balas vivas: " << Bala::vivas() << "\n";

    std::cout << "\nmoral: la transferencia de propiedad se LEE en los call\n"
              << "sites (std::move) y las firmas la expresan (por valor = cesión)\n";
    return 0;
}
