/* =============================================================================
   MÓDULO 07 - SOLUCIÓN 04: El motor y sus APIs con unique_ptr
   =============================================================================
   Práctica de las lecciones 04 y 05: la propiedad del MIEMBRO es única y
   los contratos de función separan "leer", "reemplazar" y "ver sin dueño".
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

class Motor {
private:
    std::unique_ptr<Textura> fondo_;   // miembro dueño: regla de 0 gratis

public:
    explicit Motor(std::string nombreInicial)
        : fondo_(std::make_unique<Textura>(std::move(nombreInicial))) {}

    void cambiarFondo(const std::string& nuevo) {
        // El operador= del unique_ptr: libera el viejo y adopta el nuevo
        // EN ESTA LÍNEA (por eso el dtor de la vieja corre aquí adentro).
        fondo_ = std::make_unique<Textura>(nuevo);
    }

    const Textura* fondo() const { return fondo_.get(); }
};

// const&: LEE. No puede reemplazar ni liberar (firma lo impide).
void aplicarFiltro(const std::unique_ptr<Textura>& tex) {
    tex->marcarFiltrada();
}

// & modificable: puede REEMPLAZAR el recurso; el DUEÑO sigue siendo quien
// creó el unique_ptr (acá: main).
void asegurarFondo(std::unique_ptr<Textura>& tex, const std::string& nombre) {
    if (!tex) {
        tex = std::make_unique<Textura>(nombre);
    }
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
    ASSERT_EQ(tex->nombre(), antes);   // seguimos con la misma textura
}

TEST_CASE(ContratoModificable) {
    std::unique_ptr<Textura> tex;
    const int creadasAntes = Textura::creadas();
    asegurarFondo(tex, "placeholder.png");
    ASSERT_TRUE(tex != nullptr);
    ASSERT_EQ(tex->nombre(), std::string("placeholder.png"));
    const int creadasAhora = Textura::creadas();
    ASSERT_EQ(creadasAhora, creadasAntes + 1);  // la primera sí recargó
    asegurarFondo(tex, "otro.png");    // ya tiene dueño: no recargó
    ASSERT_EQ(Textura::creadas(), creadasAhora);
}

int main() {
    std::cout << "Ejecutando SOLUCIÓN 04 (APIs unique_ptr)...\n";
    return RUN_ALL_TESTS();
}
