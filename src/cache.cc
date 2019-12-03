// Copyright 2019 YHSPY. All rights reserved.
#include "src/cache.h"

unordered_map<type_index, size_t> Cache::typeIndices = {
  {type_index(typeid(uint8_t)), 0}, 
  {type_index(typeid(uint32_t)), 1}, 
  {type_index(typeid(uint64_t)), 2}, 
  {type_index(typeid(float)), 3}, 
  {type_index(typeid(double)), 4}
};
