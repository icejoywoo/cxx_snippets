#include <boost/algorithm/string.hpp>
// Or, for fewer header dependencies:
//#include <boost/algorithm/string/predicate.hpp>

#include <gtest/gtest.h>

#include <set>
#include <unordered_set>

TEST(SetTest, SetTest) {
  auto cmp = [] (std::string l, std::string r) -> int {
    return !boost::iequals(l, r);
  };
  std::set<std::string, decltype(cmp)> ciss(cmp); // c++11
  // std::set<std::string, decltype(cmp)> ciss; // c++ 20
  ciss.insert("Persephone");  // a new element is added to the set
  ciss.insert("persephone");  // no new element is added to the set

  ASSERT_TRUE(ciss.find("persephone") != ciss.end()); // 1
  ASSERT_FALSE(std::find(ciss.begin(), ciss.end(), "persephone") != ciss.end()); // 0
}

TEST(SetTest, UnorderedSetTest) {
  // unordered_set does not support cmp
  std::unordered_set<std::string> ciss;
  //  ciss.insert(boost::algorithm::to_lower_copy(std::string("Persephone")));
  ciss.insert(boost::algorithm::to_lower_copy<std::string>("Persephone"));
  ciss.insert("persephone");

  ASSERT_TRUE(ciss.find("persephone") != ciss.end());
  ASSERT_TRUE(std::find(ciss.begin(), ciss.end(), "persephone") != ciss.end());
}

