#include "folly/Benchmark.h"
#include "folly/hash/Hash.h"

#include "my_hasher.hpp"

/// https://github.com/facebook/folly/blob/main/folly/docs/Benchmark.md
BENCHMARK(follyHasher, n) {
    for (unsigned int i = 0; i < n; ++i) {
        folly::hasher<int>{}(i);
    }
}
BENCHMARK_RELATIVE(myHasher, n) {
    for (unsigned int i = 0; i < n; ++i) {
        hasher<int>{}(i);
    }
}

int main() {
    folly::runBenchmarks();
}
