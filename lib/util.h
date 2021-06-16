// Copyright 2021 YHSPY. All rights reserved.
#ifndef LIB_UTIL_H_
#define LIB_UTIL_H_

#include <type_traits>

#if defined(LINUX)
#include <sys/sysinfo.h>
#endif

#define DEFAULT_CPU_CORE 4

namespace TWVM {
  struct Util {
    template <typename Enumeration>
    static auto asInteger(Enumeration const value)
      -> typename std::underlying_type<Enumeration>::type {
        return static_cast<typename std::underlying_type<Enumeration>::type>(value);
      }
    void printAssistantInfo(bool);
    static int getNprocs() {
#if defined(LINUX)
      return get_nprocs();
#else
      return DEFAULT_CPU_CORE;
#endif
    }
    template <typename T>
    static constexpr typename std::enable_if_t<std::is_integral_v<T> && sizeof(T) <= 8, unsigned>
    countPopulation(T value) {
      static_assert(sizeof(T) <= 8);
#if __has_builtin(__builtin_popcountll) && __has_builtin(__builtin_popcount)
      return sizeof(T) == 8 ? __builtin_popcountll(static_cast<uint64_t>(value))
                            : __builtin_popcount(static_cast<uint32_t>(value));
#else
      constexpr uint64_t mask[] = {
        0x5555555555555555, 
        0x3333333333333333, 
        0x0f0f0f0f0f0f0f0f,
      };
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
    constexpr static std::enable_if_t<std::is_integral_v<T> && sizeof(T) <= 8, unsigned>
    countTrailingZeros(T value) {
      static_assert(bits > 0);
#if __has_builtin(__builtin_ctz) && __has_builtin(__builtin_ctzll)
      return value == 0 ? bits
                        : bits == 64 ? __builtin_ctzll(static_cast<uint64_t>(value))
                                     : __builtin_ctz(static_cast<uint32_t>(value));
#else
      using U = std::make_unsigned_t<T>;
      U u = value;
      return CountPopulation(static_cast<U>(~u & (u - 1u)));
#endif
    }

    template <typename T, unsigned bits = sizeof(T) * 8>
    constexpr static std::enable_if_t<std::is_integral_v<T> && sizeof(T) <= 8, unsigned>
    countLeadingZeros(T value) {
      static_assert(bits > 0);
#if __has_builtin(__builtin_clzll) && __has_builtin(__builtin_clz)
      return value == 0 ? bits
                        : bits == 64 ? __builtin_clzll(static_cast<uint64_t>(value))
                                     : __builtin_clz(static_cast<uint32_t>(value)) - (32 - bits);
#else
      // Binary search algorithm (from "Hacker's Delight").
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
}

#endif  // LIB_UTIL_H_
