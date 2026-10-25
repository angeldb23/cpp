/* =============================================================================
   MÓDULO 07 - EJERCICIO 10: La tarea que se registra a sí misma
   =============================================================================
   DIFICULTAD: Medio-alto
   PRACTICA: lección 13 (enable_shared_from_this: el MISMO bloque de
   control; nunca shared_ptr(this)) y el patrón de callback asíncrono.

   CONSIGNA:
   Una `Tarea` que se agrega a sí misma a un `Planificador`. La clase
   tiene que HEREDAR de std::enable_shared_from_this<Tarea> (herencia
   public) para poder entregarse con el mismo bloque de control.

      1. Heredar de enable_shared_from_this<Tarea> (mirá la flechita del
         `class Tarea` de acá arriba).
      2. `Tarea::registrarEn(Planificador&)`: entregarse con
         shared_from_this(). El planificador (DADO) guarda weak_ptr: es
         OBSERVADOR de la tarea (L08-L09), no dueño.
      3. LA REGLA que los tests fuerzan: nada de
         `std::shared_ptr<Tarea>(this)` adentro (dos bloques de control,
         doble delete). El planificador tiene que recibir EL MISMO
         recurso que sostiene el llamador (comparación de punteros).

   El planificador guarda weak: al no sostener, la tarea muere con su
   dueño (main): es el segundo test.

   COMPILACIÓN:
     g++ -std=c++20 -Wall -Wextra -I../../include \
         ejercicio_10_tarea_y_planificador.cpp -o ej10 && ./ej10
   =============================================================================
 */
#include <cstddef>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include "../../include/test_framework.hpp"

class Planificador;   // forward (dada)

// ============================================================================
// La Tarea: TODO 1 y 2.
// ============================================================================
class Tarea /* TODO: heredar de std::enable_shared_from_this<Tarea> */ {
private:
    std::string nombre_;
    inline static int vivas_ = 0;

public:
    explicit Tarea(std::string n) : nombre_(std::move(n)) { ++vivas_; }
    ~Tarea() { --vivas_; }

    const std::string& nombre() const { return nombre_; }

    // TODO 2: entregarse al planificador con shared_from_this().
    void registrarEn(Planificador& p) {
        (void)p;
    }
    // (NUNCA: std::shared_ptr<Tarea> eos(this): dos bloques de control)

    static int vivas() { return vivas_; }
};

// ============================================================================
// (DADO): el planificador OBSERVA las tareas: no es dueño de ninguna.
// ============================================================================
class Planificador {
private:
    std::vector<std::weak_ptr<Tarea>> pendientes_;

public:
    void agregar(std::weak_ptr<Tarea> t) {
        pendientes_.push_back(std::move(t));
    }

    std::size_t pendientes() const { return pendientes_.size(); }

    // Para los tests: la primera tarea pendiente, como weak (nullable).
    std::weak_ptr<Tarea> primera() const {
        return pendientes_.empty() ? std::weak_ptr<Tarea>{} : pendientes_[0];
    }

    void reportarPendientes() const {
        for (const auto& w : pendientes_) {
            if (auto viva = w.lock()) {
                std::cout << "    pendiente: " << viva->nombre() << "\n";
            } else {
                std::cout << "    (pendiente ya no existe)\n";
            }
        }
    }
};

// ============= TESTS =============
TEST_CASE(SeRegistraSinDuplicarDuenos) {
    Planificador p;
    auto t = std::make_shared<Tarea>("cargar_mapa");
    ASSERT_EQ(t.use_count(), 1);
    t->registrarEn(p);
    // shared_from_this() usa el MISMO bloque; el planificador quedó con
    // un WEAK: el conteo de dueños sigue siendo 1 (solo main):
    ASSERT_EQ(t.use_count(), 1);
    ASSERT_EQ(p.pendientes(), std::size_t(1));
}

TEST_CASE(ElPlanificadorNoLaSostiene) {
    Planificador p;
    {
        auto t = std::make_shared<Tarea>("efimera");
        t->registrarEn(p);
        ASSERT_EQ(t.use_count(), 1);
    }   // la tarea muere: el planificador NO la frena (era weak)
    ASSERT_EQ(Tarea::vivas(), 0);
    p.reportarPendientes();   // visual: "(pendiente ya no existe)"
}

TEST_CASE(EsElMismoRecursoNoUnoReciclado) {
    auto t = std::make_shared<Tarea>("formal");
    Planificador pl;
    t->registrarEn(pl);
    // el planificador mira EL MISMO objeto (misma dirección de memoria):
    ASSERT_EQ(pl.primera().lock().get(), t.get());
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 10 (planificador)...\n";
    return RUN_ALL_TESTS();
}
