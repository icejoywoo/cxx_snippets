include_guard(GLOBAL)

set(VELOX_WANGLE_VERSION v2022.11.14.00)
set(VELOX_WANGLE_BUILD_SHA256_CHECKSUM
    497187d405bbf1580cc311e6996a4668d7f68082304c564ce54454be9e853506)
set(VELOX_WANGLE_SOURCE_URL
    "https://github.com/facebook/wangle/archive/${VELOX_WANGLE_VERSION}.tar.gz")

resolve_dependency_url(WANGLE)

message(STATUS "Building wangle from source")
FetchContent_Declare(
  wangle
  URL ${VELOX_WANGLE_SOURCE_URL}
  URL_HASH ${VELOX_WANGLE_BUILD_SHA256_CHECKSUM}
  OVERRIDE_FIND_PACKAGE)

if(${fizz_SOURCE} STREQUAL "BUNDLED")
  message(STATUS "${fizz_BINARY_DIR} ${fizz_SOURCE_DIR}")
  set(FIZZ_INCLUDE_DIR ${fizz_SOURCE_DIR})
  set(fizz_DIR ${fizz_BINARY_DIR})
endif()

# Force fmt to create fmt-config.cmake which can be found by other dependecies
# (e.g. folly)
set(WANGLE_INSTALL ON)
set(wangle_BUILD_TESTS OFF)
FetchContent_MakeAvailable(wangle)
list(APPEND CMAKE_PREFIX_PATH ${wangle_BINARY_DIR})
add_subdirectory(${wangle_SOURCE_DIR}/wangle)
add_library(wangle::wangle ALIAS wangle)
