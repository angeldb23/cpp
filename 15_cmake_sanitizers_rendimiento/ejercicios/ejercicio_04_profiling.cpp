/* =============================================================================
   MÓDULO 15 - EJERCICIO 04: Profiling — perf, Flame graphs, Benchmarks
   =============================================================================
   DIFICULTAD: Intermedio
   CONSIGNA:
     Completar los comandos perf y benchmark. Escribir la respuesta esperada.

   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra -I../../include ejercicio_04_profiling.cpp -o ej04 && ./ej04
   ============================================================================= */
#include <iostream>
#include <string>
#include "../../include/test_framework.hpp"

// TODO 1: Comando perf record con call graph y frecuencia
std::string get_perf_record_cmd() {
    return "";  // TU CÓDIGO: "perf record -g -F 99 ./mi_programa"
}

// TODO 2: Comando perf report con stdio y sort
std::string get_perf_report_cmd() {
    return "";  // TU CÓDIGO: "perf report --stdio --sort=overhead"
}

// TODO 3: Comando perf stat con eventos específicos
std::string get_perf_stat_cmd() {
    return "";  // TU CÓDIGO: "perf stat -e cache-misses,branch-misses ./exe"
}

// TODO 4: Pipeline para flame graph
std::string get_flamegraph_pipeline() {
    return "";  // TU CÓDIGO: "perf script | stackcollapse-perf.pl | flamegraph.pl > flame.svg"
}

// TODO 5: Google Benchmark con Range y RangeMultiplier
std::string get_benchmark_range() {
    return "";  // TU CÓDIGO: "BENCHMARK(BM_VectorPushBack)->Range(8, 1024)->RangeMultiplier(2);"
}

// TODO 6: Google Benchmark con Args
std::string get_benchmark_args() {
    return "";  // TU CÓDIGO: "BENCHMARK(func)->Arg(100)->Arg(1000);"
}

// TODO 7: hyperfine con warmup y runs
std::string get_hyperfine_cmd() {
    return "";  // TU CÓDIGO: "hyperfine './mi_programa' --warmup 3 --runs 10"
}

// TODO 8: Calcular IPC (instructions per cycle)
double calcular_ipc1() {
    double cycles = 5'000'000'000;
    double instructions = 10'000'000'000;
    return 0.0;  // TU CÓDIGO: instructions / cycles
}

// TODO 9: Concepto flame graph - meseta ancha = hotspot
std::string get_flamegraph_concept() {
    return "";  // TU CÓDIGO: "meseta ancha = hotspot"
}

// TODO 10: Calcular IPC 2
double calcular_ipc2() {
    double cycles = 2'000'000'000;
    double instructions = 6'000'000'000;
    return 0.0;  // TU CÓDIGO: instructions / cycles
}

TEST_CASE(PerfRecordCommand) {
    std::string cmd = get_perf_record_cmd();
    ASSERT_TRUE(cmd.find("-g") != std::string::npos);
    ASSERT_TRUE(cmd.find("-F 99") != std::string::npos);
}

TEST_CASE(PerfReportStdio) {
    std::string cmd = get_perf_report_cmd();
    ASSERT_TRUE(cmd.find("--stdio") != std::string::npos);
    ASSERT_TRUE(cmd.find("--sort=overhead") != std::string::npos);
}

TEST_CASE(PerfStatCommand) {
    std::string cmd = get_perf_stat_cmd();
    ASSERT_TRUE(cmd.find("cache-misses") != std::string::npos);
    ASSERT_TRUE(cmd.find("branch-misses") != std::string::npos);
}

TEST_CASE(FlameGraphPipeline) {
    std::string pipeline = get_flamegraph_pipeline();
    ASSERT_TRUE(pipeline.find("stackcollapse-perf.pl") != std::string::npos);
    ASSERT_TRUE(pipeline.find("flamegraph.pl") != std::string::npos);
    ASSERT_TRUE(pipeline.find("flame.svg") != std::string::npos);
}

TEST_CASE(GoogleBenchmarkRange) {
    std::string benchmark_code = get_benchmark_range();
    ASSERT_TRUE(benchmark_code.find("Range(8, 1024)") != std::string::npos);
    ASSERT_TRUE(benchmark_code.find("RangeMultiplier(2)") != std::string::npos);
}

TEST_CASE(BenchmarkArgs) {
    std::string code = get_benchmark_args();
    ASSERT_TRUE(code.find("Arg(100)") != std::string::npos);
    ASSERT_TRUE(code.find("Arg(1000)") != std::string::npos);
}

TEST_CASE(HyperfineCommand) {
    std::string cmd = get_hyperfine_cmd();
    ASSERT_TRUE(cmd.find("--warmup 3") != std::string::npos);
    ASSERT_TRUE(cmd.find("--runs 10") != std::string::npos);
}

TEST_CASE(PerfStatIPC) {
    double ipc = calcular_ipc1();
    ASSERT_TRUE(ipc > 1.5 && ipc < 3.0);
}

TEST_CASE(FlameGraphInterpretation) {
    std::string flame_concept = get_flamegraph_concept();
    ASSERT_TRUE(flame_concept.find("hotspot") != std::string::npos);
}

TEST_CASE(PerfStatIPC2) {
    double ipc = calcular_ipc2();
    ASSERT_TRUE(ipc > 2.5 && ipc < 4.0);
}

int main() {
    return RUN_ALL_TESTS();
}