include_guard(GLOBAL)

set(VELOX_FIZZ_VERSION v2022.11.14.00)
set(VELOX_FIZZ_BUILD_SHA256_CHECKSUM
    8a5e4d854be78ca965bed630b84b4e26cafe29b99e5701c3630fb3cea1104b7c)
set(VELOX_FIZZ_SOURCE_URL
    "https://github.com/facebookincubator/fizz/archive/${VELOX_FIZZ_VERSION}.tar.gz")

resolve_dependency_url(FIZZ)

message(STATUS "Building fizz from source")
FetchContent_Declare(
  fizz
  URL ${VELOX_FIZZ_SOURCE_URL}
  URL_HASH ${VELOX_FIZZ_BUILD_SHA256_CHECKSUM})

if(${folly_SOURCE} STREQUAL "BUNDLED")
  message(STATUS "${folly_BINARY_DIR} ${folly_SOURCE_DIR}")
  set(FOLLY_INCLUDE_DIR ${folly_SOURCE_DIR})
  set(folly_DIR ${folly_BINARY_DIR})
endif()

# Force fmt to create fmt-config.cmake which can be found by other dependecies
# (e.g. folly)
set(FIZZ_INSTALL ON)
set(fizz_BUILD_TESTS OFF)
FetchContent_MakeAvailable(fizz)
list(APPEND CMAKE_PREFIX_PATH ${fizz_BINARY_DIR})
include_directories(${fizz_SOURCE_DIR})
add_subdirectory(${fizz_SOURCE_DIR}/fizz)
add_library(fizz::fizz ALIAS fizz)
