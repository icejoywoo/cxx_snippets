#include "folly/Benchmark.h"
#include "folly/hash/Hash.h"

#include "presto_hasher.hpp"
#include "velox_hasher.hpp"

/// https://github.com/facebook/folly/blob/main/folly/docs/Benchmark.md
/// https://stackoverflow.com/questions/1152333/force-compiler-to-not-optimize-side-effect-less-statements

std::string generateRandomString(int length, time_t seed = 0) {
  std::string result = "";
  static std::string characters =
      "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

  // 设置随机种子为当前时间 + seed
  std::srand(std::time(0) + seed);

  for (int i = 0; i < length; i++) {
    int randomIndex = std::rand() % characters.length();
    result += characters[randomIndex];
  }

  return result;
}

BENCHMARK(follyHasherInt, n) {
  for (unsigned int i = 0; i < n; ++i) {
    uint64_t result = folly::hasher<int>{}(i);
    folly::doNotOptimizeAway(result);
  }
}

BENCHMARK_RELATIVE(veloxHasherInt, n) {
  for (unsigned int i = 0; i < n; ++i) {
    uint64_t result = velox::hash::hasher<int>{}(i);
    folly::doNotOptimizeAway(result);
  }
}

BENCHMARK_RELATIVE(prestoHasherInt, n) {
  for (unsigned int i = 0; i < n; ++i) {
    uint64_t result = presto::hash::hasher<int>{}(i);
    folly::doNotOptimizeAway(result);
  }
}

BENCHMARK_DRAW_LINE();

BENCHMARK(follyHasherLong, n) {
  for (unsigned int i = 0; i < n; ++i) {
    uint64_t result = folly::hasher<long>{}(i);
    folly::doNotOptimizeAway(result);
  }
}

BENCHMARK_RELATIVE(veloxHasherLong, n) {
  for (unsigned int i = 0; i < n; ++i) {
    uint64_t result = velox::hash::hasher<long>{}(i);
    folly::doNotOptimizeAway(result);
  }
}

BENCHMARK_RELATIVE(prestoHasherLong, n) {
  for (unsigned int i = 0; i < n; ++i) {
    uint64_t result = presto::hash::hasher<long>{}(i);
    folly::doNotOptimizeAway(result);
  }
}

BENCHMARK_DRAW_LINE();

static std::vector<std::string> inputs;
constexpr int INPUT_LENGTH = 1024;

BENCHMARK(follyHasherString, n) {
  for (unsigned int i = 0; i < n; ++i) {
    uint64_t result = folly::hasher<std::string>{}(inputs[i % INPUT_LENGTH]);
    folly::doNotOptimizeAway(result);
  }
}

BENCHMARK_RELATIVE(veloxHasherString, n) {
  for (unsigned int i = 0; i < n; ++i) {
    uint64_t result =
        velox::hash::hasher<std::string>{}(inputs[i % INPUT_LENGTH]);
    folly::doNotOptimizeAway(result);
  }
}

BENCHMARK_RELATIVE(prestoHasherString, n) {
  for (unsigned int i = 0; i < n; ++i) {
    uint64_t result =
        presto::hash::hasher<std::string>{}(inputs[i % INPUT_LENGTH]);
    folly::doNotOptimizeAway(result);
  }
}

int main() {
  // init input strings
  for (int i = 0; i < INPUT_LENGTH; i++) {
    std::string input = generateRandomString(20 + (i / 10 * 10), i);
    inputs.emplace_back(input);
  }
  folly::runBenchmarks();
}
