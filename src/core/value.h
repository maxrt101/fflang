#ifndef FF_CORE_VALUE_H_
#define FF_CORE_VALUE_H_

#include <string>
#include <vector>
#include <cstdint>

struct Obj;
struct ObjString;

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
  Value(const Value& rhs);
  Value(ValueType type = VAL_NULL);
  Value(bool val);
  Value(NumberType val);
  Value(Obj* obj);

 public:
  bool AsBool() const;
  NumberType AsNumber() const;
  struct Obj* AsObj() const;
  ObjString* AsString() const;

  bool IsType(ValueType expected_type) const;
  bool IsNumber() const;
  bool IsString() const;
  bool IsFalse() const;

  bool operator==(const Value& rhs);
  
  std::string ToString() const;
  void Print() const;
};


typedef std::vector<Value> ValueArray;

#endif

