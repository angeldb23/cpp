/* =============================================================================
   MÓDULO 8 - LECCIÓN 02: Contenedores Asociativos (std::unordered_map y std::set)
   =============================================================================
   En videojuegos, casi siempre necesitás buscar recursos por su nombre (clave):
   - Texturas: mapa["player"] -> TextureID
   - Sonidos:  mapa["jump.wav"] -> SoundBuffer
   - Configuración: mapa["volumen"] -> 80

   1. 'std::unordered_map<Clave, Valor>':
      - Tabla Hash (Hash Table): Acceso casi instantáneo O(1).
      - No tiene orden garantizado.
      - Métodos clave: mapa[clave], find(), count().

   2. 'std::set<Tipo>':
      - Colección donde los elementos NO se repiten y se mantienen ordenados.
   ============================================================================= */

#include <iostream>
#include <unordered_map>
#include <set>
#include <string>

using namespace std;

int main() {
    cout << "=== 1. DICCIONARIO / TABLA HASH (std::unordered_map) ===" << endl;
    // Mapa de configuraciones del juego:
    unordered_map<string, int> config;

    // Asignación directa con corchetes []:
    config["ancho_pantalla"] = 1920;
    config["alto_pantalla"] = 1080;
    config["fps_maximos"] = 144;
    config["volumen_musica"] = 80;

    cout << "Resolucion: " << config["ancho_pantalla"] << "x" << config["alto_pantalla"] << endl;
    cout << "FPS Max: " << config["fps_maximos"] << endl;

    // Verificar si una clave existe sin crearla por error:
    string opcionBuscada = "antialiasing";
    if (config.find(opcionBuscada) != config.end()) {
        cout << "Opcion encontrada: " << config[opcionBuscada] << endl;
    } else {
        cout << "La opcion '" << opcionBuscada << "' no esta configurada." << endl;
    }
    cout << endl;

    cout << "=== 2. CONJUNTOS SIN DUPLICADOS (std::set) ===" << endl;
    // Útil para logros desbloqueados por el usuario:
    set<string> logrosDesbloqueados;

    logrosDesbloqueados.insert("Primer_Paso");
    logrosDesbloqueados.insert("Derrotar_Dragon");
    logrosDesbloqueados.insert("Primer_Paso"); // Ignorado automáticamente, no se duplica

    cout << "Total de logros unicos: " << logrosDesbloqueados.size() << endl;
    for (const auto& logro : logrosDesbloqueados) {
        cout << " ★ Logro: " << logro << endl;
    }

    return 0;
}
