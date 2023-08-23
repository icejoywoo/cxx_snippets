#include "folly/Benchmark.h"
#include "folly/hash/Hash.h"

#include "velox_hasher.hpp"
#include "presto_hasher.hpp"

/// https://github.com/facebook/folly/blob/main/folly/docs/Benchmark.md
/// https://stackoverflow.com/questions/1152333/force-compiler-to-not-optimize-side-effect-less-statements
BENCHMARK(follyHasher, n) {
    for (unsigned int i = 0; i < n; ++i) {
        uint64_t result = folly::hasher<int>{}(i);
        folly::doNotOptimizeAway(result);
    }
}
BENCHMARK_RELATIVE(veloxHasher, n) {
    for (unsigned int i = 0; i < n; ++i) {
        uint64_t result = velox::hash::hasher<int>{}(i);
        folly::doNotOptimizeAway(result);
    }
}

BENCHMARK_RELATIVE(prestoHasher, n) {
    for (unsigned int i = 0; i < n; ++i) {
        uint64_t result = presto::hash::hasher<int>{}(i);
        folly::doNotOptimizeAway(result);
    }
}

int main() {
    folly::runBenchmarks();
}
