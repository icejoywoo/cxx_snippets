#include <boost/algorithm/string.hpp>
// Or, for fewer header dependencies:
//#include <boost/algorithm/string/predicate.hpp>

#include <gtest/gtest.h>

TEST(BoostTest, toLower) {
  EXPECT_EQ("persephone", boost::algorithm::to_lower_copy(std::string("Persephone")));
  EXPECT_EQ("persephone", boost::algorithm::to_lower_copy<std::string>("Persephone"));
}