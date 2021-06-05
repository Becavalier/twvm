#include "lib/reader.h"

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
}
