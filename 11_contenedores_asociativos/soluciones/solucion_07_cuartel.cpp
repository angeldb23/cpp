/* =============================================================================
   MÓDULO 11 - SOLUCIÓN 07: el cuartel de órdenes (ejercicio 07)
   =============================================================================
   L09-L10 son las lecciones de respaldo: priority_queue con comparador
   struct (mayor severidad sale primero), queue FIFO con front y stack
   LIFO con top/pop: SIEMPRE chequear empty() antes del frente.
   ============================================================================= */
#include <iostream>
#include <queue>
#include <stack>
#include <string>
#include <vector>
#include "../../include/test_framework.hpp"

struct Tarea {
    int severidad;
};

// El comparador del adaptador: "menor" sale despuess (L10):
struct emergencia {
    bool operator()(const Tarea& a, const Tarea& b) const {
        return a.severidad < b.severidad;
    }
};

class Cuartel {
  public:
    void encola(const Tarea& t) { emergencias.push(t); }

    Tarea atender() {
        if (emergencias.empty()) return Tarea{0};
        const Tarea urgente = emergencias.top();   // L10: top ANTES de pop
        emergencias.pop();
        return urgente;
    }

    void reporta(const std::string& texto) { bitacora.push(texto); }

    std::string frente() {
        if (bitacora.empty()) return "";
        const std::string texto = bitacora.front();
        bitacora.pop();    // atendido: sale de la fila
        return texto;
    }

    void apila(const std::string& texto) { historial.push(texto); }

    std::string desapila() {
        if (historial.empty()) return "";
        const std::string texto = historial.top();
        historial.pop();
        return texto;
    }

  private:
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
    ASSERT_EQ(c.atender().severidad, 0);
}

TEST_CASE(BitacoraEnOrdenDeLlegada) {
    Cuartel c;
    ASSERT_EQ(c.frente(), "");
    c.reporta("ataque en el puente");
    c.reporta("orco en el bosque");
    ASSERT_EQ(c.frente(), "ataque en el puente");
}

TEST_CASE(DeshacerEsStack) {
    Cuartel c;
    ASSERT_EQ(c.desapila(), "");
    c.apila("mover");
    c.apila("disparar");
    ASSERT_EQ(c.desapila(), "disparar");
    ASSERT_EQ(c.desapila(), "mover");
    ASSERT_EQ(c.desapila(), "");
}

int main() {
    std::cout << "Ejecutando pruebas para Solucion 07 (cuartel de ordenes)...\n";
    return RUN_ALL_TESTS();
}
