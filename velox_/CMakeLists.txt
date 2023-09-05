add_executable(velox_unit_test
  vector_hasher_test.cc
  velox_hasher_test.cc
  presto_serializer_test.cc)
target_link_libraries(velox_unit_test PRIVATE
  Folly::folly
  GTest::gtest
  GTest::gtest_main
  xsimd
  spooky_hash
  velox_exec
  velox_vector_test_lib)
target_compile_features(velox_unit_test PRIVATE
  cxx_std_17)

add_library(spooky_hash
  SpookyV2.cpp)

add_test(velox_unit_test velox_unit_test)

add_executable(hasher_benchmark hasher_benchmark.cc)
target_link_libraries(hasher_benchmark PRIVATE
  Folly::folly
  ${FOLLY_BENCHMARK_STATIC_LIB}
  spooky_hash)