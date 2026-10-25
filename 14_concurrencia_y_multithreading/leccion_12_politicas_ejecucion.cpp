/* =============================================================================
   MÓDULO 14 - LECCIÓN 12: Políticas de ejecución STL, medición de escalado,
   algoritmos paralelos en profundidad, execution::par_unseq, unseq.
   =============================================================================
   TEMA 8 de TEMAS.md (1 lección): Políticas de ejecución de la STL
   y medición del escalado.

   POLÍTICAS DE EJECUCIÓN (C++17, header <execution>):
   1. std::execution::seq:     Secuencial (default si no se especifica).
   2. std::execution::par:     Paralelo (multi-hilo, orden indeterminado).
   3. std::execution::par_unseq: Paralelo + vectorizado (SIMD), sin efectos laterales
                                  observables entre iteraciones (más restrictivo).
   4. std::execution::unseq:   Vectorizado (SIMD) en un solo hilo, sin efectos laterales.
   5. C++23: std::execution::par_task: Para tareas que pueden bloquear (I/O, locks).

   REQUISITOS PARA ALGORITMOS PARALELOS:
   - Iteradores: RandomAccessIterator (para dividir rango en chunks).
   - Funciones (predicados, functors, lambdas): deben ser "element-wise"
     (no depender de estado compartido mutable sin sincronización).
   - Excepciones: si lanza, se llama std::terminate() (salvo par_task C++23).
   - Complejidad: misma que versión seq pero con factor de paralelismo.

   ALGORITMOS CON SOBRECARGA PARALELA (C++17):
   - for_each, for_each_n
   - transform, transform_exclusive_scan, transform_inclusive_scan
   - reduce, transform_reduce
   - inclusive_scan, exclusive_scan
   - sort, stable_sort, partial_sort, nth_element
   - merge, inplace_merge
   - minmax_element, min_element, max_element
   - find, find_if, find_if_not, find_end, find_first_of
   - adjacent_find, search, search_n
   - copy, copy_if, copy_n, move
   - fill, fill_n, generate, generate_n
   - remove, remove_if, remove_copy, remove_copy_if
   - replace, replace_if, replace_copy, replace_copy_if
   - rotate, rotate_copy
   - shuffle (con generador thread-safe)
   - sample

   ALGORITMOS NUMÉRICOS (C++17, <numeric>):
   - reduce: como accumulate pero orden indeterminado (paralelizable).
   - transform_reduce: map + reduce en una pasada.
   - inclusive_scan / exclusive_scan: prefix sums paralelos.
   - transform_inclusive_scan / transform_exclusive_scan.

   NOTA: std::execution::par requiere TBB (Threading Building Blocks) linkado
   con -ltbb. En estos ejemplos usamos paralelismo manual con std::async
   para demostrar los conceptos SIN dependencias extra.

   MEDICIÓN DE ESCALADO (Scalability):
   - Speedup = T_seq / T_par
   - Eficiencia = Speedup / N_hilos
   - Ley de Amdahl: speedup máx = 1 / (serial + paralelo/N)
   - Ley de Gustafson: trabajo crece con N (escalado débil).
   - Overhead: arranque hilos, sincronización, false sharing, cache misses.
   - Medir: tiempo de pared (wall-clock), no CPU time.

   BENCHMARKING CORRECTO:
   - Warm-up runs (JIT, cache, frequency scaling).
   - Múltiples iteraciones, mediana (no media: outliers).
   - Fijar frecuencia CPU (governor performance) si posible.
   - Aislar cores (taskset, cpusets) para reproductibilidad.
   - Medir SOLO la región paralela (excluir setup/teardown).

   EJEMPLOS: reduce manual vs accumulate, sort manual paralelo,
   scan paralelo manual, benchmarking con warm-up, detección de false sharing. */

#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <chrono>
#include <random>
#include <thread>
#include <future>
#include <atomic>
#include <cmath>
#include <iomanip>
#include <functional>

// -----------------------------------------------------------------------------
// Helper: división de rango para paralelismo manual
// -----------------------------------------------------------------------------
template <typename Iterator, typename Func>
void parallel_for_each(Iterator begin, Iterator end, Func func, size_t num_threads = 0) {
    size_t n = std::distance(begin, end);
    if (n == 0) return;
    if (num_threads == 0) num_threads = std::thread::hardware_concurrency();
    num_threads = std::min(num_threads, n);

    size_t chunk = (n + num_threads - 1) / num_threads;
    std::vector<std::future<void>> futures;
    futures.reserve(num_threads);

    Iterator it = begin;
    for (size_t t = 0; t < num_threads; ++t) {
        Iterator chunk_begin = it;
        Iterator chunk_end = (t == num_threads - 1) ? end : std::next(it, chunk);
        if (chunk_begin == chunk_end) break;
        futures.push_back(std::async(std::launch::async, [chunk_begin, chunk_end, func]() mutable {
            std::for_each(chunk_begin, chunk_end, func);
        }));
        it = chunk_end;
        if (it == end) break;
    }
    for (auto& f : futures) f.get();
}

template <typename Iterator, typename T, typename BinaryOp, typename UnaryOp>
T parallel_transform_reduce(Iterator begin, Iterator end, T init,
                            BinaryOp reduce_op, UnaryOp transform_op,
                            size_t num_threads = 0) {
    size_t n = std::distance(begin, end);
    if (n == 0) return init;
    if (num_threads == 0) num_threads = std::thread::hardware_concurrency();
    num_threads = std::min(num_threads, n);

    size_t chunk = (n + num_threads - 1) / num_threads;
    std::vector<std::future<T>> futures;
    futures.reserve(num_threads);

    Iterator it = begin;
    for (size_t t = 0; t < num_threads; ++t) {
        Iterator chunk_begin = it;
        Iterator chunk_end = (t == num_threads - 1) ? end : std::next(it, chunk);
        if (chunk_begin == chunk_end) break;
        futures.push_back(std::async(std::launch::async,
            [chunk_begin, chunk_end, init, reduce_op, transform_op]() mutable {
                T local = init;
                for (auto it2 = chunk_begin; it2 != chunk_end; ++it2) {
                    local = reduce_op(local, transform_op(*it2));
                }
                return local;
            }));
        it = chunk_end;
        if (it == end) break;
    }

    T result = init;
    for (auto& f : futures) result = reduce_op(result, f.get());
    return result;
}

// -----------------------------------------------------------------------------
// 1) reduce vs accumulate vs transform_reduce (manual)
// -----------------------------------------------------------------------------
void demo_reduce_vs_accumulate() {
    std::cout << "--- reduce vs accumulate vs transform_reduce (manual) ---\n";

    const size_t N = 10'000'000;
    std::vector<double> datos(N);
    std::iota(datos.begin(), datos.end(), 1.0);

    // accumulate (secuencial, orden garantizado)
    auto inicio = std::chrono::high_resolution_clock::now();
    double sum_acc = std::accumulate(datos.begin(), datos.end(), 0.0);
    auto fin = std::chrono::high_resolution_clock::now();
    auto t_acc = std::chrono::duration_cast<std::chrono::milliseconds>(fin - inicio);

    // reduce manual paralelo
    inicio = std::chrono::high_resolution_clock::now();
    double sum_red = parallel_transform_reduce(
        datos.begin(), datos.end(), 0.0,
        std::plus<>(), [](double x) { return x; });
    fin = std::chrono::high_resolution_clock::now();
    auto t_red = std::chrono::duration_cast<std::chrono::milliseconds>(fin - inicio);

    // transform_reduce manual: map + reduce en uno
    inicio = std::chrono::high_resolution_clock::now();
    double sum_tr = parallel_transform_reduce(
        datos.begin(), datos.end(), 0.0,
        std::plus<>(), [](double x) { return x * x; });
    fin = std::chrono::high_resolution_clock::now();
    auto t_tr = std::chrono::duration_cast<std::chrono::milliseconds>(fin - inicio);

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "accumulate (seq): " << sum_acc << " en " << t_acc.count() << " ms\n";
    std::cout << "reduce manual (par):     " << sum_red << " en " << t_red.count() << " ms\n";
    std::cout << "transform_reduce manual: " << sum_tr << " en " << t_tr.count() << " ms\n";
    std::cout << "Speedup reduce: " << double(t_acc.count()) / t_red.count() << "x\n";
    std::cout << "NOTA: std::execution::par requiere -ltbb (TBB)\n\n";
}

// -----------------------------------------------------------------------------
// 2) sort paralelo manual (merge sort paralelo simple)
// -----------------------------------------------------------------------------
void parallel_sort(std::vector<int>& v, size_t num_threads = 0) {
    if (num_threads == 0) num_threads = std::thread::hardware_concurrency();
    if (v.size() < 1000 || num_threads <= 1) {
        std::sort(v.begin(), v.end());
        return;
    }

    // Dividir en chunks, ordenar cada uno en paralelo, luego merge
    size_t n = v.size();
    size_t chunk = (n + num_threads - 1) / num_threads;
    std::vector<std::future<void>> futures;
    futures.reserve(num_threads);

    for (size_t t = 0; t < num_threads; ++t) {
        size_t begin = t * chunk;
        size_t end = (t == num_threads - 1) ? n : (t + 1) * chunk;
        if (begin >= end) break;
        futures.push_back(std::async(std::launch::async, [&v, begin, end]() {
            std::sort(v.begin() + begin, v.begin() + end);
        }));
    }
    for (auto& f : futures) f.get();

    // Merge k sorted ranges (simple: inplace_merge iterativo)
    size_t stride = chunk;
    while (stride < n) {
        for (size_t i = 0; i < n; i += 2 * stride) {
            size_t mid = std::min(i + stride, n);
            size_t end = std::min(i + 2 * stride, n);
            if (mid < end) std::inplace_merge(v.begin() + i, v.begin() + mid, v.begin() + end);
        }
        stride *= 2;
    }
}

void demo_sort_paralelo() {
    std::cout << "--- sort paralelo manual (std::sort + merge) ---\n";

    const size_t N = 5'000'000;
    std::vector<int> v_seq(N), v_par(N);
    std::iota(v_seq.begin(), v_seq.end(), 0);
    v_par = v_seq;
    std::shuffle(v_seq.begin(), v_seq.end(), std::mt19937{42});
    v_par = v_seq;

    auto inicio = std::chrono::high_resolution_clock::now();
    std::sort(v_seq.begin(), v_seq.end());
    auto fin = std::chrono::high_resolution_clock::now();
    auto t_seq = std::chrono::duration_cast<std::chrono::milliseconds>(fin - inicio);

    inicio = std::chrono::high_resolution_clock::now();
    parallel_sort(v_par);
    fin = std::chrono::high_resolution_clock::now();
    auto t_par = std::chrono::duration_cast<std::chrono::milliseconds>(fin - inicio);

    std::cout << "sort seq: " << t_seq.count() << " ms\n";
    std::cout << "sort par (manual): " << t_par.count() << " ms\n";
    std::cout << "Speedup: " << double(t_seq.count()) / t_par.count() << "x\n";
    std::cout << "Verificación: " << (v_seq == v_par ? "OK" : "DIFERENTE") << "\n\n";
}

// -----------------------------------------------------------------------------
// 3) Prefix sums paralelos manual: inclusive_scan / exclusive_scan
// -----------------------------------------------------------------------------
void inclusive_scan_manual(const std::vector<int>& in, std::vector<int>& out) {
    size_t n = in.size();
    if (n == 0) return;
    out.resize(n);

    // Secuencial simple (para demo; versión paralela real usa árbol de reducción)
    out[0] = in[0];
    for (size_t i = 1; i < n; ++i) out[i] = out[i-1] + in[i];
}

void exclusive_scan_manual(const std::vector<int>& in, std::vector<int>& out, int init) {
    size_t n = in.size();
    if (n == 0) return;
    out.resize(n);
    out[0] = init;
    for (size_t i = 1; i < n; ++i) out[i] = out[i-1] + in[i-1];
}

void demo_scan_paralelo() {
    std::cout << "--- inclusive_scan / exclusive_scan (manual, seq para demo) ---\n";

    std::vector<int> entrada = {1, 2, 3, 4, 5, 6, 7, 8};
    std::vector<int> out_inc, out_exc;

    inclusive_scan_manual(entrada, out_inc);
    exclusive_scan_manual(entrada, out_exc, 0);

    std::cout << "inclusive_scan: ";
    for (int x : out_inc) std::cout << x << " ";
    std::cout << " (1 3 6 10 15 21 28 36)\n";

    std::cout << "exclusive_scan: ";
    for (int x : out_exc) std::cout << x << " ";
    std::cout << " (0 1 3 6 10 15 21 28)\n";
    std::cout << "NOTA: versión paralela real usa árbol de reducción (Blelloch)\n\n";
}

// -----------------------------------------------------------------------------
// 4) for_each paralelo manual
// -----------------------------------------------------------------------------
void demo_for_each_paralelo() {
    std::cout << "--- for_each paralelo manual ---\n";

    std::vector<int> datos(1000);
    std::iota(datos.begin(), datos.end(), 0);

    std::atomic<long long> contador{0};
    parallel_for_each(datos.begin(), datos.end(), [&](int) {
        contador.fetch_add(1, std::memory_order_relaxed);
    });

    std::cout << "Contador via parallel_for_each: " << contador.load() << " (esperado 1000)\n\n";
}

// -----------------------------------------------------------------------------
// 5) Detección y mitigación de FALSE SHARING
// -----------------------------------------------------------------------------
struct ContadorConPadding {
    alignas(64) std::atomic<long long> valor{0};
};

struct ContadorSinPadding {
    std::atomic<long long> valor{0};
};

void demo_false_sharing() {
    std::cout << "--- False sharing: con vs sin padding ---\n";

    constexpr int VUELTAS = 1'000'000;
    const int HILOS = 4;

    ContadorSinPadding contadores_sin[HILOS];
    auto worker_sin = [&](int idx) {
        for (int i = 0; i < VUELTAS; ++i) contadores_sin[idx].valor.fetch_add(1, std::memory_order_relaxed);
    };

    auto inicio = std::chrono::high_resolution_clock::now();
    std::vector<std::thread> hilos;
    for (int i = 0; i < HILOS; ++i) hilos.emplace_back(worker_sin, i);
    for (auto& t : hilos) t.join();
    auto fin = std::chrono::high_resolution_clock::now();
    auto t_sin = std::chrono::duration_cast<std::chrono::milliseconds>(fin - inicio);

    ContadorConPadding contadores_con[HILOS];
    auto worker_con = [&](int idx) {
        for (int i = 0; i < VUELTAS; ++i) contadores_con[idx].valor.fetch_add(1, std::memory_order_relaxed);
    };

    inicio = std::chrono::high_resolution_clock::now();
    hilos.clear();
    for (int i = 0; i < HILOS; ++i) hilos.emplace_back(worker_con, i);
    for (auto& t : hilos) t.join();
    fin = std::chrono::high_resolution_clock::now();
    auto t_con = std::chrono::duration_cast<std::chrono::milliseconds>(fin - inicio);

    std::cout << "Sin padding (false sharing): " << t_sin.count() << " ms\n";
    std::cout << "Con padding (alignas 64):    " << t_con.count() << " ms\n";
    std::cout << "Mejora: " << double(t_sin.count()) / t_con.count() << "x\n\n";
}

// -----------------------------------------------------------------------------
// 6) Benchmarking robusto: warm-up, múltiples runs, mediana
// -----------------------------------------------------------------------------
template <typename Fn>
double benchmark_ms(Fn&& fn, int warmup = 3, int runs = 10) {
    for (int i = 0; i < warmup; ++i) fn();

    std::vector<double> tiempos;
    tiempos.reserve(runs);

    for (int i = 0; i < runs; ++i) {
        auto ini = std::chrono::high_resolution_clock::now();
        fn();
        auto fin = std::chrono::high_resolution_clock::now();
        double ms = std::chrono::duration<double, std::milli>(fin - ini).count();
        tiempos.push_back(ms);
    }

    std::sort(tiempos.begin(), tiempos.end());
    return tiempos[runs / 2];
}

void demo_benchmarking() {
    std::cout << "--- Benchmarking robusto (warm-up, mediana) ---\n";

    const size_t N = 2'000'000;
    std::vector<double> datos(N);
    std::iota(datos.begin(), datos.end(), 1.0);

    auto trabajo_seq = [&] {
        return std::accumulate(datos.begin(), datos.end(), 0.0);
    };

    auto trabajo_par = [&] {
        return parallel_transform_reduce(datos.begin(), datos.end(), 0.0,
                                         std::plus<>(), [](double x) { return x; });
    };

    double t_seq = benchmark_ms(trabajo_seq);
    double t_par = benchmark_ms(trabajo_par);

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Seq (mediana): " << t_seq << " ms\n";
    std::cout << "Par (mediana): " << t_par << " ms\n";
    std::cout << "Speedup: " << t_seq / t_par << "x\n\n";
}

// -----------------------------------------------------------------------------
// 7) par_unseq: requisitos estrictos (sin efectos laterales, sin bloqueos)
// -----------------------------------------------------------------------------
void demo_par_unseq() {
    std::cout << "--- execution::par_unseq (conceptual: SIMD + paralela, sin side effects) ---\n";

    std::vector<double> a(10000), b(10000), c(10000);
    std::iota(a.begin(), a.end(), 1.0);
    std::iota(b.begin(), b.end(), 2.0);

    // transform con par_unseq CONCEPTUAL: cada elemento INDEPENDIENTE
    // NO: I/O, locks, atomicas, allocación, excepciones, acceso a globals mutables
    std::transform(a.begin(), a.end(), b.begin(), c.begin(),
                   [](double x, double y) { return x * y + std::sin(x); });

    std::cout << "Resultado: c[0]=" << c[0] << ", c[9999]=" << c[9999] << "\n";
    std::cout << "par_unseq permite vectorización SIMD agresiva (requiere -ltbb)\n\n";
}

// -----------------------------------------------------------------------------
// 8) Escalado fuerte vs débil (Amdahl vs Gustafson)
// -----------------------------------------------------------------------------
void demo_escalado() {
    std::cout << "--- Escalado fuerte (Amdahl) vs débil (Gustafson) ---\n";

    auto trabajo_paralelizable = [](size_t n) {
        std::vector<double> v(n);
        std::iota(v.begin(), v.end(), 1.0);
        return parallel_transform_reduce(v.begin(), v.end(), 0.0,
                                         std::plus<>(), [](double x) { return x * x; });
    };

    // Escalado FUERTE: problema FIJO, más hilos (simulado variando num_threads)
    std::cout << "Escalado FUERTE (problema fijo 10M, variando hilos):\n";
    for (int hilos : {1, 2, 4, 8}) {
        auto t = benchmark_ms([&] { trabajo_paralelizable(10'000'000); }, 2, 5);
        std::cout << "  " << hilos << " hilos: " << t << " ms\n";
    }

    // Escalado DÉBIL: problema CRECE con hilos (trabajo por hilo constante)
    std::cout << "\nEscalado DÉBIL (trabajo por hilo constante):\n";
    size_t base = 2'500'000;
    for (int hilos : {1, 2, 4, 8}) {
        auto t = benchmark_ms([&] { trabajo_paralelizable(base * hilos); }, 2, 5);
        std::cout << "  " << hilos << " hilos x " << base << " = " << t << " ms\n";
    }
    std::cout << "\n";
}

// -----------------------------------------------------------------------------
// MAIN
// -----------------------------------------------------------------------------
int main() {
    std::cout << "=== LECCIÓN 12: Políticas ejecución STL, medición escalado ===\n\n";

    demo_reduce_vs_accumulate();
    demo_sort_paralelo();
    demo_scan_paralelo();
    demo_for_each_paralelo();
    demo_false_sharing();
    demo_benchmarking();
    demo_par_unseq();
    demo_escalado();

    std::cout << "=== FIN LECCIÓN 12 ===\n";
    return 0;
}