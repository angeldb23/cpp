/* =============================================================================
   MÓDULO 07 - LECCIÓN 12: Deleters propios y recursos que no son memoria
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      RAII y los smart pointers no son "de memoria": son de RECURSOS. La
      prueba más clara es el DELETER personalizado: le decís al puntero
      inteligente CÓMO se libera el recurso que guarda.

      a) El caso rey: los HANDLE estilo C:
           - FILE* y fclose() (stdio);
           - SDL_Texture* y SDL_DestroyTexture();
           - GLuint (id de OpenGL) y glDeleteTextures();
           - fds (open/close), pthread, MYSQL*, mbedtls...
         Todos comparten la MISMA forma: una adquisición que devuelve un
         "puntero/identificador suelto" y una función de liberación
         específica. El deleter conecta eso con RAII sin escribir una
         clase wrapper por cada recurso.

      b) unique_ptr con deleter: la firma crece en un parámetro técnico:
            struct FCerrar { void operator()(FILE* f) const {
                                 if (f) fclose(f); } };
            std::unique_ptr<FILE, FCerrar> f(std::fopen(...));
         o, la versión clásica (con el costo que le sigue):
            std::unique_ptr<FILE, decltype(&fclose)> f(fopen(...), &fclose);
         El tipo deleter cambia sizeof UNIQUE (guarda la dirección de la
         función o la lambda). Con LAMBDA SIN CAPTURAS o functor chico, el
         tipo es stateless: sizeof no crece (empty base optimization:
         sigue siendo un puntero de 8 bytes).

      c) shared_ptr con deleter: NO cambia la firma del tipo: el deleter
         vive DENTRO del bloque de control (type erasure):
            std::shared_ptr<FILE> f(fopen(...), &fclose);
         Ventaja práctica: la fábrica y todos los usuarios usan el mismo
         tipo simple y el borrado correcto viaja escondido. Es la forma
         natural de envolver recursos de librerías externas.

      d) release() y el flujo con APIs de C: su escenario natural. Si le
         pasás el recurso a una función que TOMA la propiedad (una API de
         C que dice "yo lo libero"), tenés que renunciar antes: release()
         en unique_ptr entrega el handle y queda vacío. Nunca entregues
         get() a una API que libera y seguís siendo dueño vos: doble
         destrucción garantizada.

      e) Recursos que NO son memoria: la ampliación del módulo:
           - un lock de mutex (std::lock_guard es RAII canónico: módulo 14);
           - una conexión/socket (adquirir conectar, cerrar al salir);
           - una transacción de base de datos (commit/rollback en dtor);
           - un lock de licencia de un thread pool, un timer...
         El FORMULARIO es el mismo: "el constructor adquiere, el destructor
         libera". No cambia nada si el recurso es un archivo o una GPU.

   2. ¿QUÉ PASA POR DEBAJO?
      unique_ptr<T, D> guarda: T* ptr + un objeto D deleter. Cuando D no
      captura nada, el compilador reusa el espacio (empty base
      optimization) y sizeof(unique_ptr<T,D>) == sizeof(T*). Al destruir:
      if (ptr) deleter(ptr). El deleter NO puede lanzar: corre en una
      destrucción (que es noexcept implícito: una excepción ahí es
      std::terminate: L03).

      shared_ptr<T>: el deleter se BORRA DEL TIPO: el bloque de control
      guarda el puntero al recurso como void* + el puntero a la función
      de borrado. La indirección extra es una: por eso duele un poquito
      más que unique, pero existe el patrón cruzado: shared_ptr<FILE> y
      shared_ptr<SDL_Texture*> pueden convivir en el mismo vector si
      escribís... no, no pueden: el VECTOR<T> es homogéneo: lo que
      "elimina la diferencia" es que el user de FILE* no sabe que el
      bloque hace fclose.

      IMPORTANTE C: el fclose hace FLUSH del buffer antes de cerrar. Si
      saltás el fclose (recurso de RAII que sacó el handle y lo guarda en
      un int por ahí), el archivo queda con datos en el buffer de C y
      "falta contenido" al abrirlo de nuevo: otro motivo para que el
      guard del RAII sea ÚNICO del ciclo de vida.

   3. ERRORES COMUNES
      - unique_ptr<FILE, void(*)(FILE*)>: puntero a función como deleter
        PIERDE la optimización de tamaño (guarda los 8B del pf: sizeof
        crece a 16B) y lo más grave: no es inlineable. La lambda sin
        captura es gratis: es la preferida.
      - fclose LUEGO de que el RAII corre: el "deleter doble" explota.
        Regla: O el RAII maneja todo, O renunciás (release) ANTES de
        entregar el handle a alguien más.
      - OLVIDAR chequear el fopen nulo: unique_ptr vacío = recurso no
        adquirido: con operator bool se testea al instante: no hay
        "puntero válido pero archivo roto".
      - Exponer T* crudamente de una clase dueña: la clase dueña da ACCESO
        (métodos) o entrega propiedad COMPLETA (unique_ptr); un get() que
        después sufre fclose() en otro archivo es el bug de doble dueño.

   4. DEMO: FILE con deleter (y RAII de un recurso no-memoria)
 */
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>

// El recurso estilo C en RAII: sin clase wrapper de negocio, un functor
// chico hace de deleter (la alternativa clásica es decltype(&fclose),
// pero glibc marca a fclose con atributos y el warning -Wignored-attributes
// no vale la pena: el functor también optimiza a mismo tamaño).
struct FCerrar {
    void operator()(FILE* f) const {
        if (f) std::fclose(f);
    }
};
using ArchivoPtr = std::unique_ptr<FILE, FCerrar>;

ArchivoPtr abrirArchivo(const char* nombre) {
    // Fábrica DUEÑA de la adquisición: si fopen falla, unique_ptr vacío.
    ArchivoPtr f(std::fopen(nombre, "w"));
    if (f) {
        std::cout << "    abierto: " << nombre << "\n";
    } else {
        std::cout << "    falló abrir " << nombre
                  << " (null: manejable con operator bool)\n";
    }
    return f;   // sin copia real: C++17 garantiza elisión (módulo 08: RVO)
}

// Recurso NO-memoria: una "licencia" de thread. RAII clásico con clase
// propia: cuando la adquisición NO devuelve un handle guardable, el
// wrapper a mano es la forma natural.
class Licencia {
private:
    static int activas_;
    const char* quien_;

public:
    explicit Licencia(const char* quien) : quien_(quien) {
        ++activas_;
        std::cout << "    [lock   ] adquiero licencia (" << activas_
                  << " activas)\n";
    }
    ~Licencia() {
        --activas_;
        std::cout << "    [unlock ] libero licencia de " << quien_
                  << " (quedan " << activas_ << ")\n";
    }
    Licencia(const Licencia&)            = delete;
    Licencia& operator=(const Licencia&) = delete;
};
int Licencia::activas_ = 0;

int main() {
    std::cout << "=== 1. UNIQUE_PTR CON DELETER: EL FILE ESTILO C ===\n";
    {
        ArchivoPtr f = abrirArchivo("claves_juego.txt");
        if (f) {
            // fputs/fwrite informan su resultado: el RAII del fopen garantiza
            // VIDA, no garantiza ESCRITURA: los E/S se chequean igual (L12 de
            // módulo 12 entra a fondo en esto).
            if (std::fputs("x,y,frame\n0.3,-0.12,42\n", f.get()) < 0) {
                std::cout << "    error de escritura\n";
            } else {
                std::cout << "    escribí dos líneas\n";
            }
        }
    }   // fclose corre acá, automáticamente, con flush incluido.

    std::cout << "\n=== 2. LAMBDA SIN CAPTURA: DELETER SIN PESO ===\n";
    {
        auto cerrar = [](FILE* f) {
            std::cout << "    lambda-deleter: cerró (footer ASCII)\n";
            if (f) std::fclose(f);
        };
        std::unique_ptr<FILE, decltype(cerrar)> g(
            std::fopen("lambda_file.txt", "w"), cerrar);
        std::cout << "    sizeof(unique_ptr con lambda deleter) = "
                  << sizeof(g) << " bytes (el deleter no suma)\n";
    }

    std::cout << "\n=== 3. SHARED_PTR: DELETER EN EL BLOQUE DE CONTROL ===\n";
    {
        std::shared_ptr<FILE> s(
            std::fopen("compartido.txt", "w"),
            [](FILE* fd) {
                if (fd) {
                    std::cout << "    [shared deleter] cerrando\n";
                    std::fclose(fd);
                }
            });
        std::shared_ptr<FILE> copia = s;         // +1 dueño
        std::cout << "    strong: " << s.use_count()
                  << " (mismo tipo simple: el deleter viaja escondido)\n";
    }

    std::cout << "\n=== 4. RAII DE RECURSO NO-MEMORIA (licencia) ===\n";
    {
        Licencia render("render_thread");
        Licencia audio("audio_thread");
        std::cout << "    dos licencias vivas...\n";
    }   // dtors en reversa (L02): audio primero, render después.

    std::cout << "\n=== 5. LA SEÑAL DEL PATRÓN ===\n";
    std::cout << "  'adquisición + liberación' repetida en cada nivel del\n";
    std::cout << "  código = RAII faltante. Archivos, GPU, locks, sockets:\n";
    std::cout << "  TODOS tienen forma de RAII. El módulo entero es esto.\n";
    return 0;
}
