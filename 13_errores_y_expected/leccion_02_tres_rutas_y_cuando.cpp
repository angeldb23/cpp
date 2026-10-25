/* =============================================================================
   MÓDULO 13 - LECCIÓN 02: las tres rutas y cuándo elegir cada una
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
       L01 viste los códigos de retorno. Acá armamos la tabla de decisión
       completa: el mismo error puede viajar por tres canales, y la
       elección cambia el diseño de todo lo que está alrededor.

       (A) CÓDIGO DE RETORNO (L01)
           Gana: barato, visible en la firma, no altera el flujo del
           programa: el que llama revisa y decide, sin saltos ocultos.
           Pierde: compite con el valor de retorno útil (el resultado
           tiene que salir por OUT-param u optional); los ifs se APILAN
           (tres pasos en secuencia = tres ifs seguidos, y el camino real
           queda enterrado en casuística); y nada OBLIGA a mirar el
           código: `parse(x, y);` compila igual y el error cae al piso.

       (B) EXCEPCIÓN (throw/catch del módulo 04; profundizamos en L05-L09)
           Gana: separa el CAMINO FELIZ del manejo de errores (el código
           cuenta el caso normal; los errores se toman "arriba", en un
           punto único); NO SE PUEDEN IGNORAR: si nadie atrapa el throw,
           el programa termina; viajan objetos con contenido (un
           runtime_error con mensaje, no solo un número); y atraviesan
           niveles: main() puede atrapar lo que lanzó una función cinco
           niveles más abajo, y los niveles del medio ni se enteran.
           Pierde: el lanzamiento cuesta (la propagación recorre frames);
           la propia firma no avisa que puede lanzar (el aprobado con
           noexcept lo declaras vos, L03); y el ejecutable paga un peso
           fijo de tablas aunque nunca se lance nada.

       (C) RESULTADO EXPLÍCITO: std::expected<T, E> (C++23, L10-L12) y su
           primo std::optional<T> (módulo 11) cuando "no está" no tiene
           razón que contar.
           Gana: el error VIAJA EN EL TIPO: la firma
           `expected<Superficie, ErrorCarga>` dice "o el valor o este
           error" y te obliga a mirar el caso de falla; combina el precio
           del código de retorno con la dificultad de ignorar de la
           excepción, y a diferencia del optional guarda la RAZÓN (el E).
           Pierde: el manejo turn-around vez vez en cada llamada (los
           and_then/transform lo mitigan: L12); es C++23 puro (+ manual
           para usar en un proyecto compacto C++20: la mini_expected de
           L11 (y de los ejercicios) Stripe-ya cuenta la historia).

       ¿Cómo elegir en la práctica? Tres preguntas, en orden:
         1. ¿El error es RARO y el que llama no PUEDE repararlo acá
            mismo? Excepción. ("se acabó la memoria", "el sistema de
            ventanas no arrancó": no hay pregunta local que repare).
         2. ¿El error es PARTE de lo esperable y el que llama repara en
            el momento? Código / expected. ("la clave tipeada no figura
            en la config", "el archivo de opciones no existe: entramos
            con valores por defecto").
         3. ¿"No está" es un RESULTADO, no un problema? optional.
            ("no encontré el ítem en el inventario": es un dato, no
            una falla; escalarlo a error es mentir sobre el programa).
       Un frame de juego redondea la tabla: por EJECUCIÓN, los errores
       que rompen el frame entero (disco, GPU, memoria) son excepciones;
       los que se negocian por tecla o por entidad son códigos/expected.
       Y la regla que abraza todo: NO HAY UNA RESPUESTA GLOBAL; la
       correcta es CONSISTENTE: cada capa usa su estrategia y la defiende
       en todas sus funciones.
    2. ¿QUÉ PASA POR DEBAJO?
        Con excepciones activadas, el compilador tiene que poder
        DESCERCLAR en orden cualquier función entre el throw y su catch
        (es el "unwinding": lo formalizamos en L05 y L08). Con códigos,
        la propagación es explícita: cada función que "deja pasar" el
        error escribe `return err;` (una vez por capa). Dos costos
        mecánicos opuestos: el código paga en líneas y en cuidado de
        cada llamada; la excepción paga en tablas que el compilador
        escribe en el binario ("quién puede lanzar" y "cómo deshacer"),
        más algo de campo por cada objeto lanzado. Un ejecutable con
        excepciones es algo más gordo aunque nunca se lance. Por eso
        muchos motores conviven con las dos: excepciones para el setter
        del arranque y asserts/códigos dentro del frame. Con esta L02
        vos tenés la base para leer por qué: NO ESTÁS OBLIGADO a un
        solo estilo.
    3. BUENAS PRÁCTICAS:
        - Elegí una estrategia por CAPA y quedate: mezclarla llamada
          a llamada genera flujos a medias (y eso se cobra en la
          limpieza parcial de L07).
        - No uses excepciones para decisiones normales del juego
          (cor进行比较 flow): eso mare al que LEA el código y todas las
          LNO exceptions later máximas del decir: optional.
        - No uses código de error para cosas que el que llama NO puede
          arreglar nunca: empuja hacia excepción o hacia un corte del
          ciclo con log, y el flujo NO cae silencioso.
        - "No está" = optional; el error con RAZÓN = expected.
   ============================================================================= */
#include <cstddef>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace Leccion02 {

// La MISMA funcionalidad con las tres estrategias, para comparar el flujo:
// contar letras con límites de forma y tamaño.

// (A) Con código: cada paso del camino feliz se escribe tres veces.
enum class Codigo { Ok, Vacio, DemasiadoLargo };
std::size_t contarLetrasConCodigo(const std::string& s, Codigo& codigo) {
    if (s.empty()) { codigo = Codigo::Vacio; return 0; }
    if (s.size() > 1024) { codigo = Codigo::DemasiadoLargo; return 0; }
    codigo = Codigo::Ok;
    return s.size();
}

// (B) Con excepción: el camino feliz son DOS líneas; el resto viaja "arriba".
std::size_t contarLetrasConExcepcion(const std::string& s) {
    if (s.empty()) throw std::invalid_argument("string vacío");
    if (s.size() > 1024) throw std::length_error("string demasiado largo");
    return s.size();
}

// (C) Con resultado explícito (la idea de expected, en forma manual:
//     L11 formaliza esta clase como mini_expected para estudiarla en C++20).
struct ResultadoContar {
    std::size_t valor = 0;
    std::string error = "";      // vacío = ok
    bool bien() const { return error.empty(); }
};
ResultadoContar contarLetrasConResultado(const std::string& s) {
    if (s.empty()) return {0, "string vacío"};
    if (s.size() > 1024) return {0, "string demasiado largo"};
    return {s.size(), ""};
}

// Caso (1) de la tabla: error que el que llama NO puede reparar: excepción.
std::vector<int> cargarMapa(std::size_t ancho, std::size_t alto) {
    if (ancho * alto > 10) throw std::runtime_error("mapa demasiado grande");
    return std::vector<int>(ancho * alto, 0);
}

// Caso (2): error esperable y reparable en el momento: default y sigo.
int leerVelocidad(const std::string& texto) {
    try {
        return std::stoi(texto);        // stoi lanza invalid_argument con basura
    } catch (const std::logic_error&) {
        return 10;                      // el default de la capa de juego
    }
}

} // namespace Leccion02

int main() {
    std::cout << "== (A) código: normal y con error por canales separados ==\n";
    Leccion02::Codigo c = Leccion02::Codigo::Ok;
    std::size_t n = Leccion02::contarLetrasConCodigo("hola", c);
    std::cout << "contarLetrasConCodigo(\"hola\") = " << n << '\n';
    (void)Leccion02::contarLetrasConCodigo("", c);      // ¿quién revisa esto? (manual)
    std::cout << "con \"\": el que llama DEBE mirar el código: nada lo obliga\n";

    std::cout << "== (B) excepción: el camino feliz no tiene ifs ==\n";
    try {
        std::cout << "contar(\"hola\") = " << Leccion02::contarLetrasConExcepcion("hola") << '\n';
        std::cout << "contar(\"\") = " << Leccion02::contarLetrasConExcepcion("") << '\n';
    } catch (const std::exception& e) {
        std::cout << "excepción atrapada: " << e.what() << '\n';
    }

    std::cout << "== (C) resultado explícito: la razón viaja con el valor ==\n";
    const auto r1 = Leccion02::contarLetrasConResultado("juego");
    std::cout << "bien: " << (r1.bien() ? "sí" : "no") << ", valor = " << r1.valor << '\n';
    const auto r2 = Leccion02::contarLetrasConResultado("");
    std::cout << "bien: " << (r2.bien() ? "sí" : "no") << ", razón: " << r2.error << '\n';

    std::cout << "== la tabla de decisión en el código ==\n";
    try {
        const auto mapa = Leccion02::cargarMapa(3, 10);   // (1) raro -> excepción
        std::cout << "mapa de " << mapa.size() << " celdas cargado\n";
    } catch (const std::exception& e) {
        std::cout << "cargarMapa lanzó: " << e.what() << " (nada que reparar acá)\n";
    }
    std::cout << "velocidad de 'basura': " << Leccion02::leerVelocidad("basura") << " (default)\n";
    std::cout << "velocidad de '20':  " << Leccion02::leerVelocidad("20") << '\n';
    return 0;
}
