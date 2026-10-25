/* =============================================================================
   MÓDULO 09 - LECCIÓN 07: Forwarding aplicado: fábricas y delegación
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
      La lección 06 mostró la regla de la deducción de T en T&& y una
      redirección simple. Esta lección es el uso real del forwarding en
      código de todos los días: las TRES piezas donde casi siempre aparece.

      PIEZA 1: WRAPPER QUE GUARDA (envoltorio con remove_reference_t).
      La clase resultante quiere guardar UNA COPIA del tipo, no una
      referencia: en su miembro usás std::remove_reference_t<T> (le dale
      sacado el ref) y la inicializás con std::forward<T>(valor): si el
      argumento era rvalue queda una copia; si era rvalue queda un move.
      Ejercicio clásico de "elegir copiar o mover según cómo llegó".

      PIEZA 2: FÁBRICAS (estilo std::make_unique).
      Una función variádica que construye un T nunca conoce los
      constructores que se van a usar. Con Args&&... (pack de referencias
      universales) y std::forward<Args>(args)... cada argumento llega a
      T(args...) MANTENIENDO la categoría.

      PIEZA 3: DELEGACIÓN A CONSTRUCTORES.
      Un miembro de la clase se inicializa con los datos que recibís:
      forwarding directo a la lista de inicialización del miembro (o a la
      clase base), como hace nuestro Registrador/Sensor de abajo.

      En las tres, la regla es la misma: deducí T (o cada Args), y
      std::forward en la ÚNICA usada que mueve.

   2. ¿QUÉ PASA POR DEBAJO?
      Contamos con un tipo Rastreada que incrementa contadores estáticos:
      copias y movimientos. Con él se ve el beneficio medible: fabricar
      con un temporario gasta 1 movimiento y 0 copias; si el argumento es
      lvalue, gasta 1 copia (correcto: el dueño podría seguir usándolo).
      El pack Args&&... y std::forward en una llamada variádica es
      exactamente como coser todo std::move en un solo lugar.

   3. ERRORES COMUNES Y BUENAS PRÁCTICAS:
      - Forwardás dos veces el MISMO argumento: la segunda ya está
        movida. Forward al instante de uso, no antes.
      - En un cuerpo, `args` es siempre un lvalue aunque llegara rvalue:
        sin forward, todo se copia.
      - No reenvíes hacia algo que a su vez reenvía sin el <T> explícito
        en cada tramo.
      - Si el tipo es copy-only (sin move), el movimiento "decae" a copia:
        no rompe, solo se pierde la optimización.

   4. PARA PROFUNDIZAR:
      https://en.cppreference.com/w/cpp/utility/move_if_noexcept
      https://en.cppreference.com/w/cpp/utility/pair/make_pair
      https://en.cppreference.com/w/cpp/memory/unique_ptr/make_unique
   ============================================================================= */
#include <iostream>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

// --- 1. Un tipo que rastrea copias y movimientos ---
class Rastreada {
public:
    static inline int copias = 0;
    static inline int movimientos = 0;

    std::string texto;
    explicit Rastreada(std::string t) : texto(std::move(t)) {}
    Rastreada(const Rastreada& o) : texto(o.texto) { ++copias; }
    Rastreada(Rastreada&& o) noexcept : texto(std::move(o.texto)) { ++movimientos; }
    Rastreada& operator=(const Rastreada& o) { texto = o.texto; ++copias; return *this; }
    Rastreada& operator=(Rastreada&& o) noexcept { texto = std::move(o.texto); ++movimientos; return *this; }
};

static void reiniciarContadores() {
    Rastreada::copias = 0;
    Rastreada::movimientos = 0;
}

// --- 2. PIEZA 1: el wrapper que guarda ---
// El miembro es remove_reference_t<T>: el dato SIN la referencia, para que no
// exista la referencia (solo el objeto). Y se inicializa con forward: si el
// argumento era lvalue se copia; si era rvalue, se mueve.
template <typename T> class Marco {
public:
    std::remove_reference_t<T> contenido;

    // El constructor VUELVE a deducir: U&& es otra referencia universal.
    // U deduce igual que T arriba: lvalue -> U referencia, rvalue -> U = T.
    template <typename U>
    explicit Marco(U&& valor)
        : contenido(std::forward<U>(valor)) {}      // lvalue -> copia, rvalue -> move
};

template <typename T>
Marco<std::remove_reference_t<T>> envolver(T&& valor) {
    return Marco<std::remove_reference_t<T>>(std::forward<T>(valor));
}

// --- 3. PIEZA 2: la fábrica variádica (estilo make_unique) ---
class Sensor {
public:
    std::string nombre;
    std::string unidad;

    Sensor(std::string n, std::string u) : nombre(std::move(n)), unidad(std::move(u)) {}
};

template <typename T, typename... Args>
std::unique_ptr<T> fabricar(Args&&... args) {
    // igual que std::make_unique: TODOS los args con su categoría original.
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

// --- 4. PIEZA 3: delegación a un miembro en el constructor ---
class Registrador {
public:
    std::string origen;
    Sensor sensor;

    // Los datos no pasan por copia al miembro: van derecho con forward.
    Registrador(std::string o, std::string n, std::string u)
        : origen(std::move(o)), sensor(std::move(n), std::move(u)) {}
};

int main() {
    std::cout << "=== 1. WRAPPER QUE GUARDA: COPIA O MOVER SEGUN LLEGADA ===\n";
    Rastreada original("lago");
    reiniciarContadores();
    auto conLvalue = envolver(original);         // lvalue -> copia
    std::cout << "envolver(lvalue):  copias=" << Rastreada::copias
              << " movimientos=" << Rastreada::movimientos << "\n";

    reiniciarContadores();
    auto conRvalue = envolver(Rastreada("pico"));  // rvalue -> move
    std::cout << "envolver(rvalue):  copias=" << Rastreada::copias
              << " movimientos=" << Rastreada::movimientos << "\n";
    std::cout << "  contenidos: " << conLvalue.contenido.texto << " / "
              << conRvalue.contenido.texto << "\n";

    std::cout << "\n=== 2. FABRICA VARIADICA: forwarding de cada arg ===\n";
    auto directo = fabricar<Sensor>(std::string("sensor_env"), "C");
    std::cout << "  sensor fabricado: " << directo->nombre << " [" << directo->unidad << "]\n";
    Rastreada fuente("env");
    reiniciarContadores();
    auto copiada = fabricar<Rastreada>(fuente);                    // lvalue -> copia
    std::cout << "  fabricar con lvalue:  copias=" << Rastreada::copias
              << " movimientos=" << Rastreada::movimientos << "\n";
    reiniciarContadores();
    auto movida = fabricar<Rastreada>(Rastreada("env"));           // rvalue temporal -> move
    std::cout << "  fabricar con rvalue:  copias=" << Rastreada::copias
              << " movimientos=" << Rastreada::movimientos << "\n";
    std::cout << "  contenidos: " << copiada->texto << " | " << movida->texto << "\n";

    std::cout << "\n=== 3. DELEGACION A CONSTRUCTOR DEL MIEMBRO ===\n";
    Registrador registro("estacion_norte", "termo_1", "kPa");
    std::cout << "  " << registro.origen << ": " << registro.sensor.nombre
              << " [" << registro.sensor.unidad << "]\n";
    std::cout << "  (los strings viajaron por move dentro de la lista de init)\n";

    std::cout << "\n=== 4. RESUMEN DE LA REGLA ===\n";
    std::cout << "  deducí cada T (Args&&...) -> un forward por argumento, en el\n"
              << "  LUGAR de uso -> nada se mueve si el dueño original lo sigue usando.\n";
    return 0;
}
