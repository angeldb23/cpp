/* =============================================================================
   MÓDULO 15 - LECCIÓN 09: Profiling — perf, callgrind, benchmarks
   =============================================================================
   TEMA 8 de TEMAS.md (2 lecciones): Profiling y benchmarks.

   PERF (Linux) — Profiling estadístico (sampling):
   - perf record -g ./exe          # Grabar (con call graph -g)
   - perf report                   # Reporte TUI
   - perf report --stdio           # Texto plano
   - perf script                   # Raw events
   - perf stat ./exe               # Contadores HW (cycles, instructions, cache-misses)
   - perf stat -e cache-misses,branch-misses ./exe
   - perf top                      # Live profiling (top-like)

   CALLGRIND (Valgrind) — Instrumentación completa:
   - valgrind --tool=callgrind ./exe
   - callgrind_annotate callgrind.out.xxx
   - kcachegrind / qcachegrind     # GUI visual
   - Más lento que perf (~10-50x) pero preciso (cuenta instrucciones exactas).

   GOOGLE BENCHMARK — Microbenchmarks:
   - #include <benchmark/benchmark.h>
   - BENCHMARK(func)->Arg(100)->Arg(1000);
   - BENCHMARK(func)->Range(8, 1024);
   - BENCHMARK(func)->RangeMultiplier(2);
   - State::PauseTiming() / ResumeTiming()
   - state.counters["metric"] = value;
   - Compilar: -O2 -DNDEBUG -lbenchmark -lpthread

   HYPERFINE — CLI benchmarking:
   - hyperfine './exe' --warmup 3 --runs 10
   - hyperfine 'cmd1' 'cmd2' --compare
   - Exporta JSON/Markdown.

   FLAME GRAPHS (Brendan Gregg):
   - perf record -g -F 99 ./exe
   - perf script | stackcollapse-perf.pl | flamegraph.pl > flame.svg
   - flamegraph.pl en github.com/brendangregg/FlameGraph
   - Visualiza call stacks como "flamas" (ancho = tiempo).

   TAU / SCORE-P / VTUNE / NSIGHT:
   - Herramientas enterprise para HPC / GPU.
   - Intel VTune: GUI potente, análisis roofline, memory access.
   - NVIDIA Nsight: CUDA profiling.

   BENCHMARKING CORRECTO (reglas de oro):
   1. Warm-up runs (JIT, cache, frequency scaling)
   2. Múltiples runs, usar MEDIANA (no media: outliers)
   3. Desactivar frequency scaling: cpupower frequency-set -g performance
   3. Aislar cores: taskset -c 0-3 ./bench
   4. Desactivar ASLR: echo 0 > /proc/sys/kernel/randomize_va_space
   5. Medir wall-clock (steady_clock), no CPU time.
   6. Mínimo 10 runs, reportar mediana + MAD / percentiles.
   7. Reportar: throughput (ops/sec), latencia (p50, p95, p99).

   MICROBENCHMARKS VS MACROBENCHMARKS:
   - Micro: función individual, loop tight (Google Benchmark)
   - Macro: endpoint-to-endpoint, sistema completo (wrk, vegeta, k6)

   FLAME GRAPH INTERPRETACIÓN:
   - Eje X: población de samples (no tiempo lineal)
   - Eje Y: profundidad de stack
   - Ancho = % tiempo CPU
   - "Mesetas" anchas = hotspots
   - "Picos" estrechos = funciones leaf

   CONTADORES HW (perf stat):
   - cycles: ciclos de reloj
   - instructions: instrucciones retiradas
   - IPC = instructions/cycles (ideal ~3-4 en x86 moderno)
   - cache-references / cache-misses: L1/L2/L3 misses
   - branch-instructions / branch-misses: branch predictor
   - stalled-cycles-frontend/backend: pipeline stalls

   ROOFLINE MODEL (VTune / manual):
   - Memory bound vs Compute bound
   - Arithmetic intensity (FLOPs/byte)
   - Peak bandwidth vs Peak FLOPs

   CONTINUOUS BENCHMARKING:
   - GitHub Actions + benchmark-action
   - Conbench / CodSpeed para tracking histórico
   - Alertar en regresiones > 5% */

#include <iostream>
#include <chrono>
#include <vector>
#include <algorithm>
#include <random>

// Demo: microbenchmark con Google Benchmark (código de referencia, no compila sin la lib)
// static void BM_VectorPushBack(benchmark::State& state) {
//     for (auto _ : state) {
//         std::vector<int> v;
//         v.reserve(state.range(0));
//         for (int i = 0; i < state.range(0); ++i) {
//             v.push_back(i);
//         }
//         benchmark::DoNotOptimize(v);
//     }
// }
// BENCHMARK(BM_VectorPushBack)->Range(8, 1024)->RangeMultiplier(2);
//
// static void BM_StringConcat(benchmark::State& state) {
//     std::string s;
//     for (auto _ : state) {
//         s.clear();
//         for (int i = 0; i < state.range(0); ++i) {
//             s += "x";
//         }
//         benchmark::DoNotOptimize(s);
//     }
// }
// BENCHMARK(BM_StringConcat)->Range(10, 10000)->RangeMultiplier(10);

void demo_perf() {
    std::cout << "--- perf (Linux profiling) ---\n";
    std::cout << R"(
# Grabar perfil
perf record -g -F 99 ./mi_benchmark    # -g=call graph, -F=freq Hz
perf record -e cycles:u ./exe          # Solo user-space cycles

# Reportes
perf report                            # TUI interactivo
perf report --stdio --sort=overhead    # Texto, ordenado por overhead
perf report --stdio -g graph           # Call graph colapsado

# Contadores HW
perf stat ./exe                        # cycles, instructions, cache-misses...
perf stat -e cache-misses,branch-misses,L1-dcache-load-misses ./exe

# Live
perf top                               # Live profiling

# Flame graph
perf record -g -F 99 ./exe
perf script | stackcollapse-perf.pl | flamegraph.pl > flame.svg
# Ver en navegador: flame.svg
)" << "\n\n";
}

void demo_callgrind() {
    std::cout << "--- Callgrind (Valgrind) ---\n";
    std::cout << R"(
# Instrumentación completa (preciso, lento)
valgrind --tool=callgrind ./mi_programa
# Opciones:
# --collect-jumps=yes    # Saltos (para call graph)
# --collect-bus=yes      # Simulación cache
# --dump-instr=yes       # Instrucciones por línea

# Análisis
callgrind_annotate callgrind.out.12345 --threshold=0.1
# O GUI:
kcachegrind callgrind.out.12345
qcachegrind callgrind.out.12345

# Diferencia con perf:
# - callgrind: instrumentación completa, cuenta instrucciones exactas
# - perf: sampling estadístico, ~1% overhead vs 10-50x callgrind
)" << "\n\n";
}

void demo_benchmark() {
    std::cout << "--- Google Benchmark ---\n";
    std::cout << R"(
# include <benchmark/benchmark.h>

static void BM_VectorPushBack(benchmark::State& state) {
    for (auto _ : state) {
        std::vector<int> v;
        v.reserve(state.range(0));
        for (int i = 0; i < state.range(0); ++i) {
            v.push_back(i);
        }
        benchmark::DoNotOptimize(v);
    }
BENCHMARK(BM_VectorPushBack)->Range(8, 1024)->RangeMultiplier(2);

# Argumentos:
# ->Arg(100)                    # Valor fijo
# ->Range(8, 1024)              # Rango [8, 1024]
# ->RangeMultiplier(2)          # x2 cada step (8,16,32,64...)
# ->DenseRange(0, 100)          # Todos los valores
# ->ArgNames({"N", "M"})        # Nombres para 2 args

# Setup/Teardown por iteración:
state.PauseTiming();  // setup costoso
// setup...
state.ResumeTiming();

// Métricas custom
state.counters["cache_misses"] = get_cache_misses();
state.counters["rate"] = benchmark::Counter(x, benchmark::Counter::kIsRate);

# Compilar:
# g++ -O2 -DNDEBUG -std=c++20 bench.cpp -lbenchmark -lpthread -o bench

# Ejecutar:
# ./bench --benchmark_format=console
# ./bench --benchmark_format=json --benchmark_out=results.json
# ./bench --benchmark_filter="BM_VectorPushBack.*"
# ./bench --benchmark_min_time=1s --benchmark_repetitions=3
)" << "\n\n";
}

void demo_hyperfine() {
    std::cout << "--- hyperfine (CLI benchmarking) ---\n";
    std::cout << R"(
# Instalación: cargo install hyperfine / apt install hyperfine

# Básico
hyperfine './mi_programa'

# Warmup + runs
hyperfine './mi_programa' --warmup 3 --runs 10

# Comparar dos comandos
hyperfine 'gcc -O2 prog.c -o prog_gcc' 'clang -O2 prog.c -o prog_clang'

# Exportar
hyperfine './prog' --export-json results.json
hyperfine './prog' --export-markdown results.md

# Filtros
hyperfine --prepare 'sync; echo 3 | sudo tee /proc/sys/vm/drop_caches' './prog'
)" << "\n\n";
}

void demo_flamegraphs() {
    std::cout << "--- Flame Graphs (Brendan Gregg) ---\n";
    std::cout << R"(
# Generar
perf record -g -F 99 ./mi_programa
perf script | stackcollapse-perf.pl | flamegraph.pl > flame.svg

# stackcollapse-perf.pl y flamegraph.pl:
# git clone https://github.com/brendangregg/FlameGraph
# export PATH=$PATH:/path/to/FlameGraph

# Interpretación:
# - Eje X: población de samples (proporcional a tiempo CPU)
# - Eje Y: profundidad de stack (base = main)
# - Ancho = % tiempo CPU
# - "Mesetas" anchas = hotspots (funciones que consumen tiempo)
# - "Picos" estrechos = funciones leaf
# - Colores: aleatorios (solo estética)

# Variantes:
# - Differential flame graphs (diff dos perfiles)
# - Off-CPU flame graphs (blocked time)
# - Memory flame graphs (allocations)
)" << "\n\n";
}

void demo_benchmarking_rules() {
    std::cout << "--- Reglas de benchmarking correcto ---\n";
    std::cout << R"(
1. WARM-UP RUNS: 3-5 runs antes de medir (JIT, cache, freq scaling)
2. MÚLTIPLES RUNS: mínimo 10, usar MEDIANA (no media: outliers)
3. FREQUENCY SCALING: cpupower frequency-set -g performance
4. AISLAR CORES: taskset -c 0-3 ./bench
5. DESACTIVAR ASLR: echo 0 > /proc/sys/kernel/randomize_va_space
6. WALL-CLOCK: steady_clock, no CPU time
7. MÍNIMO 10 RUNS: reportar mediana + MAD / p95/p99
8. REPORTAR: throughput (ops/sec), latencia p50/p95/p99

# Ejemplo script robusto:
#!/bin/bash
cpupower frequency-set -g performance
taskset -c 0 ./bench --benchmark_min_time=1 --benchmark_repetitions=10 \
  --benchmark_out=results.json --benchmark_out_format=json

# En C++ (manual):
auto measure = [](auto&& fn) {
    std::vector<double> times;
    for (int i = 0; i < 10; ++i) {
        auto t1 = std::chrono::steady_clock::now();
        fn();
        auto t2 = std::chrono::steady_clock::now();
        times.push_back(std::chrono::duration<double, std::milli>(t2-t1).count());
    }
    std::sort(times.begin(), times.end());
    return times[times.size()/2];  // mediana
}
)" << "\n\n";
}

void demo_perf_stat() {
    std::cout << "--- perf stat: contadores HW ---\n";
    std::cout << R"(
perf stat ./exe
#  Performance counter stats for './exe':
#          1.234567 seconds time elapsed
#      5,000,000,000 cycles                    # 0.50 GHz
#     10,000,000,000 instructions              # 2.00 IPC
#        1,000,000 cache-references            # 10.0% miss rate
#           100,000 cache-misses
#        500,000,000 branch-instructions
#           10,000 branch-misses               # 0.00% miss rate

# Métricas clave:
# - IPC = instructions/cycles (ideal 3-4 en x86 moderno)
# - cache-miss rate: cache-misses / cache-references
# - branch-miss rate: branch-misses / branch-instructions
# - stalled-cycles-frontend/backend: pipeline stalls

# Eventos específicos:
perf stat -e cycles,instructions,cache-references,cache-misses,branch-instructions,branch-misses ./exe
perf stat -e L1-dcache-load-misses,LLC-load-misses ./exe
)" << "\n\n";
}

int main() {
    std::cout << "=== LECCIÓN 09: Profiling — perf, callgrind, benchmarks ===\n\n";

    demo_perf();
    demo_callgrind();
    demo_benchmark();
    demo_hyperfine();
    demo_flamegraphs();
    demo_benchmarking_rules();
    demo_perf_stat();

    std::cout << "=== FIN LECCIÓN 09 ===\n";
    return 0;
}