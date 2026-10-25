/* =============================================================================
   MÓDULO 22 - LECCIÓN 09: Render Systems — Render Graph Simple, Pass Ordering,
   Transparent Sorting (Back-to-Front), Command Buffer
   =============================================================================
   TEMA 5 de TEMAS.md (1/2): Sistemas de render — render graph simple, pass ordering,
   transparent sorting y command buffer.

   RENDER GRAPH:
   -------------
   Grafo de pases de render (render passes) con dependencias de recursos.
   Cada nodo = RenderPass (input attachments → output attachments).
   Edges = dependencias de datos (output de A = input de B).
   Ventajas:
   - Reutilización de memoria (alias de attachments).
   - Scheduling automático (paralelismo GPU, barriers).
   - Explícito: fácil de debuggear y profilear.

   PASS ORDERING (orden de pases típico):
   -------------------------------------
   1. Shadow map passes (depth only, desde luz)
   2. G-Buffer / Deferred geometry pass (albedo, normal, material, depth)
   3. Lighting passes (deferred: screen-space; forward: por objeto)
   4. Transparent/Forward pass (back-to-front)
   5. Post-process (bloom, tonemap, FXAA, color grading)
   6. UI / Debug overlay

   TRANSPARENT SORTING (Back-to-Front):
   ------------------------------------
   Objetos transparentes deben dibujarse ORDENADOS por distancia a cámara (lejos → cerca).
   - Opaque: front-to-back (early-Z optimization) o cualquier orden.
   - Transparent: back-to-front (painter's algorithm) obligatorio para blending correcto.
   - Sort key: (depth, material_id, mesh_id) para batching dentro de transparentes.

   COMMAND BUFFER:
   ---------------
   En lugar de llamadas OpenGL inmediatas, sistemas generan comandos:
   struct DrawCmd { PipelineState* pipeline; Mesh* mesh; uint32_t index_count, base_vertex, base_instance; UniformBuffer* ubo; Texture* textures[8]; };
   RenderSystem: recopila DrawCmds → sort por (pipeline, depth, material) → submit.
   Ventajas:
   - Estado mínimo cambios (sort by pipeline).
   - Instancing automático (agrupar mismo mesh+material).
   - Multi-thread: workers generan comandos, hilo principal submite.

   ESTA LECCIÓN: RenderGraph con passes, CommandBuffer con sort keys,
   transparent sorting, pass dependencies.
   ============================================================================= */
#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include <memory>
#include <functional>
#include <algorithm>
#include <cstdint>
#include <cassert>
#include <array>

// ============================================================================
// TIPOS BASE MATEMÁTICOS (simplificados)
// ============================================================================
struct Vec3 { float x,y,z; };
struct Vec4 { float x,y,z,w; };
struct Mat4 { float m[16]; };  // Column-major

// ============================================================================
// RECURSOS GPU (handles opacos)
// ============================================================================
using TextureHandle = uint32_t;
using BufferHandle = uint32_t;
using PipelineHandle = uint32_t;
using FramebufferHandle = uint32_t;

constexpr TextureHandle INVALID_TEX = 0;
constexpr BufferHandle INVALID_BUF = 0;
constexpr PipelineHandle INVALID_PIPE = 0;
constexpr FramebufferHandle INVALID_FB = 0;

// ============================================================================
// RENDER PASS DEFINITION
// ============================================================================
enum class LoadOp { Clear, Load, DontCare };
enum class StoreOp { Store, DontCare };

struct AttachmentRef {
    std::string name;           // Nombre lógico (ej. "gbuffer_albedo")
    LoadOp load_op = LoadOp::Clear;
    StoreOp store_op = StoreOp::Store;
    Vec4 clear_color = {0,0,0,0};
    float clear_depth = 1.0f;
    uint32_t clear_stencil = 0;
};

struct SubpassDesc {
    std::vector<std::string> color_attachments;  // Nombres de attachments
    std::string depth_attachment;                // Opcional
    std::vector<std::string> input_attachments;  // Para subpasses siguientes
};

struct RenderPassDesc {
    std::string name;
    std::vector<AttachmentRef> attachments;
    std::vector<SubpassDesc> subpasses;
    // Dependencias implícitas: attachment escrito → leído por siguiente pass
};

// ============================================================================
// RENDER GRAPH
// ============================================================================
class RenderGraph {
    struct PassNode {
        RenderPassDesc desc;
        std::vector<std::string> reads;    // Attachment names leídos
        std::vector<std::string> writes;   // Attachment names escritos
        int pass_index = -1;               // Orden de ejecución
    };

    std::vector<PassNode> passes_;
    std::unordered_map<std::string, PassNode*> attachment_to_writer_;

public:
    void AddPass(RenderPassDesc desc) {
        PassNode node; node.desc = std::move(desc);
        // Inferir reads/writes de subpasses
        for(const auto& sp : node.desc.subpasses){
            node.reads.insert(node.reads.end(), sp.input_attachments.begin(), sp.input_attachments.end());
            node.writes.insert(node.writes.end(), sp.color_attachments.begin(), sp.color_attachments.end());
            if(!sp.depth_attachment.empty()) node.writes.push_back(sp.depth_attachment);
        }
        passes_.push_back(std::move(node));
    }

    // Compilar: resolver orden topológico por dependencias de attachments
    void Compile() {
        // Build dependency graph: pass A writes X, pass B reads X → A before B
        std::vector<std::vector<int>> adj(passes_.size());
        std::vector<int> indeg(passes_.size(), 0);

        // Map attachment → writer pass index
        attachment_to_writer_.clear();
        for(size_t i=0;i<passes_.size();++i){
            for(const auto& w : passes_[i].writes){
                attachment_to_writer_[w] = &passes_[i];
            }
        }

        for(size_t i=0;i<passes_.size();++i){
            for(const auto& r : passes_[i].reads){
                auto it = attachment_to_writer_.find(r);
                if(it != attachment_to_writer_.end()){
                    PassNode* writer = it->second;
                    int writer_idx = &writer - &passes_[0];  // Index en vector
                    if(writer_idx != (int)i){
                        adj[writer_idx].push_back(i);
                        indeg[i]++;
                    }
                }
            }
        }

        // Topological sort (Kahn)
        std::queue<int> q;
        for(size_t i=0;i<passes_.size();++i) if(indeg[i]==0) q.push(i);
        int idx = 0;
        while(!q.empty()){
            int u = q.front(); q.pop();
            passes_[u].pass_index = idx++;
            for(int v : adj[u]) if(--indeg[v]==0) q.push(v);
        }
        // Verificar ciclos
        for(int d : indeg) if(d>0) std::cerr << "RenderGraph: ciclo detectado!\n";

        // Ordenar passes por pass_index
        std::sort(passes_.begin(), passes_.end(), [](const PassNode& a, const PassNode& b){
            return a.pass_index < b.pass_index;
        });
    }

    const std::vector<PassNode>& GetPasses() const { return passes_; }
};

// ============================================================================
// COMMAND BUFFER
// ============================================================================
struct DrawCmd {
    // Sort key: 64-bit para radix sort rápido
    // Bits: [63:48] pass_index | [47:32] pipeline_id | [31:16] depth (quantized) | [15:0] material_id
    uint64_t sort_key = 0;

    PipelineHandle pipeline = INVALID_PIPE;
    BufferHandle vertex_buffer = INVALID_BUF;
    BufferHandle index_buffer = INVALID_BUF;
    uint32_t index_count = 0;
    uint32_t base_vertex = 0;
    uint32_t instance_count = 1;
    uint32_t base_instance = 0;

    // Uniforms / Resources
    BufferHandle ubo = INVALID_BUF;           // Uniform buffer (per-draw o per-material)
    TextureHandle textures[8] = {INVALID_TEX}; // Slots de textura
    uint32_t texture_count = 0;

    // Para debug
    std::string debug_name;
};

class CommandBuffer {
    std::vector<DrawCmd> commands_;
    uint32_t current_pass_index_ = 0;

public:
    void BeginPass(uint32_t pass_index) { current_pass_index_ = pass_index; }
    void EndPass() {}

    void Draw(const DrawCmd& cmd) {
        DrawCmd c = cmd;
        // Construir sort key
        // Pipeline bits: 16 bits (asumimos <65536 pipelines)
        // Depth: 16 bits (0=near, 65535=far para transparent back-to-front)
        // Material: 16 bits
        uint64_t pipeline_bits = (uint64_t(c.pipeline) & 0xFFFF) << 32;
        uint64_t depth_bits = (uint64_t(c.sort_key & 0xFFFF)) << 16;  // Usuario setea depth en sort_key low 16
        uint64_t material_bits = (c.sort_key >> 16) & 0xFFFF;          // High 16 de sort_key original = material
        c.sort_key = (uint64_t(current_pass_index_) << 48) | pipeline_bits | depth_bits | material_bits;
        commands_.push_back(c);
    }

    // Sort y submit
    void Submit(std::function<void(const DrawCmd&)> execute_fn) {
        // Radix sort por sort_key (64-bit)
        std::sort(commands_.begin(), commands_.end(), [](const DrawCmd& a, const DrawCmd& b){
            return a.sort_key < b.sort_key;
        });

        // Ejecutar agrupando por pipeline para minimizar state changes
        PipelineHandle current_pipe = INVALID_PIPE;
        for(const auto& cmd : commands_){
            if(cmd.pipeline != current_pipe){
                // Bind pipeline
                // execute_fn({.pipeline = cmd.pipeline, .debug_name = "BIND_PIPELINE"});
                current_pipe = cmd.pipeline;
            }
            execute_fn(cmd);
        }
        commands_.clear();
    }

    size_t CommandCount() const { return commands_.size(); }
};

// ============================================================================
// TRANSPARENT SORTING SYSTEM
// ============================================================================
struct Renderable {
    Entity entity;
    Mat4 world;
    float depth;  // Distancia a cámara (para transparent sort)
    PipelineHandle pipeline;
    BufferHandle vb, ib;
    uint32_t index_count, base_vertex;
    BufferHandle ubo;
    TextureHandle textures[8];
    uint32_t tex_count;
    bool transparent;
    uint16_t material_id;
};

using Entity = uint32_t;

class TransparentRenderSystem {
    std::vector<Renderable> opaque_;
    std::vector<Renderable> transparent_;

public:
    void Add(const Renderable& r) {
        if(r.transparent) transparent_.push_back(r);
        else opaque_.push_back(r);
    }

    void Prepare(CommandBuffer& cmd_buf, const Mat4& view_proj, uint32_t pass_idx) {
        // Opaque: front-to-back (menor depth primero) para early-Z
        std::sort(opaque_.begin(), opaque_.end(), [](const Renderable& a, const Renderable& b){
            return a.depth < b.depth;
        });
        // Transparent: back-to-front (mayor depth primero)
        std::sort(transparent_.begin(), transparent_.end(), [](const Renderable& a, const Renderable& b){
            return a.depth > b.depth;
        });

        cmd_buf.BeginPass(pass_idx);
        // Submit opaque
        for(const auto& r : opaque_){
            DrawCmd cmd;
            cmd.pipeline = r.pipeline;
            cmd.vertex_buffer = r.vb;
            cmd.index_buffer = r.ib;
            cmd.index_count = r.index_count;
            cmd.base_vertex = r.base_vertex;
            cmd.ubo = r.ubo;
            for(uint32_t i=0;i<r.tex_count;++i) cmd.textures[i] = r.textures[i];
            cmd.texture_count = r.tex_count;
            // Sort key: depth quantizado (0..65535) + material_id
            uint16_t depth_q = uint16_t(std::min(65535.0f, r.depth * 100.0f));  // Quantize
            cmd.sort_key = (uint64_t(depth_q) << 16) | r.material_id;
            cmd.debug_name = "Opaque";
            cmd_buf.Draw(cmd);
        }
        // Submit transparent
        for(const auto& r : transparent_){
            DrawCmd cmd;
            cmd.pipeline = r.pipeline;
            cmd.vertex_buffer = r.vb;
            cmd.index_buffer = r.ib;
            cmd.index_count = r.index_count;
            cmd.base_vertex = r.base_vertex;
            cmd.ubo = r.ubo;
            for(uint32_t i=0;i<r.tex_count;++i) cmd.textures[i] = r.textures[i];
            cmd.texture_count = r.tex_count;
            uint16_t depth_q = uint16_t(std::min(65535.0f, r.depth * 100.0f));
            // Para back-to-front: invertir depth en sort key (mayor depth = menor sort key)
            cmd.sort_key = (uint64_t(65535 - depth_q) << 16) | r.material_id;
            cmd.debug_name = "Transparent";
            cmd_buf.Draw(cmd);
        }
        cmd_buf.EndPass();

        opaque_.clear();
        transparent_.clear();
    }
};

// ============================================================================
// VERIFICACIÓN DE SINTAXIS
// ============================================================================
int main() {
    std::cout << "=== LECCIÓN 09: Render Systems (solo sintaxis) ===\n";

    // Render Graph
    RenderGraph rg;
    rg.AddPass({
        "ShadowMap",
        {{"shadow_depth", LoadOp::Clear, StoreOp::Store, {}, 1.0f}},
        {{{}, "shadow_depth"}}
    });
    rg.AddPass({
        "GBuffer",
        {{"gbuf_albedo", LoadOp::Clear, StoreOp::Store}, {"gbuf_normal", LoadOp::Clear, StoreOp::Store}, {"gbuf_depth", LoadOp::Clear, StoreOp::Store}},
        {{{"gbuf_albedo", "gbuf_normal"}, "gbuf_depth"}}
    });
    rg.AddPass({
        "Lighting",
        {{"hdr_color", LoadOp::Clear, StoreOp::Store}},
        {{{"hdr_color"}, "", {"gbuf_albedo", "gbuf_normal", "gbuf_depth"}}}
    });
    rg.AddPass({
        "Transparent",
        {{"hdr_color", LoadOp::Load, StoreOp::Store}},
        {{{"hdr_color"}, ""}}
    });
    rg.AddPass({
        "PostProcess",
        {{"final_color", LoadOp::Clear, StoreOp::Store}},
        {{{"final_color"}, "", {"hdr_color"}}}
    });
    rg.Compile();
    std::cout << "Render passes order:\n";
    for(const auto& p : rg.GetPasses()){
        std::cout << "  " << p.pass_index << ": " << p.desc.name << "\n";
    }

    // Command Buffer + Transparent Sort
    CommandBuffer cb;
    TransparentRenderSystem trs;

    // Añadir algunos renderables
    trs.Add({1, {}, 10.0f, 1, 1, 1, 100, 0, 1, 1, {1}, 1, false, 10});
    trs.Add({2, {}, 50.0f, 1, 1, 1, 100, 0, 1, 1, {1}, 1, false, 20});
    trs.Add({3, {}, 5.0f, 2, 2, 2, 50, 0, 1, 1, {2}, 1, true, 30});   // Transparent cerca
    trs.Add({4, {}, 100.0f, 2, 2, 2, 50, 0, 1, 1, {2}, 1, true, 40}); // Transparent lejos

    Mat4 vp{};
    trs.Prepare(cb, vp, 3);  // Pass 3 = Transparent
    std::cout << "Commands in buffer: " << cb.CommandCount() << "\n";

    cb.Submit([](const DrawCmd& cmd){
        std::cout << "  Draw: pipe=" << cmd.pipeline << " sort_key=" << cmd.sort_key
                  << " (" << cmd.debug_name << ")\n";
    });

    std::cout << "=== FIN LECCIÓN 09 ===\n";
    return 0;
}