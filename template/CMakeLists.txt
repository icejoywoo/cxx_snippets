add_executable(template_unit_test
    function_types_test.cc)
target_link_libraries(template_unit_test PRIVATE
    GTest::gtest
    GTest::gtest_main)
target_compile_features(template_unit_test PRIVATE
    cxx_std_17)

add_test(template_unit_test template_unit_test)

add_executable(pow_constexpr_bench pow_constexpr_bench.cc)
target_link_libraries(pow_constexpr_bench benchmark::benchmark)
target_compile_features(pow_constexpr_bench PRIVATE cxx_std_17)