#include "folly/Benchmark.h"
#include "folly/hash/Hash.h"

#include "velox_hasher.hpp"
#include "presto_hasher.hpp"

/// https://github.com/facebook/folly/blob/main/folly/docs/Benchmark.md
/// https://stackoverflow.com/questions/1152333/force-compiler-to-not-optimize-side-effect-less-statements
static volatile uint64_t result = 0;
BENCHMARK(follyHasher, n) {
    for (unsigned int i = 0; i < n; ++i) {
        result = folly::hasher<int>{}(i);
    }
}
BENCHMARK_RELATIVE(veloxHasher, n) {
    for (unsigned int i = 0; i < n; ++i) {
        result = velox::hash::hasher<int>{}(i);
    }
}

BENCHMARK_RELATIVE(prestoHasher, n) {
    for (unsigned int i = 0; i < n; ++i) {
        result = presto::hash::hasher<int>{}(i);
    }
}

int main() {
    folly::runBenchmarks();
}
