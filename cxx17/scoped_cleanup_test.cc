#include <gtest/gtest.h>

#include "ScopedCleanup.h"

TEST(ScopedCleanupTest, Basic) {
  int* a = new int;
  {
    SCOPED_CLEANUP({
      delete a;
      a = nullptr;
    });
    ASSERT_NE(nullptr, a);
  }
  ASSERT_EQ(nullptr, a);
}