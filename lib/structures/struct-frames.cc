// Copyright 2019 YHSPY. All rights reserved.
#include <ostream>
#include "lib/structures/struct-frames.h"

using std::ostream;

void ValueFrame::outputValue(ostream &out) const {
  switch (runtimeType) {
    case ValueFrameTypes::kRTF32Value: { out << toF32(); break; }
    case ValueFrameTypes::kRTF64Value: { out << toF64(); break; }
    case ValueFrameTypes::kRTI32Value: { out << toI32(); break; }
    case ValueFrameTypes::kRTU32Value: { out << toU32(); break; }
    case ValueFrameTypes::kRTI64Value: { out << toI64(); break; }
    case ValueFrameTypes::kRTU64Value: { out << toU64(); break; }
    default: break;
  }
}
