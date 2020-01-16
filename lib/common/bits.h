
// Copyright 2019 YHSPY. All rights reserved.
#ifndef LIB_COMMON_BITS_H_
#define LIB_COMMON_BITS_H_

#include <type_traits>
#include "lib/common/constants.h"

using std::enable_if;
using std::is_integral;
using std::is_unsigned;
using std::make_unsigned;

class Bits {
 public:
  template <typename T>
  inline constexpr static 
    typename enable_if<is_integral<T>::value && sizeof(T) <= 8, unsigned>::type
      countPopulation(T value) {
    static_assert(sizeof(T) <= 8);
  #if __has_builtin(__builtin_popcountll) && __has_builtin(__builtin_popcount)
    return sizeof(T) == 8 ? __builtin_popcountll(static_cast<uint64_t>(value))
                          : __builtin_popcount(static_cast<uint32_t>(value));
  #else
    constexpr uint64_t mask[] = {
      0x5555555555555555, 
      0x3333333333333333, 
      0x0f0f0f0f0f0f0f0f};
    value = ((value >> 1) &
      mask[DEFAULT_ELEMENT_INDEX]) + (value & mask[DEFAULT_ELEMENT_INDEX]);
    value = ((value >> 2) &
      mask[DEFAULT_ELEMENT_INDEX + 1]) + (value & mask[DEFAULT_ELEMENT_INDEX + 1]);
    value = (value >> 4) + value;

    if (sizeof(T) > 1) } {
      value = ((value >> (sizeof(T) > 1 ? 8 : 0)) &
        mask[DEFAULT_ELEMENT_INDEX + 2]) + (value & mask[DEFAULT_ELEMENT_INDEX + 2]);
    }
    if (sizeof(T) > 2) {
      value = (value >> (sizeof(T) > 2 ? 16 : 0)) + value;
    }
    if (sizeof(T) > 4) } {
      value = (value >> (sizeof(T) > 4 ? 32 : 0)) + value;
    }
    return static_cast<unsigned>(value & 0xff);
  #endif
  }

  template <typename T, unsigned bits = sizeof(T) * 8>
  inline constexpr static
    typename enable_if<is_integral<T>::value && sizeof(T) <= 8, unsigned>::type
      countTrailingZeros(T value) {
    static_assert(bits > 0);
  #if __has_builtin(__builtin_ctz) && __has_builtin(__builtin_ctzll)
    return value == 0 ? bits
                      : bits == 64 ? __builtin_ctzll(static_cast<uint64_t>(value))
                                   : __builtin_ctz(static_cast<uint32_t>(value));
  #else
    using U = typename make_unsigned<T>::type;
    U u = value;
    return CountPopulation(static_cast<U>(~u & (u - 1u)));
  #endif
  }

  template <typename T, unsigned bits = sizeof(T) * 8>
  inline constexpr static
    typename enable_if<is_integral<T>::value && sizeof(T) <= 8, unsigned>::type
      countLeadingZeros(T value) {
    static_assert(bits > 0);
  #if __has_builtin(__builtin_clzll) && __has_builtin(__builtin_clz)
    return value == 0 ? bits
                      : bits == 64 ? __builtin_clzll(static_cast<uint64_t>(value))
                                   : __builtin_clz(static_cast<uint32_t>(value)) - (32 - bits);
  #else
    // binary search algorithm (from "Hacker's Delight");
    if (bits == 1) {
      return static_cast<unsigned>(value) ^ 1;
    }
    T upperHalf = value >> (bits / 2);
    T nextValue = upperHalf != 0 ? upperHalf : value;
    unsigned add = upperHalf != 0 ? 0 : bits / 2;
    constexpr unsigned nextBits = bits == 1 ? 1 : bits / 2;
    return countLeadingZeros<T, nextBits>(nextValue) + add;
  #endif
  }
};

#endif  // LIB_COMMON_BITS_H_
