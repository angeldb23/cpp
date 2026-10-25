/* =============================================================================
   MÓDULO 22 - SOLUCIÓN 06: Render Systems — Command Buffer, Transparent Sort, Instancing
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

struct DrawCmd {
    uint64_t sort_key = 0;
    PipelineHandle pipeline = 0;
    BufferHandle vertex_buffer = 0, index_buffer = 0, instance_buffer = 0;
    uint32_t index_count = 0, instance_count = 1;
    BufferHandle ubo = 0;
    TextureHandle textures[8] = {0};
    uint32_t texture_count = 0;
};

class CommandBuffer {
    std::vector<DrawCmd> commands_;
    uint32_t current_pass_ = 0;

public:
    void BeginPass(uint32_t pass) { current_pass_ = pass; }
    void EndPass() {}

    void Draw(PipelineHandle pipe, BufferHandle vb, BufferHandle ib,
              uint32_t index_count, uint64_t sort_key) {
        DrawCmd cmd;
        cmd.pipeline = pipe;
        cmd.vertex_buffer = vb;
        cmd.index_buffer = ib;
        cmd.index_count = index_count;
        cmd.sort_key = sort_key;
        commands_.push_back(cmd);
    }

    void DrawInstanced(PipelineHandle pipe, BufferHandle vb, BufferHandle ib,
                       BufferHandle instance_buf, uint32_t index_count,
                       uint32_t instance_count, uint64_t sort_key) {
        DrawCmd cmd;
        cmd.pipeline = pipe;
        cmd.vertex_buffer = vb;
        cmd.index_buffer = ib;
        cmd.instance_buffer = instance_buf;
        cmd.index_count = index_count;
        cmd.instance_count = instance_count;
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
            }
            execute_fn(cmd);
        }
        commands_.clear();
    }

    size_t CommandCount() const { return commands_.size(); }
    const std::vector<DrawCmd>& GetCommands() const { return commands_; }
};

struct Renderable {
    Entity entity;
    BufferHandle vb = 0, ib = 0;
    uint32_t index_count = 0;
    PipelineHandle pipeline = 0;
    float depth = 0;
    bool transparent = false;
    uint16_t material_id = 0;
};

class TransparentRenderSystem {
    std::vector<Renderable> opaque_;
    std::vector<Renderable> transparent_;

public:
    void Add(const Renderable& r) {
        if(r.transparent) transparent_.push_back(r);
        else opaque_.push_back(r);
    }

    void Prepare(CommandBuffer& cmd_buf, uint32_t pass_idx) {
        std::sort(opaque_.begin(), opaque_.end(),
                  [](const Renderable& a, const Renderable& b){ return a.depth < b.depth; });
        std::sort(transparent_.begin(), transparent_.end(),
                  [](const Renderable& a, const Renderable& b){ return a.depth > b.depth; });

        cmd_buf.BeginPass(pass_idx);
        for(const auto& r : opaque_){
            uint64_t key = (uint64_t(pass_idx) << 48) | (uint64_t(r.pipeline) << 32) |
                          (uint64_t(uint16_t(r.depth * 100)) << 16) | r.material_id;
            cmd_buf.Draw(r.pipeline, r.vb, r.ib, r.index_count, key);
        }
        for(const auto& r : transparent_){
            uint16_t depth_q = uint16_t(std::min(65535.0f, r.depth * 100.0f));
            uint64_t key = (uint64_t(pass_idx) << 48) | (uint64_t(r.pipeline) << 32) |
                          (uint64_t(65535 - depth_q) << 16) | r.material_id;
            cmd_buf.Draw(r.pipeline, r.vb, r.ib, r.index_count, key);
        }
        cmd_buf.EndPass();

        opaque_.clear();
        transparent_.clear();
    }
};

struct InstanceData { Mat4 world; uint32_t material_id; };

struct InstanceBatch {
    BufferHandle vb = 0, ib = 0;
    uint32_t index_count = 0;
    PipelineHandle pipeline = 0;
    std::vector<InstanceData> instances;
    bool transparent = false;
};

class InstancingSystem {
    std::vector<Renderable> renderables_;

public:
    void Add(const Renderable& r) { renderables_.push_back(r); }

    void GenerateCommands(CommandBuffer& cmd_buf, uint32_t pass_idx,
                          std::function<BufferHandle(size_t)> alloc_instance_buffer) {
        // Agrupar por (vb, ib, pipeline, material_id, transparent)
        std::unordered_map<uint64_t, InstanceBatch> batches;
        auto MakeKey = [](const Renderable& r){
            return (uint64_t(r.vb) << 40) | (uint64_t(r.ib) << 32) |
                   (uint64_t(r.pipeline) << 16) | (uint64_t(r.material_id) << 1) | r.transparent;
        };

        for(const auto& r : renderables_){
            uint64_t key = MakeKey(r);
            auto& batch = batches[key];
            if(batch.vb == 0){
                batch.vb = r.vb; batch.ib = r.ib; batch.index_count = r.index_count;
                batch.pipeline = r.pipeline; batch.transparent = r.transparent;
            }
            InstanceData id; id.world = Mat4{}; id.material_id = r.material_id;
            for(int i=0;i<16;++i) id.world.m[i] = (i%5==0)?1.0f:0.0f; // identity placeholder
            batch.instances.push_back(id);
        }

        // Separar y sort
        std::vector<InstanceBatch*> opaque, transparent;
        for(auto& [k, b] : batches){
            if(b.transparent) transparent.push_back(&b);
            else opaque.push_back(&b);
        }
        auto avg_depth = [](InstanceBatch* b){ return 0.0f; }; // placeholder
        std::sort(opaque.begin(), opaque.end(), [&](auto a, auto b){ return avg_depth(a) < avg_depth(b); });
        std::sort(transparent.begin(), transparent.end(), [&](auto a, auto b){ return avg_depth(a) > avg_depth(b); });

        for(auto* batch : opaque){
            BufferHandle inst_buf = alloc_instance_buffer(batch->instances.size() * sizeof(InstanceData));
            uint64_t key = (uint64_t(pass_idx) << 48) | (uint64_t(batch->pipeline) << 32);
            cmd_buf.DrawInstanced(batch->pipeline, batch->vb, batch->ib, inst_buf,
                                  batch->index_count, batch->instances.size(), key);
        }
        for(auto* batch : transparent){
            BufferHandle inst_buf = alloc_instance_buffer(batch->instances.size() * sizeof(InstanceData));
            uint64_t key = (uint64_t(pass_idx) << 48) | (uint64_t(batch->pipeline) << 32);
            cmd_buf.DrawInstanced(batch->pipeline, batch->vb, batch->ib, inst_buf,
                                  batch->index_count, batch->instances.size(), key);
        }

        renderables_.clear();
    }
};

inline uint64_t MakeSortKey(uint32_t pass, PipelineHandle pipe, float depth, uint16_t mat_id, bool transparent) {
    uint16_t depth_q = uint16_t(std::min(65535.0f, depth * 100.0f));
    if(transparent) depth_q = 65535 - depth_q;
    return (uint64_t(pass) << 48) | (uint64_t(pipe) << 32) | (uint64_t(depth_q) << 16) | mat_id;
}

// TESTS
TEST_CASE(CommandBufferSortsByPipelineThenDepth) {
    CommandBuffer cb;
    cb.BeginPass(0);
    cb.Draw(2, 1, 1, 36, MakeSortKey(0, 2, 10.0f, 5, false));
    cb.Draw(1, 1, 1, 36, MakeSortKey(0, 1, 5.0f, 3, false));
    cb.Draw(1, 1, 1, 36, MakeSortKey(0, 1, 15.0f, 7, false));
    cb.Draw(2, 1, 1, 36, MakeSortKey(0, 2, 1.0f, 1, false));

    std::vector<DrawCmd> executed;
    cb.Submit([&](const DrawCmd& cmd){ executed.push_back(cmd); });

    ASSERT_EQ(executed.size(), 4);
    ASSERT_EQ(executed[0].pipeline, 1);
    ASSERT_EQ(executed[1].pipeline, 1);
    ASSERT_EQ(executed[2].pipeline, 2);
    ASSERT_EQ(executed[3].pipeline, 2);
}

TEST_CASE(TransparentRenderSystemOpaqueFirstThenTransparent) {
    TransparentRenderSystem trs;
    CommandBuffer cb;

    trs.Add({1, 1,1,36, 1, 10.0f, false, 1});
    trs.Add({2, 1,1,36, 1, 5.0f, false, 2});
    trs.Add({3, 1,1,36, 2, 50.0f, true, 3});
    trs.Add({4, 1,1,36, 2, 1.0f, true, 4});

    trs.Prepare(cb, 0);

    std::vector<DrawCmd> executed;
    cb.Submit([&](const DrawCmd& cmd){ executed.push_back(cmd); });

    ASSERT_EQ(executed.size(), 4);
    // Opaque primero (front-to-back: depth 5, luego 10)
    // Transparent después (back-to-front: depth 50, luego 1)
    // Verificar que los dos primeros son opaque
    ASSERT_NEAR((executed[0].sort_key>>16)&0xFFFF, 500, 10);
    ASSERT_NEAR((executed[1].sort_key>>16)&0xFFFF, 1000, 10);
}

TEST_CASE(InstancingSystemGroupsByMeshMaterial) {
    InstancingSystem isys;
    CommandBuffer cb;

    isys.Add({1, 10, 10, 36, 1, 5.0f, false, 1});
    isys.Add({2, 10, 10, 36, 1, 10.0f, false, 1});
    isys.Add({3, 10, 10, 36, 1, 15.0f, false, 1});
    isys.Add({4, 10, 10, 36, 2, 20.0f, false, 2});
    isys.Add({5, 10, 10, 36, 2, 25.0f, false, 2});

    isys.GenerateCommands(cb, 0, [](size_t){ return BufferHandle(100); });

    std::vector<DrawCmd> executed;
    cb.Submit([&](const DrawCmd& cmd){ executed.push_back(cmd); });

    ASSERT_EQ(executed.size(), 2);
    uint32_t total_instances = 0;
    for(const auto& cmd : executed){
        ASSERT_GT(cmd.instance_count, 1);
        total_instances += cmd.instance_count;
    }
    ASSERT_EQ(total_instances, 5);
}

TEST_CASE(SortKeyEncoding) {
    uint64_t key = MakeSortKey(1, 5, 10.5f, 42, false);
    ASSERT_EQ((key >> 48) & 0xFFFF, 1);
    ASSERT_EQ((key >> 32) & 0xFFFF, 5);
    ASSERT_EQ((key >> 16) & 0xFFFF, 1050);
    ASSERT_EQ(key & 0xFFFF, 42);
}

TEST_CASE(TransparentSortKeyInverted) {
    uint64_t key_opaque = MakeSortKey(0, 1, 100.0f, 1, false);
    uint64_t key_transp = MakeSortKey(0, 1, 100.0f, 1, true);
    uint16_t depth_opaque = (key_opaque >> 16) & 0xFFFF;
    uint16_t depth_transp = (key_transp >> 16) & 0xFFFF;
    ASSERT_EQ(depth_opaque + depth_transp, 65535);
}

TEST_CASE(InstancingSeparateTransparentBatch) {
    InstancingSystem isys;
    CommandBuffer cb;

    isys.Add({1, 1,1,36, 1, 10.0f, false, 1});
    isys.Add({2, 1,1,36, 1, 20.0f, true, 1});

    isys.GenerateCommands(cb, 0, [](size_t){ return BufferHandle(1); });

    std::vector<DrawCmd> executed;
    cb.Submit([&](const DrawCmd& cmd){ executed.push_back(cmd); });

    ASSERT_EQ(executed.size(), 2);
    ASSERT_EQ(executed[0].instance_count, 1);
    ASSERT_EQ(executed[1].instance_count, 1);
}

int main() {
    std::cout << "Solución 06 (Render Systems)...\n";
    return RUN_ALL_TESTS();
}