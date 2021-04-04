#include "core/value.h"
#include "core/object.h"
#include "core/memory.h"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdio>


bool Value::IsString() const {
  return type == VAL_OBJ && as.obj->type == OBJ_STRING;
}

bool Value::IsFalse() const {
  return IsType(VAL_NULL) || (IsType(VAL_BOOL) && !AsBool()) || (IsType(VAL_NUMBER) && AsNumber() == 0);
}


bool Value::operator==(const Value& rhs) {
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
  switch (type) {
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

