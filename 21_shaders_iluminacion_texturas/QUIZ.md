# MÓDULO 21 - QUIZ: Shaders Avanzados, Iluminación Phong y Texturas

> **Instrucciones**: Respondé cada pregunta sin mirar el código. Si no estás seguro, revisá la lección correspondiente. Las respuestas están al final (pero no mires antes de intentarlo).

---

## LECCIONES 01-02: Phong, Blinn-Phong, Gamma/sRGB

**1)** En el modelo de Phong, el término especular es `pow(max(dot(R, V), 0), shininess)`. ¿Qué vector es `R` y cómo se calcula?
- a) `R = reflect(L, N)` donde `L` es el vector hacia la luz
- b) `R = reflect(-L, N)` donde `L` es el vector hacia la luz
- c) `R = 2 * dot(N, L) * N - L`
- d) b) y c) son equivalentes

**2)** ¿Cuál es la diferencia práctica entre Phong y Blinn-Phong?
- a) Blinn-Phong usa el vector halfway `H = normalize(L + V)` en lugar del vector reflejado `R`
- b) Blinn-Phong es más rápido porque evita `reflect()`
- c) Blinn-Phong produce highlights más redondos y evita el problema del "anillo" en ángulos rasantes
- d) Todas las anteriores

**3)** La conversión sRGB → lineal por tramos exacta es:
- a) `c <= 0.04045 ? c/12.92 : pow((c+0.055)/1.055, 2.4)`
- b) `c <= 0.04045 ? c*12.92 : pow((c+0.055)/1.055, 2.2)`
- c) `c <= 0.0031308 ? c/12.92 : pow(c, 2.4)`
- d) `c <= 0.04045 ? c/2.2 : pow(c, 1/2.2)`

**4)** ¿Por qué el framebuffer final debe tener `GL_FRAMEBUFFER_SRGB` habilitado?
- a) Para que el hardware aplique la corrección gamma automáticamente al escribir
- b) Para renderizar en espacio lineal y dejar que el monitor haga la conversión
- c) Ambas son correctas: el shader produce lineal, el framebuffer sRGB convierte a la salida
- d) Solo sirve para texturas, no para el framebuffer

---

## LECCIONES 03-04: Luces direccional, puntual, spot, UBO std140

**5)** La atenuación clásica de una luz puntual es `1.0 / (kc + kl*d + kq*d²)`. Para una luz que llega ~13m con valores `kc=1.0, kl=0.35, kq=0.44`, ¿cuál es la atenuación a 13m?
- a) ~0.0124
- b) ~0.021
- c) ~0.05
- d) ~0.1

**6)** En un spotlight, `smoothstep(cosOuter, cosInner, cosTheta)` hace:
- a) Devuelve 1.0 si `cosTheta > cosInner` (dentro del cono interno)
- b) Devuelve 0.0 si `cosTheta < cosOuter` (fuera del cono externo)
- c) Interpola suavemente entre 0 y 1 en la transición
- d) Todas las anteriores

**7)** En layout `std140`, un `struct Luz { vec3 color; float intensidad; vec3 pos; float alcance; }` ocupa:
- a) 28 bytes
- b) 32 bytes
- c) 40 bytes
- d) 48 bytes

**8)** Un bloque `BloqueLuces { int cuenta; Luz luces[8]; }` en std140 tiene:
- a) `cuenta` @0, `luces[0]` @4, stride 28
- b) `cuenta` @0, padding 12, `luces[0]` @16, stride 32
- c) `cuenta` @0, `luces[0]` @16, stride 28
- d) `cuenta` @0, `luces[0]` @12, stride 32

---

## LECCIONES 05-06: Materiales, PBR Cook-Torrance

**9)** En PBR, el término de Fresnel `F0` para un material se calcula como:
- a) `F0 = mix(0.04, albedo, metalness)`
- b) `F0 = albedo * metalness`
- c) `F0 = 0.04 * (1 - metalness)`
- d) `F0 = vec3(0.04)`

**10)** La función de distribución GGX (D) es:
- a) `D = alpha² / (π * (N·H)² * (alpha² - 1) + 1)²`
- b) `D = (N·H)^shininess`
- c) `D = 1 / (π * (N·H)²)`
- d) `D = alpha / (π * (N·H)²)`

**11)** El término geométrico Smith en PBR usa `k = (roughness + 1)² / 8`. Para `roughness = 0.5`, `k` es:
- a) 0.0625
- b) 0.28125
- c) 0.5
- d) 0.75

**12)** La BRDF de Lambert (difusa pura) en PBR es:
- a) `albedo / π`
- b) `albedo * N·L`
- c) `albedo`
- d) `albedo / 2`

---

## LECCIONES 07-08: TBN, Normal Mapping

**13)** El vector tangente `T` se calcula con Gram-Schmidt a partir de una normal `N` y un vector arbitrario `up`:
- a) `T = normalize(up - N * dot(N, up))`
- b) `T = normalize(cross(N, up))`
- c) `T = normalize(up)`
- d) `T = cross(N, up)`

**14)** El bitangente `B` y el handedness:
- a) `B = cross(N, T)`, handedness = sign(dot(cross(N,T), B))
- b) `B = cross(T, N)`, handedness = dot(N, T)
- c) `B = cross(N, T)` siempre positivo
- d) `B = N x T` en coordenadas de textura

**15)** Al desempaquetar un normal map (RGB en [0,1]), la fórmula correcta es:
- a) `normal = rgb * 2.0 - 1.0`
- b) `normal = rgb - 0.5`
- c) `normal = (rgb - 0.5) * 2`
- d) a) y c) son equivalentes

---

## LECCIONES 09-10: Shadow Mapping, PCF, Bias, CSM

**16)** En shadow mapping, la textura de profundidad se configura con `GL_TEXTURE_COMPARE_MODE = GL_COMPARE_REF_TO_TEXTURE` y `GL_TEXTURE_COMPARE_FUNC = GL_LEQUAL`. ¿Qué devuelve `texture(sampler2DShadow, vec3(uv, ref))`?
- a) El valor de profundidad guardado
- b) 1.0 si `depth_guardado <= ref`, 0.0 en caso contrario
- c) 1.0 si `depth_guardado >= ref`, 0.0 en caso contrario
- d) La diferencia `depth_guardado - ref`

**17)** El bias de slope-scale `bias = max(0.05 * (1.0 - dot(N, L)), 0.005)`:
- a) Aumenta el bias en caras paralelas a la luz (N·L ≈ 0)
- b) Disminuye el bias en caras frontales (N·L ≈ 1)
- c) Previene shadow acne sin causar peter-panning excesivo
- d) Todas las anteriores

**18)** PCF 3x3 promedia 9 samples alrededor del texel central. ¿Cuál es el offset en UV para cada sample?
- a) `vec2(x, y) * (1.0 / shadowMapSize)`
- b) `vec2(x, y) * shadowMapSize`
- c) `vec2(x, y) * 0.5`
- d) `vec2(x, y)`

**19)** Cascaded Shadow Maps (CSM) resuelven el problema de:
- a) Una sola sombra ortográfica: caja grande = texels/metro pocos cerca de la cámara
- b) Una sola sombra: caja chica = el suelo lejano queda fuera
- c) Ambos: partir el frustum en cascadas con distinta resolución
- d) Todas las anteriores

---

## LECCIONES 11-12: Bloom, Tone Mapping, FXAA, Color Grading

**20)** En el bright pass, se usa `luma = dot(color, vec3(0.2126, 0.7152, 0.0722))`. Este vector es:
- a) Coeficientes BT.709 para luminancia percibida
- b) Promedio simple (0.33, 0.33, 0.33)
- c) Coeficientes NTSC antiguos
- d) Solo el canal verde

**21)** El kernel gaussiano separable 5-taps `[1, 4, 6, 4, 1] / 16` se aplica:
- a) Una pasada 2D de 25 samples
- b) Dos pasadas 1D: horizontal + vertical (10 samples total)
- c) Una pasada horizontal nada más
- d) Tres pasadas para mejor calidad

**22)** Tone mapping ACES (Stephen Hill): `out = clamp((c*(2.51c+0.03))/(c*(2.43c+0.59)+0.14), 0, 1)`. ¿Qué hace?
- a) Comprime HDR a LDR preservando negros y evitando "blowout" en altas luces
- b) Simple `c/(1+c)` (Reinhard)
- c) Solo gamma correction
- d) Lineal a sRGB

**23)** FXAA detecta bordes analizando la luminancia de los píxeles vecinos. Si `lumaN + lumaS > lumaE + lumaO`, el borde es:
- a) Horizontal (mezcla en Y)
- b) Vertical (mezcla en X)
- c) Diagonal
- d) No hay borde

**24)** Una LUT 3D de color grading (`GL_TEXTURE_3D`) usa el color RGB como coordenada. Para una LUT de 8³, el sampling correcto es:
- a) `coord = color`
- b) `coord = color * (7.0/8.0) + 0.5/8.0`
- c) `coord = color * 0.5 + 0.5`
- d) `coord = color * 8.0`

---

## LECCIONES 13-14: Compute Shader (concepto), Optimización UBO/Texture Arrays

**25)** ¿Por qué NO se pueden usar compute shaders en OpenGL 3.3 core?
- a) Requieren `glDispatchCompute` (core 4.3) y SSBO (4.3)
- b) El hardware no lo soporta
- c) SDL3 no lo expone
- d) Se pueden usar con extensiones ARB

**26)** Para dibujar 3000 copias de una misma malla en OpenGL 3.3, la técnica correcta es:
- a) `glDrawArraysInstanced` / `glDrawElementsInstanced` + `gl_InstanceID` en el VS
- b) 3000 llamadas a `glDrawElements`
- c) Compute shader + indirect draw
- d) Geometry shader

**27)** ¿Cuál es la ventaja de un UBO (Uniform Buffer Object) vs uniforms sueltos?
- a) Un solo `glBindBufferBase` por frame vs N `glUniform` por draw
- b) Layout std140 garantizado entre shaders
- c) Ambos
- d) Ninguno

**28)** Un `sampler2DArray` (GL_TEXTURE_2D_ARRAY) permite:
- a) Múltiples texturas 2D en un solo handle, indexadas por layer (float → int en shader)
- b) Solo texturas 3D
- c) Solo cube maps
- d) Reemplaza completamente a los atlas 2D tradicionales

---

## LECCIÓN 15: Cámara 3D libre (yaw/pitch)

**29)** Con convención mundo +X derecha, +Y arriba, cámara mirando -Z, el vector `forward` para yaw/pitch es:
- a) `{cos(pitch)*sin(yaw), sin(pitch), -cos(pitch)*cos(yaw)}`
- b) `{sin(yaw), sin(pitch), -cos(yaw)}`
- c) `{cos(yaw), sin(pitch), -sin(yaw)}`
- d) `{-sin(yaw), sin(pitch), cos(yaw)}`

**30)** El clamp de pitch a `[-89°, 89°]` evita:
- a) Gimbal lock en yaw
- b) Degeneración de `lookAt` cuando `forward` se vuelve paralelo a `up` (cross = 0)
- c) Overflow de float
- d) Que la cámara se voltee

**31)** El movimiento WASD en el plano piso usa `adelantePiso = (sin(yaw), 0, -cos(yaw))` e ignora el pitch. ¿Por qué?
- a) Para que caminar no te haga volar/subir al mirar arriba/abajo
- b) El pitch no afecta la dirección horizontal
- c) Ambas
- d) El pitch se usa solo para la matriz de vista, no para el movimiento

---

## LECCIÓN 16: tinyobjloader, grupos por usemtl, glDrawElementsBaseVertex

**32)** tinyobjloader devuelve `attrib_t` (pools vértices/normales/UVs), `shape_t[]` (índices por cara + `material_id`) y `material_t[]` (MTL). ¿Cómo se deduplican vértices?
- a) Key = tupla `(vertex_index, texcoord_index, normal_index)` → map a índice único
- b) Solo `vertex_index`
- c) Solo posición
- d) No se deduplica

**33)** Un OBJ con `usemtl` cambiando a mitad del mesh genera grupos. El draw plan usa `glDrawElementsBaseVertex`. ¿Para qué sirve `baseVertex`?
- a) Suma un offset al índice leído del EBO, permitiendo índices locales por grupo (0..N) que apuntan al pool global
- b) Cambia el VBO activo
- c) Selecciona el material
- d) Hace frustum culling

**34)** En el parseo OBJ, los índices de cara son 1-based y pueden venir como `v/vt/vn`, `v//vn`, `v/vt`. El índice -1 en tinyobj significa:
- a) El último vértice
- b) Componente ausente (no tiene UV o normal)
- c) Error de parseo
- d) Vértice inválido

---

## RESPUESTAS

| # | Respuesta | Explicación breve |
|---|-----------|-------------------|
| 1 | d | `R = reflect(-L, N)` es igual a `2*dot(N,L)*N - L` |
| 2 | d | Blinn usa H = normalize(L+V); highlight más redondo, sin anillo |
| 3 | a | Tramos estándar IEC 61966-2-1 (sRGB) |
| 4 | c | Shader lineal → framebuffer sRGB → conversión HW |
| 5 | a | 1/(1+0.35*13+0.44*169) = 1/80.81 ≈ 0.0124 |
| 6 | d | smoothstep transiciona suave entre cono interno/externo |
| 7 | b | vec3=16 (align 16), float=4@12, vec3=16@16, float=4@28 = 32 |
| 8 | b | int@0 + pad12 = 16 base; stride 32 por Luz |
| 9 | a | Dielectrico 0.04, metálico = albedo, mix por metalness |
| 10 | a | GGX: alpha² / (π * (N·H)²*(alpha²-1)+1)² |
| 11 | b | k = (1.5)²/8 = 2.25/8 = 0.28125 |
| 12 | a | Difusa Lambert = albedo/π (conservación de energía) |
| 13 | a | Gram-Schmidt: proyectar up fuera de N |
| 14 | a | B = N x T; handedness = sign(dot(N x T, B)) |
| 15 | d | a) y c) son equivalentes: 2x - 1 |
| 16 | b | Compare mode: 1.0 si depth <= ref (LEQUAL) |
| 17 | d | Más bias en caras rasantes, mínimo en frontales |
| 18 | a | Offset = 1 texel en UV = 1/textureSize |
| 19 | d | CSM: múltiples cascadas, más detalle cerca |
| 20 | a | Luma BT.709 (Rec. 709) |
| 21 | b | Separable: 2 pasadas 1D = 10 samples vs 25 |
| 22 | a | ACES fit: compresión HDR→LDR cinematográfica |
| 23 | b | Mayor gradiente vertical → borde horizontal → mezcla en X |
| 24 | b | Centrar en el texel: (S-1)/S * uv + 0.5/S |
| 25 | a | glDispatchCompute, SSBO, imageLoadStore = 4.3+ |
| 26 | a | Instancing: 1 draw, gl_InstanceID en VS |
| 27 | c | Un bind + std140 compartido |
| 28 | a | Texture array: 1 handle, N layers, sampler2DArray |
| 29 | a | Derivación esférica: x=cp*sy, y=sp, z=-cp*cy |
| 30 | b | forward || up → cross(N,T)=0 → matriz singular |
| 31 | c | Movimiento en piso (Y=0), pitch solo rota vista |
| 32 | a | Key = (v, vt, vn) para deduplicar vértice completo |
| 33 | a | Índices locales 0..N + baseVertex = pool global |
| 34 | b | -1 = componente no presente en la cara |

---

**Puntuación:**
- 30-34: 🏆 Dominio total del módulo
- 25-29: ✅ Muy bien, revisá las falladas
- 20-24: 📚 Repasá lecciones 09-16
- <20: 🔄 Releé el módulo completo