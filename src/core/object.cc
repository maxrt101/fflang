#include "core/object.h"
#include "core/value.h"
#include "core/memory.h"
#include "core/api.h"

#include <iostream>

extern VMContext* current;


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
  ObjString* obj = memory::Allocate<ObjString>(1);
  new (obj) ObjString();
  obj->type = OBJ_STRING;
  return obj;
}

ObjString* ObjString::FromStr(const std::string& str) {
  if (current) {
    auto interned = current->GetStrings().find(str);
    if (interned != current->GetStrings().end()) {
      return current->GetStrings()[str];
    }
  }
  ObjString* obj = ObjString::New();
  obj->str = std::string(str);
  if (current) {
    current->GetStrings()[str] = obj;
  }
  return obj;
}


ObjFunction::ObjFunction() {
  type = OBJ_FUNCTION;
}

ObjFunction* ObjFunction::New() {
  ObjFunction* obj = memory::Allocate<ObjFunction>(1);
  new (obj) ObjFunction();
  return obj;
}


ObjClosure* ObjClosure::New(ObjFunction* function) {
  //
}


ObjNative* ObjNative::New(NativeFn func) {
  ObjNative* obj = memory::Allocate<ObjNative>(1);
  new (obj) ObjNative();
  obj->type = OBJ_NATIVE;
  obj->function = func;
  return obj;
}

