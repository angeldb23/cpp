# Quiz — Módulo 06: POO II: Herencia, Polimorfismo y vtables

Respondé primero sin mirar las respuestas del final. Cada pregunta tiene UNA opción correcta.

---

**1. `class Mago : public Entidad { ... };` (herencia `public`) se lee:**

- a) "Mago tiene una Entidad"
- b) "Mago es-un Entidad": toda la interfaz pública sigue disponible
- c) "Entidad es-un Mago"
- d) Es solo reutilización de código, sin relación semántica

**2. Al crear un Mago, ¿qué se construye primero?**

- a) Los datos de Mago
- b) La parte Entidad, luego los datos de Mago, y al final el cuerpo del constructor de Mago
- c) Todo a la vez, en el orden de la lista de inicialización
- d) El cuerpo del constructor de Mago, que llama al resto

**3. Un miembro `protected`...**

- a) Es visible para cualquier código
- b) Es invisible para las derivadas
- c) Es visible desde la clase y sus derivadas (por ese subobjeto derivado o más profundo)
- d) Es lo mismo que `public` con otro nombre

**4. Sin `virtual` en `Enemigo::atacar()`, `Enemigo* e = &dragon; e->atacar();` ejecuta...**

- a) `Dragon::atacar()`
- b) `Enemigo::atacar()` (el tipo del PUNTERO decide)
- c) Error de compilación
- d) Ambas, en orden

**5. En la base: `virtual void f() const;`. En el hijo: `void f() const;` sin anotar. ¿Qué pasa?**

- a) No compila
- b) Sobrescribe igual, pero nada te avisa si la firma no coincidiera: el bug silencioso
- c) No sobrescribe: es una función nueva (ocultamiento)
- d) Solo funciona con lambdas

**6. ¿Qué guarda/qué es?**

- a) La vtable es por clase; el vptr es un campo por objeto
- b) La vtable es por objeto; el vptr es por clase
- c) Ambos son por objeto
- d) C++ no tiene tablas: usa macros

**7. `Base* b = new Derivada; delete b;` y el destructor de `Base` NO es virtual:**

- a) Corre todo bien
- b) Solo se destruye la parte base: undefined behavior, recursos colgados
- c) No compila
- d) Corre el destructor de la derivada dos veces

**8. Una clase abstracta es...**

- a) Una clase con muchos `virtual` (a gusto del programador)
- b) Una que tiene al menos un método virtual puro (`= 0`): no se instancia
- c) Una clase sin datos miembros
- d) Una clase que solo compila si nadie la usa

**9. Una interfaz pura (todos los métodos puros, sin datos) aporta a cada objeto...**

- a) Varios datos y una vtable
- b) Un vptr (subobjeto polimórfico) y ningún dato propio
- c) Nada en absoluto
- d) Copias de cada método en el objeto

**10. ¿Cuál de estas copias MANTIENE el polimorfismo, siendo `dragon` un `Dragon`?**

- a) `Enemigo e = dragon;`
- b) `Enemigo& r = dragon;`
- c) `auto copia = *dragonPtr;`
- d) `funcionqueTomaPorValor(dragon);`

**11. El object slicing ocurre cuando...**

- a) Copiás un derivado sobre una variable base POR VALOR: solo llega la "rodaja" base
- b) Usás `delete` sin dtor virtual
- c) Dos bases tienen el mismo método
- d) Compilás sin RTTI

**12. `dynamic_cast<Mago*>(p)` cuando `p` no apunta a un Mago:**

- a) Lanza una excepción siempre
- b) Devuelve `nullptr` (las referencias, en cambio, lanzan `std::bad_cast`)
- c) Es un error de compilación
- d) Construye un Mago nuevo

**13. En el diamante `D : B, C` (y `B, C : A`) SIN herencia virtual, D tiene...**

- a) Un único subobjeto A
- b) DOS subobjetos A: los accesos a miembros de A y el upcast quedan ambiguos
- c) Ningún subobjeto A
- d) Error de compilación obligatorio

**14. La herencia múltiple es más sana cuando las bases extra son...**

- a) Bases cargadas de datos
- b) Interfaces de capacidad (pocos métodos puros, sin datos)
- c) Clases con destructores no virtuales
- d) Copias de la primera

**15. ¿Cuál es el criterio principal composición vs herencia pública?**

- a) La herencia siempre, por reutilizar código
- b) "Es-un" que cumple las garantías de la base → herencia pública; cualquier otra relación → composición
- c) Composición solo para tipos grandes
- d) Son lo mismo compilado distinto

**16. Con `class V : public Enemigo, public Volador` (ambas polimórficas), el objeto...**

- a) Tiene un solo vptr porque una clase solo puede tener una vtable
- b) Lleva un vptr por subobjeto polimórfico, y los punteros a cada base pueden tener direcciones distintas
- c) No puede existir
- d) Va a colapsar en runtime

**17. Si `A` es `final` y escribo `class B : public A { ... };`:**

- a) Advertencia y sigue compilando
- b) Error de compilación: nadie hereda de una clase final
- c) Error en runtime
- d) Compila si B también es final

**18. `class Pila : public std::vector<int>` es mala idea porque...**

- a) `std::vector` no tiene métodos virtual
- b) La pila queda "es-un" vector: expone push_front/insert y rompe su contrato
- c) No compila: no se puede heredar de contenedores
- d) Los mixins lo prohíben

---

## RESPUESTAS

<details>
<summary>Hacé clic para ver (después de responder)</summary>

1. **b** — la herencia pública modela "es-un": el Mago se puede usar
   donde se espere una Entidad. "Tiene-un" es composición.
2. **b** — base primero, luego los miembros de la derivada (por orden
   de declaración) y el cuerpo del ctor al final. Destructor: al revés.
3. **c** — visible para la cadena de clases; la derivada accede por su
   subobjeto derivado (o uno más profundo), no por objeto base arbitrario.
4. **b** — sin `virtual`, despacho estático: manda el tipo de la
   variable/puntero, no el del objeto real.
5. **b** — coincide por la firma: SÍ sobrescribe. El peligro real es que,
   si algún día la firma diverge, nada te avisa: por eso siempre va
   `override`.
6. **a** — una tabla por CLASE con métodos virtual; un puntero (vptr)
   por OBJETO que apunta a la tabla de SU clase real.
7. **b** — el delete consulta el tipo estático; la parte derivada nunca
   se destruye. Único arreglo: destructor virtual en la base.
8. **b** — virtual puro (`= 0`) en al menos un método: objeto imposible,
   derivadas obligadas a implementar todo lo puro para ser concretas.
9. **b** — sin datos solo queda el mecanismo polimórfico: el vptr (y la
   vtable correspondiente) que apunta la clase concreta.
10. **b** — punteros y referencias preservan el tipo dinámico; toda copia
    por valor construye un objeto del tipo de la variable (rebana).
11. **a** — la copia usa el constructor de copia de la base con una
    referencia al derivado: lee solo la parte base.
12. **b** — puntero: nullptr, sin excepciones; referencia: `bad_cast`
    (no existe esa forma de devolver "nada").
13. **b** — dos subobjetos A independientes, doble dato, accesos ambiguos.
    Con `virtual` en B y C, D comparte un único A, construido por D.
14. **b** — capacidades pequeñas sin datos (los contratos de la L10);
    las bases con datos duplicados son el diamante a arreglar.
15. **b** — sustituibilidad real: herencia pública. Solo funcionalidad
    solo sumar una utilidad (sin "es-un"): composición, con menor acoplamiento.
16. **b** — cada base polimórfica suma su subobjeto y su vptr; llamar a
    las interfaces ajusta el puntero con el offset correspondiente.
17. **b** — `final` en la clase cierra la herencia para siempre; en
    métodos, cierra la sobrescritura por debajo.
18. **b** — heredar una interfaz grande rompe el contrato: la pila queda
    con insert/push_front expuestos. La pila COMPONE un vector como
    atributo y expone solo push/pop.

</details>
