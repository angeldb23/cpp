/* =============================================================================
   MÓDULO 04 - LECCIÓN 17: Diagnóstico y std::vector como reemplazo seguro
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      Cuando sospechás un problema de memoria, no adivines: medí.

      Valgrind (Linux) detecta fugas, accesos inválidos y uso de memoria sin
      inicializar:
          g++ -std=c++20 -g programa.cpp -o programa
          valgrind --leak-check=full --show-leak-kinds=all ./programa

      AddressSanitizer (ASan) es más rápido y se integra en el compilador:
          g++ -std=c++20 -g -fsanitize=address,undefined programa.cpp -o programa
          ./programa
      UBSan (undefined) detecta desbordes de enteros, alineación incorrecta,
      punteros nulos y otras operaciones indefinidas.

      Revisión manual: por cada new/new[], preguntá
        - ¿quién es el dueño?
        - ¿se libera en todos los caminos, incluidas las excepciones?
        - ¿se usa después de liberar?
        - ¿se libera dos veces?

   2. LA SOLUCIÓN ESTRUCTURAL: std::vector
      El reemplazo directo de `new T[n] / delete[]` es std::vector<T>:
        - guarda size y capacity,
        - libera en su destructor (aunque haya excepciones),
        - se puede copiar y devolver por valor,
        - crece solo con push_back.
      Para solo observar datos sin ser dueño, std::span (módulo 03). Para
      propiedad exclusiva en el heap, std::unique_ptr (módulo 07).

   3. ERRORES COMUNES
      - Confiar en que "compila y no explota" significa que no hay UB.
      - Ejecutar valgrind sobre un binario sin -g: los reportes pierden líneas.
      - Mezclar ASan con Valgrind: no se usan juntos.
      - Ignorar el primer error de ASan y mirar solo el resumen.
      - Reemplazar un arreglo por vector y seguir usando índices sin comprobar.

   4. PARA PROFUNDIZAR
      https://valgrind.org/docs/manual/quick-start.html
      https://clang.llvm.org/docs/AddressSanitizer.html
   ============================================================================= */
#include <algorithm>
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <vector>

// Versión con puntero crudo: el llamador debe liberar.
int* crearCrudo(std::size_t n) {
    int* datos = new int[n];
    for (std::size_t i = 0; i < n; ++i) datos[i] = static_cast<int>(i + 1);
    return datos;
}

// Versión segura: devuelve por valor, sin delete manual.
std::vector<int> crearVector(std::size_t n) {
    std::vector<int> datos(n);
    for (std::size_t i = 0; i < n; ++i) datos[i] = static_cast<int>(i + 1);
    return datos;
}

int suma(const std::vector<int>& datos) {
    int total = 0;
    for (int x : datos) total += x;
    return total;
}

// at() comprueba límites y lanza; [] no comprueba.
int elementoEn(const std::vector<int>& datos, std::size_t i) {
    return datos.at(i);
}

int main() {
    std::cout << "=== 1. MISMO TRABAJO, DOS FORMAS ===\n";
    int* crudo = crearCrudo(5);
    std::cout << "crudo: ";
    for (std::size_t i = 0; i < 5; ++i) std::cout << crudo[i] << " ";
    std::cout << "\n";
    delete[] crudo;  // obligatorio
    crudo = nullptr;

    std::vector<int> seguro = crearVector(5);
    std::cout << "vector: ";
    for (int x : seguro) std::cout << x << " ";
    std::cout << "\n";
    std::cout << "suma = " << suma(seguro) << "\n";

    std::cout << "\n=== 2. CRECER SIN SABER EL TAMAÑO FINAL ===\n";
    std::vector<int> dinamico;
    for (int i = 1; i <= 6; ++i) {
        dinamico.push_back(i * i);
        std::cout << "  size = " << dinamico.size()
                  << ", capacity = " << dinamico.capacity() << "\n";
    }

    std::cout << "\n=== 3. at() VS [] ===\n";
    std::cout << "elementoEn(seguro, 0) = " << elementoEn(seguro, 0) << "\n";
    try {
        std::cout << elementoEn(seguro, 99) << "\n";
    } catch (const std::out_of_range& e) {
        std::cout << "  at() lanzó: " << e.what() << "\n";
    }
    // seguro[99] sería UB: no hay chequeo.

    std::cout << "\n=== 4. COPIAR, ORDENAR Y DEVOLVER ===\n";
    std::vector<int> copia = seguro;
    std::sort(copia.begin(), copia.end(), [](int a, int b) { return a > b; });
    std::cout << "copia ordenada: ";
    for (int x : copia) std::cout << x << " ";
    std::cout << "\n";
    std::cout << "el original no cambió: " << seguro.front()
              << " ... " << seguro.back() << "\n";

    std::cout << "\n=== 5. DEDUPLICAR CON sort + unique + erase ===\n";
    std::vector<int> repetidos = {3, 1, 3, 2, 1, 3};
    std::sort(repetidos.begin(), repetidos.end());
    auto fin = std::unique(repetidos.begin(), repetidos.end());
    repetidos.erase(fin, repetidos.end());
    std::cout << "sin repetidos: ";
    for (int x : repetidos) std::cout << x << " ";
    std::cout << "\n";
    std::cout << "unique solo colapsa vecinos iguales: por eso se ordena antes\n";

    std::cout << "\n=== 6. CHECKLIST ANTES DE ENTREGAR ===\n";
    std::cout << "1. ¿queda algún new sin su delete?\n";
    std::cout << "2. ¿el delete es del mismo tipo (new[] -> delete[])?\n";
    std::cout << "3. ¿se usa algún puntero después de liberarlo?\n";
    std::cout << "4. ¿se libera también si una función lanza?\n";
    std::cout << "5. ¿podrías usar vector o unique_ptr en lugar de new?\n";

    std::cout << "\n=== 7. COMANDOS PARA VERIFICAR ===\n";
    std::cout << "valgrind --leak-check=full ./programa\n";
    std::cout << "g++ -g -fsanitize=address,undefined programa.cpp -o programa\n";

    return 0;
}
