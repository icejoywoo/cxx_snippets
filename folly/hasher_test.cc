#include <gtest/gtest.h>

#include "folly/hash/Hash.h"

#include "my_hasher.hpp"

TEST(HasherTest, Basic) {
    EXPECT_EQ(jenkins_rev_mix32(7), folly::hasher<int32_t>()(7));
    EXPECT_EQ(jenkins_rev_mix32(7), folly::hasher<int16_t>()(7));
    EXPECT_EQ(jenkins_rev_mix32(7), folly::hasher<int8_t>()(7));
    EXPECT_EQ(twang_mix64(7), folly::hasher<int64_t>()(7));
    // std::cout << float_hasher<float>()(7) << std::endl;
    // std::cout << float_hasher<double>{}(7) << std::endl;
    EXPECT_EQ(float_hasher<float>()(7), folly::hasher<float>()(7));
    EXPECT_EQ(hasher<double>()(7), folly::hasher<double>()(7));
    EXPECT_EQ(hash_128_to_64(7), folly::hasher<__uint128_t>()(7));

    // std::cout << folly::hasher<int64_t>()(7) << std::endl;
    // 9406415178646722915

    // std::cout << hasher<std::pair<int, int>>{}(std::make_pair(7, 8)) << std::endl;
    // std::cout << folly::hasher<std::pair<int, int>>()(std::make_pair(7, 8)) << std::endl;
    // macro 中 std::pair<int, int> 范型的类型中的逗号，会导致 macro 报错
    typedef std::pair<int, int> int_pair;
    EXPECT_EQ(hasher<int_pair>{}({7, 8}), folly::hasher<int_pair>{}({7, 8}));

    const std::string input("abcde_bcdefgh_abcdefghxxxxxxx");
    EXPECT_EQ(hasher<std::string>{}(input), folly::hasher<std::string>{}(input));
    // string_view
    EXPECT_EQ(hasher<std::string_view>{}(input), folly::hasher<std::string_view>{}(input));

    std::cout << "hash array: " << hashArray<int>({1, 2, 3, 4, 5}) << std::endl;
    std::cout << "hash row: " << hashRow<int>({1, 2, 3, 4, 5}) << std::endl;
    std::cout << "hash map: " << hashMap<int, int>({{1, 1}, {2, 2}}) << std::endl;
}

// presto hash implementation
template<typename Int>
long rotateLeft(Int i, int distance) {
    const int bits = sizeof(Int) * 8;
    distance = distance % bits;  // 确保distance在0~bits之间

    // 左移distance位，将被移出的位放到右侧
    return (i << distance) | (i >> (bits - distance));
}

// AbstractLongType::hash
// AbstractIntType
// SmallintType
// TinyintType
// DoubleType::hash -> use AbstractLongType.hash
template<typename Int>
long hash(Int value) {
  // xxhash64 mix
  return rotateLeft(value * 0xC2B2AE3D27D4EB4FL, 31) * 0x9E3779B185EBCA87L;
}

// BooleanType::hash
// TODO: input argument type should be bool or byte?
long hash(bool value) {
  return value != 0 ? 1231 : 1237;
}

// LongDecimalType. decimal has its own hash implementation

TEST(HasherTest, Presto) {
    std::cout << hash(7L) << std::endl;
    std::cout << hash(7) << std::endl;
    // 2554626171521168346
    std::cout << hash(true) << std::endl;
    std::cout << hash(false) << std::endl;
}