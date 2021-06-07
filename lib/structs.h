// Copyright 2021 YHSPY. All rights reserved.
#ifndef LIB_STRUCT_H_
#define LIB_STRUCT_H_

#include <memory>
#include <map>
#include <string>
#include <vector>
#include <optional>
#include <type_traits>
#include <variant>
#include <algorithm>

#define SET_STRUCT_DISABLE_COPY_CONSTUCT(TypeName) \
  TypeName(const TypeName&) = delete; \
  TypeName& operator=(const TypeName&) = delete
#define SET_STRUCT_MOVE_ONLY(TypeName) \
  TypeName() = default; \
  TypeName(TypeName&&) noexcept = default; \
  TypeName& operator=(TypeName&&) noexcept = default; \
  SET_STRUCT_DISABLE_COPY_CONSTUCT(TypeName);

namespace TWVM {
  /* Basic Types */
  enum class LangTypes : uint8_t {
    Void = 0x40,
    FuncType = 0x60,
  };
  enum class RefTypes : uint8_t {
    AnyFnunRefType = 0x70,
  };
  enum class ValueTypes : uint8_t {
    I32 = 0x7f,
    I64 = 0x7e, 
    F32 = 0x7d,
    F64 = 0x7c,
  };
  enum class LimitFlags : uint8_t {
    MAX_NO_EXIST = 0,
    MAX_EXIST,
  };

  class Module {
   public:
    struct TableType {
      uint8_t refType;
      uint32_t initial;
      uint32_t maximum;
    };
    struct MemType {
      uint32_t initial;
      uint32_t maximum;
    };
    struct GlobalType {
      uint8_t valType;
      bool mutability;
    };
    using ext_meta_t = std::variant<uint32_t, TableType, MemType, GlobalType>;
   private:
    struct TableSeg {
      SET_STRUCT_MOVE_ONLY(TableSeg)
      TableType tableType;
      TableSeg(uint8_t refType, uint32_t initial, uint32_t maximum = 0) 
        : tableType({ refType, initial, maximum }) {}
    };
    struct MemSeg {
      SET_STRUCT_MOVE_ONLY(MemSeg)
      MemType memType;
      MemSeg(uint32_t initial, uint32_t maximum = 0) 
        : memType({ initial, maximum }) {}
    };
    struct GlobalSeg {
      SET_STRUCT_MOVE_ONLY(GlobalSeg)
      GlobalType globalType;
      std::vector<uint8_t> initOpCodes;  // Used in instantiation stage.
      GlobalSeg(uint8_t valType, bool mutability, std::vector<uint8_t>&& initOpCodes) 
        : globalType({ valType, mutability }), initOpCodes(initOpCodes) {}
    };
    struct ExportSeg {
      SET_STRUCT_MOVE_ONLY(ExportSeg)
      std::string name;
      uint8_t extKind;
      uint32_t extIdx;
      ExportSeg(std::string name, uint8_t extKind, uint32_t extIdx) 
        : name(name), extKind(extKind), extIdx(extIdx) {}
    };
    struct FuncDefSeg {
      SET_STRUCT_MOVE_ONLY(FuncDefSeg)
      std::vector<uint8_t> locals;
      std::vector<uint8_t> body;
      FuncDefSeg(std::vector<uint8_t> &&locals, std::vector<uint8_t> &&body)
        : locals(locals), body(body) {}
    };
    struct ImportSeg {
      SET_STRUCT_MOVE_ONLY(ImportSeg)
      std::string modName;
      std::string name;
      uint8_t extKind;
      ext_meta_t extMeta;
      ImportSeg(std::string modName, std::string name, uint8_t extKind) 
        : modName(modName), name(name), extKind(extKind) {}
    };
    struct ElemSeg {
      SET_STRUCT_MOVE_ONLY(ElemSeg)
      uint32_t tblIdx;
      std::vector<uint8_t> initOpCodes;
      std::vector<uint32_t> funcIndices;
      ElemSeg(uint32_t tblIdx, std::vector<uint8_t>&& initOpCodes, std::vector<uint32_t>&& funcIndices) 
        : tblIdx(tblIdx), initOpCodes(initOpCodes), funcIndices(funcIndices) {}
    };
    struct DataSeg {
      SET_STRUCT_MOVE_ONLY(DataSeg)
      uint32_t memIdx;
      std::vector<uint8_t> initOpCodes;
      std::vector<uint8_t> dataBytes;
      DataSeg(uint32_t memIdx, std::vector<uint8_t>&& initOpCodes, std::vector<uint8_t>&& dataBytes) 
        : memIdx(memIdx), initOpCodes(initOpCodes), dataBytes(dataBytes) {}
    };
   public:
    bool hasValidHeader = false;
    size_t lastParsedSectionId = 0;
    uint32_t version = 1;
    std::optional<uint32_t> startFuncIdx;
    std::vector<std::pair<std::vector<uint8_t>, std::vector<uint8_t>>> funcTypes;
    std::vector<uint32_t> funcTypesIndices;
    std::vector<TableSeg> tables;
    std::vector<MemSeg> mems;
    std::vector<GlobalSeg> globals;
    std::vector<ExportSeg> exports;
    std::vector<FuncDefSeg> funcDefs;
    std::vector<ImportSeg> imports;
    std::vector<ElemSeg> elements;
    std::vector<DataSeg> data;
  };
  using shared_module_t = std::shared_ptr<Module>;
  
  /* Runtime Types */
  using RTI32 = int32_t;
  using RTI64 = int64_t;
  using RTF32 = float;
  using RTF64 = double;
  using runtime_value_t = std::variant<RTI32, RTI64, RTF32, RTF64>;
  class Instance {
    // class ValueFrame {
    //   SET_STRUCT_MOVE_ONLY(ValueFrame)
    //   runtime_value_t v;
    // };
    class LabelFrame {
      SET_STRUCT_MOVE_ONLY(LabelFrame)
      
    };
    class CallFrame {
      SET_STRUCT_MOVE_ONLY(CallFrame)
    };
    struct RTMemDescriptor {
      SET_STRUCT_MOVE_ONLY(RTMemDescriptor)
      size_t size;
      uint8_t* ptr;
      RTMemDescriptor(size_t size, uint8_t* ptr) : size(size), ptr(ptr) {}
    };
   public:
    shared_module_t module;
    std::vector<RTMemDescriptor> rtMems;
    std::vector<std::vector<std::optional<uint32_t>>> rtTables;  // Func idx inside.
    std::vector<runtime_value_t> rtGlobals;
    std::vector<runtime_value_t> rtValueStack;
    std::vector<LabelFrame> rtLabelStack;
    std::vector<CallFrame> rtCallStack;
    std::optional<uint32_t> rtEntryIdx;
    Instance(shared_module_t module) : module(module) {};
    ~Instance() {
      // Free allocated mem.
      std::for_each(rtMems.begin(), rtMems.end(), [](RTMemDescriptor& mem) {
        std::free(mem.ptr);
      });
    }
  };
  using shared_module_instance_t = std::shared_ptr<Instance>;
}

#endif  // LIB_STRUCT_H_
