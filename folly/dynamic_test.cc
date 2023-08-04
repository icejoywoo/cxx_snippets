#include <gtest/gtest.h>

#include "folly/dynamic.h"
#include "folly/json.h" // for parseJson and toJson

// https://github.com/facebook/folly/blob/main/folly/docs/Dynamic.md

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

    // size only can be called in array, object, string,
    // otherwise throw 'folly::TypeError' exception
    EXPECT_THROW(twelve.size(), folly::TypeError);
    EXPECT_THROW(twelve.empty(), folly::TypeError);

    // A few other types.
    // null
    folly::dynamic nul = nullptr;
    EXPECT_TRUE(nul.isNull());
    EXPECT_FALSE(nul.isNumber());
    EXPECT_TRUE(nul.empty());
    EXPECT_THROW(nul.size(), folly::TypeError);

    // boolean
    folly::dynamic boolean = false;
    EXPECT_FALSE(boolean.asBool());
    EXPECT_THROW(boolean.size(), folly::TypeError);
    boolean = true;
    EXPECT_TRUE(boolean.asBool());
}

TEST(DynamicTest, Array) {
    // Arrays can be initialized with dynamic::array.
    // 可以存放不同类型的数据
    folly::dynamic array = folly::dynamic::array("array ", "of ", 4, " elements");
    assert(array.size() == 4);
    for (auto& val : array) {
      assert(val.isString() || val.isNumber());
    }

    folly::dynamic emptyArray = folly::dynamic::array;
    assert(emptyArray.empty());
    assert(emptyArray.size() == 0);
}

TEST(DynamicTest, Map) {
    // Maps from dynamics to dynamics are called objects.  The
    // dynamic::object constant is how you make an empty map from dynamics
    // to dynamics.
    folly::dynamic map = folly::dynamic::object;
    map["something"] = 12;
    map["another_something"] = map["something"] * 2;

    for (auto& pair : map.items()) {
      ASSERT_TRUE(pair.first.isString());
      ASSERT_TRUE(pair.second.isNumber());
    }

    for (auto& [key, value] : map.items()) {
      ASSERT_TRUE(key.isString());
      ASSERT_TRUE(value.isNumber());
    }

    for (auto& key : map.keys()) {
      ASSERT_TRUE(key.isString());
    }

    for (auto& value : map.values()) {
      ASSERT_TRUE(value.isNumber());
    }

    // Dynamic objects may be initialized this way
    folly::dynamic map2 = folly::dynamic::object("something", 12)("another_something", 24);
    for (auto& [key, value] : map2.items()) {
      ASSERT_TRUE(key.isString());
      ASSERT_TRUE(value.isNumber());
    }
}

TEST(DynamicTest, Object) {
    // Parsing JSON strings and using them.
    std::string jsonDocument = R"({"key":12,"key2":[false, null, true, "yay"]})";
    folly::dynamic parsed = folly::parseJson(jsonDocument);
    assert(parsed["key"] == 12);
    assert(parsed["key2"][0] == false);
    assert(parsed["key2"][1] == nullptr);

    // Building the same document programmatically.
    folly::dynamic sonOfAJ = folly::dynamic::object
      ("key", 12)
      ("key2", folly::dynamic::array(false, nullptr, true, "yay"));

    // Printing.  (See also folly::toPrettyJson)
    auto str = folly::toJson(sonOfAJ);
    assert(jsonDocument.compare(str) == 0);
}