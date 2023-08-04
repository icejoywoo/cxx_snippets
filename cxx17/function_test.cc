#include <gtest/gtest.h>

// https://en.cppreference.com/w/cpp/utility/functional/function
#include <functional>
#include <iostream>

TEST(CxxTest, Function) {
    // function & lambda
    std::function<int(int)> test = [](auto a) {return a;};
    ASSERT_EQ(5,  test(5));
}