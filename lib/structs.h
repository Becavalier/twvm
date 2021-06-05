// Copyright 2021 YHSPY. All rights reserved.
#ifndef LIB_STRUCT_H_
#define LIB_STRUCT_H_

#include <memory>
#include <map>
#include <string>
#include <vector>
#include <optional>
#include <type_traits>

#define SET_STRUCT_DISABLE_COPY_CONSTUCT(TypeName) \
  TypeName(const TypeName&) = delete; \
  TypeName& operator=(const TypeName&) = delete
#define SET_STRUCT_MOVE_ONLY(TypeName) \
  TypeName() = default; \
  TypeName(TypeName&&) noexcept = default; \
  TypeName& operator=(TypeName&&) noexcept = default; \
  SET_STRUCT_DISABLE_COPY_CONSTUCT(TypeName);

namespace TWVM {
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
    struct TableMeta {
      SET_STRUCT_MOVE_ONLY(TableMeta)
      int8_t refType;
      uint32_t initial;
      uint32_t maximum;
      TableMeta(int8_t refType, uint32_t initial, uint32_t maximum = 0) 
        : refType(refType), initial(initial), maximum(maximum) {}
    };
    struct MemMeta {
      SET_STRUCT_MOVE_ONLY(MemMeta)
      uint32_t initial;
      uint32_t maximum;
      MemMeta(uint32_t initial, uint32_t maximum = 0) 
        : initial(initial), maximum(maximum) {}
    };
    struct GlobalMeta {
      SET_STRUCT_MOVE_ONLY(GlobalMeta)
      int8_t valType;
      bool mutability;
      std::vector<uint8_t> initOpCodes;  // used in instantiation stage.
    };
    struct ExportMeta {
      SET_STRUCT_MOVE_ONLY(ExportMeta)
      std::string name;
      uint8_t kind;
      uint32_t index;
      ExportMeta(std::string name, uint8_t kind, uint32_t index) 
        : name(name), kind(kind), index(index) {}
    };
    struct FuncDefMeta {
      SET_STRUCT_MOVE_ONLY(FuncDefMeta)
      std::vector<int8_t> locals;
      std::vector<uint8_t> body;
      FuncDefMeta(std::vector<int8_t> &&locals, std::vector<uint8_t> &&body)
        : locals(locals), body(body) {}
    };
   public:
    bool hasValidHeader = false;
    size_t lastParsedSectionId = 0;
    uint32_t version = 1;
    std::optional<uint32_t> startFuncIdx;
    std::vector<std::pair<std::vector<int8_t>, std::vector<int8_t>>> funcTypes;
    std::vector<std::pair<std::vector<int8_t>, std::vector<int8_t>>*> funcTypesIndices;
    std::vector<TableMeta> tables;
    std::vector<MemMeta> mems;
    std::vector<GlobalMeta> globals;
    std::vector<ExportMeta> exports;
    std::vector<FuncDefMeta> funcDefs;
  };

  class Instance {

  };

  using shared_module_t = std::shared_ptr<Module>;
  using shared_module_instance_t = std::shared_ptr<Instance>;
}

#endif  // LIB_STRUCT_H_
