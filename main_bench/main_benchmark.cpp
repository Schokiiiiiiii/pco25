#include <benchmark/benchmark.h>

#include "primenumberdetector.h"

static void BM_SingleThread(benchmark::State& state) {
    PrimeNumberDetector pnd;
    for (auto _ : state) {
        benchmark::DoNotOptimize(pnd.isPrime(state.range(0)));
    }
}

// Argument is the number to test
BENCHMARK(BM_SingleThread)->Arg(433494437)->Arg(433494436)->Unit(benchmark::kMicrosecond)->UseRealTime();
BENCHMARK(BM_SingleThread)->Arg(99194853094755497)->Arg(99194853094755499)->Unit(benchmark::kMillisecond)->UseRealTime();

static void BM_MultiThread(benchmark::State& state) {
    PrimeNumberDetectorMultiThread pndm(state.range(0));
    for (auto _ : state) {
        benchmark::DoNotOptimize(pndm.isPrime(state.range(1)));
    }
}

// Arguments are number of threads and numbers to test
BENCHMARK(BM_MultiThread)->ArgsProduct({{1, 2, 4, 8}, {433494437, 433494436}})->Unit(benchmark::kMicrosecond)->UseRealTime();
BENCHMARK(BM_MultiThread)->ArgsProduct({{1, 2, 4, 8}, {99194853094755497, 99194853094755499}})->Unit(benchmark::kMillisecond)->UseRealTime();

BENCHMARK_MAIN();
