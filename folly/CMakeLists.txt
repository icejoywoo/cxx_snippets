add_executable(folly_unit_test
    dynamic_test.cc
    compression_test.cc
    iobuf_test.cc
    hasher_test.cc
    future_test.cc)
target_link_libraries(folly_unit_test PRIVATE
    Folly::folly
    GTest::gtest
    GTest::gtest_main
    spooky_hash)
target_compile_features(folly_unit_test PRIVATE
    cxx_std_17)

add_library(spooky_hash
    SpookyV2.cpp)

add_test(folly_unit_test folly_unit_test)

add_executable(hasher_benchmark hasher_benchmark.cc)
target_link_libraries(hasher_benchmark PRIVATE
    Folly::folly
    ${FOLLY_BENCHMARK_STATIC_LIB}
    spooky_hash)