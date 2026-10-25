/* =============================================================================
   MÓDULO 1 - LECCIÓN 01: Tipos de Datos, Variables y Memoria Básica
   =============================================================================
   En C++, a diferencia de lenguajes como Python o JavaScript, el tipo de dato
   le dice al compilador EXACTAMENTE cuántos bytes de memoria reservar y cómo
   interpretar esos ceros y unos en la RAM.

   Conceptos clave que veremos en este archivo:
   1. Tipos primitivos: int, float, double, char, bool.
   2. Tamaño en memoria con 'sizeof'.
   3. Inicialización uniforme con llaves {} (moderna y segura).
   4. Constantes con 'const'.
   5. Deducción automática de tipos con 'auto'.
   6. Conversión explícita de tipos con 'static_cast'.
   =============================================================================
 */

#include <iomanip>
#include <iostream>

using namespace std;

int main() {
  cout << "=== 1. TIPOS PRIMITIVOS Y TAMAÑOS EN MEMORIA ===" << endl;

  // Enteros (números sin decimales)
  int vida = 100;

  // Decimales (coma flotante)
  // float ocupa 4 bytes (precisión simple, unas 7 cifras significativas)
  float velocidad = 3.5f; // Nota la 'f' al final para indicar float
  // double ocupa 8 bytes (precisión doble, unas 15 cifras significativas)
  double distancia = 125000.456789;

  // Caracteres individuales (comillas simples)
  char inicial = 'A';

  // Booleanos (verdadero o falso)
  bool estaVivo = true;

  // Con sizeof() podemos ver cuántos bytes ocupa cada tipo en tu procesador:
  cout << "int vida: " << vida << " | Ocupa: " << sizeof(int) << " bytes"
       << endl;
  cout << "float velocidad: " << velocidad << " | Ocupa: " << sizeof(float)
       << " bytes" << endl;
  cout << "double distancia: " << distancia << " | Ocupa: " << sizeof(double)
       << " bytes" << endl;
  cout << "char inicial: " << inicial << " | Ocupa: " << sizeof(char) << " byte"
       << endl;
  cout << "bool estaVivo: " << boolalpha << estaVivo
       << " | Ocupa: " << sizeof(bool) << " byte" << endl;
  cout << endl;

  cout << "=== 2. INICIALIZACIÓN MODERNA CON LLAVES {} ===" << endl;
  // En C++ moderno (desde C++11), la forma recomendada de inicializar variables
  // es usando llaves: tipo variable{valor};
  // ¿Por qué? Porque el compilador te protege de perder datos por error.
  int nivel{5};

  // Si hicieras: int error{3.99}; -> El compilador da ERROR porque 3.99
  // perdería decimales. Con int error = 3.99; -> C++ tradicional lo trunca en
  // silencio a 3 sin avisar.
  cout << "Nivel inicializado con {}: " << nivel << endl << endl;

  cout << "=== 3. VARIABLES INMUTABLES (const) ===" << endl;
  // 'const' garantiza que una variable nunca pueda ser modificada después de
  // nacer. Si intentás cambiarla, el compilador no te deja compilar.
  const int MAX_JUGADORES{4};
  const float GRAVEDAD{9.81f};
  // MAX_JUGADORES = 5; // <- Si descomentás esto, da error de compilación.
  cout << "Max jugadores (constante): " << MAX_JUGADORES << endl;
  cout << "Gravedad (constante): " << GRAVEDAD << endl << endl;

  cout << "=== 4. DEDUCCIÓN AUTOMÁTICA CON 'auto' ===" << endl;
  // 'auto' le dice al compilador: "Mirá el valor que le asigné y deducí el tipo
  // vos mismo". En C++ moderno se usa mucho cuando los tipos se vuelven largos.
  auto monedas = 250;        // El compilador sabe que es int
  auto ratio = 0.75f;        // El compilador sabe que es float
  auto texto = "Hola mundo"; // Puntero a const char
  cout << "monedas (auto deducido como int): " << monedas << endl;
  cout << "ratio (auto deducido como float): " << ratio << endl;
  cout << "texto (auto deducido como const char*): " << texto << endl << endl;

  cout << "=== 5. CONVERSIÓN DE TIPOS (static_cast) ===" << endl;
  // En C++, si dividís dos enteros: 7 / 2 el resultado es 3 (descarta el
  // decimal).
  int puntos = 7;
  int partidas = 2;
  double promedioEntero =
      puntos / partidas; // ¡Cuidado! Da 3.0 porque ambos eran int.
  cout << "Division entera (7 / 2): " << promedioEntero << " (pierde el .5)"
       << endl;

  // Para evitarlo, usamos static_cast<nuevo_tipo>(variable) para convertir:
  double promedioReal = static_cast<double>(puntos) / partidas;
  cout << "Division con static_cast: " << promedioReal << " (calculo exacto)"
       << endl;

  return 0;
}
