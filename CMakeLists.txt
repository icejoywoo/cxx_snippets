cmake_minimum_required(VERSION 3.25)
project(cxx_snippets)

set(CMAKE_CXX_STANDARD 17)

list(PREPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

include(google_benchmark)
include(xsimd)

add_subdirectory(velox)
