add_executable(cxx_unit_test
  function_test.cc
  mutex_test.cc
  ptr_test.cc
  string_view_test.cc
  scoped_cleanup_test.cc)
target_link_libraries(cxx_unit_test PRIVATE
  GTest::gtest
  GTest::gtest_main)
target_compile_features(cxx_unit_test PRIVATE
  cxx_std_17)

add_test(cxx_unit_test cxx_unit_test)

add_executable(string_benchmark string_benchmark.cc)
target_link_libraries(string_benchmark PRIVATE
  Folly::folly
  ${FOLLY_BENCHMARK_STATIC_LIB})