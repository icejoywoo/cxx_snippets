#include <gtest/gtest.h>

#include "folly/dynamic.h"

TEST(DynamicTest, Basic) {
    folly::dynamic twelve = 12;

    // type check
    EXPECT_FALSE(twelve.isBool());
    EXPECT_TRUE(twelve.isNumber());

    // get value
    EXPECT_EQ(12, twelve.asInt());
    EXPECT_EQ(12.0, twelve.asDouble());
    EXPECT_EQ("12", twelve.asString());
    EXPECT_EQ(true, twelve.asBool());
}