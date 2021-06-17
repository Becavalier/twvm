// Copyright 2021 YHSPY. All rights reserved.
#ifndef LIB_LOADER_H_
#define LIB_LOADER_H_

#include <vector>
#include <functional>
#include <fstream>
#include "lib/include/structs.hh"
#include "lib/include/reader.hh"
#include "lib/include/constants.hh"

namespace TWVM {
  struct Loader {
    using handler_t = void(*)(Reader&, shared_module_t);
    static std::vector<handler_t> handlers;
    static void parseCustomSection(Reader&, shared_module_t);
    static void parseTypeSection(Reader&, shared_module_t);
    static void parseImportSection(Reader&, shared_module_t);
    static void parseFunctionSection(Reader&, shared_module_t);
    static void parseTableSection(Reader&, shared_module_t);
    static void parseMemorySection(Reader&, shared_module_t);
    static void parseStartSection(Reader&, shared_module_t);
    static void parseGlobalSection(Reader&, shared_module_t);
    static void parseExportSection(Reader&, shared_module_t);
    static void parseCodeSection(Reader&, shared_module_t);
    static void parseElementSection(Reader&, shared_module_t);
    static void parseDataSection(Reader&, shared_module_t);
    static void preamble(std::ifstream&, shared_module_t);
    static void parse(std::ifstream&, shared_module_t);
    static void walkExtMeta(Reader&, Module::external_kind_t&, uint8_t);
    static shared_module_t load(const std::string&);
  };
}

#endif  // LIB_LOADER_H_
