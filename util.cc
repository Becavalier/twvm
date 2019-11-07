#include <iostream>
#include "util.h"

using std::cerr;
using std::endl;

void Util::reportError(const string &msg) {
  cerr 
    << OUTPUT_PREFIX 
    << COLOR_CTL_ERROR 
    << "error: " 
    << COLOR_CTL_NORMAL 
    << msg 
    << endl;
}

void Util::reportDebug(const string &msg) {
  cerr 
    << OUTPUT_PREFIX 
    << COLOR_CTL_DEBUG 
    << "debug: " 
    << COLOR_CTL_NORMAL 
    << msg 
    << endl;
}
