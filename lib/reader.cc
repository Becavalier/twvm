// Copyright 2021 YHSPY. All rights reserved.
#include "lib/include/reader.hh"

namespace TWVM {

std::vector<uint8_t> Reader::retrieveBytes(size_t n) {
  char arr[n];
  std::vector<uint8_t> v = {};
  in.read(arr, n);
  for (auto i = 0; i < sizeof(arr) / sizeof(char); ++i) {
    v.emplace_back(static_cast<uint8_t>(arr[i]));
  }
  return v;
}
std::vector<uint8_t> Reader::getBytesTillDelim(uint8_t delim) {
  std::vector<uint8_t> v;
  while (true) {
    uint8_t b = walkByte();
    v.push_back(b);
    if (b == delim) break;
  }
  return v;
}

}  // namespace TWVM
