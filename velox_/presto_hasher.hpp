#pragma once

#include <map>
#include <string>
#include <vector>

#include "xxhash64.h"

#include "velox/type/StringView.h"
#include "velox/type/Timestamp.h"
#include "velox/type/Type.h"

namespace presto::hash {
// combine hash
inline uint64_t combineHash(uint64_t hash, uint64_t value) {
    return hash * 31 + value;
}

// TypeUtils.hashPosition
constexpr int NULL_HASH_CODE = 0;

template <typename Int>
inline long rotateLeft(Int i, int distance) {
    constexpr int bits = sizeof(Int) * 8;
    distance = distance % bits; // 确保distance在0~bits之间

    // 左移distance位，将被移出的位放到右侧
    return (i << distance) | (i >> (bits - distance));
}

// AbstractLongType::hash
// AbstractIntType
// SmallintType
// TinyintType
// DoubleType::hash -> use AbstractLongType.hash
template <typename Int>
inline uint64_t hashInt(Int value) {
    // xxhash64 mix
    return rotateLeft(value * 0xC2B2AE3D27D4EB4FL, 31) * 0x9E3779B185EBCA87L;
}

// VariableWidthBlock hash
inline uint64_t hashBlock(const std::string& s) {
    return XXHash64::hash((void*)s.c_str(), (uint64_t)s.length(), 0);
}

// UnscaledDecimal128Arithmetic, LongDecimalType.hash
inline uint64_t hash(uint64_t rawLow, uint64_t rawHigh) {
    static constexpr uint64_t SIGN_LONG_MASK = 1L << 63;
    return XXHash64::hash(rawLow) ^ XXHash64::hash(rawHigh & ~SIGN_LONG_MASK);
}

// LongDecimalType, long long
inline uint64_t hash_int128(__uint128_t u) {
    // little endian: low + high
    auto const lo = static_cast<uint64_t>(u >> sizeof(__uint128_t) * 4);
    auto const hi = static_cast<uint64_t>(u);
    return hash(lo, hi);
}

template <typename T>
struct hasher;

// integer hasher
namespace detail {
    struct to_unsigned_fn {
        template <typename..., typename T>
        constexpr auto operator()(T const& t) const noexcept ->
                typename std::make_unsigned<T>::type {
            using U = typename std::make_unsigned<T>::type;
            return static_cast<U>(t);
        }
    };
    constexpr to_unsigned_fn to_unsigned{};

    template <typename Int>
    struct integral_hasher {
        constexpr uint64_t operator()(Int const& i) const noexcept {
            static_assert(sizeof(Int) <= 16, "Input type is too wide");
            if constexpr (sizeof(Int) <= 4) {
                auto const i32 = static_cast<int32_t>(i); // impl accident: sign-extends
                auto const u32 = static_cast<uint32_t>(i32);
                return static_cast<uint64_t>(hashInt(u32));
            } else if (sizeof(Int) <= 8) {
                auto const u64 = static_cast<uint64_t>(i);
                return static_cast<uint64_t>(hashInt(u64));
            } else {
                auto const u = to_unsigned(i);
                return hash_int128(u);
            }
        }
    };
} // namespace detail

template <>
struct hasher<unsigned long long>
    : detail::integral_hasher<unsigned long long> {};

template <>
struct hasher<signed long long> : detail::integral_hasher<signed long long> {};

template <>
struct hasher<unsigned long> : detail::integral_hasher<unsigned long> {};

template <>
struct hasher<signed long> : detail::integral_hasher<signed long> {};

template <>
struct hasher<unsigned int> : detail::integral_hasher<unsigned int> {};

template <>
struct hasher<signed int> : detail::integral_hasher<signed int> {};

template <>
struct hasher<unsigned short> : detail::integral_hasher<unsigned short> {};

template <>
struct hasher<signed short> : detail::integral_hasher<signed short> {};

template <>
struct hasher<unsigned char> : detail::integral_hasher<unsigned char> {};

template <>
struct hasher<signed char> : detail::integral_hasher<signed char> {};

template <> // char is a different type from both signed char and unsigned char
struct hasher<char> : detail::integral_hasher<char> {};

template <>
struct hasher<signed __int128> : detail::integral_hasher<signed __int128> {};

template <>
struct hasher<unsigned __int128> : detail::integral_hasher<unsigned __int128> {
};

template <>
struct hasher<double> {
    uint64_t operator()(double value) const noexcept {
        // java Double.doubleToRawLongBits method
        auto* buf = reinterpret_cast<unsigned long*>(&value);
        return hasher<unsigned long>{}(*buf);
    }
};

template <>
struct hasher<float> {
    uint64_t operator()(float value) const noexcept {
        auto* buf = reinterpret_cast<unsigned int*>(&value);
        return hasher<unsigned int>{}(*buf);
    }
};

template <>
struct hasher<bool> {
    constexpr uint64_t operator()(bool value) const noexcept {
        return value != 0 ? 1231 : 1237;
    }
};

// string
template <>
struct hasher<std::string> {
    constexpr uint64_t operator()(const std::string& value) const noexcept {
        return hashBlock(value);
    }
};

template <>
struct hasher<facebook::velox::StringView> {
    constexpr uint64_t operator()(const facebook::velox::StringView& value) const noexcept {
        return XXHash64::hash(value.data(), value.size(), 0);
    }
};

template <>
struct hasher<facebook::velox::Timestamp> {
    constexpr uint64_t operator()(const facebook::velox::Timestamp& value) const noexcept {
        // presto Timestamp has milliseconds and microseconds time unit, use AbstractLongType.hash
        return hasher<unsigned long long>{}(value.toMicros());
    }
};

template <>
struct hasher<facebook::velox::UnknownValue> {
    constexpr uint64_t operator()(const facebook::velox::UnknownValue& value) const noexcept {
        return 0;
    }
};

template <>
struct hasher<std::shared_ptr<void>> {
    constexpr uint64_t operator()(const std::shared_ptr<void>& value) const noexcept {
        return 0;
    }
};

template <typename T>
uint64_t hashArray(std::vector<T> array) {
    uint64_t result = 0;
    for (auto& i : array) {
        result = combineHash(result, hasher<T>{}(i));
    }
    return result;
}

template <typename Key, typename Value>
uint64_t hashMap(std::map<Key, Value> m) {
    uint64_t result = 0;
    for (auto& [key, value] : m) {
        result += hasher<Key>{}(key) ^ hasher<Value>{}(value);
    }
    return result;
}
template <size_t I = 0, typename FuncT, typename... Tp>
inline typename std::enable_if_t<I == sizeof...(Tp)> for_each(
        std::tuple<Tp...>&,
        FuncT) {}

template <size_t I = 0, typename FuncT, typename... Tp>
        inline typename std::enable_if_t <
        I<sizeof...(Tp)> for_each(std::tuple<Tp...>& t, FuncT f) {
    f(std::get<I>(t));
    for_each<I + 1, FuncT, Tp...>(t, f);
}

template <typename T, typename... Args>
uint64_t hashRow(const T& t, const Args&... args) {
    uint64_t hash = 1;
    hash = combineHash(hash, hasher<T>{}(t));
    auto a = std::forward_as_tuple(args...);
    for_each(a, [&hash](auto x) {
        hash = combineHash(hash, hasher<decltype(x)>{}(x));
    });
    return hash;
}
} // namespace presto::hash
