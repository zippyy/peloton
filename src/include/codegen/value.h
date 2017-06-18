//===----------------------------------------------------------------------===//
//
//                         Peloton
//
// value.h
//
// Identification: src/include/codegen/value.h
//
// Copyright (c) 2015-17, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#pragma once

#include "codegen/codegen.h"
#include "codegen/type/type.h"

namespace peloton {
namespace codegen {

//===----------------------------------------------------------------------===//
// Mapping of SQL value types to LLVM types. This class helps us generate code
// for common operations on SQL types like comparisons, arithmetic operations
// and boolean operations.
//===----------------------------------------------------------------------===//
class Value {
 public:
  // Constructor that provides the type and the value
  Value();
  Value(const type::Type &type, llvm::Value *value = nullptr,
        llvm::Value *length = nullptr, llvm::Value *is_null = nullptr);

  //===--------------------------------------------------------------------===//
  // Accessors
  //===--------------------------------------------------------------------===//

  // Get the SQL type
  const type::Type &GetType() const { return type_; }

  // Get the type system for this value's type
  const type::TypeSystem &GetTypeSystem() const {
    return GetType().GetTypeSystem();
  }

  // Get the LLVM value
  llvm::Value *GetValue() const { return value_; }

  // Get the length of the varchar (if it is one)
  llvm::Value *GetLength() const { return length_; }

  // Is this value nullable?
  bool IsNullable() const { return GetType().nullable; }

  // Check if this value is NULL (or not NULL).
  llvm::Value *IsNull(CodeGen &codegen) const;
  llvm::Value *IsNotNull(CodeGen &codegen) const;

  //===--------------------------------------------------------------------===//
  // Comparison functions
  //===--------------------------------------------------------------------===//
  Value CastTo(CodeGen &codegen, const type::Type &to_type) const;

  Value CompareEq(CodeGen &codegen, const Value &other) const;
  Value CompareNe(CodeGen &codegen, const Value &other) const;
  Value CompareLt(CodeGen &codegen, const Value &other) const;
  Value CompareLte(CodeGen &codegen, const Value &other) const;
  Value CompareGt(CodeGen &codegen, const Value &other) const;
  Value CompareGte(CodeGen &codegen, const Value &other) const;

  static Value TestEquality(CodeGen &codegen, const std::vector<Value> &lhs,
                            const std::vector<Value> &rhs);

  // Perform a comparison used for sorting. We need a stable and transitive
  // sorting comparison function here. The function returns:
  //  < 0 - if the left value comes before the right value when sorted
  //  = 0 - if the left value is equivalent to the right element
  //  > 0 - if the left value comes after the right value when sorted
  Value CompareForSort(CodeGen &codegen, const Value &other) const;

  //===--------------------------------------------------------------------===//
  // Mathematical functions
  //===--------------------------------------------------------------------===//

  enum class OnError : uint32_t { ReturnNull, Exception };

  Value Add(CodeGen &codegen, const Value &other,
            const OnError on_error = OnError::Exception) const;
  Value Sub(CodeGen &codegen, const Value &other,
            const OnError on_error = OnError::Exception) const;
  Value Mul(CodeGen &codegen, const Value &other,
            const OnError on_error = OnError::Exception) const;
  Value Div(CodeGen &codegen, const Value &other,
            const OnError on_error = OnError::Exception) const;
  Value Mod(CodeGen &codegen, const Value &other,
            const OnError on_error = OnError::Exception) const;
  Value Min(CodeGen &codegen, const Value &other) const;
  Value Max(CodeGen &codegen, const Value &other) const;

  //===--------------------------------------------------------------------===//
  // Logical/Boolean functions
  //===--------------------------------------------------------------------===//

  Value LogicalAnd(CodeGen &codegen, const Value &other) const;
  Value LogicalOr(CodeGen &codegen, const Value &other) const;

  // Build a PHI node that combines all the given values (from their basic
  // blocks) into a single value
  static Value BuildPHI(
      CodeGen &codegen,
      const std::vector<std::pair<Value, llvm::BasicBlock *>> &vals);

  /*
  Value CallUnary(CodeGen &codegen,
                  type::TypeSystem::OperatorId operator_id) const;
  Value CallBinary(CodeGen &codegen, type::TypeSystem::OperatorId operator_id,
                   const Value &other) const;
  */

  //===--------------------------------------------------------------------===//
  // Materialization helpers
  //===--------------------------------------------------------------------===//

  // Return the a representation of this value suitable for materialization
  void ValuesForMaterialization(CodeGen &codegen, llvm::Value *&val,
                                llvm::Value *&len, llvm::Value *&null) const;

  // Return a value that can be constructed from the provided type and value
  // registers
  static Value ValueFromMaterialization(const type::Type &type,
                                        llvm::Value *val, llvm::Value *len,
                                        llvm::Value *null);

 private:
  friend class Hash;
  friend class CompactStorage;
  friend class UpdateableStorage;

  // Generate a hash for the given value
  void ValuesForHash(llvm::Value *&val, llvm::Value *&len) const;

 private:
  // The SQL type
  type::Type type_;

  // The value
  llvm::Value *value_;

  // The length of the value (if it's variable in length)
  llvm::Value *length_;

  // NULL indicator (if any)
  llvm::Value *null_;
};

}  // namespace codegen
}  // namespace peloton