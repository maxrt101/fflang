#include "core/object.h"
#include "core/value.h"
#include "core/memory.h"
#include "core/vm.h"

#include <iostream>

extern VM* current;


std::string Obj::ToString() const {
  switch (type) {
    case OBJ_STRING:
      return ((ObjString*)this)->str;
    case OBJ_NATIVE:
      return "<native fn>";
    case OBJ_FUNCTION: {
      ObjFunction* func = (ObjFunction*)this;
      if (func->name) return "<fn " + func->name->str + ">";
      return "<script>";
    }
    default:
      return "<object>";
  }
}


Value Obj::AsValue() {
  Value val;
  val.type = VAL_OBJ;
  val.as.obj = this;
  return val;
}


ObjString* ObjString::New() {
  ObjString* obj = memory::Reallocate<ObjString>(nullptr, 1);
  new (obj) ObjString();
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


ObjFunction::ObjFunction() {
  type = OBJ_FUNCTION;
}


ObjFunction* ObjFunction::New() {
  ObjFunction* obj = memory::Reallocate<ObjFunction>(nullptr, 1);
  new (obj) ObjFunction();
  return obj;
}

ObjNative* ObjNative::New(NativeFn func) {
  ObjNative* obj = memory::Reallocate<ObjNative>(nullptr, 1);
  new (obj) ObjNative();
  obj->type = OBJ_NATIVE;
  obj->function = func;
  return obj;
}

