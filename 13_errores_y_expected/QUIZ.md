# MÓDULO 13 - QUIZ: Errores y std::expected

## Instrucciones
Responde las siguientes preguntas basándote en las lecciones 1-12 del módulo 13.
Cada pregunta tiene una única respuesta correcta.

---

## Pregunta 1: Canales de error (L01, L02)
¿Cuál de las siguientes afirmaciones describe correctamente la diferencia principal entre el "canal A" (códigos de retorno) y el "canal C" (std::expected)?

A) El canal A usa excepciones, el canal C usa códigos de retorno  
B) El canal A separa el valor del error en dos variables, el canal C los une en un solo tipo de retorno  
C) El canal A es más rápido que el canal C  
D) El canal C requiere unwinding de pila, el canal A no  

---

## Pregunta 2: noexcept y optimizaciones (L03, L04)
¿Por qué es importante marcar `noexcept` el constructor de movimiento de un tipo que se guarda en `std::vector`?

A) Para que el código compile más rápido  
B) Para que `std::vector` use move en lugar de copy durante realocación (strong exception guarantee)  
C) Para evitar que el destructor lance excepciones  
D) Para que el tipo sea trivially copyable  

---

## Pregunta 3: Garantías de excepción (L05)
¿Cuál es la diferencia clave entre la garantía BÁSICA y la garantía FUERTE?

A) La básica no lanza excepciones, la fuerte sí  
B) La básica mantiene invariantes válidos pero el estado puede cambiar; la fuerte garantiza rollback total al estado original  
C) La básica usa copy-and-swap, la fuerte no  
D) Solo la fuerte es compatible con noexcept  

---

## Pregunta 4: Copy-and-swap (L06)
En el idiom copy-and-swap para operador de asignación, ¿por qué se toma el parámetro **por valor**?

A) Para evitar copias innecesarias  
B) Para que el copy constructor haga el trabajo pesado; si falla, *this no se modifica; si OK, swap publica el nuevo estado atómicamente  
C) Porque es más eficiente que tomar por const reference  
D) Para que funcione con tipos move-only  

---

## Pregunta 5: Limpieza parcial en constructores (L07)
Si un constructor adquiere tres recursos (A, B, C) en orden y la adquisición de C falla, ¿qué debe pasar con A y B?

A) No hacer nada, el destructor los limpiará  
B) Liberar A y B manualmente (o via RAII members) antes de relanzar la excepción  
C) Solo liberar B, A se limpia solo  
D) Llamar a std::terminate  

---

## Pregunta 6: Destructores noexcept (L08)
¿Qué sucede si un destructor lanza una excepción **mientras otra excepción se está propagando** (stack unwinding)?

A) La nueva excepción reemplaza a la anterior  
B) Se llaman ambos catch handlers  
C) `std::terminate()` es llamado inmediatamente  
D) La excepción del destructor se ignora silenciosamente  

---

## Pregunta 7: Jerarquías de error (L09)
¿Por qué es preferible usar una jerarquía de clases polimórficas (heredando de `ErrorBase`) en lugar de `enum class` para errores complejos?

A) Porque los enums no pueden tener métodos  
B) Porque permite adjuntar contexto rico (rutas, números de línea, errno, hosts) y distinguir tipos via dynamic_cast  
C) Porque los enums no se pueden usar en std::expected  
D) Porque es más rápido  

---

## Pregunta 8: std::expected vs optional (L10)
¿Cuál es la diferencia fundamental entre `std::optional<T>` y `std::expected<T, E>`?

A) `optional` puede estar vacío, `expected` nunca  
B) `optional` no distingue "no hay valor" de "hay error con razón"; `expected` lleva la razón del error en el tipo E  
C) `expected` usa heap, `optional` usa stack  
D) `optional` es C++23, `expected` es C++20  

---

## Pregunta 9: mini::expected construcción (L11)
En `mini::expected<int, std::string>`, ¿qué construye implícitamente un caso de **éxito** y qué construye un caso de **error**?

A) `expected e = 42;` → éxito, `expected e = "fallo";` → error  
B) `expected e = 42;` → error, `expected e = "fallo";` → éxito  
C) Ambos construyen éxito  
D) Ambos requieren `mini::unexpect`  

---

## Pregunta 10: Operaciones monádicas - and_then (L12)
Dado `auto r = expected<int, string>(5).and_then([](int x) { return expected<int, string>(x * 2); });`, ¿cuál es el valor de `r`?

A) `expected` con error  
B) `expected` con valor 10  
C) `expected` con valor 5  
D) Error de compilación  

---

## Pregunta 11: Operaciones monádicas - transform vs transform_error (L12)
¿Cuál es la diferencia entre `transform(f)` y `transform_error(f)`?

A) `transform` cambia el tipo del error, `transform_error` cambia el tipo del valor  
B) `transform` aplica f al valor si hay éxito; `transform_error` aplica f al error si hay fallo  
C) `transform` es para éxito, `transform_error` es para casos void  
D) Son alias de la misma función  

---

## Pregunta 12: or_else - recuperación local (L12)
¿Cuándo se ejecuta el callable pasado a `or_else(f)`?

A) Siempre, tanto en éxito como en error  
B) Solo si el expected tiene un **valor** (éxito)  
C) Solo si el expected tiene un **error** (fallo)  
D) Nunca, es deprecated  

---

## Pregunta 13: Composición de pipelines (L12)
En un pipeline `leer().and_then(parsear).transform(validar).or_else(default)`, ¿qué pasa si `leer()` falla?

A) `parsear` se ejecuta con valor por defecto  
B) `parsear` y `transform` se saltan; `or_else` recibe el error de `leer()`  
C) `transform` se ejecuta pero `or_else` no  
D) Lanza excepción  

---

## Pregunta 14: RAII y excepciones (L07, L08)
¿Por qué es mejor usar miembros RAII (guards) para cada recurso en una clase que adquirirlos manualmente en el constructor?

A) Porque reduce líneas de código  
B) Porque si el constructor falla tras inicializar algunos members, sus destructores limpian automáticamente sin código manual  
C) Porque evita el uso de try/catch  
D) Porque hace la clase inamovible  

---

## Pregunta 15: Patrones de error en capas (L09, L12)
En una arquitectura por capas (BD → Servicio → API → UI), ¿cuál es la práctica recomendada para `transform_error`?

A) Cada capa transforma el error a su propio lenguaje/dominio (BD → error técnico, API → error de protocolo, UI → mensaje usuario)  
B) Todas las capas usan el mismo tipo de error  
C) Solo la capa UI transforma errores  
D) `transform_error` no debe usarse en capas  

---

## Pregunta 16: std::error_code y categorías (L13)
¿Cuál es la diferencia fundamental entre `std::error_code` y `std::error_condition`?

A) `error_code` es para errores del SO, `error_condition` es para errores lógicos  
B) `error_code` guarda (valor, categoría concreta); `error_condition` guarda (valor, categoría genérica) para comparaciones portables  
C) `error_code` se usa con excepciones, `error_condition` sin excepciones  
D) Son alias del mismo tipo  

---

## Pregunta 17: Categoría propia y default_error_condition (L13)
Si definís una `std::error_category` propia para errores de protocolo de red y sobrescribís `default_error_condition`, ¿qué lográs?

A) Que `error_code` de tu categoría se pueda comparar directamente con `std::errc` (p.ej. `ec == std::errc::connection_reset`)  
B) Que tu categoría sea más rápida  
C) Que los errores se lancen como excepciones automáticamente  
D) Que el `message()` devuelva strings en inglés  

---

## Pregunta 18: APIs sin excepciones y expected<..., error_code> (L14)
¿Por qué es preferible `expected<T, std::error_code>` sobre `std::error_code func(T& out)` para funciones que devuelven un valor y pueden fallar?

A) Porque `expected` es más rápido  
B) Porque el error viaja EN el tipo de retorno, no hay out-param, y se puede encadenar con `and_then`/`transform`/`or_else`  
C) Porque `expected` usa excepciones internamente  
D) Porque `error_code` no puede representar fallos lógicos  

---

## Pregunta 19: Boundary con código C/POSIX (L14)
Al envolver una llamada a función C que setea `errno` en caso de error, ¿cuál es la forma correcta de construir el `std::error_code`?

A) `std::error_code(errno, std::generic_category())`  
B) `std::error_code(errno, std::system_category())`  
C) `std::make_error_code(std::errc(errno))`  
D) `std::error_code(errno)`  

---

## Pregunta 20: try_call y conversión de excepciones (L14)
Un wrapper `try_call` que atrapa `std::bad_alloc` y lo convierte a `std::error_code`, ¿a qué `std::errc` debe mapearlo?

A) `std::errc::not_enough_memory`  
B) `std::errc::resource_unavailable_try_again`  
C) `std::errc::io_error`  
D) `std::errc::invalid_argument`  

---

## Clave de respuestas
1.  B
2.  B
3.  B
4.  B
5.  B
6.  C
7.  B
8.  B
9.  A
10. B
11. B
12. C
13. B
14. B
15. A
16. B
17. A
18. B
19. B
20. A