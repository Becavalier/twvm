// Copyright 2021 YHSPY. All rights reserved.
#ifndef LIB_INCLUDE_STRUCTS_HH_
#define LIB_INCLUDE_STRUCTS_HH_

#include <memory>
#include <map>
#include <string>
#include <vector>
#include <optional>
#include <type_traits>
#include <variant>
#include <algorithm>
#include <utility>

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
  using type_seq_t = std::vector<uint8_t>;
  using external_kind_t = std::variant<uint32_t, TableType, MemType, GlobalType>;  // the first one is func idx.
  using func_type_t = std::pair<type_seq_t, type_seq_t>;
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
    GlobalSeg(uint8_t valType, bool mutability, std::vector<uint8_t>& initOpCodes)
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
    FuncDefSeg(std::vector<uint8_t>& locals, std::vector<uint8_t>& body)
      : locals(locals), body(body) {}
  };
  struct ImportSeg {
    SET_STRUCT_MOVE_ONLY(ImportSeg)
    std::string modName;
    std::string name;
    uint8_t extKind;
    external_kind_t extMeta;
    ImportSeg(std::string modName, std::string name, uint8_t extKind)
      : modName(modName), name(name), extKind(extKind) {}
  };
  struct ElemSeg {
    SET_STRUCT_MOVE_ONLY(ElemSeg)
    uint32_t tblIdx;
    std::vector<uint8_t> initOpCodes;
    std::vector<uint32_t> funcIndices;
    ElemSeg(uint32_t tblIdx, std::vector<uint8_t>& initOpCodes, std::vector<uint32_t>& funcIndices)
      : tblIdx(tblIdx), initOpCodes(initOpCodes), funcIndices(funcIndices) {}
  };
  struct DataSeg {
    SET_STRUCT_MOVE_ONLY(DataSeg)
    uint32_t memIdx;
    std::vector<uint8_t> initOpCodes;
    std::vector<uint8_t> dataBytes;
    DataSeg(uint32_t memIdx, std::vector<uint8_t>& initOpCodes, std::vector<uint8_t>& dataBytes)
      : memIdx(memIdx), initOpCodes(initOpCodes), dataBytes(dataBytes) {}
  };
 public:
  bool hasValidHeader = false;
  size_t lastParsedSectionId = 0;
  uint32_t version = 1;
  std::optional<uint32_t> startFuncIdx;
  std::vector<func_type_t> funcTypes;
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
struct Runtime {
  enum class STVariantIndex : int8_t {
    // The order of types here should be consistent with the below stack definition, -
    // and this will be used to test the type of the stack frame.
    VALUE = 0,
    LABEL,
    ACTIVATION,
  };
  using rt_i32_t = int32_t;
  using rt_i64_t = int64_t;
  using rt_f32_t = float;
  using rt_f64_t = double;
  using imme_u32_t = uint32_t;
  using imme_u64_t = uint64_t;
  using relative_depth_t = uint32_t;
  using index_t = uint32_t;
  using runtime_value_t = std::variant<rt_i32_t, rt_i64_t, rt_f32_t, rt_f64_t>;

  struct RTFuncDescriptor {
    SET_STRUCT_MOVE_ONLY(RTFuncDescriptor)
    const Module::func_type_t* funcType;
    uint8_t* codeEntry;
    std::vector<runtime_value_t> localsDefault;

    // JIT compilation support
    uint32_t executionCount = 0;
    void* jitCompiledPtr = nullptr;  // Native function pointer after JIT compilation
    bool isJitCompiled = false;

    RTFuncDescriptor(const Module::func_type_t* funcType, uint8_t* codeEntry)
      : funcType(funcType), codeEntry(codeEntry) {}
  };
  struct RTValueFrame {
    SET_STRUCT_MOVE_ONLY(RTValueFrame)
    runtime_value_t value;
    RTValueFrame(const runtime_value_t& value)
      : value(value) {}
  };
  struct RTLabelFrame {
    SET_STRUCT_MOVE_ONLY(RTLabelFrame)
    uint8_t* cont;
    Module::type_seq_t returnArity;
    RTLabelFrame(uint8_t* cont, const Module::type_seq_t& returnArity = {})
      : cont(cont), returnArity(returnArity) {}
  };
  struct RTActivFrame {
    SET_STRUCT_MOVE_ONLY(RTActivFrame)
    std::vector<runtime_value_t> locals;
    uint8_t* cont;
    const Module::type_seq_t* returnArity;
    RTActivFrame(std::vector<runtime_value_t>& locals, uint8_t* cont, const Module::type_seq_t* returnArity)
      : locals(locals), cont(cont), returnArity(returnArity) {}
  };
  struct RTMemHolder {
    SET_STRUCT_MOVE_ONLY(RTMemHolder)
    size_t size;  // Pages.
    uint8_t* ptr;
    uint32_t maximumPages;
    RTMemHolder(size_t size, uint8_t* ptr, uint32_t maximumPages)
      : size(size), ptr(ptr), maximumPages(maximumPages) {}
  };
  shared_module_t module;
  std::vector<RTMemHolder> rtMems;
  std::vector<std::vector<std::optional<uint32_t>>> rtTables;  // Func idx inside.
  std::vector<runtime_value_t> rtGlobals;
  using stack_frame_t = std::variant<RTValueFrame, RTLabelFrame, RTActivFrame>;
  std::vector<stack_frame_t> stack;
  std::optional<uint32_t> rtEntryIdx;
  std::vector<RTFuncDescriptor> rtFuncDescriptor;
  bool jitEnabled = false;  // JIT compilation flag
  explicit Runtime(shared_module_t module) : module(module) {}
  ~Runtime() {
    // Free allocated mem.
    std::for_each(rtMems.begin(), rtMems.end(), [](RTMemHolder& mem) {
      std::free(mem.ptr);
    });
  }
};
using shared_module_runtime_t = std::shared_ptr<Runtime>;

}  // namespace TWVM

#endif  // LIB_INCLUDE_STRUCTS_HH_
