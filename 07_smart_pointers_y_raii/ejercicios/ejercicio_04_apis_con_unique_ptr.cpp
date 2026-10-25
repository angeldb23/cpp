/* =============================================================================
   MÓDULO 07 - EJERCICIO 04: El motor y sus APIs con unique_ptr
   =============================================================================
   DIFICULTAD: Medio
   PRACTICA: lección 04 (miembro unique_ptr: el operador= libera y adopta) y
   lección 05 (los tres contratos: const& solo lectura, & modificable, crudo).

   CONSIGNA:
   El `Motor` administra exactamente una textura (miembro unique_ptr).
   Nadie escribe delete: el reemplazo pasa por make_unique y el operador=.

      1. `cambiarFondo(const std::string& nuevo)`: reemplaza la textura
         por una NUEVA (make_unique) con el nombre dado. Los tests cuentan
         texturas creadas y destruidas: el reemplazo libera la vieja YA.
      2. `fondo() const`: devuelve una VISTA sin dueño: `const Textura*`.
      3. Función libre `aplicarFiltro(const std::unique_ptr<Textura>&)`:
         contrato SOLO LECTURA (L05): marca la textura como filtrada y
         NADA más (no puede reemplazar el recurso: es const&).
      4. Función libre `asegurarFondo(std::unique_ptr<Textura>&`,
         `const std::string& nombre)`: contrato MODIFICABLE: si el puntero
         está vacío, lo llena con make_unique (placeholder.png típico).

   COMPILACIÓN:
     g++ -std=c++20 -Wall -Wextra -I../../include \
         ejercicio_04_apis_con_unique_ptr.cpp -o ej04 && ./ej04
   =============================================================================
 */
#include <cstddef>
#include <iostream>
#include <memory>
#include <string>
#include "../../include/test_framework.hpp"

class Textura {
private:
    std::string nombre_;
    bool filtrada_ = false;
    inline static int creadas_ = 0;
    inline static int destruidas_ = 0;

public:
    explicit Textura(std::string n) : nombre_(std::move(n)) { ++creadas_; }
    ~Textura() { ++destruidas_; }

    void marcarFiltrada() { filtrada_ = true; }
    bool filtrada() const { return filtrada_; }
    const std::string& nombre() const { return nombre_; }

    static int creadas() { return creadas_; }
    static int destruidas() { return destruidas_; }
};

// ----------------------------------------------------------------------------
class Motor {
private:
    std::unique_ptr<Textura> fondo_;

public:
    explicit Motor(std::string nombreInicial)
        : fondo_(std::make_unique<Textura>(std::move(nombreInicial))) {}

    // TODO 1: reemplazar la textura con make_unique (acá sucede el "=").
    void cambiarFondo(const std::string& nuevo) {
        (void)nuevo;
    }

    // TODO 2: vista sin dueño: const Textura* (nullptr si no hay).
    const Textura* fondo() const {
        (void)fondo_;
        return nullptr;                // PLACEHOLDER
    }
};

// TODO 3: SOLO lectura: marca la textura como filtrada (y nada más).
void aplicarFiltro(const std::unique_ptr<Textura>& tex) {
    (void)tex;
}

// TODO 4: MODIFICABLE: si tex está vacía, crea una con nombre dado.
void asegurarFondo(std::unique_ptr<Textura>& tex, const std::string& nombre) {
    (void)tex; (void)nombre;
}

// ============= TESTS =============
TEST_CASE(ElReemplazoLiberaLaVieja) {
    ASSERT_EQ(Textura::creadas(), 0);
    {
        Motor m("fondo.png");
        ASSERT_EQ(Textura::creadas(), 1);
        const int base = Textura::destruidas();
        m.cambiarFondo("noche.png");
        ASSERT_EQ(Textura::creadas(), 2);
        // la primera YA se destruyó (operador= de unique_ptr, L04):
        ASSERT_EQ(Textura::destruidas(), base + 1);
        ASSERT_EQ(m.fondo()->nombre(), std::string("noche.png"));
    }
    ASSERT_EQ(Textura::destruidas(), 2);
}

TEST_CASE(VistaSinDuenoLeer) {
    Motor m("lava.png");
    const Textura* vista = m.fondo();
    ASSERT_TRUE(vista != nullptr);
    ASSERT_EQ(vista->nombre(), std::string("lava.png"));
}

TEST_CASE(ContratoLectura) {
    auto tex = std::make_unique<Textura>("piedra.png");
    const std::string antes = tex->nombre();
    aplicarFiltro(tex);
    ASSERT_TRUE(tex->filtrada());
    ASSERT_EQ(tex->nombre(), antes);   // seguir siendo la misma textura
}

TEST_CASE(ContratoModificable) {
    std::unique_ptr<Textura> tex;
    const int creadasAntes = Textura::creadas();
    (void)creadasAntes;               // (se usa cuando TODOs resueltos)
    asegurarFondo(tex, "placeholder.png");
    ASSERT_TRUE(tex != nullptr);
    ASSERT_EQ(tex->nombre(), std::string("placeholder.png"));
    const int creadasAhora = Textura::creadas();
    asegurarFondo(tex, "otro.png");    // segunda llamada: NO recarga
    ASSERT_EQ(Textura::creadas(), creadasAhora);   // delta 0 desde antes
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 04 (APIs unique_ptr)...\n";
    return RUN_ALL_TESTS();
}
