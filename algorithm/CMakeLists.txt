add_executable(algorithm_test
  UnionFind_test.cc
  boost_string_test.cc)

target_link_libraries(algorithm_test PRIVATE
  GTest::gtest
  GTest::gtest_main
  Boost::headers)
target_compile_features(algorithm_test PRIVATE
  cxx_std_17)

add_test(algorithm_test algorithm_test)