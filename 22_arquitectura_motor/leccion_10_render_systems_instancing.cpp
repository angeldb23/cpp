/* =============================================================================
   MÓDULO 22 - LECCIÓN 10: Render Systems — Instancing, Multi-threaded Command
   Generation, GPU-Driven Rendering (Indirect Draw), Bindless
   =============================================================================
   TEMA 5 de TEMAS.md (2/2): Continuación — instancing, command buffer avanzado,
   multi-threaded command generation, GPU-driven rendering.

   INSTANCING (Instanced Rendering):
   ---------------------------------
   Dibujar M copias del mismo mesh con una sola draw call.
   - Vertex buffer: mesh geometry (shared)
   - Instance buffer: per-instance data (world matrix, material ID, color, etc.)
   - glDrawElementsInstanced / glDrawArraysInstanced
   - Shader: gl_InstanceID para indexar instance buffer.

   En ECS + Command Buffer:
   - Agrupar renderables por (mesh, material, pipeline) → mismo draw call instanced.
   - Instance data buffer: subir matriz world + material params por instancia.
   - Frustum culling: solo instancias visibles.

   MULTI-THREADED COMMAND GENERATION:
   ----------------------------------
   - Main thread: culling, scene traversal, build work packets.
   - Worker threads: cada worker procesa un chunk de entidades → genera DrawCmds.
   - Merge: combinar command buffers de workers (ya ordenados por sort key).
   - Sync: solo al final del frame (command buffer submission).

   GPU-DRIVEN RENDERING (Indirect Draw):
   -------------------------------------
   - GPU decide qué draw calls ejecutar (culling en compute shader).
   - DrawIndirectBuffer: array de DrawCommands en GPU memory.
   - Compute shader: frustum culling + LOD selection + escribir DrawIndirectBuffer.
   - glMultiDrawElementsIndirect: un solo call para miles de draws.
   - Requiere OpenGL 4.3+ / ARB_multi_draw_indirect / ARB_shader_draw_parameters.
   - En 3.3 core: NO disponible (solo mención conceptual).

   BINDLESS RESOURCES:
   -------------------
   - Bindless textures/samplers: acceso a texturas por handle (uint64) en shader.
   - Elimina descriptor set binding limits y draw call overhead por textura.
   - Requiere OpenGL 4.4+ / ARB_bindless_texture.
   - En 3.3 core: NO disponible (solo mención conceptual).

   ESTA LECCIÓN: Instancing system, multi-threaded command gen, indirect draw concepts.
   ============================================================================= */
#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include <memory>
#include <functional>
#include <algorithm>
#include <thread>
#include <mutex>
#include <atomic>
#include <cstdint>
#include <cassert>
#include <array>

// ============================================================================
// TIPOS BASE
// ============================================================================
struct Vec3 { float x,y,z; };
struct Vec4 { float x,y,z,w; };
struct Mat4 { float m[16]; };  // Column-major
using Entity = uint32_t;
using TextureHandle = uint32_t;
using BufferHandle = uint32_t;
using PipelineHandle = uint32_t;

// ============================================================================
// INSTANCE DATA (per-instance en GPU)
// ============================================================================
struct InstanceData {
    Mat4 world;           // 64 bytes
    Vec4 color;           // 16 bytes (tint/material color)
    uint32_t material_id; // 4 bytes
    uint32_t pad[3];      // Padding a 96 bytes (cache line friendly)
};  // Total: 96 bytes

// ============================================================================
// MESH / GEOMETRY
// ============================================================================
struct Mesh {
    BufferHandle vertex_buffer = 0;
    BufferHandle index_buffer = 0;
    uint32_t index_count = 0;
    uint32_t vertex_stride = 0;
    // Bounding sphere para frustum culling rápido
    Vec3 bounds_center = {0,0,0};
    float bounds_radius = 1.0f;
};

// ============================================================================
// MATERIAL / PIPELINE
// ============================================================================
struct Material {
    PipelineHandle pipeline = 0;
    TextureHandle textures[8] = {0};
    uint32_t texture_count = 0;
    Vec4 base_color = {1,1,1,1};
    float metallic = 0, roughness = 1;
    uint32_t material_id = 0;
};

// ============================================================================
// RENDERABLE (entrada del sistema de render)
// ============================================================================
struct Renderable {
    Entity entity;
    Mesh* mesh = nullptr;
    Material* material = nullptr;
    Mat4 world;
    float depth = 0;        // Distancia a cámara (para sorting)
    bool transparent = false;
    bool visible = true;
};

// ============================================================================
// INSTANCE BATCH (agrupa instancias del mismo mesh+material)
// ============================================================================
struct InstanceBatch {
    Mesh* mesh = nullptr;
    Material* material = nullptr;
    std::vector<InstanceData> instances;
    bool transparent = false;

    void Add(const Renderable& r, const Mat4& view_proj) {
        InstanceData id;
        id.world = r.world;
        id.color = r.material->base_color;
        id.material_id = r.material->material_id;
        instances.push_back(id);
    }

    size_t InstanceCount() const { return instances.size(); }
    size_t BufferSize() const { return instances.size() * sizeof(InstanceData); }
};

// ============================================================================
// COMMAND BUFFER CON INSTANCING
// ============================================================================
struct DrawCmd {
    uint64_t sort_key = 0;
    PipelineHandle pipeline = 0;
    BufferHandle vertex_buffer = 0;
    BufferHandle index_buffer = 0;
    BufferHandle instance_buffer = 0;  // Nuevo: instance buffer
    uint32_t index_count = 0;
    uint32_t instance_count = 0;
    uint32_t base_vertex = 0;
    uint32_t base_instance = 0;
    BufferHandle ubo = 0;
    TextureHandle textures[8] = {0};
    uint32_t texture_count = 0;
};

class CommandBuffer {
    std::vector<DrawCmd> commands_;

public:
    void DrawInstanced(Mesh* mesh, Material* mat, BufferHandle instance_buf,
                       uint32_t instance_count, uint64_t sort_key) {
        DrawCmd cmd;
        cmd.pipeline = mat->pipeline;
        cmd.vertex_buffer = mesh->vertex_buffer;
        cmd.index_buffer = mesh->index_buffer;
        cmd.instance_buffer = instance_buf;
        cmd.index_count = mesh->index_count;
        cmd.instance_count = instance_count;
        cmd.ubo = 0;  // Per-instance data en instance buffer
        for(uint32_t i=0;i<mat->texture_count;++i) cmd.textures[i] = mat->textures[i];
        cmd.texture_count = mat->texture_count;
        cmd.sort_key = sort_key;
        commands_.push_back(cmd);
    }

    void Submit(std::function<void(const DrawCmd&)> execute_fn) {
        std::sort(commands_.begin(), commands_.end(),
                  [](const DrawCmd& a, const DrawCmd& b){ return a.sort_key < b.sort_key; });
        PipelineHandle current_pipe = 0;
        for(const auto& cmd : commands_){
            if(cmd.pipeline != current_pipe){
                current_pipe = cmd.pipeline;
                // execute_fn({.pipeline=cmd.pipeline, .debug_name="BIND_PIPE"});
            }
            execute_fn(cmd);
        }
        commands_.clear();
    }
};

// ============================================================================
// FRUSTUM CULLING (CPU-side)
// ============================================================================
struct Frustum {
    Vec4 planes[6];  // left, right, bottom, top, near, far
};

bool SphereInFrustum(const Frustum& f, const Vec3& center, float radius) {
    for(int i=0;i<6;++i){
        float dist = f.planes[i].x*center.x + f.planes[i].y*center.y + f.planes[i].z*center.z + f.planes[i].w;
        if(dist < -radius) return false;
    }
    return true;
}

// ============================================================================
// INSTANCING SYSTEM (single-threaded para demo)
// ============================================================================
class InstancingSystem {
    std::vector<Renderable> renderables_;
    std::unordered_map<uint64_t, InstanceBatch> batches_;  // key = (mesh_id << 32) | material_id

    uint64_t MakeKey(Mesh* m, Material* mat) {
        return (uint64_t(reinterpret_cast<uintptr_t>(m)) << 32) | mat->material_id;
    }

public:
    void Add(const Renderable& r) { renderables_.push_back(r); }

    void CullAndBatch(const Frustum& frustum, const Mat4& view_proj) {
        batches_.clear();
        for(const auto& r : renderables_){
            if(!r.visible) continue;
            if(!SphereInFrustum(frustum, r.mesh->bounds_center, r.mesh->bounds_radius)) continue;

            // Calcular depth para sorting
            Vec3 cam_pos = {view_proj.m[12], view_proj.m[13], view_proj.m[14]};  // Simplificado
            float dx = r.world.m[12] - cam_pos.x;
            float dy = r.world.m[13] - cam_pos.y;
            float dz = r.world.m[14] - cam_pos.z;
            float depth = dx*dx + dy*dy + dz*dz;

            uint64_t key = MakeKey(r.mesh, r.material);
            auto& batch = batches_[key];
            if(!batch.mesh) { batch.mesh = r.mesh; batch.material = r.material; batch.transparent = r.transparent; }
            InstanceData id; id.world = r.world; id.color = r.material->base_color; id.material_id = r.material->material_id;
            batch.instances.push_back(id);
        }
        renderables_.clear();
    }

    void GenerateCommands(CommandBuffer& cmd_buf, uint32_t pass_index,
                          std::function<BufferHandle(size_t)> alloc_instance_buffer) {
        // Separar opaque y transparent
        std::vector<InstanceBatch*> opaque, transparent;
        for(auto& [key, batch] : batches_){
            if(batch.transparent) transparent.push_back(&batch);
            else opaque.push_back(&batch);
        }

        // Sort opaque front-to-back (menor depth)
        // Sort transparent back-to-front (mayor depth)
        // Para instancing, usamos depth promedio del batch
        auto avg_depth = [](InstanceBatch* b){
            float sum=0; for(auto& i:b->instances) sum += i.world.m[14]; return sum/b->instances.size();
        };
        std::sort(opaque.begin(), opaque.end(), [&](auto a, auto b){ return avg_depth(a) < avg_depth(b); });
        std::sort(transparent.begin(), transparent.end(), [&](auto a, auto b){ return avg_depth(a) > avg_depth(b); });

        // Generar commands
        for(auto* batch : opaque){
            BufferHandle inst_buf = alloc_instance_buffer(batch->BufferSize());
            // Subir datos (en real: glBufferSubData o map/unmap)
            cmd_buf.DrawInstanced(batch->mesh, batch->material, inst_buf,
                                  batch->InstanceCount(), MakeSortKey(avg_depth(batch), batch->material->material_id, false));
        }
        for(auto* batch : transparent){
            BufferHandle inst_buf = alloc_instance_buffer(batch->BufferSize());
            cmd_buf.DrawInstanced(batch->mesh, batch->material, inst_buf,
                                  batch->InstanceCount(), MakeSortKey(avg_depth(batch), batch->material->material_id, true));
        }
    }

private:
    uint64_t MakeSortKey(float depth, uint32_t mat_id, bool transparent) {
        uint16_t depth_q = uint16_t(std::min(65535.0f, depth * 10.0f));
        if(transparent) depth_q = 65535 - depth_q;  // Invertir para back-to-front
        return (uint64_t(depth_q) << 16) | mat_id;
    }
};

// ============================================================================
// MULTI-THREADED COMMAND GENERATION
// ============================================================================
struct WorkPacket {
    std::vector<Renderable> renderables;
    Frustum frustum;
    Mat4 view_proj;
    uint32_t thread_id;
    std::vector<InstanceBatch> output_batches;
};

void WorkerThread(WorkPacket& packet) {
    // Culling + batching local
    std::unordered_map<uint64_t, InstanceBatch> local_batches;
    auto MakeKey = [](Mesh* m, Material* mat){
        return (uint64_t(reinterpret_cast<uintptr_t>(m)) << 32) | mat->material_id;
    };

    for(const auto& r : packet.renderables){
        if(!r.visible) continue;
        if(!SphereInFrustum(packet.frustum, r.mesh->bounds_center, r.mesh->bounds_radius)) continue;

        Vec3 cam_pos = {packet.view_proj.m[12], packet.view_proj.m[13], packet.view_proj.m[14]};
        float dx = r.world.m[12] - cam_pos.x;
        float dy = r.world.m[13] - cam_pos.y;
        float dz = r.world.m[14] - cam_pos.z;
        float depth = dx*dx + dy*dy + dz*dz;

        uint64_t key = MakeKey(r.mesh, r.material);
        auto& batch = local_batches[key];
        if(!batch.mesh) { batch.mesh = r.mesh; batch.material = r.material; batch.transparent = r.transparent; }
        InstanceData id; id.world = r.world; id.color = r.material->base_color; id.material_id = r.material->material_id;
        batch.instances.push_back(id);
    }

    // Mover a output
    packet.output_batches.reserve(local_batches.size());
    for(auto& [k, b] : local_batches) packet.output_batches.push_back(std::move(b));
}

class ThreadedInstancingSystem {
    std::vector<Renderable> renderables_;
    size_t num_threads_ = std::thread::hardware_concurrency();

public:
    void Add(const Renderable& r) { renderables_.push_back(r); }

    void Process(const Frustum& frustum, const Mat4& view_proj,
                 std::vector<InstanceBatch>& out_batches) {
        // Dividir trabajo
        size_t per_thread = (renderables_.size() + num_threads_ - 1) / num_threads_;
        std::vector<WorkPacket> packets(num_threads_);
        std::vector<std::thread> threads;

        for(size_t t=0; t<num_threads_; ++t){
            size_t start = t * per_thread;
            size_t end = std::min(start + per_thread, renderables_.size());
            packets[t].renderables.assign(renderables_.begin() + start, renderables_.begin() + end);
            packets[t].frustum = frustum;
            packets[t].view_proj = view_proj;
            packets[t].thread_id = t;
            threads.emplace_back(WorkerThread, std::ref(packets[t]));
        }

        for(auto& th : threads) th.join();

        // Merge batches
        std::unordered_map<uint64_t, InstanceBatch> merged;
        auto MakeKey = [](Mesh* m, Material* mat){
            return (uint64_t(reinterpret_cast<uintptr_t>(m)) << 32) | mat->material_id;
        };

        for(auto& packet : packets){
            for(auto& batch : packet.output_batches){
                uint64_t key = MakeKey(batch.mesh, batch.material);
                auto& mb = merged[key];
                if(!mb.mesh) { mb.mesh = batch.mesh; mb.material = batch.material; mb.transparent = batch.transparent; }
                mb.instances.insert(mb.instances.end(), batch.instances.begin(), batch.instances.end());
            }
        }

        out_batches.clear();
        out_batches.reserve(merged.size());
        for(auto& [k, b] : merged) out_batches.push_back(std::move(b));

        renderables_.clear();
    }
};

// ============================================================================
// GPU-DRIVEN RENDERING (Conceptual - OpenGL 4.3+)
// ============================================================================
/*
// Estructura para DrawElementsIndirectCommand (OpenGL 4.3+)
struct DrawElementsIndirectCommand {
    uint32_t count;
    uint32_t instanceCount;
    uint32_t firstIndex;
    uint32_t baseVertex;
    uint32_t baseInstance;
};

// Compute shader para frustum culling (GLSL 430)
#version 430
layout(local_size_x = 64) in;
struct Instance { mat4 world; vec4 color; uint material_id; };
layout(std430, binding = 0) readonly buffer InstanceBuffer { Instance instances[]; };
layout(std430, binding = 1) buffer IndirectBuffer { DrawElementsIndirectCommand draws[]; };
layout(std430, binding = 2) readonly buffer FrustumBuffer { vec4 planes[6]; };
uniform uint num_instances;
void main() {
    uint idx = gl_GlobalInvocationID.x;
    if(idx >= num_instances) return;
    // Frustum test
    vec3 center = instances[idx].world[3].xyz;
    float radius = 1.0; // Simplificado
    bool visible = true;
    for(int i=0;i<6;++i){
        if(dot(planes[i].xyz, center) + planes[i].w < -radius) { visible = false; break; }
    }
    if(visible){
        // Atomic increment para contador de draws visibles
        // En real: append buffer o atomic counter + compactación
    }
}
// glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, indirect_buffer, draw_count, stride);
*/

// ============================================================================
// VERIFICACIÓN DE SINTAXIS
// ============================================================================
int main() {
    std::cout << "=== LECCIÓN 10: Render Systems Instancing/Threaded (solo sintaxis) ===\n";

    // Setup mock resources
    Mesh mesh; mesh.vertex_buffer = 1; mesh.index_buffer = 2; mesh.index_count = 36;
    Material mat; mat.pipeline = 1; mat.material_id = 42; mat.base_color = {1,0.5f,0,1};

    // Instancing system
    InstancingSystem inst_sys;
    for(int i=0;i<100;++i){
        Renderable r; r.entity = i; r.mesh = &mesh; r.material = &mat;
        r.world.m[12] = float(i%10)*2.0f; r.world.m[13] = float(i/10)*2.0f; r.world.m[14] = -10.0f;
        for(int j=0;j<16;++j) r.world.m[j] = (j%5==0)?1.0f:0.0f;  // Identity-ish
        r.visible = true; r.transparent = (i%10==0);
        inst_sys.Add(r);
    }

    Frustum frustum{};  // Planes dummy
    Mat4 view_proj{};
    inst_sys.CullAndBatch(frustum, view_proj);

    CommandBuffer cb;
    inst_sys.GenerateCommands(cb, 0, [](size_t sz){ return BufferHandle(1); });
    std::cout << "Commands generated: " << cb.CommandCount() << "\n";

    // Threaded system
    ThreadedInstancingSystem threaded_sys;
    for(int i=0;i<500;++i){
        Renderable r; r.entity = i; r.mesh = &mesh; r.material = &mat;
        r.world.m[12] = float(i%20)*2.0f; r.world.m[13] = float(i/20)*2.0f; r.world.m[14] = -10.0f;
        for(int j=0;j<16;++j) r.world.m[j] = (j%5==0)?1.0f:0.0f;
        r.visible = true;
        threaded_sys.Add(r);
    }
    std::vector<InstanceBatch> batches;
    threaded_sys.Process(frustum, view_proj, batches);
    std::cout << "Threaded batches: " << batches.size() << "\n";
    for(auto& b : batches) std::cout << "  Batch: " << b.InstanceCount() << " instances\n";

    std::cout << "=== FIN LECCIÓN 10 ===\n";
    return 0;
}