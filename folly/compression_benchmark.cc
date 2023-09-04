#include "folly/Benchmark.h"
#include "folly/compression/Compression.h"
#include "folly/io/IOBuf.h"

#include "my_lz4.h"

std::string generateRandomString(int length, time_t seed = 0) {
  std::string result;
  result.reserve(length);
  // 设置随机种子为当前时间 + seed
  std::srand(std::time(0) + seed);

  int loop4 = length / 4;
  for (int i = 0; i < loop4; i++) {
    int tmp = std::rand();
    result.append(reinterpret_cast<char*>(&tmp), sizeof(int));
  }

  int left = length % 4;
  for (int i = 0; i < left; i++) {
    result += (char) (std::rand() % 256);
  }
  return result;
}

static std::vector<std::string> inputs;
constexpr int INPUT_LENGTH = 1024;

BENCHMARK(follyLz4Codec, n) {
  for (unsigned int i = 0; i < n; ++i) {
    auto codec = folly::io::getCodec(folly::io::CodecType::LZ4);
    auto input = inputs[i % INPUT_LENGTH];
    auto buffer = folly::IOBuf::copyBuffer(input);
    auto compressed = codec->compress(buffer.get());
    folly::doNotOptimizeAway(compressed);
    auto uncompressed = codec->uncompress(compressed.get(), inputs[i % INPUT_LENGTH].length());
    folly::doNotOptimizeAway(uncompressed);
  }
}

BENCHMARK_RELATIVE(myLz4Codec, n) {
  for (unsigned int i = 0; i < n; ++i) {
    auto codec = my::lz4::LZ4Codec::create();
    auto input = inputs[i % INPUT_LENGTH];
    auto buffer = folly::IOBuf::copyBuffer(input);
    auto compressed = codec->compress(buffer.get());
    folly::doNotOptimizeAway(compressed);
    auto uncompressed = codec->uncompress(compressed.get(), inputs[i % INPUT_LENGTH].length());
    folly::doNotOptimizeAway(uncompressed);
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
