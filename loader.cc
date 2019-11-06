#include <iostream>
#include <fstream>
#include "loader.h"
#include "types.h"
#include "decoder.h"

vector<uchar_t> Loader::buf;

using std::cout;
using std::endl;
using std::cerr;
using std::ifstream;
using std::ios;

void Loader::cli_load_file(const std::string &file_name) {
  ifstream in(file_name, ios::binary);
  char d, counter = 0;

  if(in.is_open()) {
    while(in.good()) {
      in.read(&d, sizeof(d));
      Loader::buf.push_back(d);
      if (++counter == 8 && !validate_magic(buf)) {
        cerr << "[TWVM] invalid wasm magic word, expect 0x6d736100."<< endl;
        return;
      }
    }
  }

  if(!in.eof() && in.fail()) {
    cerr << "[TWVM] error reading " << file_name << endl;
  }
  in.close();
}

bool Loader::validate_magic(vector<unsigned char> &buf) {
  return Decoder::read_u32(buf.data()) == k_wasm_magic;
}
