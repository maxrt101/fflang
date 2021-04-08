#include "core/value.h"
#include "core/object.h"
#include "core/memory.h"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdio>


Value::Value(const Value& rhs) {
  // std::cout << "cc: " << &rhs << " -> " << this << "\n";
  type = rhs.type;
  assignable = rhs.assignable;
  switch (type) {
    case VAL_NULL: as.number = 0; break;
    case VAL_BOOL: as.boolean = rhs.as.boolean; break;
    case VAL_NUMBER: as.number = rhs.as.number; break;
    case VAL_OBJ: as.obj = rhs.as.obj; break;
  }
}


Value::Value(ValueType type) : type(type) {
  as.number = 0;
}


Value::Value(bool val) {
  type = VAL_BOOL;
  as.boolean = val;
}


Value::Value(NumberType val) {
  type = VAL_NUMBER;
  as.number = val;
}


Value::Value(Obj* obj) {
  type = VAL_OBJ;
  as.obj = obj;
}


bool Value::AsBool() const {
  return as.boolean;
}


NumberType Value::AsNumber() const {
  return as.number;
}


struct Obj* Value::AsObj() const {
  return as.obj;
}


ObjString* Value::AsString() const {
  return (ObjString*)as.obj;
}


bool Value::IsType(ValueType expected_type) const {
  return type == expected_type;
}


bool Value::IsString() const {
  return type == VAL_OBJ && as.obj->type == OBJ_STRING;
}


bool Value::IsFalse() const {
  return IsType(VAL_NULL) || (IsType(VAL_BOOL) && !AsBool()) || (IsType(VAL_NUMBER) && AsNumber() == 0);
}


bool Value::operator==(const Value& rhs) {
  // std::cout << "== " << this << " " << &rhs << "\n";
  if (type != rhs.type) return false;

  switch (type) {
    case VAL_NULL:   return true;
    case VAL_BOOL:   return AsBool() == rhs.AsBool();
    case VAL_NUMBER: return AsNumber() == rhs.AsNumber();
    case VAL_OBJ: {
      switch (AsObj()->type) {
        case OBJ_STRING: return AsString()->str == rhs.AsString()->str;
        default:
          return AsObj() == rhs.AsObj();
      }
    }
    default:
      return false;
  }
}

std::string Value::ToString() const {
  // std::cout << "Value::ToString " << this << "\n";
  switch (this->type) {
    case VAL_NULL:
      return "null";
    case VAL_BOOL:
      return AsBool() ? "true" : "false";
    case VAL_NUMBER: {
      std::ostringstream oss;
      oss << std::setprecision(8) << std::noshowpoint << AsNumber();
      return oss.str();
    }
    case VAL_OBJ: return AsObj()->ToString();
  }
  return "";
}

void Value::Print() const {
  std::cout << ToString();
}

