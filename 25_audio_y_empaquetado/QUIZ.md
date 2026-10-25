# QUIZ — Módulo 25: Audio y Empaquetado de Assets

Respondé antes de mirar las respuestas. Están al final.

---

## Parte A — Miniaudio: contexto, device, conversiones y callbacks

**1.** ¿Qué diferencia hay entre un `ma_context`, un `ma_device` y un `ma_engine`? ¿Qué te crea miniaudio por vos si no los pasás?

**2.** ¿Qué describe un `ma_device_config` y cuál es su función de inicialización? ¿Qué campo conecta el device con TU código?

**3.** ¿Por qué el data callback corre en un hilo aparte del juego y qué reglas tiene que respetar un callback bien escrito?

**4.** ¿Qué formato de muestras es el default del playback de miniaudio (`ma_format_f32`)? ¿Qué rango tiene y por qué es preferible a s16 para mezclar?

**5.** Si tenés `x = 0.8` en float y lo querés como int16, ¿qué hay que hacer ANTES de castear y a qué constantes clampea la conversión s16 (y por qué es asimétrica)?

**6.** ¿Qué hace `ma_clip_f32` y por qué hard-clampear la mezcla evita un problema peor que la distorsión?

**7.** Un device pide f32 stereo 48 kHz y tu dato es un WAV mono 22050 s16. ¿En qué orden aplicá vos las tres conversiones a mano, y por qué ese orden?

**8.** ¿Qué firma tiene `ma_data_converter_process_pcm_frames` (verificada contra el header 0.11.25)? ¿Los contadores de frames se pasan directos o por puntero, y por qué?

---

## Parte B — Streaming vs decode, y pool de voces

**9.** ¿Qué diferencia hay entre un sonido streameado y uno decodificado de una en RAM? ¿Cuándo usás cada uno?

**10.** ¿Para qué sirve el doble buffer de páginas del streamer y qué cuenta un "starvation counter"?

**11.** ¿Qué es el voice stealing y cuál es la política clásica de "a quién matás"?

**12.** Con un pool de 32 voces: ¿qué hace el motor cuando llega el disparo #33 y por qué cortar un loop EN SU BOUNDARY suena mejor que cortarlo al medio?

**13.** ¿Qué campos importan de `ma_engine_config` (verificados en el header) para adaptar el engine al presupuesto de CPU del juego?

---

## Parte C — Buses, pan y ducking

**14.** ¿Qué es un bus (`ma_sound_group`) y por qué un juego típico tiene 4 (master, SFX, music, ambience)?

**15.** ¿Qué hace `ma_sound_set_pan` y `ma_sound_set_pan_mode`? ¿Y qué dice el propio header de miniaudio 0.11.25 sobre HRTF?

**16.** ¿Por qué el ducking de la música usa ataque con paso rápido y release con paso lento? ¿Qué bug produce implementar "irse hacia el target siempre" con un solo paso?

---

## Parte D — FX: delay, biquad, freeverb y distorsión

**17.** Delay con feedback: ¿cuál es la fórmula del nodo y por qué la señal decae si el feedback es 0.95? ¿Qué pasa si le pasás 1.0?

**18.** ¿Cómo calculás la cantidad de muestras de un delay de 375 ms a 48 kHz, y por qué conviene buffer circular en vez de desplazar un array?

**19.** ¿Qué es un biquad y cómo queda Direct Form I en dos líneas (calculo de `y[n]` y rotación de estados)? ¿Por qué el orden de la rotación importa?

**20.** ¿De dónde salen los coeficientes del LP 1000 Hz Q=0.7071 a 48 kHz? Nombrá el material de referencia y por qué se normaliza por a0.

**21.** Freeverb: ¿cuál es la estructura de la red (¿cuántos comb, cuántos allpass, cómo se conectan) y qué aporta cada etapa?

**22.** ¿Por qué una distorsión waveshaper usa `tanh` (soft clipping) y no un sign/clamp duro? ¿Qué es el "drive" y cómo queda la fórmula compuesta?

**23.** ¿Por qué la duración de un delay se guarda en FRAMES y no en segundos?

---

## Parte E — Packs, CRC32, pipeline y localización

**24.** Tres razones (de sistema) por las que un juego mete sus assets en UN pack en vez de abrir mil archivos.

**25.** ¿Qué estructura tiene nuestro pack: el header, la TOC y los datos? ¿Por qué el writer hace los "3 momentos" (medir TOC → computar offsets → serializar)?

**26.** ¿Para qué la alineación a 16 de cada lump y de dónde arranca el primer offset del dato?

**27.** CRC32: ¿qué polinomio, qué init y qué xorout usa el estándar de ZIP/PNG/GZIP? ¿Qué vector universal confirma la implementación ("check value")?

**28.** ¿Por qué el update del CRC se puede partir en chunks (varios updates concatenados dan el MISMO crc que el one-shot)? ¿Qué se hace al final para "cerrar" el valor?

**29.** ¿Qué es el manifest del cooker y qué tipo de cook hace cada rol (textura/audio/datos)?

**30.** Localización: ¿cómo funciona el chain de fallback (idioma pedido → base → default) y qué cambia el hot-swap cuando el jugador cambia idioma? ¿Qué se hace cuando un audio_id NO existe en ningún banco del chain activo?

---

# Respuestas

**1.** `ma_context` describe el entorno (qué backends hay disponibles); `ma_device` es EL hardware real (playback/capture) ligado a un backend; `ma_engine` es el grafo de nodos que me fabrica un device por dentro y da ma_sound/ma_sound_group. Si no los pasás, miniaudio crea lazy-init internos de cada uno (un engine ya crea su context/device).

**2.** `ma_device_config_init(ma_device_type_playback)`. Configura tipo, formato/canales/sampleRate, `dataCallback`, `periodSizeInFrames` / `periods`. El campo `dataCallback` apunta a TU función: es el único punto de contacto con el audio.

**3.** El callback corre en el hilo del driver (prioridad de audio). Disciplina: (a) JAMÁS locks/alocaciones de memoria/I/O de archivos (si algo pesado hay que hacerlo, se despacha al hilo del juego con colas lock-free); (b) LLENAR todo el buffer de salida pedido (audio faltante = ruido); (c) usar mucho menos CPU de la disponible (dejar margen: un xrun y el kernel te corta del grafo).

**4.** Float32 en [-1, 1]: sin UB por overflow, precisión uniforme (se mezcla sin perder bits en cada etapa), no perdés "headroom" al sumar voces y el overshoot queda a un clamp controlado (no a un cast peligroso). Es el default del playback de miniaudio.

**5.** Antes del cast, CLAMPEAR x a [-1.0, 1.0]: positivo se escala por 32767.0f y negativo por 32768.0f (asimetría del complemento a dos del s16). Sin el clamp, un float fuera de rango cast a s16 es UB por el estándar de C++ (en C++20, el valor no cabe en el destino): NUNCA confiar, siempre clamp. La regla valió en el ejercicio 01 (f32AS16).

**6.** `ma_clip_f32` hace hard-clipping saturado a [-1.0, 1.0] por bloque. Sin clip, un float fuera de rango llega al driver (y cualquier cast a s16 sin clamp, UB): el clip es el "fusible" entre el juego y el hardware. Suena distorsionado, pero nunca ruidos ilógicos ni crasheos.

**7.** Orden: 1) SAMPLE FORMAT: s16 → f32 (escala 1/32768.0f), 2) CANALES: mono → stereo (copiar el canal L == R), 3) RESAMPLE: 22050 → 48000 (interpolación lineal como mínimo). Así la interpolación trabaja en f32 (resolución fina); si convertieras el formato después del resample, el resampler de s16 acumularía error de redondeo en cada tap.

**8.** `ma_data_converter_process_pcm_frames(&converter, pFramesIn, &framesInCount, pFramesOut, &framesOutCount)`: los contadores IN/OUT van por PUNTERO (ma_uint64*) y el converter los MODIFICA: dice cuántos frames consumió y cuántos produjo (la firma del header no acepta enteros directos).

**9.** Streameado: el resource manager lee POR PÁGINAS del archivo bajo demanda (poca RAM, soporta música/OGG largos). Decodificado: todo el waveform decodificado en RAM de una vez (rápido, sin I/O en runtime, pero gasta memoria). Regla: música/larga → stream; SFX cortos y repetidos → decodificar.

**10.** El streamer usa DOS páginas (`ma_resource_manager_data_stream` tiene pages[2] atómicas) que se turnan: mientras el mixer consume una, la otra se decodifica. El starvation counter cuenta cuántas veces el mixer pidió datos y NO HABÍA página lista (disco lento, CPU ocupada). Si sube, se abre el margen del decode (más páginas pre-cargadas: más RAM a cambio de menos riesgo de rotura).

**11.** Cuando el pool se llena se roba (steal) la voz "peor". La política clásica: matar la voz más VIEJA (o la del mismo sonido recién arrancado, para un retrigger limpio), y NUNCA a la música/narración (tienen prioridad máxima).

**12.** El #33 roba la voz al sonido "menos valioso" (un SFX corto repetido); los sonidos en loop (pasos, motores) se cortan EN SU loop boundary (el waveform termina su ciclo: el empalme es continuo); y la música NUNCA se roba (prioridad). Cortar al medio de una forma de onda produce un salto de amplitud (click); en el boundary la onda coincide en fase y no se escucha el corte.

**13.** (verificados en ma_engine_config): `channels` / `sampleRate` (formato del mezclador interno), `periodSizeInFrames` (latencia del callback: 480 frames ≈ 10 ms a 48k), y `defaultVolumeSmoothTimeInPCMFrames` (smoothing de los cambios de volumen).

**14.** Un bus = `ma_sound_group`: un nodo intermedio del grafo al que le pegas sonidos y al que le seteás volumen/balance en bloque. Tres usos: (a) volúmenes de opciones del juego (sliders de master/SFX/música); (b) fade del grupo para el hot-swap de idioma (lección 11); (c) ducking (bajar la música cuando entra un diálogo). Más buses son overhead de mantenimiento sin beneficio de diseño.

**15.** `ma_sound_set_pan(sound, pan)`: pan en [-1, 1] (izq/der). `ma_sound_set_pan_mode(sound, mode)`: `ma_pan_mode_normal` = energía constante (los canales compensan: -3 dB por lado: bien para música), `ma_pan_mode_linear` = ganancia lineal (sin compensar: más presente, bien para SFX mono). Sobre HRTF: el propio header de miniaudio 0.11.25 lo dice explícito: "HRTF and environmental occlusion are not currently supported". El "3D" del juego = pan + volumen por distancia (ma_sound_set_position).

**16.** Ataque rápido hace el ducking perceptible INSTANTÁNEO (la música "deja paso" al disparo); release lento hace que vuelva con un fade natural. Bug clásico: con un ÚNICO paso fijo, un delta mayor que el paso salta directo al target (sin suavizado: la música salta); el ejercicio 05 lo corrige con pasoAtaque/pasoRelease y dirección explícita del delta.

**17.** `y[n] = x[n] + feedback * delayBuf[i]` y el resultado se ESCRIBE en el buffer (el feedback es el retroalimentado del delay). Con feedback 0.95 la señal decae por 0.95^n (≈ 0.45 dB por cada vuelta: extinción gradual). Con feedback 1.0 NUNCA decae: el delay satura (clipping) y queda una resonancia infinita metálica. Por eso el límite real es ≤ ~0.95.

**18.** `msAmuestras(ms, sr) = ms * (sr/1000.0)`: 375 ms * 48 = 18.000 muestras (a 48k). Buffer circular: índice de WRAP `(i + offset) % size` en vez de desplazar 18000 floats por muestra (desplazar es O(n) por muestra → O(n²) total: lag del Celeron).

**19.** `y[n] = b0*x[n] + b1*x[n-1] + b2*x[n-2] - a1*y[n-1] - a2*y[n-2]` y DESPUÉS rotar `x2←x1, x1←x; y2←y1, y1←y`. El orden importa: y[n] se calcula con los estados VIEJOS; si rotás ANTES, y[n] usaría los estados YA rotados y la respuesta real del filtro cambia. La estructura es EXACTAMENTE la de ma_biquad (con `ma_biquad_config_init` que recibe b0..a2 ya normalizados).

**20.** Del "Audio EQ Cookbook" de Robert Bristow-Johnson (RBJ): las fórmulas cerradas del low-pass con Q (Butterworth a Q=0.7071): w0 = 2*pi*f0/fs, alpha = sin(w0)/(2*Q), a0 = 1+alpha, y b0 = (1-cos w0)/2, etc. Normalizar por a0 (dividir CADA coeficiente por a0 UNA vez en el setup): así el filtro no divide por muestra (la división se paga una sola vez al armar el filtro).

**21.** Freeverb (Schroeder-Moorer): 8 comb filters en PARALELO (cada uno delay + feedback + damping: generan la densidad inicial de la cola) seguidos de 4 allpass en SERIE (difuminan el color: los allpass NO generan duración solos). Los tunings del original: combs de 1116, 1188, 1277, 1356, 1422, 1491, 1557, 1617 y allpass 556, 441, 341, 225 (intercalado el stereo spread). La red completa es la que suena a reverb; los comb por separado solo producen eco.

**22.** `tanh` (soft clipping) se suaviza hacia ±1: sin discontinuidades en la curva (el clamp/sign duro dispara aliasing). Fórmula compuesta: `y = tanh(drive * x) / tanh(drive)`: el drive define la cantidad de saturación; dividir por tanh(drive) mantiene ganancia ~1 para señal chica (los valores bajos pasan limpios). El drive se fija en el setup del sonido (no se recalcula por muestra).

**23.** Porque el DSP corre en muestras (frames): "375 ms" solo tiene sentido CON UN sample rate (si cambia el rate, cambia la cantidad de muestras). El motor guarda la duración del delay EN FRAMES (u64) y convierte ms→frames EN EL SETUP (no en el callback: cero divisiones por muestra).

**24.** (a) Menos I/O: UN open + UN fd vs 1.000 (syscalls, inodes, antivirus escaneando, cache del disco); (b) Streaming por offset: cada lump tiene offset conocido (leé la PORCIÓN que importa sin descomprimir todo); (c) El orden de la TOC es el orden de carga (determinismo: la salida del cooker es reproducible y se planifica el loading).

**25.** Header: magic "VPAK" (4 B LE) + version (4 B) + count (4 B). TOC: `nameLen u32 + nombre + offset u64 + size u64` por entrada. Datos: los lumps concatenados alineados a 16 con padding de ceros. Los 3 momentos del writer: 1) serializar el TOC con offsets falsos (0) para SABER su tamaño (depende de los largos de los nombres), 2) cursor = alinear(12 + tocSize) y por cada lump alinear de nuevo → offset real, 3) serializar todo con esos reales (header + TOC + datos con padding).

**26.** Para que cualquier puntero dentro del pack sea alineado (16 bytes es la unidad del SSE y la mayoría de los decoders/la GPU lo espera); el primer offset arranca en `alinear(12 + tocSize)` (el header son 12 bytes y después el TOC).

**27.** Polinomio reflejanizado 0xEDB88320 con registro inicial 0xFFFFFFFF y XOR de salida (xorout) 0xFFFFFFFF; update por byte: `crc = (crc >> 8) ^ tabla[(crc ^ byte) & 0xFF]`. El vector universal: CRC32("123456789") = 0xCBF43926 (el "check value" del estándar, el mismo del zlib/gzip).

**28.** Porque el estado del CRC es SOLO el registro `crc_` (arranca 0xFFFFFFFF y NO se resetea entre updates): cada byte pasa por la MISMA fórmula encadenada, así que los chunks concatenados dan lo mismo que el one-shot. Al final se "cierra" con XOR de 0xFFFFFFFF en `final()`.

**29.** El manifest es el inventario del cooker: cada asset tiene UNA línea (`fuente @out destino`) agrupada por secciones que definen el rol: Textura → comprimir a BC3/BC7 (binario GPU-direct, sin decoder en runtime), Audio → encode OGG (Vorbis: 10x más chico que el wav del source), Datos → copiar tal cual (JSON/Tiled ya es final). "Stale": el cooker guarda el CRC32 del src y solo re-cocina lo que cambió (build determinista: re-cocinar todo en cada build sería carísimo).

**30.** El chain: `idioma pedido → idioma base (casi siempre completo) → default (en-US)`. `resolver(id)` recorre el chain EN ORDEN: el primer banco que tenga el audio_id gana. Hot-swap: al cambiar idioma el motor reemplaza el chain ACTIVO (un swap de variable, nunca dentro del data callback) y el audio hace crossfade a nivel del bus (ma_sound_group + fade en ms del engine). Si el id NO existe en ningún banco del chain activo: SILENCIO (y contado en debug): el juego sigue corriendo, una ficha faltante NO tira abajo el juego.
