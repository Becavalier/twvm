#include <iostream>
#include "lib/include/util.hh"

#define BUILDSTAMP (__DATE__ " " __TIME__)

namespace TWVM {
  void Util::printAssistantInfo(bool simplify) {
    std::cout << R"(
  //////    //    //
    // //  ////  ////
    //  ////  ////  //
    //   //    //    //
    )" << std::endl;
    if (!simplify) {
#if defined(BUILD_VERSION)
      std::cout << "  V " << BUILD_VERSION << " - ";
#endif
      std::cout 
        << BUILDSTAMP 
        << "\n\n";
    }
  }
}