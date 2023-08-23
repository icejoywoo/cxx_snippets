#include "velox/exec/VectorHasher.h"
#include "velox/vector/tests/utils/VectorMaker.h"
#include "velox/vector/tests/utils/VectorTestBase.h"

using namespace facebook::velox;
using namespace facebook::velox::exec;
using namespace facebook::velox::test;

class ComplexVectorHasherTest : public testing::Test, public VectorTestBase {};

TEST_F(ComplexVectorHasherTest, arrayVector) {
  {
    int len = 5;
    ArrayVectorPtr data = makeArrayVector<int32_t>(
        len,
        [](auto row) { return 5; },
        [](auto row, auto index) { return index; },
        [](auto row) { return false; });

    auto hasher = VectorHasher::create(ARRAY(INTEGER()), 0);
    SelectivityVector rows(data->size());
    hasher->decode(*data, rows);
    raw_vector<uint64_t> hashes(len);
    std::fill(hashes.begin(), hashes.end(), 0);
    hasher->hash(rows, false, hashes);
    EXPECT_EQ(4971740975845359195UL, hashes[len-1]);
  }
  {
    int len = 5;
    ArrayVectorPtr data = makeArrayVector<StringView>(
        len,
        [](auto row) { return 5; },
        [](auto row, auto index) {
          return StringView::makeInline(std::to_string(index));
        },
        [](auto row) { return false; });

    auto hasher = VectorHasher::create(ARRAY(VARCHAR()), 0);
    SelectivityVector rows(data->size());
    hasher->decode(*data, rows);
    raw_vector<uint64_t> hashes(len);
    std::fill(hashes.begin(), hashes.end(), 0);
    hasher->hash(rows, false, hashes);
    EXPECT_EQ(17820802234886935425UL, hashes[len-1]);
  }
}

TEST_F(ComplexVectorHasherTest, rowVector) {
  int len = 5;
  RowVectorPtr data = makeRowVector({
      makeFlatVector<int32_t>(len, [](auto row) { return 0; }),
      makeFlatVector<int32_t>(len, [](auto row) { return 1; }),
      makeFlatVector<int32_t>(len, [](auto row) { return 2; }),
      makeFlatVector<int32_t>(len, [](auto row) { return 3; }),
      makeFlatVector<int32_t>(len, [](auto row) { return 4; })
  });

  auto hasher = VectorHasher::create(
      ROW({INTEGER(), INTEGER(), INTEGER(), INTEGER(), INTEGER()}), 0);
  SelectivityVector rows(data->size());
  hasher->decode(*data, rows);
  raw_vector<uint64_t> hashes(len);
  std::fill(hashes.begin(), hashes.end(), 0);
  hasher->hash(rows, false, hashes);
  EXPECT_EQ(8795432144090112219UL, hashes[len-1]);
}

TEST_F(ComplexVectorHasherTest, mapVector) {
  std::vector<std::vector<std::pair<int32_t, std::optional<int32_t>>>> map{
      {{1, 1}, {2, 2}},
      {{2, 102}, {3, 103}},
      {{4, 104}, {5, 105}, {6, 106}},
      {{7, 107}, {8, 108}, {9, 109}, {10, 110}},
      {{11, 111}, {12, 112}, {13, 113}, {14, 114}, {15, 115}}};
  auto data = makeMapVector(map);
  int len = map.size();

  auto hasher = VectorHasher::create(MAP(INTEGER(), INTEGER()), 0);
  SelectivityVector rows(data->size());
  hasher->decode(*data, rows);
  raw_vector<uint64_t> hashes(len);
  std::fill(hashes.begin(), hashes.end(), 0);
  hasher->hash(rows, false, hashes);
  EXPECT_EQ(14742748263231395393UL, hashes[0]);
  EXPECT_EQ(18340151164760653449UL, hashes[1]);
}