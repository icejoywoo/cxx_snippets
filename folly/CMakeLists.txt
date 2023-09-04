add_executable(folly_unit_test
  dynamic_test.cc
  compression_test.cc
  iobuf_test.cc
  hasher_test.cc
  future_test.cc
  my_lz4.cc)
target_link_libraries(folly_unit_test PRIVATE
  Folly::folly
  GTest::gtest
  GTest::gtest_main)
target_compile_features(folly_unit_test PRIVATE
  cxx_std_17)

add_test(folly_unit_test folly_unit_test)

add_executable(compression_benchmark
  compression_benchmark.cc
  my_lz4.cc)
target_link_libraries(compression_benchmark PRIVATE
  Folly::folly
  ${FOLLY_BENCHMARK_STATIC_LIB})