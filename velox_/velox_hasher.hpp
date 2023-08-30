#pragma once

#include "SpookyV2.h"

#include <map>
#include <string>
#include <vector>

namespace velox::hash {
// copied from folly
namespace {
struct to_unsigned_fn {
    template<typename..., typename T>
    constexpr auto operator()(T const &t) const noexcept ->
            typename std::make_unsigned<T>::type {
        using U = typename std::make_unsigned<T>::type;
        return static_cast<U>(t);
    }
};
constexpr to_unsigned_fn to_unsigned{};
}

// Order-independent way to reduce multiple 64 bit hashes into a
// single hash. Copied from folly/hash/Hash.h because this is not
// defined in some versions of folly.
inline uint64_t commutativeHashMix(
    const uint64_t upper,
    const uint64_t lower) noexcept {
  // Commutative accumulator taken from this paper:
  // https://www.preprints.org/manuscript/201710.0192/v1/download
  return 3860031 + (upper + lower) * 2779 + (upper * lower * 2);
}

// This is the Hash128to64 function from Google's cityhash (available
// under the MIT License).  We use it to reduce multiple 64 bit hashes
// into a single hash.
inline uint64_t hashMix(const uint64_t upper, const uint64_t lower) noexcept {
  // Murmur-inspired hashing.
  const uint64_t kMul = 0x9ddfea08eb382d69ULL;
  uint64_t a = (lower ^ upper) * kMul;
  a ^= (a >> 47);
  uint64_t b = (upper ^ a) * kMul;
  b ^= (b >> 47);
  b *= kMul;
  return b;
}

/**
 * Robert Jenkins' reversible 32 bit mix hash function.
 *
 * @methodset jenkins
 */
constexpr uint32_t jenkins_rev_mix32(uint32_t key) noexcept {
  key += (key << 12); // key *= (1 + (1 << 12))
  key ^= (key >> 22);
  key += (key << 4); // key *= (1 + (1 << 4))
  key ^= (key >> 9);
  key += (key << 10); // key *= (1 + (1 << 10))
  key ^= (key >> 2);
  // key *= (1 + (1 << 7)) * (1 + (1 << 12))
  key += (key << 7);
  key += (key << 12);
  return key;
}

/**
 * Thomas Wang 64 bit mix hash function.
 *
 * @methodset twang
 */
constexpr uint64_t twang_mix64(uint64_t key) noexcept {
  key = (~key) + (key << 21); // key *= (1 << 21) - 1; key -= 1;
  key = key ^ (key >> 24);
  key = key + (key << 3) + (key << 8); // key *= 1 + (1 << 3) + (1 << 8)
  key = key ^ (key >> 14);
  key = key + (key << 2) + (key << 4); // key *= 1 + (1 << 2) + (1 << 4)
  key = key ^ (key >> 28);
  key = key + (key << 31); // key *= 1 + (1 << 31)
  return key;
}

/**
 * Reduce two 64-bit hashes into one.
 *
 * hash_128_to_64 uses the Hash128to64 function from Google's cityhash (under
 * the MIT License).
 */
constexpr uint64_t hash_128_to_64(
    const uint64_t upper, const uint64_t lower) noexcept {
  // Murmur-inspired hashing.
  const uint64_t kMul = 0x9ddfea08eb382d69ULL;
  uint64_t a = (lower ^ upper) * kMul;
  a ^= (a >> 47);
  uint64_t b = (upper ^ a) * kMul;
  b ^= (b >> 47);
  b *= kMul;
  return b;
}

constexpr uint64_t hash_128_to_64(__uint128_t u) {
    // sizeof(__uint128_t) * 8 是完整 bit 数，sizeof(__uint128_t) * 4 就是一半的 bit 数
    auto const hi = static_cast<uint64_t>(u >> sizeof(__uint128_t) * 4);
    auto const lo = static_cast<uint64_t>(u);
    return hash_128_to_64(hi, lo);
}

namespace detail {
template <typename Int>
struct integral_hasher {
constexpr size_t operator()(Int const& i) const noexcept {
    static_assert(sizeof(Int) <= 16, "Input type is too wide");
    if constexpr (sizeof(Int) <= 4) {
        auto const i32 = static_cast<int32_t>(i); // impl accident: sign-extends
        auto const u32 = static_cast<uint32_t>(i32);
        return static_cast<size_t>(jenkins_rev_mix32(u32));
    } else if constexpr (sizeof(Int) <= 8) {
        auto const u64 = static_cast<uint64_t>(i);
        return static_cast<size_t>(twang_mix64(u64));
    } else {
        auto const u = to_unsigned(i);
        auto const hi = static_cast<uint64_t>(u >> sizeof(Int) * 4);
        auto const lo = static_cast<uint64_t>(u);
        return hash_128_to_64(hi, lo);
    }
}
};
}

// float
template <typename F>
struct float_hasher {
  size_t operator()(F const& f) const noexcept {
    static_assert(sizeof(F) <= 8, "Input type is too wide");

    if (f == F{}) { // Ensure 0 and -0 get the same hash.
      return 0;
    }

    uint64_t u64 = 0;
    memcpy(&u64, &f, sizeof(F));
    return static_cast<size_t>(twang_mix64(u64));
  }
};

// std::pair
template <typename T>
struct hasher;

struct Hash {
  template <class T>
  constexpr size_t operator()(const T& v) const
      noexcept(noexcept(hasher<T>()(v))) {
    return hasher<T>()(v);
  }

  template <class T, class... Ts>
  constexpr size_t operator()(const T& t, const Ts&... ts) const {
    return hash_128_to_64((*this)(t), (*this)(ts...));
  }

  constexpr size_t operator()() const noexcept { return 0; }
};

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
struct hasher<float> : float_hasher<float> {};

template <>
struct hasher<double> : float_hasher<double> {};

template <typename T1, typename T2>
struct hasher<std::pair<T1, T2>> {
  size_t operator()(const std::pair<T1, T2>& key) const {
    return Hash()(key.first, key.second);
  }
};

template <>
struct hasher<bool> {
  constexpr size_t operator()(bool key) const noexcept {
    // Make sure that all the output bits depend on the input.
    return key ? std::numeric_limits<size_t>::max() : 0;
  }
};


// string
template <>
struct hasher<std::string> {
  size_t operator()(const std::string& key) const {
    return static_cast<size_t>(
        SpookyHash::Hash64(key.data(), key.size(), 0));
  }
};

template <>
struct hasher<std::string_view> {
  size_t operator()(const std::string_view& key) const {
    return static_cast<size_t>(
        SpookyHash::Hash64(key.data(), key.size(), 0));
  }
};

// complex type hash implementation
static constexpr uint64_t nullHash = 1;
template <typename T>
uint64_t hashArray(
    uint64_t hash,
    const std::vector<T>& elements) {
  for (auto i = 0; i < elements.size(); ++i) {
    auto elementHash = hasher<T>{}(elements[i]);
    hash = commutativeHashMix(hash, elementHash);
  }
  return hash;
}

template <typename T>
uint64_t hashArray(
    const std::vector<T>& elements) {
  return hashArray(nullHash, elements);
}

template <typename T>
struct hasher<std::vector<T>> {
  size_t operator()(const std::vector<T>& elements) const {
    return hashArray(elements);
  }
};

template <typename Key, typename Value>
uint64_t hashMap(
    const std::map<Key, Value>& elements) {
  std::vector<Key> keys;
  std::vector<Value> values;
  for (auto& [key, value] : elements) {
    keys.emplace_back(key);
    values.emplace_back(value);
  }
  return hashArray<Value>(hashArray<Key>(nullHash, keys), values);
}

template <typename Key, typename Value>
struct hasher<std::map<Key, Value>> {
  size_t operator()(const std::map<Key, Value>& elements) const {
    return hashMap(elements);
  }
};

template <size_t I = 0, typename FuncT, typename... Tp>
inline typename std::enable_if_t<I == sizeof...(Tp)>
    for_each(std::tuple<Tp...>&, FuncT) {}

template <size_t I = 0, typename FuncT, typename... Tp>
inline typename std::enable_if_t <I<sizeof...(Tp)>
    for_each(std::tuple<Tp...>& t, FuncT f) {
  f(std::get<I>(t));
  for_each<I + 1, FuncT, Tp...>(t, f);
}

template <typename T, typename... Args>
uint64_t hashRow(const T& t, const Args&... args) {
  uint64_t hash = nullHash;
  hash = hasher<T>{}(t);
  auto a = std::forward_as_tuple(args...);
  for_each(a, [&hash](auto x) {
    hash = hashMix(hash, hasher<decltype(x)>{}(x));
  });
  return hash;
}

template <typename... Ts>
struct hasher<std::tuple<Ts...>> {
  size_t operator()(const std::tuple<Ts...>& key) const {
    uint64_t hash = nullHash;
    bool isFirst = true;
    std::apply([&hash, &isFirst](Ts... args) {
      auto a = std::forward_as_tuple(args...);
      for_each(a, [&hash, &isFirst](auto x) {
        hash = isFirst ? hasher<decltype(x)>{}(x) : hashMix(hash, hasher<decltype(x)>{}(x));
        isFirst = false;
      });
    }, key);
    return hash;
  }
};

} // namespace velox::hash