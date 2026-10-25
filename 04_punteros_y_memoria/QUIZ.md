# Quiz — Módulo 04: Punteros y Memoria Dinámica

Respondé primero sin mirar las respuestas del final. Cada pregunta tiene UNA opción correcta.

---

**1. Después de `int x = 5; int* p = &x;`, ¿qué guarda `p`?**

- a) El valor `5`
- b) Una copia de `x`
- c) La dirección de `x`
- d) El tamaño de `x`

**2. ¿Cuál de estas desreferenciaciones es segura si `p` puede ser nulo?**

- a) `*p`
- b) `p[0]`
- c) `if (p != nullptr) { *p; }`
- d) `reinterpret_cast<int>(p)`

**3. ¿Qué diferencia hay entre `const int* p` y `int* const p`?**

- a) Son exactamente lo mismo
- b) En el primero no podés escribir `*p`; en el segundo no podés cambiar `p`
- c) En el primero no podés cambiar `p`; en el segundo no podés escribir `*p`
- d) El primero es un puntero a `const int`; el segundo también, pero más rápido

**4. Con `int x = 1; int* p = &x; int** pp = &p;`, ¿qué hace `**pp = 9;`?**

- a) Cambia `pp`
- b) Cambia `p` para que apunte a otro lado
- c) Cambia `x` a 9
- d) No compila

**5. Pasás `int arr[10]` a una función `void f(int* p)`. Dentro de `f`, ¿qué devuelve `sizeof(p)`?**

- a) `10 * sizeof(int)`
- b) `sizeof(int)`
- c) El tamaño de un puntero, no el del arreglo
- d) Depende de cuántos elementos se hayan inicializado

**6. Si `int* p` apunta a `datos[0]`, ¿qué es `*(p + 2)`?**

- a) El byte 2 de `datos[0]`
- b) `datos[2]`
- c) `datos[1]`
- d) Siempre `end`

**7. ¿Cuál de estos objetos vive en el heap?**

- a) Una variable local `int x`
- b) Un `std::vector` declarado como local (el objeto cabeza)
- c) El bloque que devuelve `new int[100]`
- d) Una variable global `int g`

**8. ¿Qué problema tiene esta función?**

```cpp
int* f() {
    int local = 3;
    return &local;
}
```

- a) Ninguno: el puntero se copia y sigue válido
- b) Devuelve un puntero colgante: `local` muere al salir de `f`
- c) Fuga de memoria
- d) Doble liberación

**9. Tenés `int* p = new int[5];`. ¿Cómo lo liberás?**

- a) `delete p;`
- b) `free(p);`
- c) `delete[] p;`
- d) `p = nullptr;`

**10. ¿Qué captura la lambda `[x](int y) { return x + y; }`?**

- a) Una referencia a `x`
- b) Una copia de `x` hecha al crear la lambda
- c) El valor de `y`
- d) Nada: las lambdas no capturan

**11. ¿Por qué conviene `catch (const std::exception& e)` en lugar de `catch (std::exception e)`?**

- a) Porque el compilador lo exige
- b) Porque evita copiar y evita el slicing del tipo dinámico
- c) Porque `const&` atrapa también los enteros
- d) Porque así se relanza automáticamente

**12. ¿Cuál es el reemplazo más directo y seguro de `new int[n]` cuando necesitás un arreglo dinámico?**

- a) `int* p = new int[n]` con `delete` (sin corchetes)
- b) `std::vector<int> v(n)`
- c) `reinterpret_cast<int*>(malloc(n))`
- d) Un arreglo local `int v[n]`

---

## Respuestas

1. **c)** Un puntero guarda una dirección; `*p` es el valor y `&p` es la dirección del propio puntero.
2. **c)** Comprobar `p != nullptr` antes de desreferenciar evita el caso "no apunta a nada".
3. **b)** `const int*` es puntero a dato constante; `int* const` es puntero constante a dato modificable.
4. **c)** `*pp` es `p` y `**pp` es `x`; la doble indirección llega hasta el entero.
5. **c)** El arreglo decae a puntero al pasarlo; el tamaño del arreglo se pierde y hay que pasarlo aparte.
6. **b)** La aritmética escala por `sizeof(int)`: `p + 2` apunta a `datos[2]`.
7. **c)** `new` reserva en el heap; las locales van al stack y las globales a data/bss.
8. **b)** `local` es automática: su frame desaparece al volver y el puntero queda colgando.
9. **c)** `new[]` se libera con `delete[]`; mezclarlo con `delete` o `free` es comportamiento indefinido.
10. **b)** `[x]` captura por valor una copia; los cambios posteriores de `x` no se reflejan.
11. **b)** La referencia evita la copia y conserva el tipo derivado; atrapar por valor puede hacer slicing.
12. **b)** `std::vector<int>` guarda tamaño, crece y libera solo en su destructor.
