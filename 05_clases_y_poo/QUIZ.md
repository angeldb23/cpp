# Quiz — Módulo 05: POO I: Clases, Encapsulamiento y Regla de 0/3/5

Respondé primero sin mirar las respuestas del final. Cada pregunta tiene UNA opción correcta.

---

**1. ¿Cuál es la ÚNICA diferencia técnica entre `struct` y `class` en C++?**

- a) `struct` no puede tener métodos
- b) `class` ocupa más memoria por objeto
- c) El acceso por defecto: `public` en `struct`, `private` en `class`
- d) `struct` no puede tener destructor

**2. Un miembro `private` es invisible para...**

- a) El compilador, pero visible en el binario generado
- b) Toda la memoria: es imposible leerlo con punteros
- c) El acceso desde afuera de la clase, lo que el compilador rechaza
- d) El linker, que lo borra del binario

**3. Con lista de inicialización, ¿en qué orden se inicializan los miembros?**

- a) En el orden en que aparecen en la lista
- b) En el orden de DECLARACIÓN dentro de la clase
- c) Alfabético por nombre
- d) El que inicialice primero el constructor por defecto

**4. `Enemigo e();` (con paréntesis) ¿qué es?**

- a) Un objeto Enemigo construido por defecto
- b) La declaración de una función que devuelve `Enemigo`
- c) Un error de compilación siempre
- d) Un constructor delegante

**5. ¿Qué garantiza el destructor de una variable local respecto a RAII?**

- a) Que el recurso se libera solo si el scope termina con `return`
- b) Que el recurso se libera al salir del scope, en CUALQUIER salida
- c) Que la memoria se limpia al cerrar el programa
- d) Que se ejecuta solo si vos lo llamás

**6. ¿Qué pasa si aplicás `copia = original;` sobre un tipo con copia superficial y recursos crudos?**

- a) Cada objeto tiene su recurso independiente
- b) Se copia el contenido byte a byte
- c) Los dos objetos comparten el recurso y doble liberación al morir
- d) El compilador lo rechaza si es demasiado grande

**7. ¿Cuándo se llama al constructor de COPIA en vez del `operator=` de copia?**

- a) `b = a;` con `b` ya existente
- b) Cuando el objeto muere
- c) `Bateria copia = original;` (variable recién nacida)
- d) Al pasar un objeto por referencia

**8. La regla de TRES exige implementar...**

- a) Tres constructores, dos destructores y un operador
- b) Destructor, constructor de copia y `operator=` de copia
- c) Constructor por defecto, de copia y de movimiento
- d) Tres métodos con el mismo nombre

**9. La regla de CERO dice que tu clase NO debe declarar operaciones especiales CUANDO...**

- a) La clase no tiene recursos propios que administrar
- b) No puedes escribir el destructor
- c) No hay herencia (módulo 06)
- d) La clase es pequeña (menos de 3 miembros)

**10. ¿Qué es `explicit` en un constructor de un parámetro?**

- a) Impide que el compilador use ese constructor como conversión implícita
- b) Hace que la función sea más rápida
- c) Obliga al constructor a ejecutarse en tiempo de compilación
- d) Marca el constructor como amistoso con otras clases

**11. ¿Qué logra `friend` en una clase?**

- a) Comparte los privados con TODAS las funciones del archivo
- b) Da a UNA función/clase concreta acceso a los miembros privados
- c) Convierte en públicos los atributos private
- d) Expone la clase a los hijos (herederos)

**12. ¿Qué promete un método `const`?**

- a) Que el objeto no cambie según el compilador (this como const*)
- b) Que el método es rápido
- c) Que puede ser llamado solo desde la propia clase
- d) Que no devuelve nada

**13. ¿Qué significa el parámetro `mutable` en un atributo?**

- a) Que el valor nunca cambia
- b) Que puede escribirse en métodos const (detalle de administración)
- c) Solo funciona en clases mayores
- d) Que se copia con `= default`

**14. `Valor(const Valor&) = delete;` ¿qué causa?**

- a) Que la copia se permita pero con advertencia de compilación
- b) Que la copia produzca un error de compilación claro si se usa
- b) La copia falla en runtime con una excepción
- d) El constructor se convierte en privado

**15. Para `Vector2` con `operator+=` y `operator+` libre en términos de `+=`: ¿por qué conviene ese diseño?**

- a) Porque `+=` se corre más rápido que `+`
- b) Porque hay una sola lógica de suma y menor riesgo de divergencia
- c) Porque `+` debe ser siempre método miembro
- d) Porque el compilador no permite dos operadores con la misma lógica

**16. ¿Por qué `operator<<(std::ostream&, const Perro&)` debe ser función libre y no método?**

- a) El stream queda a la IZQUIERDA (no es de la clase Perro)
- b) Los streams no aceptan operadores de objetos
- c) El método const no permite encadenar
- d) ostream no compilaría con friend

**17. ¿Qué pasa con `sizeof(Clase)` si agregás un `static int` como miembro?**

- a) Aumenta en sizeof(int)
- b) No cambia: el static es un símbolo global, no parte del objeto
- c) Se duplica
- d) Depende del compilador

**18. ¿Qué diferencia importante tiene `enum class` frente a `enum` clásico?**

- a) No admite valores con nombre propios
- b) No se convierte implícitamente a int y sus valores llevan el nombre del enum
- c) Tiene métodos automáticamente
- d) Ocupa un bit por valor

**19. `enum class Id : unsigned char` ¿para qué sirve el `: unsigned char`?**

- a) Indica el nombre del archivo donde se declara
- b) Fija el TIPO SUBYACENTE: sizeof y rango exacto (1 byte)
- c) Convierte todo a char en runtime
- d) Marca el enum como estático

**20. La ODR: si `withdraw(...)` se declara en `cuenta.h` y se define en `cuenta.cpp`...**

- a) Puede tener DOS definiciones distintas con cuerpos distintos
- b) Se define UNA sola vez en el programa y el linker conecta los usos
- c) El compilador elige la mejor de las dos
- d) No hay problema: el include duplica la definición gratis

---

## RESPUESTAS

1. **c** — el acceso por defecto es la única diferencia del lenguaje (convención: struct para datos, class para tipos con invariantes).
2. **c** — privado el compilador lo controla; la memoria sigue ahí para quien no use el lenguaje.
3. **b** — la declaración dentro de la clase manda (gcc `-Wreorder` avisa si la lista está en otro orden).
4. **b** — most vexing parse: es una declaración de función. Se escribe `Enemigo e;` o `Enemigo e{};`.
5. **b** — RAII: el destructor corre en cada salida del scope (return, throw, break — módulo 04).
6. **c** — el puntero copiado apunta a la MISMA memoria: doble `delete` y cambios que "se contagian".
7. **c** — el `=` en una variable recién creada es CONSTRUCCIÓN (ctor de copia); en una variable viva, ASIGNACIÓN.
8. **b** — las tres piezas de cualquier clase que administra un recurso crudo.
9. **a** — con recursos encapsulados (std::string, std::vector), la copia implícita ya es correcta.
10. **a** — corta la conversión implícita `Nave x = 100;`, permitiendo solo construcción directa.
11. **b** — es un permiso puntual para una función o clase concreta (no se hereda ni es transitivo).
12. **a** — `this` pasa a `const Clase*`: el compilador rechaza escribir el estado.
13. **b** — para caches/contadores de uso: detalle de administración, no estado observable.
14. **b** — `= delete` quita la sobrecarga del overload set y el error del compilador es claro.
15. **b** — una sola lógica de suma: `+` copia, muta con `+=` y devuelve; imposible que diverjan.
16. **a** — el receptor izquierdo es el stream, no un Perro: tiene que ser función libre.
17. **b** — el static pertenece a la CLASE (un solo símbolo en el programa).
18. **b** — alcance propio (`Estado::Vivo`) y sin conversión implícita a int.
19. **b** — fija el tipo subyacente (storage y rango garantizado).
20. **b** — una declaración puede repetirse; la definición existe una única vez (ODR), y el linker une.
