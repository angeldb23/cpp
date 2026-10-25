/* =============================================================================
   MÓDULO 07 - SOLUCIÓN 10: La tarea que se registra a sí misma
   =============================================================================
   Práctica de la lección 13: la herencia de enable_shared_from_this hace
   que shared_from_this() devuelva un shared del MISMO bloque de control
   (nunca un segundo bloque sobre `this`).
   =============================================================================
 */
#include <cstddef>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include "../../include/test_framework.hpp"

class Planificador;

class Tarea : public std::enable_shared_from_this<Tarea> {
private:
    std::string nombre_;
    inline static int vivas_ = 0;

public:
    explicit Tarea(std::string n) : nombre_(std::move(n)) { ++vivas_; }
    ~Tarea() { --vivas_; }

    const std::string& nombre() const { return nombre_; }

    void registrarEn(Planificador& p);
    // NOTA: shared_from_this() es válido SOLO porque en los tests la tarea
    // nace con make_shared (ya hay dueños). En el constructor NO: ahí no
    // existe todavía ningún bloque que enlazar (L13).

    static int vivas() { return vivas_; }
};

// (DADO): el planificador OBSERVA las tareas: no es dueño de ninguna.
class Planificador {
private:
    std::vector<std::weak_ptr<Tarea>> pendientes_;

public:
    void agregar(std::weak_ptr<Tarea> t) {
        pendientes_.push_back(std::move(t));
    }

    std::size_t pendientes() const { return pendientes_.size(); }

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

void Tarea::registrarEn(Planificador& p) {
    // El MISMO bloque de control (el receptor recibirá un recurso válido):
    p.agregar(shared_from_this());
}

// ============= TESTS =============
TEST_CASE(SeRegistraSinDuplicarDuenos) {
    Planificador p;
    auto t = std::make_shared<Tarea>("cargar_mapa");
    ASSERT_EQ(t.use_count(), 1);
    t->registrarEn(p);
    ASSERT_EQ(t.use_count(), 1);          // el planificador guardó WEAK
    ASSERT_EQ(p.pendientes(), std::size_t(1));
}

TEST_CASE(ElPlanificadorNoLaSostiene) {
    Planificador p;
    {
        auto t = std::make_shared<Tarea>("efimera");
        t->registrarEn(p);
        ASSERT_EQ(t.use_count(), 1);
    }
    ASSERT_EQ(Tarea::vivas(), 0);
    p.reportarPendientes();
}

TEST_CASE(EsElMismoRecursoNoUnoReciclado) {
    auto t = std::make_shared<Tarea>("formal");
    Planificador pl;
    t->registrarEn(pl);
    // la dirección de 'formal' es la misma en el planificador: es el
    // MISMO objeto con el MISMO bloque de control (no un bloque nuevo):
    ASSERT_EQ(pl.primera().lock().get(), t.get());
}

int main() {
    std::cout << "Ejecutando SOLUCIÓN 10 (planificador)...\n";
    return RUN_ALL_TESTS();
}
