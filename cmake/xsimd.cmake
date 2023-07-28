include_guard(GLOBAL)

set(CXX_XSIMD_VERSION 10.0.0)
set(CXX_XSIMD_BUILD_MD5_CHECKSUM
        e0dfed5da51b0d34d02b42f5b2ddf830)
set(CXX_XSIMD_SOURCE_URL
        "https://github.com/xtensor-stack/xsimd/archive/refs/tags/${CXX_XSIMD_VERSION}.tar.gz"
        )

message(STATUS "Building xsimd from source")
FetchContent_Declare(
        xsimd
        URL ${CXX_XSIMD_SOURCE_URL}
        URL_HASH MD5=${CXX_XSIMD_BUILD_MD5_CHECKSUM})

FetchContent_MakeAvailable(xsimd)