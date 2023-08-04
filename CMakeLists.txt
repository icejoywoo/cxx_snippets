cmake_minimum_required(VERSION 3.25)
project(cxx_snippets)

set(CMAKE_CXX_STANDARD 17)

list(PREPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

# Include our ThirdPartyToolchain dependencies macros
include(ResolveDependency)

set_with_default(VELOX_DEPENDENCY_SOURCE_DEFAULT VELOX_DEPENDENCY_SOURCE AUTO)
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

# Locate or build folly.
add_compile_definitions(FOLLY_HAVE_INT128_T=1)
set_source(folly)
resolve_dependency(folly)

include(google_benchmark)
include(xsimd)

add_subdirectory(velox)
