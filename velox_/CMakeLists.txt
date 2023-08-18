add_executable(velox_unit_test
    velox_vector_hasher_test.cc)
target_link_libraries(velox_unit_test PRIVATE
    Folly::folly
    GTest::gtest
    GTest::gtest_main
    velox_exec
    velox_vector_test_lib)
target_compile_features(velox_unit_test PRIVATE
    cxx_std_17)

add_test(velox_unit_test velox_unit_test)