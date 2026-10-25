/* =============================================================================
   MÓDULO 22 - EJERCICIO 06: Render Systems — Command Buffer, Transparent Sort,
   Instancing
   =============================================================================
   DIFICULTAD: Medio-Alto
   APOYO: Lección 09 (Render Graph, Command Buffer), Lección 10 (Instancing)

   CONSIGNA:
   Implementa:
   - DrawCmd con sort_key (pass, pipeline, depth, material)
   - CommandBuffer: Draw(), Submit() con sort por sort_key
   - TransparentRenderSystem: separa opaque/transparent, sort front-to-back / back-to-front
   - Instancing: agrupar por (mesh, material) → DrawInstanced

   TESTS:
   - CommandBuffer ordena por pipeline y depth
   - TransparentRenderSystem dibuja opaque primero (front-to-back), luego transparent (back-to-front)
   - Instancing agrupa múltiples renderables en un DrawCmd con instance_count > 1
   - Sort key encoding correcto
   ============================================================================= */
#include <vector>
#include <algorithm>
#include <cstdint>
#include <functional>
#include <cassert>
#include "../../include/test_framework.hpp"

struct Vec3 { float x,y,z; };
struct Mat4 { float m[16]; };
using Entity = uint32_t;
using TextureHandle = uint32_t;
using BufferHandle = uint32_t;
using PipelineHandle = uint32_t;

// ============================================================================
// DRAW COMMAND
// ============================================================================
struct DrawCmd {
    uint64_t sort_key = 0;
    PipelineHandle pipeline = 0;
    BufferHandle vertex_buffer = 0, index_buffer = 0, instance_buffer = 0;
    uint32_t index_count = 0, instance_count = 1;
    BufferHandle ubo = 0;
    TextureHandle textures[8] = {0};
    uint32_t texture_count = 0;
};

// ============================================================================
// TODO: Implementa CommandBuffer
// ============================================================================
class CommandBuffer {
    std::vector<DrawCmd> commands_;
    uint32_t current_pass_ = 0;

public:
    void BeginPass(uint32_t pass) { current_pass_ = pass; }
    void EndPass() {}

    // Draw simple (no instancing)
    void Draw(PipelineHandle pipe, BufferHandle vb, BufferHandle ib,
              uint32_t index_count, uint64_t sort_key) {
        // TU CÓDIGO AQUÍ: crear DrawCmd, construir sort_key, push a commands_
        (void)pipe; (void)vb; (void)ib; (void)index_count; (void)sort_key;
    }

    // Draw instanced
    void DrawInstanced(PipelineHandle pipe, BufferHandle vb, BufferHandle ib,
                       BufferHandle instance_buf, uint32_t index_count,
                       uint32_t instance_count, uint64_t sort_key) {
        // TU CÓDIGO AQUÍ
        (void)pipe; (void)vb; (void)ib; (void)instance_buf;
        (void)index_count; (void)instance_count; (void)sort_key;
    }

    // Submit: sort y ejecutar callback
    void Submit(std::function<void(const DrawCmd&)> execute_fn) {
        // TU CÓDIGO AQUÍ: sort commands_ por sort_key, luego execute_fn(cmd)
        (void)execute_fn;
    }

    size_t CommandCount() const { return commands_.size(); }
    const std::vector<DrawCmd>& GetCommands() const { return commands_; }
};

// ============================================================================
// RENDERABLE
// ============================================================================
struct Renderable {
    Entity entity;
    BufferHandle vb = 0, ib = 0;
    uint32_t index_count = 0;
    PipelineHandle pipeline = 0;
    float depth = 0;
    bool transparent = false;
    uint16_t material_id = 0;
};

// ============================================================================
// TODO: Implementa TransparentRenderSystem
// ============================================================================
class TransparentRenderSystem {
    std::vector<Renderable> opaque_;
    std::vector<Renderable> transparent_;

public:
    void Add(const Renderable& r) {
        // TU CÓDIGO AQUÍ: separar en opaque_ / transparent_
        (void)r;
    }

    void Prepare(CommandBuffer& cmd_buf, uint32_t pass_idx) {
        // TU CÓDIGO AQUÍ:
        // 1. Sort opaque front-to-back (depth ascendente)
        // 2. Sort transparent back-to-front (depth descendente)
        // 3. cmd_buf.BeginPass(pass_idx)
        // 4. Submit opaque DrawCmds con sort_key encoding depth+material
        // 5. Submit transparent DrawCmds con sort_key invertido para back-to-front
        // 6. cmd_buf.EndPass()
        (void)cmd_buf; (void)pass_idx;
    }
};

// ============================================================================
// INSTANCE BATCH
// ============================================================================
struct InstanceData { Mat4 world; uint32_t material_id; };

struct InstanceBatch {
    BufferHandle vb = 0, ib = 0;
    uint32_t index_count = 0;
    PipelineHandle pipeline = 0;
    std::vector<InstanceData> instances;
    bool transparent = false;
};

// ============================================================================
// TODO: Implementa InstancingSystem
// ============================================================================
class InstancingSystem {
    std::vector<Renderable> renderables_;

public:
    void Add(const Renderable& r) { renderables_.push_back(r); }

    void GenerateCommands(CommandBuffer& cmd_buf, uint32_t pass_idx,
                          std::function<BufferHandle(size_t)> alloc_instance_buffer) {
        // TU CÓDIGO AQUÍ:
        // 1. Agrupar renderables por (vb, ib, pipeline, material_id, transparent) → InstanceBatch
        // 2. Para cada batch: alloc_instance_buffer, subir instances, DrawInstanced
        // 3. Sort batches: opaque front-to-back, transparent back-to-front
        (void)cmd_buf; (void)pass_idx; (void)alloc_instance_buffer;
    }
};

// ============================================================================
// SORT KEY HELPERS
// ============================================================================
// Bits: [63:48] pass | [47:32] pipeline | [31:16] depth (quantized) | [15:0] material_id
inline uint64_t MakeSortKey(uint32_t pass, PipelineHandle pipe, float depth, uint16_t mat_id, bool transparent) {
    uint16_t depth_q = uint16_t(std::min(65535.0f, depth * 100.0f));
    if(transparent) depth_q = 65535 - depth_q;  // Invertir para back-to-front
    return (uint64_t(pass) << 48) | (uint64_t(pipe) << 32) | (uint64_t(depth_q) << 16) | mat_id;
}

// ============================================================================
// TESTS
// ============================================================================
TEST_CASE(CommandBufferSortsByPipelineThenDepth) {
    CommandBuffer cb;
    cb.BeginPass(0);
    cb.Draw(2, 1, 1, 36, MakeSortKey(0, 2, 10.0f, 5, false));  // pipeline 2, depth 10
    cb.Draw(1, 1, 1, 36, MakeSortKey(0, 1, 5.0f, 3, false));   // pipeline 1, depth 5
    cb.Draw(1, 1, 1, 36, MakeSortKey(0, 1, 15.0f, 7, false));  // pipeline 1, depth 15
    cb.Draw(2, 1, 1, 36, MakeSortKey(0, 2, 1.0f, 1, false));   // pipeline 2, depth 1

    std::vector<DrawCmd> executed;
    cb.Submit([&](const DrawCmd& cmd){ executed.push_back(cmd); });

    // Debe ordenar: pipeline 1 (depth 5, 15), luego pipeline 2 (depth 1, 10)
    ASSERT_EQ(executed.size(), 4);
    ASSERT_EQ(executed[0].pipeline, 1); ASSERT_NEAR((executed[0].sort_key>>16)&0xFFFF, 500, 10);  // depth 5
    ASSERT_EQ(executed[1].pipeline, 1); ASSERT_NEAR((executed[1].sort_key>>16)&0xFFFF, 1500, 10); // depth 15
    ASSERT_EQ(executed[2].pipeline, 2); ASSERT_NEAR((executed[2].sort_key>>16)&0xFFFF, 100, 10);  // depth 1
    ASSERT_EQ(executed[3].pipeline, 2); ASSERT_NEAR((executed[3].sort_key>>16)&0xFFFF, 1000, 10); // depth 10
}

TEST_CASE(TransparentRenderSystemOpaqueFirstThenTransparent) {
    TransparentRenderSystem trs;
    CommandBuffer cb;

    trs.Add({1, 1,1,36, 1, 10.0f, false, 1});  // opaque depth 10
    trs.Add({2, 1,1,36, 1, 5.0f, false, 2});   // opaque depth 5 (más cerca)
    trs.Add({3, 1,1,36, 2, 50.0f, true, 3});   // transparent depth 50 (lejos)
    trs.Add({4, 1,1,36, 2, 1.0f, true, 4});    // transparent depth 1 (cerca)

    trs.Prepare(cb, 0);

    std::vector<DrawCmd> executed;
    cb.Submit([&](const DrawCmd& cmd){ executed.push_back(cmd); });

    // Primero 2 opaques (front-to-back: depth 5, luego 10)
    // Luego 2 transparentes (back-to-front: depth 50, luego 1)
    ASSERT_EQ(executed.size(), 4);
    ASSERT_FALSE(executed[0].sort_key & (1ull<<15)); // opaque flag? No, usamos depth invertido
    // Verificar orden: opaque depth 5, opaque depth 10, transparent depth 50, transparent depth 1
}

TEST_CASE(InstancingSystemGroupsByMeshMaterial) {
    InstancingSystem isys;
    CommandBuffer cb;

    // 3 renderables mismo mesh/material → 1 batch instanced
    isys.Add({1, 10, 10, 36, 1, 5.0f, false, 1});
    isys.Add({2, 10, 10, 36, 1, 10.0f, false, 1});
    isys.Add({3, 10, 10, 36, 1, 15.0f, false, 1});
    // 2 renderables distinto material → batch separado
    isys.Add({4, 10, 10, 36, 2, 20.0f, false, 2});
    isys.Add({5, 10, 10, 36, 2, 25.0f, false, 2});

    isys.GenerateCommands(cb, 0, [](size_t){ return BufferHandle(100); });

    std::vector<DrawCmd> executed;
    cb.Submit([&](const DrawCmd& cmd){ executed.push_back(cmd); });

    // Debe haber 2 draw calls instanced
    ASSERT_EQ(executed.size(), 2);
    // Batch 1: 3 instances, material 1
    // Batch 2: 2 instances, material 2
    uint32_t total_instances = 0;
    for(const auto& cmd : executed){
        ASSERT_GT(cmd.instance_count, 1);
        total_instances += cmd.instance_count;
    }
    ASSERT_EQ(total_instances, 5);
}

TEST_CASE(SortKeyEncoding) {
    uint64_t key = MakeSortKey(1, 5, 10.5f, 42, false);
    ASSERT_EQ((key >> 48) & 0xFFFF, 1);      // pass
    ASSERT_EQ((key >> 32) & 0xFFFF, 5);      // pipeline
    ASSERT_EQ((key >> 16) & 0xFFFF, 1050);   // depth quantized (10.5 * 100)
    ASSERT_EQ(key & 0xFFFF, 42);             // material_id
}

TEST_CASE(TransparentSortKeyInverted) {
    uint64_t key_opaque = MakeSortKey(0, 1, 100.0f, 1, false);
    uint64_t key_transp = MakeSortKey(0, 1, 100.0f, 1, true);
    // Transparent debe tener depth invertido para back-to-front
    uint16_t depth_opaque = (key_opaque >> 16) & 0xFFFF;
    uint16_t depth_transp = (key_transp >> 16) & 0xFFFF;
    ASSERT_EQ(depth_opaque + depth_transp, 65535);  // Invertidos
}

TEST_CASE(InstancingSeparateTransparentBatch) {
    InstancingSystem isys;
    CommandBuffer cb;

    isys.Add({1, 1,1,36, 1, 10.0f, false, 1});
    isys.Add({2, 1,1,36, 1, 20.0f, true, 1});   // Mismo mesh/material pero transparent

    isys.GenerateCommands(cb, 0, [](size_t){ return BufferHandle(1); });

    std::vector<DrawCmd> executed;
    cb.Submit([&](const DrawCmd& cmd){ executed.push_back(cmd); });

    // Debe haber 2 batches: uno opaque, uno transparent (no se pueden mezclar)
    ASSERT_EQ(executed.size(), 2);
    ASSERT_EQ(executed[0].instance_count, 1);
    ASSERT_EQ(executed[1].instance_count, 1);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 06 (Render Systems)...\n";
    return RUN_ALL_TESTS();
}