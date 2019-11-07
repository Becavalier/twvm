// Copyright 2019 YHSPY. All rights reserved.
#include <fstream>
#include <algorithm>
#include <iterator>
#include "./loader.h"
#include "./constants.h"
#include "./decoder.h"
#include "./util.h"

vector<uchar_t> Loader::buf;

using std::ifstream;
using std::ios;
using std::make_shared;
using std::copy;

shared_ptr<Module> Loader::init(const std::string &fileName) {
  ifstream in(fileName, ios::binary);
  char d, counter = 1;
  shared_ptr<Module> wasmModule(new Module());

  if (in.is_open()) {
    while (in.good()) {
      in.read(&d, sizeof(d));
      buf.push_back(d);
      // checking magic word / version number;
      if (counter == BYTE_LENGTH_8) {
        if (!validateWords(buf)) {
          return wasmModule;
        }
      }
      counter++;
    }
  }

  if (!in.eof() && in.fail()) {
    Util::reportError("can not reading file.");
  }
  in.close();

  // wrapping and returning a module instance;
  wasmModule->setModContent(buf);
  return wasmModule;
}

shared_ptr<Module> Loader::init(const uchar_t *source, size_t len) {
  shared_ptr<Module> wasmModule;
  // one-time copying;
  buf = vector<uchar_t>(source, source + len);

  if (validateWords(buf)) {
    wasmModule->setModContent(buf);
  }
  return wasmModule;
}

bool Loader::validateWords(const vector<uchar_t> &buf) {
  if (!validateMagicWord(buf)) {
    Util::reportError("invalid wasm magic word, expect 0x6d736100.");
    return false;
  }
  if (!validateVersionWord(buf)) {
    Util::reportError("invalid wasm version, expect 0x01.");
    return false;
  }
  return true;
}

bool Loader::validateMagicWord(const vector<unsigned char> &buf) {
  return Decoder::readU32(buf.data()) == kWasmMagicWord;
}

bool Loader::validateVersionWord(const vector<unsigned char> &buf) {
  // set up offset;
  auto sp = buf.data() + BYTE_LENGTH_4;
  return Decoder::readU32(sp) == kWasmVersion;
}
