cmake_minimum_required(VERSION 3.20)

project(velox_snippets)

# enable FetchContent
include(FetchContent)

list(PREPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

execute_process(
        COMMAND
        bash -c
        "( source ${CMAKE_CURRENT_SOURCE_DIR}/velox/scripts/setup-helper-functions.sh && echo -n $(get_cxx_flags $ENV{CPU_TARGET}))"
        OUTPUT_VARIABLE SCRIPT_CXX_FLAGS
        RESULT_VARIABLE COMMAND_STATUS)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
message("Appending CMAKE_CXX_FLAGS with ${SCRIPT_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SCRIPT_CXX_FLAGS}")


# Include our ThirdPartyToolchain dependencies macros
include(ResolveDependency)

set_with_default(VELOX_DEPENDENCY_SOURCE_DEFAULT VELOX_DEPENDENCY_SOURCE AUTO)
# set CMAKE_BUILD_TYPE
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RELEASE)
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# * AUTO: Try SYSTEM first fall back to BUNDLED.
# * SYSTEM: Use installed dependencies via find_package.
# * BUNDLED: Build dependencies from source.
set(VELOX_DEPENDENCY_SOURCE
    ${VELOX_DEPENDENCY_SOURCE_DEFAULT}
    CACHE
      STRING
      "Default source for all dependencies with source builds enabled: AUTO SYSTEM BUNDLED."
)

set_source(gflags)
resolve_dependency(gflags COMPONENTS shared)
if(NOT TARGET gflags::gflags)
  # This is a bit convoluted, but we want to be able to use gflags::gflags as a
  # target even when velox is built as a subproject which uses
  # `find_package(gflags)` which does not create a globally imported target that
  # we can ALIAS.
  add_library(gflags_gflags INTERFACE)
  target_link_libraries(gflags_gflags INTERFACE gflags)
  add_library(gflags::gflags ALIAS gflags_gflags)
endif()

if(${gflags_SOURCE} STREQUAL "BUNDLED")
  # we force glog from source to avoid issues with a system version built
  # against another gflags version (which is likely)
  set(glog_SOURCE BUNDLED)
else()
  set(glog_SOURCE SYSTEM)
endif()
resolve_dependency(glog)

enable_testing()

set_source(fmt)
resolve_dependency(fmt)


# Locate or build folly.
add_compile_definitions(FOLLY_HAVE_INT128_T=1)
set_source(folly)
resolve_dependency(folly)
if(DEFINED FOLLY_BENCHMARK_STATIC_LIB)
  set(FOLLY_BENCHMARK ${FOLLY_BENCHMARK_STATIC_LIB})
else()
  set(FOLLY_BENCHMARK Folly::follybenchmark)
endif()

# TODO fix external dependencies
# set_source(fizz)
# resolve_dependency(fizz)
# set_source(wangle)
# resolve_dependency(wangle)
# set_source(proxygen)
# resolve_dependency(proxygen)

set_source(GTest)
resolve_dependency(GTest)

# Disable the Google Benchmark requirement on Google Test
set(BENCHMARK_ENABLE_TESTING NO)

FetchContent_Declare(
    googlebenchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG v1.8.2
)

FetchContent_MakeAvailable(googlebenchmark)

# for generated pb.h
include_directories(${CMAKE_BINARY_DIR})

# failed to build icu from source
# set_source(ICU)
# resolve_dependency(ICU)
set(ICU_SOURCE SYSTEM)
add_subdirectory(velox)
include_directories(velox)

add_subdirectory(cxx17)
add_subdirectory(template)
add_subdirectory(folly)
add_subdirectory(algorithm)
add_subdirectory(velox_)