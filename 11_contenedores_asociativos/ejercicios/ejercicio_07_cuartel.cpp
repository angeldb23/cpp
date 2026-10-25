/* =============================================================================
   MÓDULO 11 - EJERCICIO 07: el cuartel de órdenes (lecciones 09 y 10)
   =============================================================================
   DIFICULTAD: Medio
   CONSIGNA (tema 5 de TEMAS.md: stack / queue / priority_queue):
     Un cuartel maneja órdenes en TRES canales, cada uno con SU
     contenedor correcto:

       1. EMERGENCIAS (sale por severidad, no por llegada):
          struct Tarea { int severidad; };   // mayor = más urgente
          El comparador del adaptador (L10, struct con operator()) lo
          tenés que completar vos. Métodos:
            void encola(const Tarea& t);
            Tarea atender();    // la de MAYOR severidad (y la saca).
       2. BITÁCORA (orden de llegada, FIFO): queue (L09). Métodos:
            void reporta(const std::string& texto);
            std::string ultimo_reporte();    // el PRIMERO en la fila,
                                             // o "" si no hay.
       3. DESHACER (LIFO): stack (L09). Métodos:
            void apila(const std::string& texto);
            std::string deshacer();          // el ÚLTIMO texto, o "".

      La clase Cuartel abajo tiene los tres fondos elegidos: LOS
      métodos son los TODO (y el comparador del priority_queue).
      (emptiness: usar empty() ANTES de top()/front(): L09).
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_07_cuartel.cpp -o ej07 && ./ej07
   ============================================================================= */
#include <iostream>
#include <queue>
#include <stack>
#include <string>
#include <vector>
#include "../../include/test_framework.hpp"

struct Tarea {
    int severidad;    // mayor = más urgente
};

struct emergencia {
    // TODO: comparador del priority_queue (L10): "menor sale después":
    bool operator()([[maybe_unused]] const Tarea& a,
                    [[maybe_unused]] const Tarea& b) const {
        return false;    // placeholder
    }
};

class Cuartel {
  public:
    // TODO 1a: meter una tarea en la cola de emergencias:
    void encola([[maybe_unused]] const Tarea& t) { }

    // TODO 1b: ATENDER: quitar y devolver la de mayor severidad.
    //          En cola vacía devolver Tarea{0}.
    Tarea atender() {
        return Tarea{0};    // placeholder
    }

    // TODO 2a: reportar un texto (orden de llegada):
    void reporta([[maybe_unused]] const std::string& texto) { }

    // TODO 2b: el PRIMER reporte pendiente, ATENDIDO (lo saca) o ""
    //          si no hay. (front() antes de pop(): L09.)
    std::string frente() {
        return "";    // placeholder
    }

    // TODO 3a: apilar un texto (deshacer):
    void apila([[maybe_unused]] const std::string& texto) { }

    // TODO 3b: desapilar el ÚLTIMO texto (o "" si no hay).
    std::string desapila() {
        return "";    // placeholder
    }

  private:
    // Los TRES fondos elegidos (tema 5: el/el/el):
    std::priority_queue<Tarea, std::vector<Tarea>, emergencia> emergencias;
    std::queue<std::string>                                    bitacora;
    std::stack<std::string>                                    historial;
};

// ============= TESTS =============
TEST_CASE(AtiendePorSeveridad) {
    Cuartel c;
    c.encola(Tarea{3});
    c.encola(Tarea{9});
    c.encola(Tarea{7});
    ASSERT_EQ(c.atender().severidad, 9);
    ASSERT_EQ(c.atender().severidad, 7);
    ASSERT_EQ(c.atender().severidad, 3);
    ASSERT_EQ(c.atender().severidad, 0);   // cola vacía: Tarea{0}
}

TEST_CASE(BitacoraEnOrdenDeLlegada) {
    Cuartel c;
    ASSERT_EQ(c.frente(), "");             // vacía
    c.reporta("ataque en el puente");
    c.reporta("orco en el bosque");
    ASSERT_EQ(c.frente(), "ataque en el puente");    // la PRIMERA
}

TEST_CASE(DeshacerEsStack) {
    Cuartel c;
    ASSERT_EQ(c.desapila(), "");           // vacía
    c.apila("mover");
    c.apila("disparar");
    ASSERT_EQ(c.desapila(), "disparar");   // la ÚLTIMA (top)
    ASSERT_EQ(c.desapila(), "mover");
    ASSERT_EQ(c.desapila(), "");
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 07 (cuartel de ordenes)...\n";
    return RUN_ALL_TESTS();
}
