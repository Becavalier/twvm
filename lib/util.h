// Copyright 2021 YHSPY. All rights reserved.
#ifndef LIB_UTIL_H_
#define LIB_UTIL_H_

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
  };
}

#endif  // LIB_UTIL_H_
