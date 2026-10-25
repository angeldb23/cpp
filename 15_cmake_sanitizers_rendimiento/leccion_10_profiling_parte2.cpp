/* =============================================================================
   MÓDULO 15 - LECCIÓN 10: Profiling avanzado — VTune, NSight, perf scripting,
   continuous benchmarking, regression detection
   =============================================================================
   TEMA 8 de TEMAS.md (2da lección): Profiling avanzado.

   INTEL VTUNE PROFILER:
   - GUI potente, análisis roofline, memory access, threading
   - vtune -collect hotspots ./exe
   - vtune -collect memory-access ./exe
   - vtune -collect threading ./exe
   - vtune -collect hpc-performance ./exe (HPC)
   - vtune-gui para análisis visual

   NVIDIA NSIGHT SYSTEMS / COMPUTE:
   - nsight-systems: system-wide profiling (CPU+GPU)
   - nsight-compute: kernel-level GPU profiling
   - nccl/loggers para distributed training

   PERF SCRIPTING / PYTHON:
   - perf script -s script.py
   - Python API para procesar perf.data
   - Exportar a pandas/plotly para análisis

   CONTINUOUS BENCHMARKING:
   - GitHub Actions + benchmark-action
   - Conbench / CodSpeed para tracking histórico
   - Alertar en regresiones > 5%
   - Benchmark en PR: comparar base vs PR

   REGRESSION DETECTION:
   - Statistical: t-test / Mann-Whitney entre base y PR
   - Threshold: >5% regression = fail
   - Noise handling: median + MAD, mínimo 30 runs
   - Hardware noise: cpupower frequency-set -g performance

   MICROBENCHMARKS VS MACROBENCHMARKS:
   - Micro: función individual (Google Benchmark)
   - Macro: endpoint-to-endpoint (wrk, vegeta, k6, locust)
   - Soak testing: carga sostenida 24h+

   FLAME GRAPHS AVANZADOS:
   - Differential: diff dos perfiles (antes/después)
   - Off-CPU: tiempo bloqueado (I/O, locks)
   - Memory: allocations profile
   - Differential flame graphs: --diff

   PROFILING EN PRODUCCIÓN:
   - eBPF profilers: py-spy, rbspy, gdbpy
   - Continuous profiling: Parca, Polar Signals
   - Sampling profilers en prod (overhead <1%)

   ROOFLINE MODEL:
   - Arithmetic Intensity (FLOPs/Byte)
   - Memory Bound vs Compute Bound
   - Peak Bandwidth vs Peak FLOPs
   - VTune roofline analysis

   HPC PROFILING:
   - MPI profiling: mpiP, IPM
   - OpenMP: OMPT tools
   - CUDA: NCU/NSight
   - SYCL/oneAPI: VTune

   BENCHMARKING EN CI:
   - benchmark-action (GitHub Actions)
   - Conbench / CodSpeed (tracking histórico)
   - Alertar en regresiones >5%
   - Baseline: main branch, PR: comparar

   MICROBENCHMARKS BEST PRACTICES:
   - DoNotOptimize / ClobberMemory
   - PauseTiming / ResumeTiming para setup
   - Counters custom (cache misses, branch misses)
   - State::SetItemsProcessed / SetBytesProcessed

   STATISTICAL RIGOR:
   - Warmup: 5-10 runs
   - Runs: mínimo 30 para significancia estadística
   - Reportar: mediana + MAD, p50/p95/p99
   - Effect size: Cohen's d para comparaciones
   - Multiple comparison correction (Bonferroni) */

#include <iostream>
#include <chrono>
#include <vector>
#include <algorithm>
#include <random>
#include <cmath>

void demo_continuous_benchmarking() {
    std::cout << "--- Continuous Benchmarking ---\n";
    std::cout << R"(
# GitHub Actions + benchmark-action
# .github/workflows/benchmark.yml:
name: Benchmark
on: [push, pull_request]
jobs:
  benchmark:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - name: Build
        run: cmake --preset=release && cmake --build --preset=release
      - name: Run benchmarks
        run: ./bench --benchmark_out=results.json --benchmark_out_format=json
      - uses: benchmark-action/github-action-benchmark@v1
        with:
          tool: 'google_benchmark'
          output-file-path: results.json
          github-token: ${{ secrets.GITHUB_TOKEN }}
          alert-threshold: '5%'  # Alerta si regresión >5%

# Conbench / CodSpeed (tracking histórico)
# - Conbench: OSS, self-hosted
# - CodSpeed: SaaS, gratis para open source
# - Tracking histórico de performance
# - Alertas en regresiones
)" << "\n\n";
    std::cout << "--- Regression Detection ---\n";
    std::cout << R"(
# Statistical regression detection
# 1. Recopilar N runs (mín 30) en base y PR
# 2. Calcular mediana + MAD en cada grupo
# 3. Test estadístico: Mann-Whitney U (no paramétrico) o t-test
# 4. Effect size: Cohen's d
# 4. Threshold: >5% regresión = fail

# Cohen's d = (median_PR - median_base) / pooled_MAD
# d > 0.8 = large effect

# Múltiples comparaciones: Bonferroni correction
# alpha_adjusted = 0.05 / n_comparisons

# Ejemplo Python:
import numpy as np
from scipy.stats import mannwhitneyu

base = [1.2, 1.1, 1.3, 1.2, 1.1] * 10  # 50 runs
pr   = [1.3, 1.4, 1.2, 1.3, 1.2] * 10  # 50 runs

stat, p = mannwhitneyu(base, pr, alternative='two-sided')
median_base = np.median(base)
median_pr = np.median(pr)
regression_pct = (median_pr - median_base) / median_base * 100

print(f"p-value: {p:.4f}, Regression: {regression_pct:.1f}%")
if p < 0.05 and regression_pct > 5:
    print("REGRESSION DETECTED!")
)" << "\n\n";
}

void demo_vtune_nsight() {
    std::cout << "--- Intel VTune / NVIDIA Nsight ---\n";
    std::cout << R"(
# Intel VTune Profiler
vtune -collect hotspots ./exe
vtune -collect memory-access ./exe
vtune -collect threading ./exe
vtune -collect hpc-performance ./exe
vtune-gui  # GUI potente

# Análisis Roofline (VTune):
# - Arithmetic Intensity (FLOPs/Byte) vs Performance (GFLOPS)
# - Memory Bound vs Compute Bound
# - Identifica: L1/L2/L3 bound, DRAM bound, Compute bound

# NVIDIA Nsight Systems (system-wide)
nsight-systems ./exe
nsight-systems --trace=cuda,nvtx,osrt ./exe

# NVIDIA Nsight Compute (kernel-level)
ncu --metrics all ./kernel_exe
# Análisis: occupancy, memory throughput, compute throughput
)" << "\n\n";
}

void demo_perf_scripting() {
    std::cout << "--- Perf Scripting (Python) ---\n";
    std::cout << R"(
# perf script -s script.py
# Python API para perf.data

import sys
import perf

def trace_begin():
    print("Trace begin")

def trace_end():
    print("Trace end")

def sample(event):
    # event: dict con ip, pid, tid, time, etc.
    if event['ip']:
        print(f"IP: {event['ip']:#x}, PID: {event['pid']}, Comm: {event['comm']}")

# Exportar a pandas/plotly
)" << "\n\n";
}

void demo_off_cpu_flame() {
    std::cout << "--- Flame graphs avanzados ---\n";
    std::cout << R"(
# Off-CPU flame graphs (tiempo bloqueado)
perf record -g -e sched:sched_switch ./exe
perf script | stackcollapse-perf.pl | flamegraph.pl --color=io > offcpu.svg

# Differential flame graphs (diff dos perfiles)
perf record -g ./exe_before
perf record -g ./exe_after
# diff: flamegraph.pl --diff fold1 fold2 > diff.svg

# Memory flame graphs
perf record -g -e mem-load ./exe
perf script | stackcollapse-perf.pl | flamegraph.pl --color=mem > mem.svg

# Differential flame graphs (comparar antes/después)
# https://github.com/brendangregg/FlameGraph
# ./flamegraph.pl --diff fold1 fold2 > diff.svg
# Rojo = más lento, Azul = más rápido
)" << "\n\n";
}

void demo_roofline() {
    std::cout << "--- Roofline Model ---\n";
    std::cout << R"(
# Roofline Model (VTune / manual)
# Eje X: Arithmetic Intensity (FLOPs/Byte)
# Eje Y: Performance (GFLOPS)

# Techo horizontal: Peak FLOPS (compute bound)
# Techo diagonal: Memory Bandwidth * AI (memory bound)

# Zonas:
# - Arriba a la derecha: Compute bound (CPU saturated)
# - Abajo a la izquierda: Memory bound (bandwidth saturated)
# - Esquina: L1/L2/L3 bound, DRAM bound

# Métricas clave:
# - Arithmetic Intensity = FLOPs / Bytes accesados
# - Memory Bandwidth utilization = achieved / peak
# - Compute utilization = achieved FLOPS / peak FLOPS

# VTune: vtune -collect hpc-performance -knob enable-roofline=true ./exe
)" << "\n\n";
}

void demo_hpc() {
    std::cout << "--- HPC Profiling ---\n";
    std::cout << R"(
# MPI Profiling
mpiP: mpirun -np 4 mpiP ./mpi_exe
IPM: module load ipm; mpirun ./exe
TAU: tau_exec -T mpi,omp,pthread ./exe

# OpenMP
OMPT tools: OMP_TOOL=libomp_tool.so ./exe
OMP_DISPLAY_ENV=TRUE

# CUDA/SYCL
NSight Compute: ncu --metrics all ./kernel
oneAPI: vtune -collect gpu-hotspots ./sycl_exe
Intel Advisor: advisor --collect=roofline -- ./exe
)" << "\n\n";
}

void demo_statistical_rigor() {
    std::cout << "--- Statistical Rigor en Benchmarking ---\n";
    std::cout << R"(
# Reglas de oro:
# 1. Warmup: 5-10 runs (JIT, cache, frequency scaling)
# 2. Runs: mínimo 30 para significancia estadística
# 3. Reportar: mediana + MAD, p50/p95/p99
# 4. Effect size: Cohen's d para comparaciones
# 4. Multiple comparison correction (Bonferroni)

# Ejemplo C++ manual:
auto measure = [](auto&& fn) {
    std::vector<double> times;
    for (int i = 0; i < 40; ++i) {  // 10 warmup + 30 measured
        auto t1 = std::chrono::steady_clock::now();
        fn();
        auto t2 = std::chrono::steady_clock::now();
        if (i >= 10)  // skip warmup
            times.push_back(std::chrono::duration<double, std::milli>(t2-t1).count());
    }
    std::sort(times.begin(), times.end());
    double med = times[times.size()/2];
    double mad = 0;
    for (double x : times) mad += std::abs(x - med);
    mad /= times.size();
    return std::pair(med, mad);
}
)" << "\n\n";
}

void demo_regression_detection() {
    std::cout << "--- Regression Detection ---\n";
    std::cout << R"(
# Statistical regression detection
# 1. Collect N runs (min 30) in base and PR
# 2. Calculate median + MAD in each group
# 3. Statistical test: Mann-Whitney U or t-test
# 4. Effect size: Cohen's d
# Threshold: >5% regression = fail
# Cohen's d = (median_PR - median_base) / pooled_MAD
# d > 0.8 = large effect
# Multiple comparisons: Bonferroni correction
# alpha_adjusted = 0.05 / n_comparisons
# Example Python:
# import numpy as np
# from scipy.stats import mannwhitneyu
# base = [1.2, 1.1, 1.3, 1.2, 1.1] * 10
# pr = [1.3, 1.4, 1.2, 1.3, 1.2] * 10
# stat, p = mannwhitneyu(base, pr, alternative="two-sided")
# median_base = np.median(base)
# median_pr = np.median(pr)
# regression_pct = (median_pr - median_base) / median_base * 100
# if (p < 0.05 && regression_pct > 5)
#     std::cout << "REGRESSION DETECTED!"
)" << "\n\n";
}

int main() {
    std::cout << "=== LECCIÓN 10: Profiling avanzado (parte 2) ===\n\n";

    demo_continuous_benchmarking();
    demo_regression_detection();
    demo_vtune_nsight();
    demo_perf_scripting();
    demo_off_cpu_flame();
    demo_roofline();
    demo_hpc();
    demo_statistical_rigor();

    std::cout << "=== FIN LECCIÓN 10 (Profiling parte 2) ===\n";
    return 0;
}