/* =============================================================================
   MÓDULO 3 - LECCIÓN 01: Cadenas de Texto Modernas (std::string y string_view)
   =============================================================================
   En C antiguo, los textos eran arreglos de caracteres terminados en cero ('\0')
   muy propensos a desbordamientos de buffer (buffer overflows).
   En C++ moderno usamos 'std::string' que gestiona automáticamente la memoria.

   Conceptos clave:
   1. Creación, asignación y concatenación con '+'.
   2. Métodos útiles: length(), empty(), substr(), find(), push_back().
   3. Lectura de líneas completas con espacios usando 'getline(cin, texto)'.
   4. 'std::string_view' (C++17): Una vista de solo lectura que NO copia el texto.
   ============================================================================= */

#include <iostream>
#include <string>
#include <string_view> // Introducido en C++17

using namespace std;

// string_view recibe cualquier texto (literal, char*, std::string) SIN duplicar memoria en RAM.
void imprimirAlerta(string_view mensaje) {
    cout << "[ALERTA DE SISTEMA]: " << mensaje << endl;
}

int main() {
    cout << "=== 1. MANEJO BÁSICO DE std::string ===" << endl;
    string saludo = "Hola";
    string nombre = "Angel";

    // Concatenación sencilla con '+'
    string mensajeCompleto = saludo + ", " + nombre + "!";
    cout << mensajeCompleto << endl;
    cout << "Longitud: " << mensajeCompleto.length() << " caracteres." << endl;
    cout << endl;

    cout << "=== 2. MÉTODOS Y OPERACIONES COMUNES ===" << endl;
    string email = "usuario@correo.com";

    // Buscar una subcadena (retorna la posición o string::npos si no está)
    size_t posArroba = email.find('@');
    if (posArroba != string::npos) {
        cout << "El caracter '@' esta en el indice: " << posArroba << endl;
        
        // Extraer una parte del texto con substr(inicio, cantidad_caracteres)
        string usuario = email.substr(0, posArroba);
        string dominio = email.substr(posArroba + 1);
        cout << "Usuario: " << usuario << " | Dominio: " << dominio << endl;
    }
    cout << endl;

    cout << "=== 3. C++17 string_view (Rendimiento Extremo) ===" << endl;
    // Si pasás un string grande a una función que solo lee, pasarlo por string_view
    // es infinitamente más rápido porque solo pasa un puntero y la longitud.
    imprimirAlerta("El servidor se reiniciara en 5 minutos.");
    imprimirAlerta(mensajeCompleto);

    return 0;
}
