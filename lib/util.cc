#include <iostream>
#include "lib/util.h"

#define BUILDSTAMP (__DATE__ " " __TIME__)

namespace TWVM {
  void Util::printAssistantInfo(bool simplify) {
    std::cout << R"(
    //////    //    //
      // //  ////  ////
      //  ////  ////  //
      //   //    //    //
    )" << std::endl;
  #if defined(BUILD_VERSION)
    std::cout 
      << "  [Version] " 
      << BUILD_VERSION << "\n\n";
  #endif
    if (!simplify) {
      std::cout 
        << "  [Last Build] " 
        << BUILDSTAMP 
        << "\n\n";
    }
  }
}