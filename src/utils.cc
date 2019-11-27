// Copyright 2019 YHSPY. All rights reserved.
#include <algorithm>
#include <iomanip>
#include <numeric>
#include "src/utils.h"
#include "src/macros.h"

using std::max_element;
using std::left;
using std::setw;
using std::setfill;
using std::transform;
using std::accumulate;
using std::ostream;

vector<string> Utils::splitStr(const string &str, char delimiter) {
  vector<string> tokens;
  string token;
  istringstream tokenStream(str);
  while (getline(tokenStream, token, delimiter)) {
    tokens.push_back(token);
  }
  return tokens;
}

void Printer::printTableView() {
  if (lines.size() > 0) {
    // calculate column width;
    vector<size_t> columnWidth;
    vector<vector<string>> columnContent;
    for (const auto &line : lines) {
      const auto snippets = Utils::splitStr(line, '|');
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
    // calculate line width;
    const auto columnWidthSize = columnWidth.size();
    const auto maxLengthPadding =
      accumulate(begin(columnWidth), end(columnWidth), 0) + columnWidthSize + 3;
    INTERNAL_DEBUG_PREFIX_OUTPUT() << setw(maxLengthPadding) << setfill('-') << left << '|' << endl;
    for (auto i = 0; i < lines.size(); i++) {
      INTERNAL_DEBUG_PREFIX_OUTPUT() << "| ";
      for (auto j = 0; j < columnWidthSize; j++) {
        cout << setw(columnWidth[j]) << setfill(' ') << columnContent[i][j];
        if (j != columnWidthSize - 1) {
          cout << '|';
        }
      }
      cout << " |" << endl;
    }
    INTERNAL_DEBUG_PREFIX_OUTPUT() << setw(maxLengthPadding) << setfill('-') << left << '|' << endl;
    lines.clear();
  }
}
