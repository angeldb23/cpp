/* =============================================================================
   MÓDULO 12 - LECCIÓN 08: archivos binarios, write/read crudo y un save canonico
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
       Texto vs binario: en TEXTO el stream TRADUCE: un '\n' puede
       volverse "\r\n" en Windows y un "3.14" son los caracteres "3",
       ".", "1", "4". En BINARIO no: los BYTES van tal cual a disco
       (write/read crudos): misma cantidad, mismos valores.

       ¿Cuándo binario? Cuando el save es VOLUMEN: una malla de 100k
       vértices como texto pesa y parsea lento; como binario es el dump
       literal. LA CONTRA: no es legible ni auto-descriptivo: SIEMPRE
       abrilo con un header: MAGIC + VERSION, y al cargar VALIDÁ ambos:
       sin eso, el save de hoy es basura en seis meses.

       El patrón canon del archivo binario armado por vos:
         1) HEADER fijo MAGIC + VERSION: si no coinciden al abrir,
            RECHAZAR YA (no migrar on the fly sin saber la version).
         2) LA DATA: campos de tamaño conocido (write de bytes por
            campo) o un struct POD completo de una vez.
         3) VALIDACIÓN de cada lectura: read es un bool + gcount():
            solo "leí exactamente los bytes pedidos" confía.

       La API mínima: write() y read(): operan con `const char*`/
       `char*` + cantidades: el reinterpret_cast es EL puente del
       struct al char:
         f.write(reinterpret_cast<const char*>(&objeto), sizeof(objeto));
         f.read (reinterpret_cast<char*>(&destino), sizeof(destino));
       read/write NO son more v como "todo o nada": validá el stream
       y mirá gcount() (L02).

       LA TRAMPA del POD: `struct Pod {int a; char pad[3]; double b;}`
       tiene PADDING que depende del compilador/plataforma: un archivo
       binario de POD crudo NO es portable de máquina a máquina. El fix
       serio es campos de ancho fijo INDEPENDIENTE del compilador (los
       u32/u64 del <cstdint> como en SaveJugador de abajo). Para
       certificar de verdad (todos los juegos de motor) existen
       formatos planificados: eso ya es otro capítulo; acá te
       alcanzamos el header + version + POD fijo.
   2. ¿QUÉ PASA POR DEBAJO?
       fstream con std::ios::binary desactiva la traducción de EOL: los bytes van crudos (en Linux el '\n' ya es un byte
       igual; es en Windows donde sin binary un '\n' la cosa pasa a
       DOS bytes: por eso el flag del binary es OBLIGATORIO para el
       archivo binario en cualquier plataforma).
       read/write son NON-FORMATEADO FULL (L02): gcount() fresco dice
       exactamente cuántos bytes llegaron: si pediste 10 y llegaron 5,
       el estado del stream y el gcount lo muestran (L12).
       El seekp/seekg funcionan con offsets absolutos o relativos
       (tell/tellp en L13): el "escribir en el offset X de un save" es
       un patrón clásico de motor y sigue siendo read/write + offsets:
       la base de abajo ya sirve.
       El escudo del save: escribir primero en .tmp y después
       filesystem::rename (L11): un crash a mitad del write NO rompe
       el save anterior.
   3. BUENAS PRÁCTICAS:
       - SIEMPRE header (magic + version) antes de la data.
       - Validar TODAS las lecturas (el bool + gcount ANTES de confiar).
       - POD SI: standard-layout con tipos de ancho fijo (<cstdint>);
         punteros y std::string NUNCA van crudos en un binario (sus
         bytes internos son memoria nueva: el save lo rompe).
       - El "escudo del save": write en .tmp + filesystem::rename (L11):
         un crash en el medio nunca corrompe el save anterior.
       - Validar el LARGO esperado (L13): el acortado sucede cuando un
         jugador abre el save con el editor de texto y toca algo.
   ============================================================================= */

#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

namespace binario {
    constexpr char MAGIA[] = "CBIN";   // code binario: nuestro del curso
    constexpr std::uint32_t VERSION = 1;

    struct SaveJugador {               // POD standard-layout, sin punteros
        std::uint32_t vidaMaxima = 0;
        std::uint32_t puntos = 0;
        char nombre[16] = {};          // redondeado, tamaño FIJO
    };

    bool guardar(const std::string& ruta, const SaveJugador& s) {
        std::ofstream f(ruta, std::ios::binary | std::ios::trunc);
        if (!f) return false;
        // 1) header: magic + version (el parche: el save-que-era):
        f.write(MAGIA, 4);
        f.write(reinterpret_cast<const char*>(&VERSION), sizeof(VERSION));
        // 2) data: el struct completo (POD: los bytes son el estado):
        f.write(reinterpret_cast<const char*>(&s), sizeof(s));
        return static_cast<bool>(f);   // true SOLO si no falló nada
    }

    bool cargar(const std::string& ruta, SaveJugador& s) {
        std::ifstream f(ruta, std::ios::binary);
        if (!f) return false;
        // 1) header: validar magic y version:
        char magia[4] = {};
        if (!f.read(magia, 4)) return false;
        if (std::memcmp(magia, MAGIA, 4) != 0) return false;   // no es nuestro
        std::uint32_t version = 0;
        if (!f.read(reinterpret_cast<char*>(&version), sizeof(version)))
            return false;
        if (version != VERSION) return false;  // save viejo: migrar o rechazar
        // 2) data: OJO validar el gcount/post-status cuando termina:
        if (!f.read(reinterpret_cast<char*>(&s), sizeof(s))) return false;
        return true;
    }
} // namespace binario

int main() {
    const std::string ruta = "/tmp/opencode/curso12_leccion08_demo.bin";

    std::cout << "=== 1) guardar el save completo ===\n";
    binario::SaveJugador hero;
    hero.vidaMaxima = 120;
    hero.puntos = 13370;
    std::memcpy(hero.nombre, "HeroePrueba", strlen("HeroePrueba"));
    std::cout << "guardar: " << (binario::guardar(ruta, hero) ? "ok" : "fallo")
              << "\n";

    std::cout << "\n=== 2) cargar y comportar ===\n";
    binario::SaveJugador otro;
    if (binario::cargar(ruta, otro)) {
        std::cout << "vidaMaxima=" << otro.vidaMaxima
                  << " (<- val malloc memcmp'd minusculo)\n";
        std::cout << "puntos=" << otro.puntos
                  << "; nombre='" << otro.nombre << "'\n";
        std::cout << "(los POD los presenté: en mem y disco byte a byte)\n";
    }

    std::cout << "\n=== 3) el header salva de un archivo extranjero ===\n";
    {
        std::ofstream f("/tmp/opencode/curso12_leccion08_falso.bin", std::ios::binary);
        f.write("XXXX", 4);  // magic equivocado: no es un save válido
        const char chorrada[1000] = {};
        f.write(chorrada, 1000);
    }
    binario::SaveJugador probe;
    std::cout << "cargar de archivo sin nuestro magic: "
              << (binario::cargar("/tmp/opencode/curso12_leccion08_falso.bin", probe) ? "true" : "false")
              << "<- rechazado por header, no crasheo\n";

    std::cout << "\n=== 4) archivo TRUNCADO A MITAD ===\n";
    binario::guardar("/tmp/opencode/curso12_leccion08_corto.bin", hero);
    {
        std::ifstream fin("/tmp/opencode/curso12_leccion08_corto.bin", std::ios::binary);
        std::string bytes(8, '\0');
        fin.read(bytes.data(), 8);          // solo el header, sin data
    }
    std::ofstream("/tmp/opencode/curso12_leccion08_corto.bin", std::ios::binary | std::ios::trunc);
    std::cout << "archivo vaciado: cargar -> "
              << (binario::cargar("/tmp/opencode/curso12_leccion08_corto.bin", probe) ? "true" : "false")
              << " (false: read falla o data incompleta: VALIDADO)\n";
    return 0;
}
