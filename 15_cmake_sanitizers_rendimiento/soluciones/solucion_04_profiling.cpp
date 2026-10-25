/* =============================================================================
   MÓDULO 15 - SOLUCIÓN 04: Profiling
   ============================================================================= */
#include <iostream>
#include <string>
#include "../../include/test_framework.hpp"

TEST_CASE(PerfRecordCommand) {
    std::string cmd = "perf record -g -F 99 ./mi_programa";
    ASSERT_TRUE(cmd.find("-g") != std::string::npos);
    ASSERT_TRUE(cmd.find("-F 99") != std::string::npos);
}

TEST_CASE(PerfReportStdio) {
    std::string cmd = "perf report --stdio --sort=overhead";
    ASSERT_TRUE(cmd.find("--stdio") != std::string::npos);
    ASSERT_TRUE(cmd.find("--sort=overhead") != std::string::npos);
}

TEST_CASE(PerfStatCommand) {
    std::string cmd = "perf stat -e cache-misses,branch-misses ./exe";
    ASSERT_TRUE(cmd.find("cache-misses") != std::string::npos);
    ASSERT_TRUE(cmd.find("branch-misses") != std::string::npos);
}

TEST_CASE(FlameGraphPipeline) {
    std::string pipeline = "perf script | stackcollapse-perf.pl | flamegraph.pl > flame.svg";
    ASSERT_TRUE(pipeline.find("stackcollapse-perf.pl") != std::string::npos);
    ASSERT_TRUE(pipeline.find("flamegraph.pl") != std::string::npos);
    ASSERT_TRUE(pipeline.find("flame.svg") != std::string::npos);
}

TEST_CASE(GoogleBenchmarkRange) {
    std::string benchmark_code = "BENCHMARK(BM_VectorPushBack)->Range(8, 1024)->RangeMultiplier(2);";
    ASSERT_TRUE(benchmark_code.find("Range(8, 1024)") != std::string::npos);
    ASSERT_TRUE(benchmark_code.find("RangeMultiplier(2)") != std::string::npos);
}

TEST_CASE(BenchmarkArgs) {
    std::string code = "BENCHMARK(func)->Arg(100)->Arg(1000);";
    ASSERT_TRUE(code.find("Arg(100)") != std::string::npos);
    ASSERT_TRUE(code.find("Arg(1000)") != std::string::npos);
}

TEST_CASE(HyperfineCommand) {
    std::string cmd = "hyperfine './mi_programa' --warmup 3 --runs 10";
    ASSERT_TRUE(cmd.find("--warmup 3") != std::string::npos);
    ASSERT_TRUE(cmd.find("--runs 10") != std::string::npos);
}

TEST_CASE(PerfStatIPC) {
    double cycles = 5'000'000'000;
    double instructions = 10'000'000'000;
    double ipc = instructions / cycles;
    ASSERT_TRUE(ipc > 1.5 && ipc < 3.0);
}

TEST_CASE(FlameGraphInterpretation) {
    std::string flame_concept = "meseta ancha = hotspot";
    ASSERT_TRUE(flame_concept.find("hotspot") != std::string::npos);
}

TEST_CASE(PerfStatIPC2) {
    double cycles = 2'000'000'000;
    double instructions = 6'000'000'000;
    double ipc = instructions / cycles;
    ASSERT_TRUE(ipc > 2.5 && ipc < 4.0);
}

int main() {
    return RUN_ALL_TESTS();
}