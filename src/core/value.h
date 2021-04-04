#ifndef FF_CORE_VALUE_H_
#define FF_CORE_VALUE_H_

#include <string>
#include <vector>
#include <cstdint>

#include "core/object.h"


typedef double NumberType;

enum ValueType {
  VAL_NULL,
  VAL_BOOL,
  VAL_NUMBER,
  VAL_OBJ,
};


struct Value {
 public:
  ValueType type;
  bool assignable = true;
  union {
    bool boolean;
    NumberType number;
    Obj* obj;
  } as;

 public:
  inline Value(ValueType type = VAL_NULL) : type(type) {
    as.number = 0;
  }

  inline Value(bool val) {
    type = VAL_BOOL;
    as.boolean = val;
  }

  inline Value(NumberType val) {
    type = VAL_NUMBER;
    as.number = val;
  }

  inline Value(Obj* obj) {
    type = VAL_OBJ;
    as.obj = obj;
  }

 public:
  inline bool AsBool() const { return as.boolean; }
  inline NumberType AsNumber() const { return as.number; }
  inline struct Obj* AsObj() const { return as.obj; }
  inline ObjString* AsString() const { return (ObjString*)as.obj; }

  inline bool IsType(ValueType expected_type) const { return type == expected_type; }
  bool IsString() const;
  bool IsFalse() const;

  bool operator==(const Value& rhs);
  
  std::string ToString() const;
  void Print() const;
};


typedef std::vector<Value> ValueArray;

#endif

