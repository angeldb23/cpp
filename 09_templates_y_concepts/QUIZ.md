# Quiz — Módulo 09: Templates, Concepts y Perfect Forwarding

Respondé primero sin mirar las respuestas del final. Cada pregunta tiene UNA opción correcta.

---

**1. Llamás `maximo(3, 2.5)` con `template <typename T> T maximo(T a, T b)`. ¿Qué pasa?**

- a) Compila y devuelve `3.0`
- b) Error: T se deduce como `int` y como `double` a la vez
- c) Compila y devuelve `3`
- d) Error en tiempo de ejecución

**2. Usás `maximo` con `int` y con `double` en el mismo programa. ¿Cuántas funciones `maximo` hay en el binario?**

- a) Una, genérica
- b) Dos, una por cada instanciación
- c) Ninguna: los templates no generan código
- d) Depende del sistema operativo

**3. ¿Qué calcula la fold `(args + ... + 0)` cuando el pack `args` está vacío?**

- a) Error de compilación
- b) `0`
- c) Comportamiento indefinido
- d) `nullptr`

**4. `Contador<int>::usos` y `Contador<double>::usos` (miembro `static`) son…**

- a) La misma variable
- b) Dos variables distintas, una por instanciación
- c) Un error: los templates no pueden tener miembros `static`
- d) La misma variable, pero con dos nombres

**5. ¿Cuál es la diferencia entre especialización TOTAL y PARCIAL?**

- a) La total no puede usarse con clases
- b) La total fija un tipo exacto (`template <>` con `int`); la parcial cubre una familia (`T*`, `vector<T>`)
- c) La parcial solo existe para funciones
- d) No hay diferencia

**6. ¿Qué significa SFINAE?**

- a) Un error de sustitución en la firma de un template descarta esa sobrecarga en vez de dar error
- b) Un tipo de puntero inteligente
- c) Una optimización del linker
- d) Una macro del preprocesador

**7. ¿Qué hace `if constexpr` dentro de un template?**

- a) Evalúa la condición en tiempo de ejecución
- b) La rama no elegida NO se compila para ese tipo
- c) Es idéntico a un `if` común
- d) Solo sirve con enteros

**8. Tenés tres sobrecargas de `describir`: una sin restricción, una con `Numerico` y una con `ConSize`. Llamás `describir(5)`. ¿Cuál se elige?**

- a) La sin restricción, por ser la primera
- b) La de `Numerico`, porque es la más restringida que aplica
- c) Ninguna: es ambiguo
- d) La de `ConSize`

**9. ¿Cuál es el principal costo de `std::function` frente a un parámetro de template genérico con callable?**

- a) Ocupa más espacio de código por cada instanciación
- b) La llamada atraviesa una indirección (no se puede hacer inline en general) y puede alocar
- c) No acepta lambdas con captura
- d) Solo acepta punteros a función

**10. Adentro de `template <typename T> void f(T&& x)` llamás `g(x)` sin forward. Si f fue llamada con un rvalue, ¿qué llega a g?**

- a) Un rvalue, porque el argumento original era un rvalue
- b) Un lvalue: x tiene nombre, y con nombre es lvalue siempre
- c) Un xvalue, por las reglas de deducción
- d) Depende de si T es const

**11. ¿Cuándo es `T&&` una referencia universal (forwarding reference)?**

- a) Siempre que la firma tenga doble ampersand
- b) Solo cuando T se está deduciendo para ese parámetro
- c) Solo si T es `auto`
- d) Solo si el parámetro es `std::move`

**12. ¿Qué colapsa `int& &&`?**

- a) `int&&`
- b) `int&`
- c) No colapsa: es un error de compilación
- d) `int`

**13. ¿Qué es `std::forward<T>(x)` por debajo?**

- a) Un `static_cast<T&&>`: condicional según si T dedujo referencia o no
- b) Un movimiento fijo: siempre convierte a rvalue
- c) Una copia con timestamp
- d) Una macro del preprocesador

**14. Una clase tiene `std::string dato() const&` y `std::string dato() &&`. Llamás `MiClase{}​.dato()`. ¿Cuál se elige y por qué?**

- a) `const&`, porque las constantes tienen prioridad
- b) `&&`: la instancia es un temporario (rvalue) y el compilador elige por su categoría
- c) Es ambiguo y no compila
- d) `const&`, porque la copia es más segura

---

## Respuestas

1. **b)** Los dos argumentos deducen T distinto. Se arregla con `maximo<double>(3, 2.5)`.
2. **b)** Cada instanciación genera una función real y separada.
3. **b)** El valor final de la fold (`0`) se usa cuando no hay argumentos.
4. **b)** Cada instanciación es una clase distinta con sus propios `static`.
5. **b)** Total: `template <> struct X<int>`. Parcial: `template <typename T> struct X<T*>`.
6. **a)** "Substitution Failure Is Not An Error": la sobrecarga inválida se descarta en silencio.
7. **b)** Dentro de un template, la rama descartada ni siquiera se compila.
8. **b)** Aplican la genérica y la de `Numerico`; gana la más restringida. `int` no tiene `.size()`, así que `ConSize` no aplica.
9. **b)** El type erasure paga con indirección y (a veces) una alocación; el template con call directo puede inlinearse.
10. **b)** La categoría del nombre x es lvalue; `std::forward<T>` es la que recupera la original.
11. **b)** `T&&` es universal solo cuando T se deduce ahí (parámetro de template, o auto&&). `void f(int&&)` no es.
12. **b)** Regla de cuatro: si aparece un `&`, gana `int&`.
13. **a)** Es un `static_cast<T&&>`: con `T = X&` da lvalue (no mueve), con `T = X` da rvalue (se puede mover).
14. **b)** Con `&&` el compilador elige esa sobrecarga para rvalue (y `&`/`const&` para los vivos): el temporario se lleva su interno movido.
