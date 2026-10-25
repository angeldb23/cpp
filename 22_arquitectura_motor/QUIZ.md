# MÓDULO 22 — QUIZ: Arquitectura de Motor: ECS, Scene Graph y Recursos

Respuestas al final.

---

## Preguntas Conceptuales

**1. En un ECS basado en Archetypes, ¿qué ocurre cuando se añade un componente a una entidad?**
A) El componente se añade al final del array del archetype actual
B) La entidad migra a un archetype diferente que incluye el nuevo componente
C) Se crea un nuevo archetype solo para esa entidad
D) No se puede añadir componentes en runtime

**2. ¿Cuál es la ventaja principal de usar Sparse Set (dense/sparse) para almacenar componentes?**
A) Menor uso de memoria que vector
B) O(1) add, remove, contains y iteración densa sobre dense[]
C) Mejor cache locality que archetype storage
D) No requiere generation para detectar use-after-free

**3. En la propagación de "dirty flags" en un Scene Graph, ¿cuándo se marca dirty un hijo?**
A) Solo cuando cambia su LocalTransform
B) Cuando cambia el WorldTransform del padre O su propio LocalTransform
C) Cada frame incondicionalmente
D) Solo cuando se hace SetParent

**4. ¿Qué información almacena un WorldTransform en column-major 3x3 para 2D?**
A) Solo posición world (x, y)
B) Matriz completa: escala, rotación, traducción en m[0]..m[8]
C) Solo rotación y escala
D) Posición y rotación, sin escala

**5. En un Resource Manager con handles (id + generation), ¿qué detecta el generation mismatch?**
A) Que el asset no está cargado
B) Uso de handle tras unload/reload (use-after-free)
C) Que el archivo en disco cambió
D) Que el ref_count llegó a 0

**6. ¿Por qué LoadAsync retorna un handle inmediatamente en lugar de bloquear?**
A) Para que el hilo principal pueda usar el handle (pasárselo a sistemas) mientras carga en background
B) Porque la carga siempre es instantánea
C) Para evitar usar memoria
D) Porque no soporta callbacks

**7. En un Command Buffer con sort keys, ¿qué orden produce front-to-back para opacos?**
A) Depth ascendente (cerca → lejos)
B) Depth descendente (lejos → cerca)
C) Por material_id ascendente
D) Por pipeline_id descendente

**8. ¿Por qué los objetos transparentes requieren back-to-front (painter's algorithm)?**
A) Para early-Z optimization
B) Porque el blending alpha necesita acumular color de atrás hacia adelante
C) Para agrupar por material
D) Porque la GPU lo exige

**9. En Instanced Rendering, ¿qué dato típicamente va en el instance buffer?**
A) Vértices del mesh (compartidos)
B) Índices del mesh
C) Matriz world + material_id + color por instancia
D) Texturas

**10. En un Job System con DAG, ¿cuándo se encola un job dependiente?**
A) Inmediatamente al crearse
B) Cuando su pending_deps llega a 0 (todos sus predecesores completaron)
C) Al final del frame
D) Cuando el hilo principal lo pide

**11. ¿Qué hace ParallelFor en un Job System propio (sin TBB)?**
A) Usa std::execution::par internamente
B) Divide el rango [0, count) en chunks y lanza una task por worker con std::async
C) Ejecuta todo en el hilo principal
D) Requiere OpenMP

**12. En serialización estilo Flatbuffers con component mask, ¿qué permite el mask?**
A) Saber qué componentes están presentes sin leerlos todos
B) Comprimir los datos
C) Encriptar la entidad
D) Ordenar las entidades

**13. En un Prefab System con overrides, ¿qué ocurre con campos no sobrescritos?**
A) Se ignoran (valoran 0)
B) Usan el valor del template/prefab
C) Causan error de deserialización
D) Se leen del archivo original

**14. ¿Qué detecta un Scene Diff semántico (no byte-a-byte)?**
A) Cambios en componentes específicos, parent changes, add/remove entity
B) Solo si el archivo cambió de tamaño
C) Solo entidades añadidas
D) Hash del buffer completo

**15. En hot-reload de scripts C++ (.so), ¿qué se debe hacer con el estado del script antes de dlclose?**
A) Nada, se pierde
B) Serializar estado (ScriptSerialize), restaurar tras dlopen nuevo (ScriptDeserialize)
C) Guardar en archivo temporal automáticamente
D) Mantener el handle abierto

**16. ¿Por qué el Resource Manager usa ref_count en AssetSlot?**
A) Para saber cuándo hacer unload (ref_count == 0)
B) Para limitar cuántos handles pueden existir
C) Para ordenar por uso
D) Para detectar memory leaks

**17. En la iteración cache-friendly de un Sistema ECS con Archetypes, ¿por qué no hay branching?**
A) El compilador optimiza los if
B) Todos los componentes del archetype existen garantizados (With = filtro de archetype)
C) Se usa branch prediction
D) Los componentes son opcionales

**18. ¿Qué ventajas tiene usar C++ como scripting (hot-reload .so) vs Lua/Python?**
A) Performance nativo, type safety, IDE support, mismo lenguaje que el motor
B) Menor uso de memoria
C) Más fácil de sandboxear
D) No requiere compilación

**19. En el orden de passes de un Render Graph típico, ¿qué va ANTES: Lighting o Transparent?**
A) Transparent
B) Lighting (deferred/forward opaque)
C) Post-process
D) Shadow maps

**20. ¿Qué permite std::execution::par en algoritmos STL?**
A) Ejecución paralela automática (requiere TBB en GCC)
B) Ejecución en GPU
C) Vectorización SIMD
D) Ejecución distribuida

---

## Preguntas de Análisis de Código

**21. Dado este código de SetParent, ¿qué lista se actualiza incorrectamente?**
```cpp
void SetParent(Registry& reg, Entity child, Entity new_parent) {
    // ... unlink old parent ...
    if(new_parent) {
        Entity last = 0;
        if(auto* fc = reg.Get<FirstChild>(new_parent)) {
            Entity cur = fc->child;
            while(cur) { last = cur; cur = reg.Get<NextSibling>(cur)->next; }
        }
        if(last) { reg.Add<NextSibling>(last).next = child; reg.Add<PrevSibling>(child).prev = last; }
        else { reg.Add<FirstChild>(new_parent).child = child; }
        reg.Add<Parent>(child).parent = new_parent;
    }
}
```
A) FirstChild del nuevo padre
B) NextSibling del último hijo previo
C) PrevSibling del nuevo hijo
D) **No actualiza PrevSibling del primer hijo existente cuando se añade al final**

**22. En UpdateWorldTransforms con stack DFS, ¿por qué se pasa parent_world como const float*?**
A) Para evitar copias de matriz 3x3 (9 floats) en cada push
B) Porque la matriz es inmutable
C) Para que el compilador haga inline
D) Es un bug, debería ser por valor

**23. En LoadAsync del ResourceManager, ¿por qué se usa job_queue_ (thread pool) en lugar de std::async?**
A) std::async no soporta callbacks
B) Control sobre número de hilos, cola con prioridades, reutilización de threads
C) std::async es más lento
D) No hay diferencia

**24. En el sort key de CommandBuffer: `(pass << 48) | (pipeline << 32) | (depth << 16) | material`, ¿por qué depth va en bits 31:16?**
A) Para que agrupe por pipeline primero, luego por depth, luego material
B) Porque depth es más importante que pipeline
C) Para que material sea lo más significativo
D) Es arbitrario

**25. En SceneDiffer::ComputeDiff, al comparar optional<Transform>, ¿qué hace SerializeComponent + memcmp?**
A) Compara punteros
B) Serializa ambos y compara bytes → detecta si el componente cambió
C) Compara solo el mask
D) Siempre devuelve true

---

## Respuestas

1.  **B** — Archetype = combinación única de componentes. Add/Remove migra la entidad a otro archetype.
2.  **B** — Sparse Set: dense[] para iteración, sparse[] (entity→index) para O(1) lookup. Generations en dense para safety.
3.  **B** — Dirty = world matrix inválida. Cambio en padre invalida hijos; cambio en local invalida a sí mismo y descendientes.
4.  **B** — Matriz 3x3 column-major: [sx*cos, -sy*sin, 0; sx*sin, sy*cos, 0; x, y, 1] en m[0]..m[8].
5.  **B** — Handle {id, gen}. Slot {data, gen}. Si slot.gen != handle.gen → handle inválido (asset reciclado/reload).
6.  **A** — Handle válido ya para pasar a sistemas; callback/notificación cuando data lista. No bloquea frame.
7.  **A** — Opaque: front-to-back (menor depth primero) para early-Z. Transparent: back-to-front.
8.  **B** — Alpha blending: dst = src*srcA + dst*(1-srcA). Orden correcto = pintar de atrás hacia adelante.
9.  **C** — Instance buffer: per-instance data (world matrix, material_id, color). Vertex/index buffer = mesh compartido.
10. **B** — pending_deps contador atómico. Cada predecesor al completar hace fetch_sub; si llega a 0 → enqueue.
11. **B** — Fallback sin TBB: divide rango en chunks, std::async(std::launch::async) por worker.
12. **A** — Component mask (bitmask) indica qué componentes siguen en el stream. Lectura condicional.
13. **B** — Override = solo campos que cambian. Ausentes = usan default del prefab. Merge en Instantiate.
14. **A** — Diff semántico: itera entidades, compara component mask y valores por tipo. Output: ops Add/Remove/Modify/ChangeParent.
15. **B** — ScriptDefine ScriptSerialize/Deserialize. Motor llama serialize antes de unload, deserialize tras reload. Entity handles (con gen) válidos tras reload.
16. **A** — Acquire/Release en handle (RAII AssetRef). Cuando último handle suelta (ref_count 0) → unload o mover a cache LRU.
17. **B** — Query With<T...> = filtro de archetype (mask match). Archetype match garantiza que T existe → Get() no null, sin branching.
18. **A** — Código nativo, cero overhead VM, type safety compile-time, refactoring, hot-reload .so. Tradeoff: no sandbox real (usar WASM si hace falta).
19. **B** — Orden típico: Shadow → GBuffer → Lighting → Transparent → PostProcess → UI.
20. **A** — std::execution::par delega a TBB (GCC) o backend propio (Clang). Verificar -ltbb. Fallback: thread pool propio.

21. **D** — Al añadir al final, el primer hijo existente (fc->child) debería actualizar su PrevSibling al nuevo último. El código no lo hace.
22. **A** — Evita copiar 9 floats (36 bytes) por push en stack. Puntero a matriz del padre (ya en memoria) es más eficiente.
23. **B** — Job queue propio: work-stealing, prioridades, profiling, cancellation tokens, frame budget. std::async crea thread por task (overhead).
24. **A** — Sort key jerárquico: pass (más significativo) → pipeline (minimiza state changes) → depth (early-Z / painter) → material (batching).
25. **B** — SerializeComponent escribe bytes del optional (si tiene valor). memcmp de buffers serializados = comparación de valor real.