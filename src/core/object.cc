#include "core/object.h"
#include "core/value.h"
#include "core/memory.h"
#include "core/vm.h"

#include <iostream>

extern VM* current;


ObjString* ObjString::New() {
  ObjString* obj = memory::Reallocate<ObjString>(nullptr, 1);
  obj->type = OBJ_STRING;
  return obj;
}


ObjString* ObjString::FromStr(std::string& str) {
  if (current) {
    auto interned = current->strings_.find(str);
    if (interned != current->strings_.end()) {
      return current->strings_[str];
    }
  }
  ObjString* obj = ObjString::New();
  obj->str = std::string(str);
  if (current) {
    current->strings_[str] = obj;
  }
  return obj;
}


Value Obj::AsValue() {
  return Value(this);
}


std::string Obj::ToString() const {
  switch (type) {
    case OBJ_STRING:
      return ((ObjString*)this)->str;
    default:
      return "<object>";
  }
}

