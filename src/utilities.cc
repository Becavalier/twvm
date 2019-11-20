// Copyright 2019 YHSPY. All rights reserved.
#include <stdexcept>
#include <algorithm>
#include <iomanip>
#include "src/utilities.h"

using std::runtime_error;
using std::max_element;
using std::left;
using std::setw;
using std::setfill;

// will throw exception if no "msg" provided;
std::ostream& Utilities::reportError(const string &msg, bool throwException) {
  REPORT(COLOR_CTL_ERROR, "error: ", msg);
  if (throwException) {
    throw runtime_error(msg);
  }
}

std::ostream& Utilities::reportDebug(const string &msg) {
  REPORT(COLOR_CTL_DEBUG, "info: ", msg);
}

std::ostream& Utilities::reportWarning(const string &msg) {
  REPORT(COLOR_CTL_WARNING, "warning: ", msg);
}

void Printer::printTableView() {
  if (lines.size() > 0) {
    const auto maxLength = (*max_element(begin(lines), end(lines))).length();
    const auto maxLengthPadding = maxLength + 4;
    Utilities::reportDebug() 
      << setw(maxLengthPadding)
      << setfill('-') << left << '|' << endl;
    for (const auto &line : lines) {
      Utilities::reportDebug() 
        << "| " 
        << left << setw(maxLength) << setfill(' ') << line 
        << " |" << endl;
    }
    Utilities::reportDebug() 
      << setw(maxLengthPadding)
      << setfill('-') << left << '|' << endl;
    lines.clear();
  }
}
