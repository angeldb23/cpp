# 📝 QUIZ — Módulo 19: Matemática para Motores (vectores, matrices y quats)

*Respuestas al final. Antes de mirar, intentá contestar de cabeza; después
contrasto con las lecciones (cada pregunta nombra la lección de respaldo).
El cierre del módulo ES la librería: mate3d.hpp con 17 lecciones, 12
ejercicios resueltos y este quiz.*

1. **P/I** (L01): ¿Qué produce `glm::mat4 m{}` (sin más) y cuál es la forma de la IDENTIDAD en GLM? ¿Qué bug clásico del módulo 20 aparece si te confundís?
2. **P/I** (L01): En GLM (column-major), ¿qué es `m[2]` y qué significa `m[2][1]`? ¿Cómo se escribe "la columna 3 contiene la traslación (tx, ty, tz)"?
3. **código** (L02): ¿Para qué existe `glm::value_ptr` y por qué en `glUniformMatrix4fv` del módulo 20 se pasa SIEMPRE `transpose = GL_FALSE` con GLM?
4. **conceptual** (L03): ¿Qué mide `dot(a, b)` cuando a y b son unitarios, y por qué su COSTO no depende del largo de los vectores?
5. **código** (L03): ¿Por qué para "quién está más cerca" alcanza `largoCuadrado` y no hace falta `sqrt`? ¿Cuándo el sqrt ES obligatorio?
6. **código** (L04): Si `n` NO es unitario, `n * dot(v, n)` está mal. ¿Cuál es la fórmula GENERAL de la proyección (con `dot(n, n)` en el denominador) y por qué se reduce a la corta cuando n es unitario?
7. **código** (L04): Al reflejar un rayo contra un piso, ¿qué componente del vector se invierte y cuál queda intacta? ¿Por qué la fórmula exige la normal NORMALIZADA?
8. **P/I** (L05): ¿Qué significa "column-major" EN MEMORIA (qué está contiguo)? ¿Cómo se indexa en mate3d con `at(c, r)` y por qué es CLAVE que sea el MISMO layout de GLM?
9. **código** (L05): SRT: ¿en qué orden se compone (¿T·R·S o el inverso?) y con el punto (1,0,0), S=2, R=90°, T=(10,0,0), recorre las tres etapas — qué punto da y por qué un orden distinto daría un punto DIFERENTE?
10. **P/I** (L06): ¿Por qué la cadena modelo→clip se ESCRIBE `P * V * M * p` y el orden de APLICACIÓN se lee "al revés"? ¿Qué cambia entre pasar un PUNTO (w=1) y una DIRECCIÓN (w=0)?
11. **código** (L06): el clip `w <= 0` ¿qué significa exacto (¿delante o detrás de la cámara?) y por qué NUNCA se divide por w en ese caso?
12. **código** (L07): ¿Cómo se compone el MUNDO de un hijo (¿mundoPadre * localHijo o al revés?) y qué DEMUESTRA el contraejemplo del compuesto al revés? ¿Y cómo se convierte un punto DEL MUNDO en coordenadas del nodo (¿cuál operación)?
13. **conceptual** (L08): Dirty flags: si muevo el CUERPO (nodo del medio de raíz→cuerpo→propela→escudo), ¿cuántos nodos recomputan y por qué la raíz NO? ¿Y si muevo la HOJA?
14. **código** (L08): En la descomposición TRS de una world: ¿qué son `largo(col0)` y `atan2(col0.y, col0.x)`? ¿En qué caso el método NO funciona (¿rotación de 3 ejes? ¿corte/shear?) y cuál es el camino general (¿quats?).
15. **P/I** (L09): ¿De dónde sale el HALF-ANGLE del quat eje-ángulo? Escribí el quat de θ=90° en z como (x,y,z,w) y explicá por qué q y -q son LA MISMA rotación (doble cobertura).
16. **código** (L09): La fórmula `v' = v + w·t + cross(eje, t)` con `t = 2·cross(eje, v)`: ¿por qué NO construye ninguna matriz? ¿Cuándo CONVIENE convertir el quat a matriz igual (mat3DesdeQuat)?
17. **código** (L10): ¿Por qué si `dot(q0, q1) < 0` el nlerp/slerp niega q1 antes de mezclar? ¿Qué pasa geométricamente SIN ese flip (¿por qué el camino "gira por el otro lado")?
18. **P/I** (L10): NLERP vs SLERP: ¿cuándo alcanza el nlerp y qué error tolera? ¿Qué operaciones evita y por qué el slerp con θ ≈ 0 DEBE caer al nlerp (entre el divide y qué)?
19. **código** (L11): Perspectiva: ¿por qué el FOV es VERTICAL y de dónde sale el ANCHO (¿el aspect?)? ¿Qué hace `m(2,3) = -1` en el W del clip y cómo de ahí sale "w > 0 = el punto está delante" en right-handed?
20. **código** (L11): Gribb-Hartmann: ¿de qué filas del VP salen los 6 planos y qué combinación (+/-) corresponde a cada plano? ¿Por qué la NORMAL del plano no hace falta normalizarla para el test "punto dentro"?
21. **conceptual** (L11): near=0.01 / far=100000: ¿qué rompe exactamente (¿dónde se pierde la precisión del depth?) y qué criterio usa el curso para elegir n/f? ¿Qué es del FRUSTUM cuando la proyección es ORTOGRÁFICA (¿forma?) y qué usos tiene?
22. **código** (L12): Slab del ray-AABB: ¿qué hacen `inv = 1/d` con d == 0 (¿por qué el infinito IEEE funciona?) y por qué el SWAP de t0/t1 es necesario cuando `d < 0`?
23. **código** (L12): Ray-esfera: ¿qué distingue `disc < 0`, `= 0` y `> 0`? Si el ORIGEN está adentro de la esfera, ¿qué raíz se devuelve (¿por qué la menor t sale negativa o cero?)?
24. **código** (L13): Möller-Trumbore: enunciá las 4 guardias de salida ¿qué detecta cada una (¿paralelo? ¿fuera por u? ¿fuera por v o la suma? ¿triángulo detrás?) y qué REPRESENTAN (u, v)?
25. **P/I** (L13): ¿Para qué sirven exactamente las baricéntricas después del rayo-toca (¿qué se interpola con ellas?) y cómo sale w0, w1, w2 desde (u, v)?
26. **código** (L14): ¿Por qué un `k = 0.1` por FRAME cambia de velocidad cuando el dt cambia? ¿Cuál es la fórmula exponencial (independiente del dt) y qué significa su k?
27. **código** (L14): ¿Qué garantiza `smoothstep` (¿C¹: pendiente cero en los bordes?) y qué agrega la `quintic` (¿C²?) para el noise de L16? ¿Dónde va el `clamp01` y por qué SIEMPRE antes de los easing?
28. **código** (L15): Catmull-Rom: ¿qué da CR que Bezier NO (¿pasar POR los waypoints?) y por qué importa que editor y gameplay coincidan? ¿De dónde salen las tangentes m0/m1 y qué resuelve el CLONADO de extremos (¿el primer y último segmento?)?
29. **código** (L15): ¿Por qué el parámetro t de la cúbica NO es proporcional a la distancia recorrida? ¿Qué construye `tablaDeArco` (¿qué es acum.back()? ¿cuántas entradas?) y cómo ayuda el fmod para rutas que ciclan?
30. **P/I** (L16): ¿Qué rompe volver a SEMBRAR mt19937 cada frame y qué garantizan las semillas (¿mismo seed = mismo mundo?)? ¿Por qué el value noise usa HASH DE ENTEROS y qué función interpola entre las cuadrículas (¿la quintic de qué lección?)?
31. **conceptual** (L17): ¿Qué cambia al pasar de AoS (`struct Ent { pos, dir, vida }`) a SoA (`vector<pos>, vector<dir>...`)? ¿Por qué la contigüidad paga (¿cache del CPU + autovectorización SSE?) y qué regla "no forzar" aplica al SIMD de GLM (y a fast_inverse_sqrt)?

---

## 🎯 Respuestas

> Las respuestas están en orden y remiten a la lección. Las fórmulas de
> este módulo están TODAS probadas por los 12 ejercicios: si algo te
> sorprende acá, echale un ojo al ejercicio correspondiente.

1. **(L01)** `glm::mat4 m{}` vale **TODO CERO** (no identidad). La identidad es
   `glm::mat4(1.0f)`. El bug clásico: inicializar la matriz del modelo con
   `{}` creyendo que es identidad — multiplicar por la matriz CERO aplasta
   todo a (0,0,0) y el objeto DESAPARECE del render.

2. **(L01)** `m[2]` es la TERCERA COLUMNA (un vec4 completo, no la fila 2).
   `m[2][1]` es "columna 2 (tercera), fila 1 (segunda)". En column-major el
   índice EXTERIOR elige columna porque las columnas son contiguas en
   memoria (idéntico a mate3d: `m[c*4 + r]`). La traslación se escribe
   `m[3] = vec4(tx, ty, tz, 1.0f)` (con w=1).

3. **(L02)** `value_ptr(m)` entrega el array plano de 16 floats EN EL ORDEN
   DE MEMORIA de GLM (columnas contiguas). Como ese es exactamente el
   layout que espera OpenGL (column-major), `glUniformMatrix4fv(loc, 1,
   GL_FALSE, value_ptr(m))` en CADA frame: con `GL_TRUE` el driver
   transpondría en caliente, pagando per-frame y al revés del sentido.

4. **(L03)** `dot(a, b) = |a||b|·cos(θ)`. Con ambos NORMALIZADOS es exacto
   `cos(θ)`: 1 = misma dirección, 0 = perpendiculares, -1 = opuestos. El
   dot son 3 multiplicaciones y 2 sumas SIN raíz: su costo no depende de
   los largos, y por eso es la operación que el módulo reutiliza en
   proyección, reflexión, culling y raycast.

5. **(L03)** `largoCuadrado` (la suma de cuadrados, sin `sqrt`) es la
   version BARATA para comparar: la raíz es monotónica creciente, así que
   "¿a² < b²?" responde lo mismo que "¿a < b?" sin el costo. El sqrt es
   obligatorio solo cuando la distancia REAL se mide (normalización, HUD).

6. **(L04)** `proyección = n * (dot(v, n) / dot(n, n))`. Funciona con
   cualquier n ≠ 0: `dot(n, n)` es |n|² y compensa en el denominador el
   largo de la normal. Con n unitario |n|² = 1 y queda la fórmula corta
   `n * dot(v, n)` que el resto del módulo reutiliza.

7. **(L04)** El rebote invierte la componente NORMAL (la parte del vector
   que apunta contra el piso: `dot(v, n̂)·n̂`) y deja intacta la
   TANGENCIAL (la velocidad a lo largo del piso). La normal UNITARIA es
   obligatoria porque el factor "2·dot" pesa la inversión en unidades de
   la normal: con una normal de otro largo, daría otro rebote. Completa:
   `v - 2·dot(v, n̂)·n̂`.

8. **(L05)** Column-major: los 4 floats de una COLUMNA son contiguos en la
   RAM (m[0..3] es la primera). En mate3d se indexa `at(c, r)` o por
   `m[c*4 + r]` (columna c, fila r) y se escribe columnas con
   `ponerColumna`. Es CLAVE que sea el MISMO layout de GLM: entre la lib
   del curso y GLM NO hay copias ni transposiciones (el módulo 20 sube
   las matrices directo a la GPU).

9. **(L05)** `SRT` = `T * R * S`: con column-major, el factor de la
   DERECHA aplica PRIMERO. El punto (1,0,0): S=2 → (2,0,0); R=90° →
   (0,2,0); T → (10,2,0). El orden estándar de un objeto de juego: escalar
   en local, girar en local, trasladar al mundo. (Al revés: T primero
   movería el punto sin rotarlo: daría otro punto — el test SRT orden
   del ej 03 lo verifica con el punto (1,0,0)).

10. **(L06)** Se ESCRIBE `P * V * M * p` pero se APLICA de derecha a
    izquierda: M primero (local→mundo), V después (mundo→cámara), P al
    final (cámara→clip). Un punto con w=1: la TRASLACIÓN lo mueve; una
    dirección con w=0: la traslación NO la mueve (transformarPunto vs
    transformarDireccion de mate3d).

11. **(L06)** `w <= 0` del clip significa que el punto está EN o DETRÁS del
    plano de la cámara (w = -z de vista en RH: delante, z de vista < 0
    ⇒ w > 0). NO se divide por w: la división daría un NDC con el signo
    invertido/comprimido hacia el infinito; el llamador descarta el
    punto con su w y el clipper del GPU maneja los triángulos que
    atraviesan el plano.

12. **(L07)** `mundoHijo = mundoPadre * localHijo` (el orden CORRECTO):
    el local del hijo aplica PRIMERO (el punto se coloca EN EL ESPACIO
    del hijo y termina en el espacio del padre). Al revés da un punto
    DISTINTO (el contraejemplo del ej 05: (10,2,0) vs (12,0,0)). Y para
    pasar de mundo → local del nodo: aplicar la INVERSA RÍGIDA:
    `transformarPunto(inversaRigida(mundoDelNodo), puntoMundo)` — la
    ida-vuelta cierra EXACTA (el RoundtripDeUnPunto del ej 05).

13. **(L08)** Mover el CUERPO marca dirty al cuerpo y su SUBÁRBOL (contagio
    hacia abajo): recomputan 3 (cuerpo, propela, escudo); la RAÍZ no
    (su mundo no depende del cuerpo). Mover la HOJA recomputa SOLO esa
    (1 nodo). El hit frecuente "nada cambió" = que el frame es 0.

14. **(L08)** `largo(col0)` = la ESCALA X (cuánto se estiró el eje X del
    local en el mundo) y `atan2(col0.y, col0.x)` = la ROTACIÓN Z (hacia
    dónde apunta el eje X local en el plano XY). La TRASLACIÓN es la
    columna 3 directa. NO funciona con rotación en 3 EJES ni con corte
    (shear): para el caso general descomponé con cuaterniones
    (`glm::quat_cast(glm::mat3(m))` verificado contra GLM 1.0.1).

15. **(L09)** El quat representa el GIRO POR LA MITAD: por eso la fórmula
    es (eje·sin(θ/2), cos(θ/2)); el "aplicar" compone el giro completo
    (ida con q, vuelta con q*). θ=90° en z: (0, 0, sin45°, cos45°) =
    (0, 0, 0.7071, 0.7071). Y -q: el MISMO giro mirado del otro lado de
    la esfera unitaria 4D: por eso casiIgualQuat compara |dot| ~ 1, y la
    doble cobertura es la BASE del camino corto del slerp (no un bug).

16. **(L09)** La fórmula directa es puro producto cruz: NO construye una
    matriz 3x3 por llamada (27 multiplicaciones menos). Es la expansión
    de q · v · q* (el conjugado escapa del medio). CUÁNDO conviene la
    matriz: cuando el MISMO quat rota MUCHOS vectores por frame (huesos
    de un esqueleto, partículas de un anchor): convertís UNA vez con
    mat3DesdeQuat (L10) y rotás con matriz.

17. **(L10)** q1 y -q1 son LA MISMA rotación pero el arco ENTRE q0 y -q1
    es el LARGO de la esfera. Sin el flip, un 60° interpolado a mitad
    camina 150° "por el otro lado" y da la rotación de -150° en vez de
    la de 30°: la cámara gira al revés de lo esperado. Con "si dot < 0,
    q1 = -q1" el camino SIEMPRE es el ARCO MÍNIMO.

18. **(L10)** Nlerp alcanza para ángulos chicos: la divergencia cuerda vs
    arco es ~O(θ²); a 10° queda < 0.001 rad (lo comprueba el ej 08).
    Evita acos + sin por mezcla (1 sqrt en total). El slerp con θ ≈ 0
    DIVIDE por sin(θ) → NaN; por eso la implementación correcta cae en
    nlerp para θ < 1e-5 (lección 10, línea 109).

19. **(L11)** El fov es el ángulo VERTICAL porque el semialto de la
    pirámide es tan(fovY/2) y el SEMIANCHO sale del aspect (ancho/alto):
    `m(0,0) = f/aspect`. `m(2,3) = -1` hace que el W del clip sea -z de
    vista: los puntos DELANTE de la cámara (z de vista < 0) ganan w > 0,
    y los de "detrás" (w <= 0) son los que NO se dividen (L06) ni se
    dibujan.

20. **(L11)** Los 6 planos salen de las combinaciones de la FILA 3 del VP
    (= P·V) con las filas 0..2: fila3+fila0 = izquierda, fila3-fila0 =
    derecha; fila3±fila1 = abajo/arriba; fila3±fila2 = cerca/lejos.
    El test del punto es un solo dot `a·x + b·y + c·z + d >= 0`: lo que
    manda es el SIGNO, así que la normal NO se normaliza (se normaliza
    SOLO cuando querés MEDIR distancia al plano).

21. **(L11)** Con far/near desproporcionado, la precisión del DEPTH BUFFER
    se concentra en el near y las superficies LEJANAS pelean el mismo
    delta (z-fighting). Se elige n/f por escena: interior (0.1/100),
    shooter (0.5/300), espacio (far enorme con near proporcional).
    Ortográfica: rayos PARALELOS (sin achique por distancia) → el
    frustum es un PARALELEPÍPEDO y sirve para HUD, mapas y UI/menú.

22. **(L12)** Con d == 0 el rayo es PARALELO al par de paredes del slab y
    nunca las cruza: `1/0 = ±infinito` de IEEE deja los t0/t1 en ±inf y
    el max/min sigue funcionando (el intervalo no restringe). El swap es
    necesario porque si `d < 0` el rayo ENTRA por la pared "max" y sale
    por la "min" (el rango queda dado vuelta): sin el swap, las cajas
    cruzadas "en el otro sentido" marcan falso negativo.

23. **(L12)** La cuadrática en t del rayo-esfera: `disc < 0` = la recta NO
    toca; `disc = 0` = tangente (una sola raíz); `disc > 0` = dos puntos
    de corte (se toma el MENOR t > EPS: el más cercano). Origen ADENTRO:
    la raíz menor sale t <= 0 (está "hacia atrás" del rayo) y se devuelve
    la OTRA raíz (t2), que es el punto por donde el rayo SALE de la
    esfera (el ej 10 lo prueba con la esfera radio 10 en (0,0,5): t=15).

24. **(L13)** Las guardias: (1) `|a| < EPS`: rayo PARALELO al plano del
    triángulo (o triángulo degenerado); (2) `u < 0 || u > 1`: fuera del
    lado e1; (3) `v < 0 || u + v > 1`: fuera por e2 o por la "hipotenusa"
    (la suma se lleva a 1 en el vértice); (4) `t <= EPS`: el triángulo
    está DETRÁS del origen del rayo. (u, v) son las COORDENADAS
    BARICÉNTRICAS del punto de impacto (Möller las entrega gratis: no
    hay que calcular el plano aparte).

25. **(L13)** Interpolan ATRIBUTOS dentro del triángulo: el punto de
    impacto es `p0 + u·e1 + v·e2`, y los pesos (w0, w1, w2) = (1-(u+v),
    u, v) suman 1 y mezclan normal/UV/color de los TRES vértices en esa
    posición (el vertex shader de GL lo hace gratis en el módulo 20; en
    CPU-side la hacés a mano, como el ej 11).

26. **(L14)** Un `k` por frame mide "0.1 del resto POR FRAME": si el dt del
    frame cambia (hitch, vsync off), la cámara AVANZA a otra velocidad
    (el seguimiento se arruina). La fórmula independiente del dt:
    `k = 1 - exp(-suavizado·dt)`: el k pasa a ser "suavizado por SEGUNDO"
    (unidades/segundo): aunque el dt varíe, el acercamiento por segundo
    es idéntico (sin jitters por frame rate).

27. **(L14)** `smoothstep = t²(3-2t)`: pendiente CERO en los extremos (C¹:
    arranca y termina suaves, sin saltitos). La QUINTIC (6t⁵−15t⁴+10t³)
    agrega la SEGUNDA derivada cero en los bordes (C²): por eso el value
    noise de L16 la usa (superficie SIN pliegues visibles entre
    cuadrículas). El `clamp01` va SIEMPRE ANTES del easing: un t fuera
    de [0..1] en un lerp EXTRAPOLA (útil para overshoot, pero hay que
    elegirlo a propósito).

28. **(L15)** Catmull-Rom PASA POR los waypoints (con Bezier el camino casi
    nunca toca los puntos): los puntos del editor SON los del gameplay.
    Las tangentes salen de los VECINOS: `m0 = 0.5·(p2 - p0)` en p1 y
    `m1 = 0.5·(p3 - p1)` en p2. El CLONADO de extremos (p0 = q[0] en el
    primer segmento, p3 = q[N-1] en el último) resuelve los bordes; con
    solo 2 waypoints la curva queda casi recta (lo que una ruta de 2
    puntos DEBE ser).

29. **(L15)** El `t` de la cúbica NO es proporción de la distancia: donde
    la curva está COMPRIMIDA avanza menos metros por unidad de t. La
    tabla de arco muestrea K veces por segmento (el ej 12 usa 10) y
    acumula lados: `acum.back()` es la LONGITUD TOTAL y el tamaño de la
    tabla es K·(N-1)+1 (31 con 4 waypoints y K=10). El punto a distancia
    s: encontrar el intervalo [acum[j], acum[j+1]) (el ej pide BÚSQUEDA
    BINARIA), fracción → sParam = (j + frac)/K → evaluarCamino. El
    `fmod(s, total)` abre rutas CÍCLICAS (patrullas infinitas) gratis
    (y con total == 0 devolver q[0]: la ruta degenerada).

30. **(L16)** Re-sembrar cada frame RESETA el estado del generador: todos
    los frames tiran el MISMO stream y el "random" se repite. `mt19937`
    se siembra UNA vez por sistema. Mismo seed ⇒ mismo mundo exacto:
    REPLAYS y determinismo entre plataformas (la contraparte es que el
    orden de tiradas debe ser igual). El value noise 2D usa HASH de
    enteros (Wang-ish) porque es barato y determinista: dado
    (x, y, semilla) SIEMPRE el mismo pseudo-random; la interpolación
    entre las cuadrículas usa la QUINTIC de la lección 14 (C² en los
    bordes: sin escalones entre celdas).

31. **(L17)** AoS: cada entidad trae pos+dir+vida seguidos; un loop que
    SOLO lee posiciones "pasa" por vida/estado (el cache L1/L2 trae
    bytes que NO se usan). SoA: posiciones contiguas en un array propio →
    solo caen al cache los floats RELEVANTES y el loop autovectoriza
    (SSE: la máquina del curso NO tiene AVX/AVX2 y el SSE igual paga).
    Regla "no forzar" del SIMD: GLM usa el SIMD disponible en silencio
    (NO se lo exige: el código del juego es el MISMO con y sin). Y
    `fast_inverse_sqrt` es HISTORIA de la era Athlon: el hardware moderno
    trae rsqrt/rcp más simples y más precisos; igual que el resto, solo
    se aplica con MEDICIÓN: si el loop no es el hotspot, no paga.

---

*Fin del quiz. El módulo 19 queda: librería mate3d.hpp (Vec2/3/4, Mat3/4
column-major, Quat con Hamilton y doble cobertura, inversaRigida), 17
lecciones (los 10 temas), 12 ejercicios con sus 12 soluciones (todos
pasan por `./verificar.sh 19`) y este quiz. El siguiente (módulo 20:
OpenGL 3.3 Core) consume TODO: las matrices P·V·M de este módulo se
suben a la GPU y el vértice vuela por un VAO.*
