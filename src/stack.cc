// Copyright 2019 YHSPY. All rights reserved.
#include "src/stack.h"

const bool Stack::checkStackState(bool startEntry) const {
  // check the status of stack;
  const auto leftValueSize = valueStack->size();
  (Printer::instance() << '(' << (startEntry ? "start" : "main") << "): ").say();
  if (leftValueSize == 1) {
    valueStack->top()->outputValue(cout << dec);
    valueStack->pop();
  } else {
    cout << "(void)";
  }
  cout << endl;
  return leftValueSize <= 1;
}
