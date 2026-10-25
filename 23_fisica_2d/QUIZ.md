# MÓDULO 23 — QUIZ: Física 2D

30 preguntas. Respuestas al final.

---

## Preguntas Conceptuales

**1. En un bucle de tiempo fijo, ¿qué representa `alpha = acumulador / dtFijo`?**
A) La cantidad de pasos que faltan para terminar el frame
B) La fracción de paso transcurrida desde la última simulación, usada para interpolar el render
C) El damping aplicado al cuerpo
D) La escala de gravedad del mundo

**2. ¿Qué es el "spiral of death" y cómo se evita?**
A) Un bug de Box2D al dormir cuerpos; se evita desactivando el sleep
B) Que cada frame tarde más que el anterior porque se acumulan pasos; se evita limitando el máximo de pasos por frame
C) Que la gravedad crece sin límite; se evita con damping
D) Que los cuerpos ganan energía al chocar; se evita con restitución 1

**3. ¿Por qué se interpola solo el render y no la simulación entre pasos fijos?**
A) Porque la simulación interpolada es más rápida
B) Porque mover la lógica a una posición interpolada rompe colisiones y gameplay (la física quedaría un paso atrás)
C) Porque Box2D no acepta floats interpolados
D) Porque el render necesita menos precisión y no importa

**4. Al elegir el tamaño de celda de un spatial hash, ¿qué compromiso existe?**
A) Celdas muy chicas generan muchísimas celdas; muy grandes generan muchos candidatos por celda
B) No hay compromiso: siempre conviene el tamaño del objeto más chico
C) El tamaño debe ser múltiplo de la gravedad
D) Solo importa para cuerpos estáticos

**5. ¿Por qué la celda de un spatial hash usa `floor(x / cellSize)` y no un cast a int?**
A) Porque el cast es más lento
B) Porque el cast trunca hacia cero: -0.3 y +0.3 caerían en la misma celda, y deben estar en celdas distintas
C) Porque floor devuelve float
D) Porque el cast no compila con negativos

**6. En Sweep and Prune (SAP), ¿cuál es la ventaja de ordenar por un eje y usar insertion sort entre frames?**
A) El orden casi no cambia entre frames (coherencia temporal), así que insertion sort queda cerca de O(n)
B) SAP no necesita ordenar
C) Insertion sort es el único algoritmo estable
D) Evita calcular AABBs

**7. ¿Cuál es el peor caso de un quadtree con objetos amontonados en el mismo punto?**
A) O(1)
B) Se subdivide hasta el límite de profundidad y todos los objetos quedan en la misma hoja: degrada a O(n²)
C) Se convierte automáticamente en grid
D) No admite más de 4 objetos

**8. ¿Cuántos ejes de separación hay que probar en SAT entre dos polígonos convexos de n y m lados?**
A) 2 (x e y)
B) n + m (una normal por arista de cada polígono)
C) n × m
D) 4, sin importar los lados

**9. En SAT, ¿cómo se elige la normal de colisión?**
A) La primera arista del polígono A
B) El eje de menor solape, orientado desde el centro de A hacia el centro de B
C) Siempre (0, 1)
D) La suma de todas las normales

**10. ¿Qué mide la profundidad (depth) devuelta por SAT?**
A) La distancia entre centros
B) El solape mínimo de las proyecciones sobre el eje de colisión
C) El área de intersección
D) La velocidad relativa

**11. Para círculo vs polígono, ¿qué punto se busca primero?**
A) El vértice más lejano al centro del círculo
B) El punto del polígono más cercano al centro del círculo (en arista o vértice)
C) El centro del polígono
D) El punto medio del AABB

**12. Si el centro del círculo está DENTRO del polígono, ¿cómo se calcula la normal?**
A) Se devuelve (0, 0) y no hay colisión
B) Se usa la dirección desde el centro hacia el punto más cercano de la frontera y la profundidad es radio + distancia
C) Se usa siempre la normal de la primera arista
D) Se invierte la gravedad

**13. ¿Qué ventaja tiene GJK frente a SAT?**
A) GJK solo funciona con cajas
B) GJK usa funciones de soporte y sirve para cualquier forma convexa, incluidos círculos y cápsulas, sin enumerar ejes
C) GJK calcula el área de contacto
D) GJK no necesita iteraciones

**14. En un impulso normal, ¿qué término hace que un golpe descentrado genere rotación?**
A) La masa lineal
B) El producto cruz entre el vector desde el centro de masa al punto de contacto y la normal (r × n)
C) El damping
D) La gravedad

**15. ¿Para qué sirve un umbral de restitución (restitution threshold)?**
A) Para que los rebotes muy lentos no generen jitter infinito; por debajo del umbral se trata como inelástico
B) Para aumentar la restitución con la velocidad
C) Para desactivar la fricción
D) Para dormir el cuerpo

**16. La fricción de Coulomb en un solver de impulsos se implementa como...**
A) Restar una constante a la velocidad
B) Un impulso tangencial acotado: |j_t| <= μ * j_n
C) Multiplicar la masa por μ
D) Un joint de distancia

**17. ¿Qué hacen `percent` (0.2) y `slop` (0.01) en la corrección de posición de Baumgarte?**
A) `percent` reparte la corrección entre los cuerpos y `slop` ignora penetraciones mínimas para evitar jitter
B) `percent` es el coeficiente de restitución y `slop` la fricción
C) `percent` limita la velocidad y `slop` la gravedad
D) Ambos son solo para debug

**18. ¿Qué es warm starting?**
A) Arrancar el mundo con todos los cuerpos dormidos
B) Reutilizar los impulsos acumulados del paso anterior como punto de partida del solver, acelerando la convergencia
C) Calentar la GPU antes de renderizar
D) Aplicar gravedad dos veces al inicio

**19. ¿Por qué un cuerpo cinemático tiene `invMass = 0` pero igual se mueve?**
A) Porque las fuerzas e impulsos no deben moverlo, pero su velocidad propia sí integra la posición
B) Porque es un error de Box2D
C) Porque su masa es negativa
D) Porque no tiene collider

**20. ¿Cuál es la inercia de una caja de masa m, ancho w y alto h alrededor de su centro?**
A) m*w*h
B) m*(w² + h²)/12
C) 0.5*m*r²
D) m*(w + h)/2

**21. ¿Por qué se usa `v *= 1/(1 + damping*dt)` en vez de `v *= (1 - damping*dt)`?**
A) Porque la segunda puede dar velocidad negativa con damping*dt > 1; la primera es estable para cualquier dt
B) Porque la primera es más precisa en todos los casos
C) Porque la segunda no compila
D) Porque Box2D lo exige

**22. En Euler semi-implícito, ¿cuál es el orden correcto?**
A) `pos += v*dt` y después `v += a*dt`
B) `v += a*dt` y después `pos += v*dt` (con la velocidad nueva)
C) Actualizar posición y velocidad en paralelo
D) Solo importa el orden si hay rotación

**23. ¿Qué restringe un joint prismático?**
A) Solo la distancia entre dos puntos
B) Permite deslizar por un eje y bloquea la rotación relativa (a diferencia de revolute, que permite rotar)
C) Fija posición y rotación por completo
D) Solo la velocidad angular

**24. ¿Por qué el impulso de un motor se limita a `maxTorque * dt`?**
A) Para que el motor no teletransporte la velocidad angular: simula un torque físico finito
B) Para que el motor nunca arranque
C) Para convertir grados a radianes
D) Porque el solver lo requiere para joints de distancia

**25. ¿Dónde se guardan los anchors de un joint y por qué?**
A) En world space, para no recalcularlos
B) En espacio local de cada cuerpo, y se transforman a world cada paso; guardarlos en world los rompe al rotar
C) En el centro del mundo
D) En coordenadas de pantalla

**26. En Box2D, ¿qué representa `world.Step(dt, velocityIterations, positionIterations)`?**
A) Un paso de simulación con iteraciones del solver de velocidad y de posición; más iteraciones = más precisión y más costo
B) Un paso de render
C) La carga de un nivel
D) La creación del mundo

**27. ¿Qué callback de `b2ContactListener` usarías para una plataforma de un solo sentido?**
A) `BeginContact`
B) `PreSolve`, llamando a `contact->SetEnabled(false)` según la dirección
C) `PostSolve`
D) `EndContact`

**28. La regla de filtros de Box2D entre los fixtures A y B es...**
A) `(catA & maskA) && (catB & maskB)`
B) `(catA & maskB) && (catB & maskA)` (salvo groupIndex)
C) Solo `catA == catB`
D) Siempre colisionan si los AABB se tocan

**29. En la integración ECS, ¿cuál es la "única fuente de verdad" de la posición?**
A) El sprite
B) `Transform2D` (el body integra y al final sincroniza; el render y el gameplay leen el transform)
C) La cámara
D) El input

**30. ¿Por qué se guarda el conjunto de pares activos entre frames al emitir eventos?**
A) Para dibujar contactos
B) Para emitir `BeginContact` una sola vez por par y `EndContact` al separarse, en vez de repetir begin cada frame
C) Para calcular la gravedad
D) Para ordenar los cuerpos por masa

---

## Preguntas de Análisis de Código

**31. Con `dtFijo = 1/120`, un frame tarda `1/30` s y el acumulador arranca en 0, ¿cuántos pasos se ejecutan y cuánto queda en el acumulador?**
A) 2 pasos, acumulador 0
B) 4 pasos, acumulador 0
C) 4 pasos, acumulador 1/120
D) 1 paso, acumulador 3/120

**32. SAT entre dos cajas AABB de lado 2 centradas en (0,0) y (1.5,0). ¿Profundidad y normal (de A hacia B)?**
A) 0.5, (1, 0)
B) 0.5, (-1, 0)
C) 2.0, (1, 0)
D) 0.25, (0, 1)

**33. Un cuerpo de masa 2 kg recibe una fuerza de 4 N durante 0.5 s partiendo del reposo. Con Euler semi-implícito, ¿velocidad y desplazamiento?**
A) v = 1 m/s, x = 0.5 m
B) v = 2 m/s, x = 1 m
C) v = 0.5 m/s, x = 0.25 m
D) v = 1 m/s, x = 0.25 m

**34. `baumgarte(normal=(1,0), penetracion=0.1, invMassA=1, invMassB=1, percent=0.2, slop=0.01)`. ¿Delta de A en x?**
A) -0.009
B) +0.009
C) -0.018
D) -0.1

**35. A tiene `category=0x0002`, `mask=0x0001`; B tiene `category=0x0001`, `mask=0x0002`; ambos `groupIndex=0`. ¿Colisionan?**
A) Sí, cada uno acepta la categoría del otro
B) No, las máscaras no coinciden consigo mismas
C) Solo si sus AABB se tocan y son dinámicos
D) Depende de la gravedad

**36. Dos AABB se solapan en el frame 1 y siguen solapando en los frames 2 y 3. ¿Cuántos eventos `BeginContact` se emiten con el patrón de pares activos?**
A) 3
B) 1
C) 0
D) 2

**37. Un transform tiene `posAnterior.x = 0`, `pos.x = 10`, `anguloAnterior = 0`, `angulo = 2`. ¿Qué devuelve `paraDibujar(alpha=0.25)`?**
A) x=2.5, ángulo=0.5
B) x=10, ángulo=2
C) x=7.5, ángulo=1.5
D) x=2.5, ángulo=1.5

**38. Una caja de semiextensiones 0.5×0.5 rotada 45° centrada en el origen. ¿Semiextensión del AABB en x?**
A) 0.5
B) 0.7071
C) 1.0
D) 0.25

**39. Un cuerpo con `dampingLineal = 1.0`, `v = 10 m/s`, `dt = 1 s`, sin fuerzas. ¿Velocidad después del paso?**
A) 0 m/s
B) 5 m/s
C) 9 m/s
D) 10/e² m/s

**40. Un cuerpo dinámico de masa 4 kg e inercia 8 kg·m². ¿`invMass` e `invInertia`?**
A) 4 y 8
B) 0.25 y 0.125
C) 0.125 y 0.25
D) 1 y 1

---

## Respuestas

1. **B** — El acumulador puede contener una fracción de paso; `alpha` interpola entre la posición anterior y la actual para que el render no se vea a saltos.
2. **B** — Si el frame tarda más que el paso fijo, se acumulan pasos y el siguiente frame tarda aún más. Se limita con un máximo de pasos por frame (y se descarta el excedente).
3. **B** — La lógica debe correr en posiciones exactas de paso fijo; interpolar la simulación introduce colisiones y eventos desfasados. Solo el render interpola.
4. **A** — Celdas chicas = más memoria y más recorrido; celdas grandes = más pares candidatos por celda. Se busca un tamaño cercano al de los objetos típicos.
5. **B** — `static_cast<int>` trunca hacia cero, así que -0.3 y +0.3 caerían en la celda 0. `floor` garantiza celdas contiguas y correctas para negativos.
6. **A** — Entre frames los AABB se mueven poco; el arreglo queda casi ordenado e insertion sort es casi O(n). SAP además elige el eje de mayor dispersión.
7. **B** — Sin un límite de profundidad ni una estrategia para objetos coincidentes, todos caen en la misma hoja y la detección vuelve a ser cuadrática.
8. **B** — Por cada arista de cada polígono se prueba su normal. En cajas son 4 ejes (2 por caja, con duplicados).
9. **B** — Se toma el eje de menor solape y se invierte si no apunta de A hacia B, para que el impulso separe los cuerpos.
10. **B** — Es el solape mínimo de las proyecciones; no es el área ni la distancia entre centros.
11. **B** — Se busca el punto más cercano de la frontera del polígono al centro del círculo; si está en una arista o vértice se usan ambos.
12. **B** — Si el centro está dentro, la normal sale del centro hacia la frontera y la penetración es el radio más la distancia a esa frontera (el círculo está "enterrado").
13. **B** — GJK trabaja con funciones de soporte y la diferencia de Minkowski; no enumera ejes. Para profundidad de penetración se complementa con EPA.
14. **B** — El impulso se aplica en un punto; `r × n` acopla la parte lineal con la angular. Sin ese término, todos los golpes serían centrales.
15. **A** — Rebotes de baja velocidad producen jitter y energía numérica. Por debajo del umbral se usa restitución 0.
16. **B** — Se calcula un impulso en la tangente y se limita por el cono de fricción de Coulomb, `|j_t| <= μ j_n`.
17. **A** — `percent` controla qué fracción del error se corrige por paso (0.2 es suave) y `slop` deja una penetración pequeña sin corregir para evitar oscilación.
18. **B** — Los impulsos acumulados se guardan por contacto y se reaplican; el solver converge en menos iteraciones y los stacks son más estables.
19. **A** — `invMass = 0` hace que fuerzas e impulsos no lo afecten, pero su velocidad se integra igual porque la posición la controla el gameplay.
20. **B** — Es el momento de inercia de un rectángulo alrededor del centro. Para un círculo es `0.5*m*r²`.
21. **A** — `1 - damping*dt` puede volverse negativo si `damping*dt > 1` y la velocidad cambia de signo. La forma racional siempre queda entre 0 y 1.
22. **B** — Semi-implícito actualiza primero la velocidad y usa esa velocidad nueva para la posición. Es estable para resortes y es lo que usa Box2D.
23. **B** — Prismatic permite un grado de libertad traslacional y bloquea la rotación relativa. Revolute comparte un punto y permite rotar.
24. **A** — El motor es un actuador con torque máximo; sin el clamp, el impulso cambiaría ω al valor objetivo instantáneamente, sin importar la inercia ni el tiempo.
25. **B** — Los anchors locales se transforman con la rotación/posición de cada cuerpo. Si se guardan en world, al rotar el cuerpo el punto de unión se desplaza y el joint se rompe.
26. **A** — Es un paso de simulación; las iteraciones del solver intercambian precisión por CPU. El dt debe ser fijo y en segundos.
27. **B** — `PreSolve` permite desactivar el contacto antes de resolverlo, sin borrar cuerpos ni usar filtros permanentes.
28. **B** — La regla es simétrica: ambos deben aceptar la categoría del otro. `groupIndex` positivo fuerza colisión y negativo la prohíbe dentro del mismo grupo.
29. **B** — El `RigidBody2D` integra y sincroniza `Transform2D` al final del paso; el render y el gameplay leen el transform. Guardar la posición en dos lugares lleva a desincronización.
30. **B** — Sin memoria de pares activos, cada frame se emitiría un nuevo begin y los triggers se dispararían muchas veces por segundo.
31. **B** — `(1/30) / (1/120) = 4` pasos exactos y el acumulador queda en 0.
32. **A** — Los centros están a 1.5 y las semiextensiones suman 2; el solape es 0.5. La normal va de A (0,0) hacia B (1.5,0): (1,0).
33. **A** — `a = F/m = 2 m/s²`; `v = 2*0.5 = 1 m/s`; `x = v*dt = 1*0.5 = 0.5 m` (con la velocidad nueva).
34. **A** — `magnitud = (0.1 - 0.01)*0.2/2 = 0.009`; A se mueve en `-normal`, así que `deltaA.x = -0.009`.
35. **A** — `(0x0002 & 0x0002) != 0` y `(0x0001 & 0x0001) != 0`. La regla es simétrica y ambos se aceptan; el grupo 0 no la altera.
36. **B** — Se emite un solo `BeginContact` al aparecer el par; los frames siguientes no repiten el evento.
37. **A** — `0*(0.75) + 10*0.25 = 2.5`; `0*0.75 + 2*0.25 = 0.5`.
38. **B** — `ext = |cos45|*0.5 + |sin45|*0.5 = 0.7071`. El AABB de un OBB rotado es mayor que el shape (falsos positivos de broad-phase).
39. **B** — `v *= 1/(1 + 1*1) = 1/2`, así que `10/2 = 5 m/s`. No es exactamente `10/e` porque la fórmula es una aproximación racional.
40. **B** — `invMass = 1/4 = 0.25`; `invInertia = 1/8 = 0.125`.
