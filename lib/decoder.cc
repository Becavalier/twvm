#include <vector>
#include "lib/decoder.h"

namespace TWVM {
  std::vector<uint8_t> Decoder::retrievePackedLEB128Bytes(std::ifstream& in) {
    std::vector<uint8_t> v = {};
      while (true) {
        uint8_t byte = static_cast<uint8_t>(in.get());
        v.emplace_back(byte);
        if (!(byte & 0x80)) {
          break;
        }
      }
      return v;
  }
}
