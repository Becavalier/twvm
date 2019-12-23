// Copyright 2019 YHSPY. All rights reserved.
#include "lib/stack.h"

const bool Stack::checkStackState(bool startEntry) const {
  // check the status of stack;
  const auto leftValueSize = valueStack->size();
  (Printer::instance() << '(' << (startEntry ? "start" : "main") << "): ").say();
  if (leftValueSize == 1) {
    valueStack->top()->outputValue(cout << dec);
    // keep the top value on stack, just use it as final result;
    // valueStack->pop();
  } else {
    cout << "(void)";
  }
  cout << endl;
  return leftValueSize <= 1;
}
