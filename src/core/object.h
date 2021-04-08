#ifndef FF_CORE_OBJECT_H_
#define FF_CORE_OBJECT_H_

#include <string>
#include <functional>

#include "core/value.h"
#include "core/chunk.h"

struct Value;

typedef std::function<Value(int, Value*)> NativeFn;

enum ObjType {
  OBJ_STRING,
  OBJ_NATIVE,
  OBJ_FUNCTION,
};


struct Obj {
 public:
  ObjType type;
  // Obj* next = nullptr;

 public:
  inline bool IsType(ObjType expected_type) const { return type == expected_type; }
  inline struct Obj* AsObj() { return (Obj*)this; }
  Value AsValue();

  std::string ToString() const;
};


struct ObjString : public Obj {
 public:
  std::string str;

 public:
  static ObjString* FromStr(std::string& str);
  static ObjString* New();
};


struct ObjFunction : public Obj {
 public:
  int arity = 0;
  Chunk chunk;
  ObjString* name = nullptr;
 
 public:
  ObjFunction();
  static ObjFunction* New();
};


struct ObjNative : public Obj {
 public:
  NativeFn function;

 public:
  static ObjNative* New(NativeFn func);
};

#endif

