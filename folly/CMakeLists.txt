add_executable(folly_unit_test
    dynamic_test.cc
    compression_test.cc
    iobuf_test.cc)
target_link_libraries(folly_unit_test PRIVATE
    Folly::folly
    GTest::gtest
    GTest::gtest_main)
target_compile_features(folly_unit_test PRIVATE
    cxx_std_17)

add_test(folly_unit_test folly_unit_test)