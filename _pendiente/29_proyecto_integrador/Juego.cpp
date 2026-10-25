#include "Juego.hpp"
#include <iostream>

using namespace std;

Juego::Juego() : ejecutando(true) {
    cout << "=== INICIALIZANDO MOTOR DE JUEGO ===" << endl;
    jugador = make_unique<Jugador>(5, 5);

    // Agregamos algunos monstruos al mapa
    entidades.push_back(make_unique<Monstruo>("Slime", 2, 3));
    entidades.push_back(make_unique<Monstruo>("Esqueleto", 8, 4));
}

Juego::~Juego() {
    cout << "=== CERRANDO MOTOR Y LIBERANDO RECURSOS LIMPIAMENTE ===" << endl;
}

void Juego::iniciar() {
    cout << "Bienvenido. Usa W/A/S/D para moverte, o Q para salir." << endl;
    while (ejecutando) {
        renderizar();
        procesarEventos();
        actualizar();
    }
}

void Juego::procesarEventos() {
    cout << "\nIngresa comando (W=Arriba, S=Abajo, A=Izq, D=Der, Q=Salir): ";
    char input;
    if (!(cin >> input)) {
        ejecutando = false;
        return;
    }

    if (input == 'q' || input == 'Q') {
        ejecutando = false;
    } else {
        jugador->mover(input);
    }
}

void Juego::actualizar() {
    // Actualizamos al jugador y a todas las entidades polimórficas
    jugador->actualizar();
    for (auto& entidad : entidades) {
        entidad->actualizar();
    }
}

void Juego::renderizar() {
    cout << "\n---------------- MAPA DE JUEGO ----------------" << endl;
    jugador->render();
    for (const auto& entidad : entidades) {
        entidad->render();
    }
    cout << "-----------------------------------------------" << endl;
}
