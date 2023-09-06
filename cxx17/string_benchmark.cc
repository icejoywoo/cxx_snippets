#include "folly/Benchmark.h"

#include <string>
#include <string_view>
#include <random>
#include <vector>

std::string generateRandomString(int length, unsigned seed) {
  std::random_device rd;
  std::mt19937 gen(seed == 0 ? rd() : seed);
  std::uniform_int_distribution<> dis('a', 'z');
  std::string result;
  for (int i = 0; i < length; i++) {
    result += dis(gen);
  }
  return result;
}

static std::vector<std::string> inputs;
constexpr int INPUT_LENGTH = 1024;

BENCHMARK(string_substr, n) {
  for (unsigned int i = 0; i < n; ++i) {
    std::string& result = inputs[i % INPUT_LENGTH];
    folly::doNotOptimizeAway(result.substr(10).length());
  }
}

BENCHMARK_RELATIVE(string_view_substr, n) {
  for (unsigned int i = 0; i < n; ++i) {
    std::string_view result = inputs[i % INPUT_LENGTH];
    folly::doNotOptimizeAway(result.substr(10).length());
  }
}

BENCHMARK_DRAW_LINE();

constexpr int factor = 1000000;

BENCHMARK(string_without_reserve, n) {
  std::string result;
  size_t size = n * factor;
  for (unsigned int i = 0; i < size; ++i) {
    result.push_back(i);
    folly::doNotOptimizeAway(result);
  }
}

BENCHMARK_RELATIVE(string_with_reserve, n) {
  std::string result;
  size_t size = n * factor;
  result.reserve(size);
  for (unsigned int i = 0; i < size; ++i) {
    result.push_back(i);
    folly::doNotOptimizeAway(result);
  }
}

BENCHMARK_DRAW_LINE();

BENCHMARK(vector_without_reserve, n) {
  std::vector<unsigned int> result;
  size_t size = n * factor;
  for (unsigned int i = 0; i < size; ++i) {
    result.push_back(i);
    folly::doNotOptimizeAway(result);
  }
}

BENCHMARK_RELATIVE(vector_with_reserve, n) {
  size_t size = n * factor;
  std::vector<unsigned int> result;
  result.reserve(size);
  for (unsigned int i = 0; i < size; ++i) {
    result.push_back(i);
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
