/* =============================================================================
   MÓDULO 12 - LECCIÓN 04: format_to, format_to_n, formatted_size y vformat
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
       format() devuelve std::string, pero no es lo único disponible. La
       familia completa:

         std::format(fmt, args...)          -> std::string nuevo
         std::format_to(out, fmt, args...)  -> escribe IN SITU en un OutIt
                                               (por ejemplo un
                                               back_inserter de un string
                                               ya reservado: cero
                                               allocations extra)
         std::format_to_n(out, n, ...)      -> escribe a lo SUMO n chars
                                               (el truncador para columnas
                                               de ancho fijo)
         std::formatted_size(fmt, ...)      -> cuántos chars representarían
         std::vformat(fmt, fargs)           -> el camino para format-strings
                                               que NO son literales: los
                                               parsea en runtime

       ¿Cuándo usar cada uno?
         - format con literal fijo: el caso default (chequeo compile-time
           intacto).
         - armar texto EN PARTES (una tabla, una fila por iteración del
           loop): format_to sobre un buffer reservado: evitás crear 50
           strings que después se concatenan (el módulo 03 ya contó el
           costo de las realocaciones).
         - columnas con límite de ancho (HUD de consola, celda en UI):
           format_to_n es el truncador oficial: devuelve
           {out, size} con size = lo que HABRIA escrito: si size > n,
           quedó recortado y vos lo sabés por código.
         - texto definido por DATO externo (config del jugador, i18n):
           vformat: el parseo corre en runtime y un error de sintaxis
           lanza std::format_error: validarlo ES TU trabajo.

       La integración con ostream: formatear y SALIR son dos decisiones
       separadas (cout << std::format(...) por ejemplo): por eso el
       mismo DSL sirve para consola, archivo y UI sin cambiar el
       format-string.
   2. ¿QUÉ PASA POR DEBAJO?
       format_to crea un format_context parametrizado por el OutputIt
       (es el concepto de iterador del módulo 10: cualquier destino que
       acepte ese OutIt acepta format_to). El parseo del literal es
       compile-time SIEMPRE (format y format_to_n: el literal es template
       PARAMETER), así que el chequeo sigue vivo en la familia.
       format_to_n escribe hasta n y NO descarta "silenciosamente":
       reporta con format_to_n_result el output real y el size que
       habría escrito.
       vformat pierde el chequeo compile-time POR DEFINICIÓN (el literal
       es un std::string nuevo en runtime): por eso lanza en el fallo.
       C++23 trae std::print("{}", x) que fusiona format + cout con
       flush inteligente; GCC moderno lo tiene, pero el curso se ancla
       en C++20 (D-01): lo que ves acá traslada 1:1.
   3. BUENAS PRÁCTICAS:
       - Buffers de reportes: `std::string out; out.reserve(n);` +
         format_to(back_inserter(out), ...) dentro del loop: un alloc.
       - format_to_n: chequeá SIEMPRE result.size para avisar "texto
         recortado"; sino el jugador ve la columna partida sin saber
         por qué.
       - vformat: NUNCA con un format-string QUE ESCRIBE el usuario
         directamente (el DSL es complejo y él puede generar errores
         y riesgo): el DSL fijo es TU idioma; el dato va como argumento.
       - Un "map de plantillas" (claves -> format-strings de runtime,
         p.ej. las plantillas del save en L15) es EL caso legítimo de
         vformat: el DSL lo elege vos, el dato va como argumento.
   ============================================================================= */
#include <format>
#include <iostream>
#include <string>
#include <vector>

int main() {
    std::cout << "=== 1) format_to: armar un reporte fila por fila ===\n";
    struct Item { std::string nombre; int nivel; };
    const std::vector<Item> items{{"espada", 3}, {"pocion", 1}, {"yelmo", 7}};

    std::string reporte;
    reporte.reserve(128);
    std::format_to(std::back_inserter(reporte), "{:<8}|{:>5}\n", "NOMBRE", "NIVEL");
    for (const Item& item : items) {
        std::format_to(std::back_inserter(reporte), "{:<8}|{:>5}\n", item.nombre, item.nivel);
    }
    std::cout << reporte;
    std::cout << "buffer final: " << reporte.size() << " chars, "
              << "alocado UNA vez (reserve) y escrito in-situ\n";

    std::cout << "\n=== 2) format_to_n: el HUD de linea fija ===\n";
    const std::string nombre = "Guerrero_de_la_noche_sin_luna";
    char hud[24] = {};                                  // columna de 23 + null
    auto resultado = std::format_to_n(hud, 23, "{:^23}", nombre);
    std::cout << "HUD   : [" << hud << "]\n";
    std::cout << "escr  : " << resultado.size << " pedidos; truncado? "
              << (resultado.size > 23 ? "si" : "no") << "\n";
    std::cout << "nota  : size > 23 => el texto SE RECORTA: hay que avisarlo\n";

    std::cout << "\n=== 3) formatted_size: medir antes de decidir ===\n";
    const auto largo = std::formatted_size("{:>10.2f}", 123.456);
    std::cout << "un double 123.456 en {:>10.2f} ocupa " << largo << " chars\n";

    std::cout << "\n=== 4) vformat: cuando la plantilla viene no-literal ===\n";
    const std::string plantilla = "pos=({1}, {2}) vida={0}";   // guardada en config
    const int vida = 100, posX = 3, posY = 7;
    try {
        std::cout << std::vformat(plantilla,
                                  std::make_format_args(vida, posX, posY))
                  << "\n";
        // ahora una plantilla que pide {3} y no lo tiene:
        const int soloVida = vida;
        std::cout << std::vformat("vida={3}", std::make_format_args(soloVida))
                  << "\n";
    } catch (const std::format_error& e) {
        std::cout << "format_error atrapada: " << e.what() << "\n";
        std::cout << "== con plantillas no literales, el parseo es RUNTIME:\n"
                     "el chequeo firme existe solo para literals (L03)\n";
    }

    std::cout << "\n=== 5) formato + stream: dos decisiones separadas ===\n";
    std::cout << std::format("[{:04}] {} / {:5}\n", 7, "goblins wave", 100);
    std::cout << "(formatea a string y sout el stream: podes cambiarla por archivo "
                 "sin tocar el format)\n";
    return 0;
}
