# Módulo 26 — QUIZ (APIs Gráficas Modernas)

> Estilo: una línea por pregunta, respuesta corta. Para autoevaluación
> antes de pasar al módulo 27.

---

## Vulkan — instancia, colas, sincronización (lecciones 01–03)

1. **¿Qué devuelve `vkEnumeratePhysicalDevices` y por qué no "crea" el GPU?**
   Enumera GPUs físicas ya existentes; Vulkan no crea dispositivos, solo los descubre.

2. **Diferencia clave entre `VkSemaphore` y `VkFence`: ¿quién espera a quién?**
   Semáforo: GPU↔GPU (cola a cola). Fence: GPU→CPU (host consulta `vkWaitForFences`).

3. **¿Para qué sirve el "in-flight frames" ring (N=2 o 3)?**
   Evita sobrescribir command buffers que el GPU aún está leyendo; la CPU corre un frame adelante.

4. **En `vkQueueSubmit`, ¿qué semáforo se pone en `pWaitSemaphores` y cuál en `pSignalSemaphores`?**
   Wait = "imageAvailable" (adquirida por `vkAcquireNextImageKHR`). Signal = "renderFinished" (esperada por `vkQueuePresentKHR`).

5. **¿Qué información da `vkGetPhysicalDeviceQueueFamilyProperties` y por qué es el paso 3 del arranque?**
   Dice qué familias de colas existen y sus flags (GRAPHICS, COMPUTE, TRANSFER, SPARSE). El device lógico se crea pidiendo colas de esas familias.

6. **Capa de validación `VK_LAYER_KHRONOS_validation`: ¿cuándo se activa y por qué NO en release?**
   En `VkInstanceCreateInfo::ppEnabledLayerNames` solo en debug; en release añade overhead y ruido.

---

## SDL_GPU — abstracción portable (lecciones 04–05)

7. **`SDL_CreateGPUDevice` recibe un bitfield de formato de shader. ¿Cuál es el valor numérico de `SDL_GPU_SHADERFORMAT_SPIRV`?**
   `1 << 1` = 2 (bit 1). PRIVATE=1, SPIRV=2, DXBC=4, DXIL=8, MSL=16, METALLIB=32.

8. **¿Por qué SDL pide el formato de shader AL CREAR EL DEVICE y no al crear el pipeline?**
   Porque el backend (Vulkan/Metal/DX12) se elige ahí y cada backend acepta solo su formato nativo.

9. **`SDL_GPUColorTargetDescription::blend_state.enable_blend = false`: ¿qué logra en el fragment shader?**
   Desactiva blending: el color de salida escribe directo sin mezcla con el framebuffer.

10. **Diferencia entre `SDL_AcquireGPUSwapchainTexture` y `SDL_WaitAndAcquireGPUSwapchainTexture`.**
    La primera retorna `false` si no hay imagen lista (non-blocking); la segunda bloquea hasta tenerla.

11. **En SDL_GPU, ¿qué objeto agrupa TODO el estado fijo de dibujo (shader, vertex layout, blend, topology, formatos)?**
    `SDL_GPUGraphicsPipeline` (inmutable tras creación, igual que `VkPipeline`).

12. **Para subir datos a un `SDL_GPUBuffer` (VBO/IBO/UBO) ¿qué tres pasos se usan?**
    1) `SDL_CreateGPUTransferBuffer` (USAGE_UPLOAD) → 2) `SDL_MapGPUTransferBuffer` + memcpy → 3) `SDL_BeginGPUCopyPass` + `SDL_UploadToGPUBuffer` + `SDL_EndGPUCopyPass` + submit.

13. **`SDL_GPUSamplerCreateInfo::min_filter = SDL_GPU_FILTER_LINEAR`: ¿qué hace en el fragment shader al samplear entre texels?**
    Interpola bilinealmente (promedia 4 texels) en vez de nearest (un solo texel).

14. **`SDL_GPU_TEXTUREUSAGE_SAMPLER` vs `SDL_GPU_TEXTUREUSAGE_COLOR_TARGET`: ¿cuál va en la textura del quad y cuál en el swapchain?**
    Quad/textura = SAMPLER. Swapchain = COLOR_TARGET (SDL la crea internamente).

---

## WebGPU / wgpu-native — WGSL y bind groups (lección 06)

15. **En WGSL, `var<uniform>` vs `var<storage, read_write>`: ¿cuál mapea a UBO y cuál a SSBO de Vulkan?**
    `uniform` → UBO (read-only, layout std140/std430). `storage` → SSBO (read_write, buffer device address).

16. **`@group(0) @binding(0)` en WGSL: ¿a qué equivale en Vulkan?**
    `VkDescriptorSetLayoutBinding` con `descriptorSet = 0, binding = 0` en el set 0.

17. **Feature `Immediates` (`var<immediate>` en WGSL): ¿qué hardware feature expone y para qué sirve?**
    Push constants (Vulkan) / root constants (DX12) / argument buffers (Metal): datos pequeños por draw sin descriptor set.

18. **Backends de wgpu-native: ¿cuáles son los 5 y cuál usa esta máquina (Intel UHD 600 + Mesa)?**
    Vulkan, Metal, DX12, OpenGL ES, Browser WebGPU. Esta máquina usa **Vulkan (ANV)**.

---

## SPIR-V, glslang, reflection (lección 07)

19. **Magic number de un archivo `.spv` válido (primeros 4 bytes little-endian).**
    `0x07230203` (leído como `uint32_t` little-endian → bytes `03 02 23 07`).

20. **`glslangValidator -V -S vert -o out.spv in.vert`: ¿qué hace `-V` y qué `-S vert`?**
    `-V` = validar + generar SPIR-V. `-S vert` = stage = vertex (también `frag`, `comp`, etc.).

21. **`OpDecorate %var Location 0` en SPIR-V: ¿qué significa para el vertex input?**
    Atributo de vértice con `location = 0` (coincide con `layout(location=0)` en GLSL y `SDL_GPUVertexAttribute::location`).

22. **`spirv-cross --wgsl shader.spv`: ¿qué produce y por qué es útil para WebGPU?**
    Código WGSL legible y válido a partir de SPIR-V; permite un solo fuente GLSL → SPIR-V → WGSL.

---

## VMA — memoria y budget (lección 08)

23. **`VmaAllocationCreateInfo::usage = VMA_MEMORY_USAGE_AUTO`: ¿qué decide VMA internamente?**
    Elige el memory type óptimo (DEVICE_LOCAL vs HOST_VISIBLE) según `VkBufferCreateInfo::usage` y `requiredFlags/preferredFlags`.

24. **`VK_EXT_memory_budget`: ¿qué dos valores devuelve `VkPhysicalDeviceMemoryBudgetPropertiesEXT` por heap?**
    `heapBudget` (límite del driver) y `heapUsage` (actual). Permite decidir streaming / evicción de assets.

25. **`vmaCreateBuffer` vs `vkCreateBuffer` + `vkAllocateMemory` + `vkBindBufferMemory`: ¿qué ahorra VMA?**
    Una sola llamada, sub-alocación automática, dedicated allocations, mapeo persistente, defragmentación, stats.

---

## Render Graph (lección 09)

26. **En un render graph, los "edges" entre passes representan ¿qué recurso y qué transición?**
    Representan una `Resource` (texture/buffer) y su *layout transition* implícita (ej. `COLOR_ATTACHMENT` → `SHADER_READ_ONLY`).

27. **`FrameGraph::compile()` produce un `ExecuteList`: ¿qué contiene y por qué es más rápido que barreras manuales?**
    Lista ordenada de passes con barriers *fusionadas* y aliasing de memoria (alias = misma memoria, distinto uso, no simultáneo).

28. **`FrameGraphResource::import(swapchain_image)`: ¿por qué el swapchain es recurso "externo"?**
    Porque lo crea el window system (SDL/Vulkan), no el render graph; el graph solo lo *usa* como color target final.

---

## Decisión de API + techo de draws (lección 10 + M10)

29. **En `f26::PipelineSpec::decidir()`, ¿qué condición sube el techo GL de 500 a 1200 draws/frame?**
    `gpu.iso_ready == true` (extensiones AZDO presentes: multi-draw-indirect + buffer_storage + shader_draw_parameters).

30. **¿Por qué `f26::BudgetHeap` usa `std::pmr::polymorphic_allocator` y no `new/delete`?**
    Para que el motor pueda inyectar arenas (frame arena, persistent arena, streaming arena) y trackear memoria por subsistema.

31. **Resumen: ¿qué API elige el motor base del curso y por qué? (una frase)**
    **SDL_GPU + SPIR-V** — una sola base C++, backend automático (Vulkan/Metal/DX12), shaders GLSL→SPIR-V, control real sin boilerplate de Vulkan manual.

---

*Fin del QUIZ M26 — 31 preguntas. Objetivo: ≥ 27 correctas para pasar a M27.*