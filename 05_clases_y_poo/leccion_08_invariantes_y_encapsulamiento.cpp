/* =============================================================================
   MÓDULO 05 - LECCIÓN 08: Encapsulamiento: invariantes, getters y setters
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      Un invariante es una condición que debe cumplirse SIEMPRE para que
      el objeto tenga sentido: "la vida está entre 0 y 100", "la fecha es
      una fecha real", "el inventario no tiene ítems de cantidad negativa".

      El encapsulamiento es el pacto: el estado es private, y los métodos
      públicos VALIDAN antes de cambiarlo. Nadie puede dejar al objeto
      inválido por accidente.

      Getters y setters con criterio:
      - Getter: devuelve el valor (por valor si es chico, const& si es
        pesado como std::string).
      - Setter: VALIDA; si el valor es razonable, cambia; si no, lo
        rechaza (o clampea). Un setter que solo hace `x = v;` es un
        miembro público con pasos extra.
      - No todos los atributos merecen getter/setter: si el atributo es
        un detalle de implementación, no expongas ni siquiera la lectura
        (el futuro te lo agradecerá: podrás cambiar la implementación
        sin romper a nadie).

      Anatomía de una clase bien encapsulada:
          public:      constructor(es)_validos, operaciones de negocio,
                       getters mínimos
          private:     el estado
          (static:     constantes de clase y de utilidad — lección 18)

   2. ¿QUÉ PASA POR DEBAJO?
      El getter `int vida() const` se compila a una lectura de memoria +
      una llamada (que inlinea a nada). El SETTER sí tiene contenido real
      cuando hay invariantes: la validación es código que el compilador
      no inventa, es tu lógica.

      La palabra clave aquí es la CUENTA de las rutas de escape: si un
      atributo es public, existen infinitas formas de invalidarlo. Si es
      private, las rutas son los métodos public que escribiste vos:
      auditables en un solo lugar.

   3. ERRORES COMUNES
      - Getters/setters automáticos para cada atributo "por si acaso":
        eso es encerrar el objeto para dejarlo afuera. Preguntá qué
        operaciones de negocio tiene el tipo, no qué atributos tiene.
      - Setter que no valida: encapsulamiento de mentira.
      - Getter que devuelve referencia NO const al estado interno: el
        que lo recibe puede escribir sin pasar por tus validaciones.
      - Exponer el tipo concreto de la implementación (devolver
        std::vector<int>&): te amarrás a un detalle interno.

   4. PARA PROFUNDIZAR
      https://en.cppreference.com/w/cpp/language/access
      C++ Core Guidelines: C.9, "Keep classes simple"
      https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#c9-use-standardized-idioms
   ============================================================================= */
#include <iostream>

// =========================================================================
// Clase con invariantes reales y validación centralizada.
// =========================================================================
class SaludVital {
public:
    // sin constructor por defecto: obligamos a datos válidos al nacer
    // (declaramos el parametrizado — lección 05) y sus setters validan.
    SaludVital(const char* nombre, int vida)
        : m_nombre(nombre), m_vida(0) {
        setVida(vida);          // validación ÚNICA, reutilizada por el ctor
    }

    int vida() const { return m_vida; }        // getter mínimo
    const char* nombre() const { return m_nombre; }

    void setVida(int nueva) {
        if (nueva < 0)  m_vida = 0;            // clamp: invariante 0..100
        else if (nueva > 100) m_vida = 100;
        else m_vida = nueva;
    }

    void recibirDanio(int danio) {
        if (danio > 0) setVida(m_vida - danio);
    }

    void curar(int puntos) {
        if (puntos > 0) setVida(m_vida + puntos);
    }

private:
    const char* m_nombre;
    int m_vida;    // INVARIANTE: 0 <= m_vida <= 100
};

// Otra clase: aquí el atributo es un DETALLE interno, no hay getter.
class GeneradorId {
public:
    int siguiente() { return ++m_contador; }
private:
    int m_contador = 0;    // nadie necesita LEER esto, solo avanzarlo
};

int main() {
    std::cout << "=== 1. INVARIANTE EN EL NACIMIENTO Y EN CADA CAMBIO ===\n";
    SaludVital heroe("Heroe", 90);
    heroe.setVida(150);      // setter: se clampa a 100
    heroe.recibirDanio(120); // combate: se clampa a 0
    heroe.curar(10);
    std::cout << "  " << heroe.nombre() << " vida: " << heroe.vida()
              << " (una única ruta de cambio, siempre validada)\n";

    std::cout << "\n=== 2. ATRIBUTO SIN GETTER: DETALLE INTERNO ===\n";
    GeneradorId generador;
    std::cout << "  ids: " << generador.siguiente() << ", "
              << generador.siguiente() << ", " << generador.siguiente() << "\n";
    std::cout << "  (m_contador no tiene getter: si mañana se cambia por"
                 " un uint64_t, nadie se entera)\n";
    return 0;
}
