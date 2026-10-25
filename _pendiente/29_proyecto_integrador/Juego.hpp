#ifndef JUEGO_HPP
#define JUEGO_HPP

#include <vector>
#include <memory>
#include "Entidad.hpp"

class Juego {
private:
    bool ejecutando;
    std::unique_ptr<Jugador> jugador;
    std::vector<std::unique_ptr<Entidad>> entidades;

public:
    Juego();
    ~Juego();

    void iniciar();
    void procesarEventos();
    void actualizar();
    void renderizar();

    bool estaEjecutando() const { return ejecutando; }
};

#endif // JUEGO_HPP
