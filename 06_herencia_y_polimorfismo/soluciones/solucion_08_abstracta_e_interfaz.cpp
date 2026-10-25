/* =============================================================================
   MÓDULO 06 - SOLUCIÓN 08: Figuras de UI: abstracta + interfaz pura
   =============================================================================
   Claves de las lecciones 09 y 10:
   - Figura es la base de dominio (tiene datos) y Clicable la interfaz
     de capacidad (sin datos, sin implementación).
   - Una clase con dos roles: DERIVA de ambas bases.
   - Implementar los puros: si falta uno, la derivada sigue abstracta.
   ============================================================================= */
#include <iostream>
#include <string>
#include "../../include/test_framework.hpp"

class Figura {
protected:
    std::string nombre;
    int x = 0, y = 0;

public:
    Figura(std::string n, int x_, int y_) : nombre(std::move(n)), x(x_), y(y_) {}
    virtual ~Figura() = default;

    virtual std::string dimeNombre() const = 0;
    virtual int         dimeArea()   const = 0;
    virtual void        mover(int dx, int dy) = 0;
protected:
    std::string nom() const { return nombre; }
};

class Clicable {
public:
    virtual ~Clicable() = default;
    virtual bool alClic() = 0;
};

class Boton : public Figura, public Clicable {
    bool activable = false;

public:
    // La base PRIMERO en la lista (orden de la lección 02):
    Boton(const std::string& n, int x_, int y_, bool act_)
        : Figura(n, x_, y_), activable(act_) {}

    std::string dimeNombre() const override { return nom(); }
    int dimeArea() const override { return 100; }   // botón fijo
    void mover(int dx, int dy) override {           // el estado x/y del padre
        x += dx;
        y += dy;
    }
    bool alClic() override { return activable; }    // promesa de la interfaz
};

class Slider : public Figura, public Clicable {
public:
    Slider(const std::string& n, int x_, int y_) : Figura(n, x_, y_) {}

    std::string dimeNombre() const override { return nom(); }
    int dimeArea() const override { return 400; }
    void mover(int dx, int dy) override { x += dx; y += dy; }
    bool alClic() override { return false; }
};

TEST_CASE(TestBotonFigura) {
    Boton b("aceptar", 10, 20, true);
    ASSERT_EQ(b.dimeNombre(), std::string("aceptar"));
    ASSERT_EQ(b.dimeArea(), 100);
}

TEST_CASE(TestBotonClic) {
    Boton activo("aceptar", 10, 20, true);
    Boton dormido("cancelar", 0, 0, false);
    ASSERT_TRUE(activo.alClic());
    ASSERT_FALSE(dormido.alClic());
}

TEST_CASE(TestPolimorfismoDobleInterfaz) {
    Boton b("menu", 1, 1, true);
    Figura* f = &b;
    Clicable* c = &b;
    ASSERT_EQ(f->dimeNombre(), std::string("menu"));
    ASSERT_TRUE(c->alClic());

    Slider s("brillo", 0, 0);
    Clicable* sc = &s;
    ASSERT_FALSE(sc->alClic());
    ASSERT_EQ(s.dimeArea(), 400);
}

int main() {
    std::cout << "Solucion Ejercicio 08: abstracta + interfaz pura.\n";
    return RUN_ALL_TESTS();
}
