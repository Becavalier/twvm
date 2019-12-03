// Copyright 2019 YHSPY. All rights reserved.
#ifndef CACHE_H_
#define CACHE_H_

#include <type_traits>
#include <unordered_map>
#include <typeinfo>
#include <typeindex>
#include <string>
#include <vector>
#include "src/utils.h"

using std::unordered_map;
using std::is_same;
using std::string;
using std::type_index;
using std::vector;

#define DECLARE_CACHE_CONTAINER(name, type) \
  unordered_map<uint64_t, vector<type>> name##Vector = {};

#define DECLARE_CACHE_SET_METHODS(name, type) \
  void name##SetValueCache(uint32_t index, size_t offset, type v, size_t step) { \
    name##Vector[hashLoc(index, offset)] = {v, static_cast<type>(step)}; \
  }
#define DECLARE_CACHE_GET_METHODS(name, type) \
  const auto& name##GetValueCache(uint32_t index, size_t offset) { \
    return name##Vector[hashLoc(index, offset)]; \
  }

#define ITERATE_IMMEDIATES_VALUE_TYPES(V) \
  V(uint8, uint8_t) \
  V(uint32, uint32_t) \
  V(uint64, uint64_t) \
  V(int32, int32_t) \
  V(int64, int64_t) \
  V(float, float) \
  V(double, double)

// keep the runtime immediates in memory, and then can be re-used;
class Cache {
 private:
  // id + offset;
  ITERATE_IMMEDIATES_VALUE_TYPES(DECLARE_CACHE_CONTAINER);

  uint64_t hashLoc(uint32_t index, size_t offset) {
    // avaiable size: 131072;
    return index * (2 << 16) + offset;
  }

 public:
  ITERATE_IMMEDIATES_VALUE_TYPES(DECLARE_CACHE_SET_METHODS)
  ITERATE_IMMEDIATES_VALUE_TYPES(DECLARE_CACHE_GET_METHODS)
};

#endif  // CACHE_H_
