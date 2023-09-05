// copied from velox/serializers/tests/PrestoSerializerTest.cpp
#include "velox/serializers/PrestoSerializer.h"
#include <folly/Random.h>
#include <gtest/gtest.h>
#include <vector>

#include "velox/common/base/tests/GTestUtils.h"
#include "velox/common/memory/ByteStream.h"
#include "velox/expression/ComplexViewTypes.h"
#include "velox/expression/VectorReaders.h"
#include "velox/expression/VectorWriters.h"
#include "velox/functions/prestosql/types/TimestampWithTimeZoneType.h"
#include "velox/vector/BaseVector.h"
#include "velox/vector/ComplexVector.h"
#include "velox/vector/fuzzer/VectorFuzzer.h"
#include "velox/vector/tests/utils/VectorTestBase.h"

using namespace facebook::velox;
using namespace facebook::velox::test;

class MyPrestoSerializerTest
   : public ::testing::TestWithParam<common::CompressionKind> {
protected:
 static void SetUpTestCase() {
   serializer::presto::PrestoVectorSerde::registerVectorSerde();
 }

 void SetUp() override {
   pool_ = memory::addDefaultLeafMemoryPool();
   serde_ = std::make_unique<serializer::presto::PrestoVectorSerde>();
   vectorMaker_ = std::make_unique<test::VectorMaker>(pool_.get());
 }

 void sanityCheckEstimateSerializedSize(const RowVectorPtr& rowVector) {
   const auto numRows = rowVector->size();

   std::vector<IndexRange> rows(numRows);
   for (int i = 0; i < numRows; i++) {
     rows[i] = IndexRange{i, 1};
   }

   std::vector<vector_size_t> rowSizes(numRows, 0);
   std::vector<vector_size_t*> rawRowSizes(numRows);
   for (auto i = 0; i < numRows; i++) {
     rawRowSizes[i] = &rowSizes[i];
   }
   serde_->estimateSerializedSize(
       rowVector, folly::Range(rows.data(), numRows), rawRowSizes.data());
 }

 serializer::presto::PrestoVectorSerde::PrestoOptions getParamSerdeOptions(
     const serializer::presto::PrestoVectorSerde::PrestoOptions*
         serdeOptions) {
   const bool useLosslessTimestamp =
       serdeOptions == nullptr ? false : serdeOptions->useLosslessTimestamp;
   common::CompressionKind kind = GetParam();
   serializer::presto::PrestoVectorSerde::PrestoOptions paramOptions{
       useLosslessTimestamp, kind};
   return paramOptions;
 }

 void serialize(
     const RowVectorPtr& rowVector,
     std::ostream* output,
     const serializer::presto::PrestoVectorSerde::PrestoOptions*
         serdeOptions) {
   auto streamInitialSize = output->tellp();
   sanityCheckEstimateSerializedSize(rowVector);

   auto arena = std::make_unique<StreamArena>(pool_.get());
   auto rowType = asRowType(rowVector->type());
   auto numRows = rowVector->size();
   auto paramOptions = getParamSerdeOptions(serdeOptions);
   auto serializer =
       serde_->createSerializer(rowType, numRows, arena.get(), &paramOptions);

   serializer->append(rowVector);
   auto size = serializer->maxSerializedSize();
   facebook::velox::serializer::presto::PrestoOutputStreamListener listener;
   OStreamOutputStream out(output, &listener);
   serializer->flush(&out);
   if (paramOptions.compressionKind == common::CompressionKind_NONE) {
     ASSERT_EQ(size, out.tellp() - streamInitialSize);
   } else {
     ASSERT_GE(size, out.tellp() - streamInitialSize);
   }
 }

 std::unique_ptr<ByteStream> toByteStream(const std::string& input) {
   auto byteStream = std::make_unique<ByteStream>();
   ByteRange byteRange{
       reinterpret_cast<uint8_t*>(const_cast<char*>(input.data())),
       (int32_t)input.length(),
       0};
   byteStream->resetInput({byteRange});
   return byteStream;
 }

 RowVectorPtr deserialize(
     const RowTypePtr& rowType,
     const std::string& input,
     const serializer::presto::PrestoVectorSerde::PrestoOptions*
         serdeOptions) {
   auto byteStream = toByteStream(input);
   auto paramOptions = getParamSerdeOptions(serdeOptions);
   RowVectorPtr result;
   serde_->deserialize(
       byteStream.get(), pool_.get(), rowType, &result, &paramOptions);
   ASSERT_TRUE(byteStream->atEnd());
   return result;
 }

 RowVectorPtr makeTestVector(vector_size_t size) {
   auto a = vectorMaker_->flatVector<int64_t>(
       size, [](vector_size_t row) { return row; });
   auto b = vectorMaker_->flatVector<double>(
       size, [](vector_size_t row) { return row * 0.1; });
   auto c = vectorMaker_->flatVector<std::string>(size, [](vector_size_t row) {
     return row % 2 == 0 ? "LaaaaaaaaargeString" : "inlineStr";
   });

   std::vector<VectorPtr> childVectors = {a, b, c};

   return vectorMaker_->rowVector(childVectors);
 }

 void testRoundTrip(
     VectorPtr vector,
     const serializer::presto::PrestoVectorSerde::PrestoOptions* serdeOptions =
         nullptr) {
   auto rowVector = vectorMaker_->rowVector({vector});
   std::ostringstream out;
   serialize(rowVector, &out, serdeOptions);

   auto rowType = asRowType(rowVector->type());
   auto deserialized = deserialize(rowType, out.str(), serdeOptions);
   assertEqualVectors(deserialized, rowVector);
 }

 std::shared_ptr<memory::MemoryPool> pool_;
 std::unique_ptr<serializer::presto::PrestoVectorSerde> serde_;
 std::unique_ptr<test::VectorMaker> vectorMaker_;
};

TEST_P(MyPrestoSerializerTest, nestedArray) {
 // Define rows to write
 int num_rows = 3;
 SelectivityVector rows{num_rows};
 auto elementType =
     ArrayType(std::make_shared<ArrayType>(ArrayType(INTEGER())));
 VectorPtr result;
 BaseVector::ensureWritable(
     rows,
     std::make_shared<ArrayType>(elementType),
     pool_.get(),
     result);

 exec::VectorWriter<Array<Array<int32_t>>> vectorWriter;
 vectorWriter.init(*result->as<ArrayVector>());
 for (int i = 0; i < num_rows; i++) {
   vectorWriter.setOffset(i);
   auto& arrayWriter = vectorWriter.current();
   // Only general interface is allowed for nested arrays.
   {
     auto& innerArrayWriter = arrayWriter.add_item();
     innerArrayWriter.resize(2);
     innerArrayWriter[0] = 1;
     innerArrayWriter[1] = 2;
   }

   arrayWriter.add_null();

   {
     auto& innerArrayWriter = arrayWriter.add_item();
     innerArrayWriter.resize(3);
     innerArrayWriter[0] = 1;
     innerArrayWriter[1] = std::nullopt;
     innerArrayWriter[2] = 2;
   }
   vectorWriter.commit();
 }

 EXPECT_EQ(num_rows, result->size());
 // 1. Decode the vector for rows of interest.
 DecodedVector decoded;
 decoded.decode(*result, rows);

 // 2. Define vectorReader<T> where T is the type of the vector being read, T
 // is expressed in the simple function type system
 exec::VectorReader<Array<Array<int32_t>>> vectorReader(&decoded);
 std::cout << "Reading Vector: " << std::endl;
 rows.applyToSelected([&](vector_size_t row) {
   // Check if the row is null.
   if (!vectorReader.isSet(row)) {
     std::cout << "[]" << std::endl;
     return;
   }

   std::cout << "[";

   // 3. To read a row call reader[row] and it will return a std::like object
   // that represents the elements at the row.
   // arrayView has std::vector<std::optional<V>> interface
   auto arrayView = vectorReader[row];

   // Elements of the array have std::map<int, std::optional<int>>
   // interface.
   for (const auto& container : arrayView) {
     if (container.has_value()) {
       bool first = true;
       std::cout << " [";
       for (const auto& v : container.value()) {
         if (!first) {
           std::cout << ",";
         }
         std::cout << (v.has_value() ? std::to_string(v.value()) : "null");
         first = false;
       }
       std::cout << "] ";
     } else {
       std::cout << " null ";
     }
   }

   std::cout << "]" << std::endl;
 });

 testRoundTrip(result);
}

TEST_P(MyPrestoSerializerTest, nestedMap) {
 int num_rows = 3;
 SelectivityVector rows{num_rows};
 auto elementType =
     MapType(INTEGER(),
             std::make_shared<ArrayType>(ArrayType(INTEGER())));
 VectorPtr result;

 BaseVector::ensureWritable(
     rows,
     std::make_shared<MapType>(elementType),
     pool_.get(),
     result);

 exec::VectorWriter<Map<int32_t, Array<int32_t>>> vectorWriter;
 vectorWriter.init(*result->as<MapVector>());
 for (int i = 0; i < num_rows; i++) {
   vectorWriter.setOffset(i);
   auto& mapWriter = vectorWriter.current();
   {
     auto [keyWriter, valueWriter] = mapWriter.add_item();
     keyWriter = 1;
     valueWriter.resize(2);
     valueWriter[0] = 1;
     valueWriter[1] = 2;
   }

   mapWriter.add_null();

   {
     auto [keyWriter, valueWriter] = mapWriter.add_item();
     keyWriter = 1;
     valueWriter.resize(3);
     valueWriter[0] = 1;
     valueWriter[1] = std::nullopt;
     valueWriter[2] = 2;
   }

   vectorWriter.commit();
 }

 EXPECT_EQ(num_rows, result->size());

 // 1. Decode the vector for rows of interest.
 DecodedVector decoded;
 decoded.decode(*result, rows);

 // 2. Define vectorReader<T> where T is the type of the vector being read, T
 // is expressed in the simple function type system
 exec::VectorReader<Map<int32_t, Array<int32_t>>> vectorReader(&decoded);
 std::cout << "Reading Vector: " << std::endl;
 rows.applyToSelected([&](vector_size_t row) {
   // Check if the row is null.
   if (!vectorReader.isSet(row)) {
     std::cout << "{}" << std::endl;
     return;
   }

   std::cout << "[";

   // 3. To read a row call reader[row] and it will return a std::like object
   // that represents the elements at the row.
   // arrayView has std::vector<std::optional<V>> interface
   auto mapView = vectorReader[row];

   // Elements of the array have std::map<int, std::optional<int>>
   // interface.
   std::cout << " {";
   for (const auto [k, v] : mapView) {
     if (v.has_value()) {
       std::cout << std::to_string(k) << ": ";
       std::cout << "[";
       bool first = true;
       for (const auto& i : v.value()) {
         if (!first) {
           std::cout << ",";
         }
         std::cout << (i.has_value() ? std::to_string(i.value()) : "");
         first = false;
       }
       std::cout << "], ";
     } else {
       std::cout << " null, ";
     }
   }
   std::cout << "}" << std::endl;
 });

 testRoundTrip(result);
}

INSTANTIATE_TEST_SUITE_P(
   PrestoSerializerTest,
   PrestoSerializerTest,
   ::testing::Values(
       common::CompressionKind::CompressionKind_NONE,
       common::CompressionKind::CompressionKind_ZLIB,
       common::CompressionKind::CompressionKind_SNAPPY,
       common::CompressionKind::CompressionKind_ZSTD,
       common::CompressionKind::CompressionKind_LZ4,
       common::CompressionKind::CompressionKind_GZIP));
