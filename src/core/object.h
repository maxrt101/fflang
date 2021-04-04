#ifndef FF_CORE_OBJECT_H_
#define FF_CORE_OBJECT_H_

#include <string>


struct Value;

enum ObjType {
  OBJ_STRING,
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

 private:
  static ObjString* New();
};

#endif

