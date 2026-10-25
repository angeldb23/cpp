/* =============================================================================
   MÓDULO 05 - EJERCICIO 12: static, constantes de clase y agregados
   =============================================================================
   DIFICULTAD: Intermedio
   PRACTICA: lección 18 (métodos static, static constexpr, contador
   compartido por la clase, agregado con designated initializers).

   CONSIGNA: clase `Sala`:
     1. static constexpr int CAPACIDAD_MAX = 4 (constante de clase).
     2. static int s_construidas: contador de salas (el contador NO
        baja al destruir; la definición va fuera de la clase).
     3. Constructor (nombre): guarda el nombre, ++s_construidas.
     4. static int construidas().
     5. `static bool cabe(int personas)`: 1 <= personas <= CAPACIDAD_MAX.
     6. Método `void entra(int personas)`: suma al contador de la sala
        validando con cabe() (si no cabe, no cambia nada); lee el
        estado con `int enSala() const`.
   Además, completá el struct `SalaConfig` como AGREGADO: sus miembros
   (aforoTeorico con default 8, conHumidificador false, volumen 50)
   se inicializan por la vía agregada, y el test prueba designated
   initializers nombrando solo algunos campos.

   ============================================================================= */
#include <iostream>
#include "../../include/test_framework.hpp"

class Sala {
public:
    // Constante de clase (lección 18): cambiale el valor para resolver.
    static constexpr int CAPACIDAD_MAX = -1;  // placeholder: debe ser 4

    explicit Sala(const char* nombre) {
        (void)nombre;
        return;  // placeholder
    }

    // TODO 4-5: statics.
    static int construidas() { return 0; }
    static bool cabe(int personas) {
        (void)personas;
        return false;  // placeholder
    }

    // TODO 3 y 6: construcción y entrada validada.
    void entra(int personas) { (void)personas; }
    int enSala() const { return -1; }

private:
    // TODO: const char* m_nombre, static int s_construidas (decl).
};

// int Sala::s_construidas = 0;   // definición única (TODO descomentar)

// Agregado (sin constructores: aggregate init).
struct SalaConfig {
    int aforoTeorico = 8;
    bool conHumidificador = false;
    int volumen = 50;
};

// ============= TESTS =============
TEST_CASE(TestConstanteYContador) {
    ASSERT_EQ(Sala::CAPACIDAD_MAX, 4);
    int antes = Sala::construidas();
    {
        Sala a("a");
        Sala b("b");
        ASSERT_EQ(Sala::construidas(), antes + 2);
    }
    // las salas murieron; el contador static del TIPO no baja:
    ASSERT_EQ(Sala::construidas(), antes + 2);
}

TEST_CASE(TestCabe) {
    ASSERT_TRUE(Sala::cabe(1));
    ASSERT_TRUE(Sala::cabe(Sala::CAPACIDAD_MAX));
    ASSERT_FALSE(Sala::cabe(0));
    ASSERT_FALSE(Sala::cabe(Sala::CAPACIDAD_MAX + 1));
    ASSERT_FALSE(Sala::cabe(-3));
}

TEST_CASE(TestEntraValidada) {
    Sala laboratorio("lab");
    ASSERT_EQ(laboratorio.enSala(), 0);
    laboratorio.entra(3);
    ASSERT_EQ(laboratorio.enSala(), 3);
    laboratorio.entra(2);              // 3+2 = 5 > 4: rechazado
    ASSERT_EQ(laboratorio.enSala(), 3);
    laboratorio.entra(1);              // ahora sí: 4
    ASSERT_EQ(laboratorio.enSala(), 4);
}

TEST_CASE(TestAgregadoDesignated) {
    // El agregado NO tiene constructor: así le pasa valores nombrados.
    SalaConfig config{ .aforoTeorico = 4, .volumen = 90 };  // el resto default
    ASSERT_EQ(config.aforoTeorico, 4);
    ASSERT_FALSE(config.conHumidificador);
    ASSERT_EQ(config.volumen, 90);

    SalaConfig base;                   // todo con sus defaults
    ASSERT_EQ(base.aforoTeorico, 8);   // el default del declarar la consigna
    ASSERT_EQ(base.volumen, 50);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 12...\n";
    return RUN_ALL_TESTS();
}
