/* =============================================================================
   MÓDULO 12 - LECCIÓN 10: directory_iterator, recursive_iterator y metadata
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
       Saber el nombre ya no alcanza: la semana que viene necesitás
       LISTAR carpetas y consultar METADATA. El API de iteradores de
       filesystem:

         fs::directory_iterator(dir)   -> itera las entradas del nivel
                                          (no baja a subcarpetas)
         fs::recursive_directory_iterator(dir)
                                       -> la misma, pero desciende a
                                          subcarpetas (recorrido profundo)
       Cada ++ entrega un directory_entry: un path + el "estado" cacheado
       de esa entrada (regular_file, directory, symlink...). El range-for
       idiomático:
         for (const auto& entry : fs::directory_iterator(dir)) { ... }

       Metadata útil (en la entry o con las funciones libres):
         entry.is_regular_file() / is_directory() / is_symlink()
         entry.file_size()       -> bytes del archivo (0 en carpetas)
         entry.last_write_time() -> time_point (filesystem clock)
       El filtro típico une extension (L09) + estado:
         if (entry.is_regular_file() && entry.path().extension() == ".png")

       Recursión controlada: el recursive iterator tiene depth() y
       disable_recursion_pending(): los dos JUNTOS son el "skip de
       carpetas": `for (auto it = fs::recursive_directory_iterator(dir);
       it != end; ++it)` y en un if (carpeta prohibida) mandás
       it.disable_recursion_pending() y no baja ahí.
       UNA entry puede desaparecer MIENTRAS iterás (otro proceso):
       por eso el overload con std::error_code de is_* (L11) es el
       guard: si la entrada murió, `continue` y no se cae.
   2. ¿QUÉ PASA POR DEBAJO?
       directory_iterator abre un "directory stream" del kernel (en
       Linux: opendir/readdir) y ++ lee el próximo nombre; la entry
       cachea el stat mientras esté viva el iterador: tu bucle NO
       consulta el sistema por cada atributo.
       El recursive mantiene un STACK interno de streams de carpetas:
       el ++ hace push al entrar a una y pop al salir: la recursion
       es ITERATIVA (depth() es el nivel en ese stack).
       Si la carpeta base NO existe: el CONSTRUCTOR lanza
       fs::filesystem_error (captura por const&, módulo 04): el
       overload con std::error_code (dir, ec) NO lanza: L11 muestra
       los dos estilos juntos.
   3. BUENAS PRÁCTICAS:
       - Filtrar SIEMPRE por is_regular_file() antes de extension():
         una carpeta llamada "mis_saves.dat" te colaría como archivo.
       - Skip de carpetas con disable_recursion_pending(): build/,
         .git/, node_modules/: el rendimiento lo agradece (y el cache).
       - last_write_time() es caro por archivo y NO escaneo por línea:
         usalo cuando la metadata manda (ordenar por fecha de mod).
       - El ORDEN del listado NO es garantía: para "ordenado" recojá
         en un vector y sort (el módulo 10 hace el sort).
   ============================================================================= */

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

int main() {
    const fs::path demo = "/tmp/opencode/curso12_leccion10_demo";
    std::error_code ec;
    fs::remove_all(demo, ec);   // estado limpio en cada run (no-lanza estilo)
    ec.clear();
    // armamos una carpeta demo en el punto temporal: situaciones reales:
    fs::create_directories(demo / "assets/ui");
    fs::create_directories(demo / "build");
    for (const char* archivo : {"hero.png", "menu.png", "musica.ogg", "leeme.txt"}) {
        std::ofstream{demo / archivo} << "bytes";
    }
    for (const char* archivo : {"boton.png", "marco.png"}) {
        std::ofstream{demo / "assets/ui" / archivo} << "bytes";
    }
    std::ofstream{demo / "build" / "curso.bin"} << "binaria";

    std::cout << "=== 1) directory_iterator: SOLO nivel 1 ===\n";
    for (const fs::directory_entry& entry : fs::directory_iterator(demo)) {
        std::cout << "  " << entry.path().filename().string() << "  ("
                  << (entry.is_directory() ? "carpeta" : "archivo") << ", "
                  << (entry.is_directory() ? 0 : entry.file_size())
                  << " bytes)\n";
    }

    std::cout << "\n=== 2) filtrar por extension y por tipo ===\n";
    for (const fs::directory_entry& entry : fs::directory_iterator(demo)) {
        if (!entry.is_regular_file()) continue;
        if (entry.path().extension() != ".png") continue;
        std::cout << "  sprite: " << entry.path().filename() << "\n";
    }

    std::cout << "\n=== 3) recursive: desciende 'to path' por todo el árbol ===\n";
    for (const fs::directory_entry& entry : fs::recursive_directory_iterator(demo)) {
        std::cout << "  " << fs::relative(entry.path(), demo) << "\n";
    }

    std::cout << "\n=== 4) skip de carpetas (disable_recursion_pending) ===\n";
    for (fs::recursive_directory_iterator it(demo); it != fs::end(it); ++it) {
        if (it->is_directory() && it->path().filename() == "build") {
            it.disable_recursion_pending();
            std::cout << "  (no desciendo en build/)\n";
            continue;
        }
        if (it->is_regular_file()) {
            std::cout << "  visto: " << fs::relative(it->path(), demo) << "\n";
        }
    }

    std::cout << "\n=== 5) orden determinista con sort (el listado crudo no ordena) ===\n";
    std::vector<fs::path> pngs;
    for (const fs::directory_entry& entry : fs::recursive_directory_iterator(demo)) {
        if (entry.is_regular_file() && entry.path().extension() == ".png") {
            pngs.push_back(entry.path());
        }
    }
    std::sort(pngs.begin(), pngs.end());
    for (const fs::path& ruta : pngs) {
        std::cout << "  " << fs::relative(ruta, demo) << "\n";
    }
    std::cout << "(determinista y reproducible: útil para cargar assets en orden)\n";
    return 0;
}
