#include <gtest/gtest.h>

#include "folly/hash/Hash.h"

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