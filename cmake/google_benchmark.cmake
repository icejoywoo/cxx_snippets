# https://github.com/CraigN/GoogleBenchmark-CMake-Example/blob/master/src/cmake/GoogleBenchmark.cmake
include_guard(GLOBAL)



include(FetchContent)

set(CXX_GOOGLE_BENCHMARK_VERSION 1.8.2)
set(CXX_GOOGLE_BENCHMARK_BUILD_MD5_CHECKSUM b2987cb9da36b1b121c0325195b91755)

message(STATUS "Building google benchmark from source")
# Disable the Google Benchmark requirement on Google Test
set(BENCHMARK_ENABLE_TESTING NO)

FetchContent_Declare(
        googlebenchmark
        URL https://github.com/google/benchmark/archive/refs/tags/v${CXX_GOOGLE_BENCHMARK_VERSION}.tar.gz
        URL_HASH MD5=${CXX_GOOGLE_BENCHMARK_BUILD_MD5_CHECKSUM}
)

FetchContent_MakeAvailable(googlebenchmark)