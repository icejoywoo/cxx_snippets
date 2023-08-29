include_guard(GLOBAL)

set(VELOX_PROXYGEN_VERSION v2022.11.14.00)
set(VELOX_PROXYGEN_BUILD_SHA256_CHECKSUM
    c4fe42ae3c17dd51a99887a12ed14a4d0afff19d1897af2cf78f94429760e39c)
set(VELOX_PROXYGEN_SOURCE_URL
    "https://github.com/facebook/proxygen/archive/${VELOX_PROXYGEN_VERSION}.tar.gz")

resolve_dependency_url(PROXYGEN)

if(${folly_SOURCE} STREQUAL "BUNDLED")
  message(STATUS "${folly_BINARY_DIR} ${folly_SOURCE_DIR}")
  set(FOLLY_INCLUDE_DIR ${folly_SOURCE_DIR})
  set(folly_DIR ${folly_BINARY_DIR})
endif()

if(${fizz_SOURCE} STREQUAL "BUNDLED")
  message(STATUS "${fizz_BINARY_DIR} ${fizz_SOURCE_DIR}")
  set(FIZZ_INCLUDE_DIR ${fizz_SOURCE_DIR})
  set(fizz_DIR ${fizz_BINARY_DIR})
endif()

if(${wangle_SOURCE} STREQUAL "BUNDLED")
  message(STATUS "${wangle_BINARY_DIR} ${wangle_SOURCE_DIR}")
  set(WANGLE_INCLUDE_DIR ${wangle_SOURCE_DIR})
  set(wangle_DIR ${wangle_BINARY_DIR})
endif()

message(STATUS "Building proxygen from source")
FetchContent_Declare(
  proxygen
  URL ${VELOX_PROXYGEN_SOURCE_URL}
  URL_HASH ${VELOX_PROXYGEN_BUILD_SHA256_CHECKSUM})

# Force fmt to create fmt-config.cmake which can be found by other dependecies
# (e.g. folly)
set(PROXYGEN_INSTALL ON)
set(proxygen_BUILD_TESTS OFF)
FetchContent_MakeAvailable(proxygen)
list(APPEND CMAKE_PREFIX_PATH ${proxygen_BINARY_DIR})
