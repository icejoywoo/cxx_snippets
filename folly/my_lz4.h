#pragma once

#include "folly/compression/Compression.h"
#include "folly/io/IOBuf.h"

#include <memory>

// ported from airlift compressor Lz4 java implementation
namespace my::lz4 {
// Lz4Constants
constexpr int LAST_LITERAL_SIZE = 5;
constexpr int MIN_MATCH = 4;

constexpr int SIZE_OF_SHORT = 2;
constexpr int SIZE_OF_INT = 4;
constexpr int SIZE_OF_LONG = 8;

namespace compressor {
// Lz4RawCompressor
constexpr int MAX_INPUT_SIZE = 0x7E000000;
constexpr int HASH_LOG = 12;
constexpr int MIN_TABLE_SIZE = 16;
constexpr int MAX_TABLE_SIZE = (1 << HASH_LOG);
constexpr int COPY_LENGTH = 8;
constexpr int MATCH_FIND_LIMIT = COPY_LENGTH + MIN_MATCH;

constexpr int MIN_LENGTH = MATCH_FIND_LIMIT + 1;

constexpr int ML_BITS = 4;
constexpr int ML_MASK = (1 << ML_BITS) - 1;
constexpr int RUN_BITS = 8 - ML_BITS;
constexpr int RUN_MASK = (1 << RUN_BITS) - 1;

constexpr int MAX_DISTANCE = ((1 << 16) - 1);

/* Increase this value ==> compression run slower on incompressible data */
constexpr int SKIP_TRIGGER = 6;

FOLLY_ALWAYS_INLINE uint32_t maxCompressedLength(uint32_t sourceLength)
{
  return sourceLength + sourceLength / 255 + 16;
}

int compress(
    uint8_t* input,
    int inputOffset,
    uint32_t inputLength,
    uint8_t* output,
    int outputOffset,
    uint32_t maxOutputLength);

int compress(
    uint8_t* inputBase,
    long inputAddress,
    uint32_t inputLength,
    uint8_t* outputBase,
    long outputAddress,
    uint32_t maxOutputLength,
    int* table);

} // compressor

namespace decompressor {
// Lz4RawDecompressor
constexpr int DEC_32_TABLE[]{4, 1, 2, 1, 4, 4, 4, 4};
constexpr int DEC_64_TABLE[]{0, 0, 0, -1, 0, 1, 2, 3};

constexpr int OFFSET_SIZE = 2;
constexpr int TOKEN_SIZE = 1;

int decompress(
    uint8_t* inputBase,
    long inputAddress,
    long inputLimit,
    uint8_t* outputBase,
    long outputAddress,
    long outputLimit);

} // decompressor

/**
 * LZ4 compression
 */
class LZ4Codec final : public folly::io::Codec {
 public:
  static std::unique_ptr<Codec> create();
  explicit LZ4Codec();

 private:
  bool doNeedsUncompressedLength() const override;
  uint64_t doMaxUncompressedLength() const override;
  uint64_t doMaxCompressedLength(uint64_t uncompressedLength) const override;

  std::unique_ptr<folly::IOBuf> doCompress(const folly::IOBuf* data) override;
  std::unique_ptr<folly::IOBuf> doUncompress(
      const folly::IOBuf* data, folly::Optional<uint64_t> uncompressedLength) override;
};
}