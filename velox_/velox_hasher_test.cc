#include <gtest/gtest.h>

#include "folly/hash/Hash.h"

#include "my_hasher.hpp"

TEST(HasherTest, Basic) {
    using namespace velox::hash;
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

    for (int i : {0, 1, 2, 3, 4}) {
        std::cout << "hash " << i << ": " << hasher<int>{}(i) << std::endl;
    }
    std::cout << "hash array: " << hashArray<int>({0, 1, 2, 3, 4}) << std::endl;
    std::cout << "hash array: " << hasher<std::vector<int>>{}({0, 1, 2, 3, 4}) << std::endl;
    // 4971740975845359195
    std::cout << "hash string array: " << hashArray<std::string>({"0", "1", "2", "3", "4"}) << std::endl;
    std::cout << "hash string array: " << hasher<std::vector<std::string>>{}({"0", "1", "2", "3", "4"}) << std::endl;
    // 17820802234886935425
    std::cout << "hash row: " << hashRow<int>({0, 1, 2, 3, 4}) << std::endl;
    // 8795432144090112219
    std::cout << "hash map: " << hashMap<int, int>({{1, 1}, {2, 2}}) << std::endl;
    std::cout << "hash map: " << hasher<std::map<int, int>>{}({{1, 1}, {2, 2}}) << std::endl;
    // 14742748263231395393
    std::cout << "hash map: " << hashMap<int, int>({{2, 102}, {3, 103}}) << std::endl;
    std::cout << "hash map: " << hasher<std::map<int, int>>{}({{2, 102}, {3, 103}}) << std::endl;
    // 18340151164760653449
}