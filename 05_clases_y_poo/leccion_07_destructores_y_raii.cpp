/* =============================================================================
   MÓDULO 05 - LECCIÓN 07: Destructor, orden de vida y primer contacto con RAII
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      El destructor es el método que se ejecuta automáticamente cuando el
      objeto MUERE: al terminar el scope de una variable local, al hacer
      delete de un puntero, o al desarmarse un contenedor que lo guardaba.

          ~Jugador();          // tilde (~) + nombre de la clase, sin retorno

      Reglas:
      - Solo puede existir UNO: no reciben argumentos.
      - Se llama solo, sin que vos lo invoques (y si lo invocás a mano
        casi siempre es un error).
      - El orden de destrucción es el OPUESTO al de construcción, como
        con los miembros (lección 06) y como con variables locales:
        la primera en nacer es la última en morir.

          Jugador a;      // nace 1°
          Jugador b;      // nace 2°
          { Jugador c; }  // c muere al salir del bloque
          // al final del main: muere b, después a

      RAII (Resource Acquisition Is Initialization) es la idea que cambia
      todo: en vez de "pedir un recurso y acordarte de devolverlo",
      metés el recurso en un objeto:
      - el constructor lo OBTIENE,
      - el destructor lo SUELTA, siempre, por cualquier salida del scope.

      void demo() {
          std::string nombre = "gran cadena";   // pide memoria al heap
          if (algoFallo()) {
              return;    // el destructor de nombre corre igual
          }
          // ... más código ...
      }  // sin return ni throw: el destructor corre al cierre del scope
      No escribiste un delete y no quedan fugas en ninguna rama. El
      costo de esa garantía es cero. Esa puerta se cierra sola.

   2. ¿QUÉ PASA POR DEBAJO?
      El compilador inserta las llamadas a destructores en cada punto de
      salida del scope. Para una clase con miembros, el destructor de la
      clase corre primero (tu cuerpo) y después los destructores de los
      miembros en orden inverso de declaración. Por eso un objeto que
      contiene std::string y std::vector no libera nada a mano: sus
      miembros ya son objetos con destructor y se limpian en cascada.

      En RAII, "resource" no es solo memoria: es lock por mutex, archivo
      abierto, conexión, sprite cargado. El patrón siempre es el mismo:
      adquirir en el ctor, liberar en el dtor. El módulo 07 arma las
      piezas canónicas (los smart pointers), y el módulo 06 agrega el
      destructor virtual necesario cuando hay herencia.

   3. ERRORES COMUNES
      - Poner lógica que depende del orden entre objetos distintos:
        el orden de destrucción entre objetos globales de distintos
        archivos .cpp no está garantizado.
      - Acordarse de liberar el recurso en un rama y no en otra (if sin
        else + return temprano): RAII existe para no tener que acordarse.
      - Escribir delete dentro de un scope en vez de dejar que muera el
        objeto RAII: doble liberación o fuga según la rama.
      - Llamar al destructor a mano (`obj.~T();`): casi nunca es lo que
        querés (se usa en pools con placement new, tema muy avanzado).

   4. PARA PROFUNDIZAR
      https://en.cppreference.com/w/cpp/language/destructor
      https://en.cppreference.com/w/cpp/language/raii
   ============================================================================= */
#include <cstdio>
#include <iostream>
#include <string>

// =========================================================================
// Demostración del orden de vida: cada objeto anuncia su nacimiento/muerte.
// =========================================================================
class Tracer {
public:
    explicit Tracer(const char* nombre) : m_nombre(nombre) {
        std::cout << "  nace   " << m_nombre << "\n";
    }
    ~Tracer() {
        std::cout << "  muere  " << m_nombre << "\n";
    }
private:
    std::string m_nombre;
};

// =========================================================================
// RAII de un recurso que NO es memoria: un archivo con std::FILE*.
// (En el curso usamos std::ifstream para archivos reales; este wrapper
// es para mostrar el patrón crudo.)
// =========================================================================
class Archivo {
public:
    explicit Archivo(const char* ruta) : m_archivo(std::fopen(ruta, "w")) {
        if (m_archivo == nullptr) {
            std::cout << "  [Archivo] no se pudo abrir '" << ruta << "'\n";
        }
    }
    ~Archivo() {
        if (m_archivo != nullptr) {
            std::fclose(m_archivo);           // suelta el recurso SIEMPRE
            std::cout << "  [Archivo] cerrado\n";
        }
    }
    void escribir(const char* texto) {
        if (m_archivo != nullptr) {
            std::fputs(texto, m_archivo);
        }
    }
private:
    std::FILE* m_archivo = nullptr;   // una FILE* envuelta en RAII
};

int main() {
    std::cout << "=== 1. ORDEN DE CONSTRUCCIÓN Y DESTRUCCIÓN ===\n";
    Tracer a("a");
    Tracer b("b");
    {
        Tracer c("c");
        Tracer d("d");
        std::cout << "  (fin del bloque interno)\n";
    }
    std::cout << "  (siguiente línea ya está fuera del bloque)\n";
    std::cout << "  fin de main: quedan por morir b y luego a\n";

    std::cout << "\n=== 2. RAII: recurso que se libera a la salida ===\n";
    {
        Archivo log("leccion_07_demo.txt");
        log.escribir("salida del destructor garantizada\n");
        std::cout << "  [main] escribi en el archivo: lo cierro al salir del"
                     " bloque con o sin errores\n";
    }
    std::cout << "  [main] salí del bloque: el Archivo ya se autocerró\n";

    std::cout << "\n=== 3. STRING COMO RAII DE MEMORIA ===\n";
    auto secciones = []() {
        std::string pesado(1024 * 1024, 'x');  // 1 MB pedido al heap
        std::cout << "  string de " << pesado.size() << " chars en el heap\n";
        return 0;   // salida POR RETURN y aún así el destructor corre
    };
    secciones();
    std::cout << "  heap de vuelta a la normalidad, sin delete manual\n";
    return 0;
}
