#ifndef ENTIDAD_HPP
#define ENTIDAD_HPP

#include <iostream>
#include <string>

// Clase Base Abstracta (Polimórfica)
class Entidad {
protected:
    std::string nombre;
    int posX;
    int posY;
    int vida;

public:
    Entidad(std::string nombre, int x, int y, int vida)
        : nombre(nombre), posX(x), posY(y), vida(vida) {}

    virtual ~Entidad() = default;

    virtual void actualizar() = 0;
    virtual void render() const = 0;

    bool estaViva() const { return vida > 0; }
    std::string getNombre() const { return nombre; }
    int getX() const { return posX; }
    int getY() const { return posY; }
};

// Clase Derivada: Jugador
class Jugador : public Entidad {
public:
    Jugador(int x, int y) : Entidad("Heroe", x, y, 100) {}

    void mover(char direccion) {
        if (direccion == 'w' || direccion == 'W') posY--;
        if (direccion == 's' || direccion == 'S') posY++;
        if (direccion == 'a' || direccion == 'A') posX--;
        if (direccion == 'd' || direccion == 'D') posX++;
    }

    void actualizar() override {
        // Lógica adicional del jugador si hiciera falta
    }

    void render() const override {
        std::cout << " [@] " << nombre << " en (" << posX << ", " << posY << ") - HP: " << vida << std::endl;
    }
};

// Clase Derivada: Monstruo
class Monstruo : public Entidad {
public:
    Monstruo(std::string nombre, int x, int y) : Entidad(nombre, x, y, 40) {}

    void actualizar() override {
        // Movimiento simple oscilatorio
        posX += 1;
    }

    void render() const override {
        std::cout << " [X] " << nombre << " en (" << posX << ", " << posY << ") - HP: " << vida << std::endl;
    }
};

#endif // ENTIDAD_HPP
