#include <gtest/gtest.h>

#include "folly/hash/Hash.h"

/**
 * Robert Jenkins' reversible 32 bit mix hash function.
 *
 * @methodset jenkins
 */
constexpr uint32_t jenkins_rev_mix32(uint32_t key) noexcept {
  key += (key << 12); // key *= (1 + (1 << 12))
  key ^= (key >> 22);
  key += (key << 4); // key *= (1 + (1 << 4))
  key ^= (key >> 9);
  key += (key << 10); // key *= (1 + (1 << 10))
  key ^= (key >> 2);
  // key *= (1 + (1 << 7)) * (1 + (1 << 12))
  key += (key << 7);
  key += (key << 12);
  return key;
}

/**
 * Thomas Wang 64 bit mix hash function.
 *
 * @methodset twang
 */
constexpr uint64_t twang_mix64(uint64_t key) noexcept {
  key = (~key) + (key << 21); // key *= (1 << 21) - 1; key -= 1;
  key = key ^ (key >> 24);
  key = key + (key << 3) + (key << 8); // key *= 1 + (1 << 3) + (1 << 8)
  key = key ^ (key >> 14);
  key = key + (key << 2) + (key << 4); // key *= 1 + (1 << 2) + (1 << 4)
  key = key ^ (key >> 28);
  key = key + (key << 31); // key *= 1 + (1 << 31)
  return key;
}

/**
 * Reduce two 64-bit hashes into one.
 *
 * hash_128_to_64 uses the Hash128to64 function from Google's cityhash (under
 * the MIT License).
 */
constexpr uint64_t hash_128_to_64(
    const uint64_t upper, const uint64_t lower) noexcept {
  // Murmur-inspired hashing.
  const uint64_t kMul = 0x9ddfea08eb382d69ULL;
  uint64_t a = (lower ^ upper) * kMul;
  a ^= (a >> 47);
  uint64_t b = (upper ^ a) * kMul;
  b ^= (b >> 47);
  b *= kMul;
  return b;
}

constexpr uint64_t hash_128_to_64(__uint128_t u) {
    // sizeof(__uint128_t) * 8 是完整 bit 数，sizeof(__uint128_t) * 4 就是一半的 bit 数
    auto const hi = static_cast<uint64_t>(u >> sizeof(__uint128_t) * 4);
    auto const lo = static_cast<uint64_t>(u);
    return hash_128_to_64(hi, lo);
}

TEST(HasherTest, Int) {
    EXPECT_EQ(jenkins_rev_mix32(7), folly::hasher<int32_t>()(7));
    EXPECT_EQ(jenkins_rev_mix32(7), folly::hasher<int16_t>()(7));
    EXPECT_EQ(jenkins_rev_mix32(7), folly::hasher<int8_t>()(7));
    EXPECT_EQ(twang_mix64(7), folly::hasher<int64_t>()(7));
    EXPECT_EQ(hash_128_to_64(7), folly::hasher<__uint128_t>()(7));
}