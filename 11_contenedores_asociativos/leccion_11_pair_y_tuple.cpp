/* =============================================================================
   MÓDULO 11 - LECCIÓN 11: std::pair y std::tuple
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
        A veces el resultado es "dos cosas sin nombre propio": un valor +
        un bool de éxito, una posición + un daño, una clave + un conteo.
        std::pair<A, B> empaqueta exactamente dos; std::tuple<A, B, C...>
        generaliza a N. Son un "struct anónimo", listo para usar, con
        utilidades incluidas:

            std::pair<int, bool> registro{10, true};
            std::tuple<int, std::string, float> fila{7, "orco", 3.5f};

        Interfaz básica:
          - std::get<0>(t), std::get<1>(t)...: acceso por índice en
            tiempo de compilación (el índice debe existir: si no, NO
            compila).
          - std::make_pair / std::make_tuple: deducen los tipos solos.
          - Comparación lista para usar: pair y tuple se comparan
            lexicográficamente campo por campo: ja hace que claves
            compuestas funcionen en map/set sin escribir nada.
          - std::tie(a, b) = par: "ataja las variables"; el separador
           	std::ignore descarta una posición.

        Cuándo NO usarlos: si los campos tienen significado (nombre,
        nivel, precio), un struct con NOMBRES autodocumenta
        (más barato de leer a las seis semanas). pair/tuple van bien
        cuando la posición ES el significado (el "first" siempre es el
        ID) o para resultados de APIs locales.
    2. ¿QUÉ PASA POR DEBAJO?
        get devuelve REFERENCIAS al slot interno: modificables si el
        tuple es modificable. make_tuple "decae" los argumentos como los
        parámetros de una función; si querés que guarde una referencia,
        std::ref(x). El par tiene los dos miembros PÚBLICOS clásicos
        (first/second); el tuple accede por get<ÍNDICE>.

        Comparación lexicográfica: se deriva automáticamente en pair y
        tuple (compará el primer campo; si empatan, el segundo; etc.).
        Consecuencia útil: un map con clave std::pair<A, B> YA compila
        si A y B son comparables: el comparador viene de regalo.
    3. ERRORES COMUNES Y BUENAS PRÁCTICAS:
        - std::get<5>(t) sobre un tuple de 3: error de COMPILACIÓN (el
          índice se valida en tiempo de compilación): mejor atrapado acá
          que con un vector[i] fuera de rango en runtime.
        - Si el orden de los campos CAMBIA, la comparación
          lexicográfica cambia con él y tu sort/map ordena distinto:
          no cambies campos "a escondidas".
        - tuple para "dos cosas que van juntas": bien; para "cinco cosas
          anónimas que el lector no sabe qué es": mal: un struct con
          nombres es documentación gratuita del mismo costo.
        - Ojo con el par que viene de insert (L01-L02): first es la
          CLAVE y second es el VALOR; en el par<iterador, bool> el
          bool es second: la convención cambia contexto a contexto;
          léelo antes de presumirlo.
    4. PARA PROFUNDIZAR:
        https://en.cppreference.com/w/cpp/utility/pair
        https://en.cppreference.com/w/cpp/utility/tuple
        https://en.cppreference.com/w/cpp/utility/tuple/tie
   ============================================================================= */
#include <iostream>
#include <map>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

int main() {
    // --- 1. pair: dos cosas, con utilidades capaces -----------------------------------
    std::pair<int, bool> registro{10, true};
    std::cout << "registro: " << std::get<0>(registro) << " / "
              << (std::get<1>(registro) ? "si" : "no") << '\n';

    auto sumaYconteo = std::make_pair(42, 3);
    std::cout << "suma: " << sumaYconteo.first << " | conteo: "
              << sumaYconteo.second << '\n';

    // --- 2. tie: nombres para un resultado que ya existe ------------------------------
    int suma = 0;
    int conteo = 0;
    std::tie(suma, conteo) = std::make_pair(42, 3);   // asigna a variables existentes
    std::cout << "\ntie: suma=" << suma << " conteo=" << conteo << '\n';

    std::tie(suma, std::ignore) = std::make_pair(99, 999);   // solo quiero el primero
    std::cout << "ignorar el segundo: suma=" << suma << '\n';

    // --- 3. tuple: N campos con tipos distintos ---------------------------------------
    std::tuple<int, std::string, float> fila{7, "orco", 3.5f};
    std::cout << "\nfila: " << std::get<0>(fila) << " " << std::get<1>(fila)
              << " " << std::get<2>(fila) << '\n';

    // comparación lexicográfica lista para usar:
    auto a = std::make_pair(1, 'x');
    auto b = std::make_pair(2, 'a');
    std::cout << "\n(1,'x') < (2,'a')? " << (a < b)   // primero compara 1 < 2
              << " ; (2,'a') == (2,'a')? " << (b == std::make_pair(2, 'a')) << '\n';

    // --- 4. Ejemplo real: map con clave COMPUESTA (pair) -------------------------------
    std::map<std::pair<int, std::string>, int> porZona;
    porZona[{3, "torre"}]  = 14;
    porZona[{3, "sotano"}] = 7;
    std::cout << "\ndanio en (3, torre): " << porZona[{3, "torre"}] << '\n';
    return 0;
}
