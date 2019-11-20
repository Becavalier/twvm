// Copyright 2019 YHSPY. All rights reserved.
#include <stdexcept>
#include <algorithm>
#include <iomanip>
#include <numeric>
#include "src/utilities.h"

using std::runtime_error;
using std::max_element;
using std::left;
using std::setw;
using std::setfill;
using std::transform;
using std::accumulate;

vector<string> Utilities::splitStr(const string &str, char delimiter) {
  vector<string> tokens;
  string token;
  istringstream tokenStream(str);
  while (getline(tokenStream, token, delimiter)) {
    tokens.push_back(token);
  }
  return tokens;
}

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
    // calc column width;
    vector<size_t> columnWidth;
    vector<vector<string>> columnContent;
    for (const auto &line : lines) {
      const auto snippets = Utilities::splitStr(line, '|');
      columnContent.push_back(snippets);
      if (columnWidth.size() == 0) {
        columnWidth = vector<size_t>(snippets.size(), 0);
        transform(begin(snippets), end(snippets), begin(columnWidth),
          [](const string &snippet) -> auto {
            return snippet.length();
          });
      } else {
        for (auto i = 0; i < snippets.size(); i++) {
          const auto len = snippets[i].length();
          columnWidth[i] = columnWidth[i] < len ? len : columnWidth[i];
        }
      }
    }
    // calc line width;
    const auto columnWidthSize = columnWidth.size();
    const auto maxLengthPadding =
      accumulate(begin(columnWidth), end(columnWidth), 0) + columnWidthSize + 3;
    Utilities::reportDebug() << setw(maxLengthPadding) << setfill('-') << left << '|' << endl;
    for (auto i = 0; i < lines.size(); i++) {
      Utilities::reportDebug() << "| ";
        for (auto j = 0; j < columnWidthSize; j++) {
          cout << setw(columnWidth[j]) << setfill(' ') << columnContent[i][j];
          if (j != columnWidthSize - 1) {
            cout << '|';
          }
        }
        cout << " |" << endl;
    }
    Utilities::reportDebug() << setw(maxLengthPadding) << setfill('-') << left << '|' << endl;
    lines.clear();
  }
}
