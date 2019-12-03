// Copyright 2019 YHSPY. All rights reserved.
#ifndef CACHE_H_
#define CACHE_H_

#include <type_traits>
#include <unordered_map>
#include <typeinfo>
#include <typeindex>
#include <string>
#include "src/utils.h"

using std::unordered_map;
using std::is_same;
using std::string;
using std::type_index;

#define DECLARE_CACHE_CONTAINER(name, type) \
  unordered_map<uint32_t, unordered_map<size_t, type>> name##Vector = {};

#define ITERATE_IMMEDIATES_VALUE_TYPES(V) \
  V(uint8, uint8_t) \
  V(uint32, uint32_t) \
  V(uint64, uint64_t) \
  V(float, float) \
  V(double, double)

// keep the runtime opcode&immediates in memory, and then can be reused;
class Cache {
 private:
  // id + offset;
  ITERATE_IMMEDIATES_VALUE_TYPES(DECLARE_CACHE_CONTAINER);
  static unordered_map<type_index, size_t> typeIndices;

 public:
  template<typename T>
  void setValueCache(uint32_t index, size_t offset, T v) {
    switch (typeIndices[type_index(typeid(T))]) {
      case 0: { uint8Vector[index][offset] = v; break; }
      case 1: { uint32Vector[index][offset] = v; break; }
      case 2: { uint64Vector[index][offset] = v; break; }
      case 3: { floatVector[index][offset] = v; break; }
      case 4: { doubleVector[index][offset] = v; break; }
      default: {
        (Printer::instance() << "invalid type of the caching value.").error();
      }
    }
  }
  
  template<typename T>
  T getValueCache(uint32_t index, size_t offset) {
    switch (typeIndices[type_index(typeid(T))]) {
      case 0: { return uint8Vector[index][offset]; }
      case 1: { return uint32Vector[index][offset]; }
      case 2: { return uint64Vector[index][offset]; }
      case 3: { return floatVector[index][offset]; }
      case 4: { return doubleVector[index][offset]; }
      default: {
        (Printer::instance() << "invalid type of the caching value.").error();
      }
    }
  }
};

#endif  // CACHE_H_
