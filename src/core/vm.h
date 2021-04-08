#ifndef FF_CORE_VM_H_
#define FF_CORE_VM_H_

#include <string>
#include <unordered_map>

#include "core/chunk.h"
#include "core/value.h"
#include "core/object.h"
#include "core/config.h"


enum class InterpretResult {
  kOk,
  kCompileError,
  kRuntimeError,
};


struct CallFrame {
  ObjFunction* function;
  uint8_t* ip;
  Value* slots;
};


class VM {
 private:
  Value stack_[kStackMaxSize];
  Value* stack_top_;

  CallFrame frames_[kFramesMax];
  int frame_count_;
  CallFrame* frame = nullptr;

  std::unordered_map<ObjString*, Value> globals_;

 public:
  std::unordered_map<std::string, ObjString*> strings_;

 public:
  VM();
  ~VM();

  InterpretResult Interpret(std::string& source);
  void InitBuiltins();
  void DefineNative(const char* name, NativeFn function);

 private:
  void RuntimeError(const char* fmt, ...);
  void StackTrace();

 private:
  uint8_t ReadByte();
  uint16_t ReadShort();
  
  Value ReadConstant();
  Value ReadConstantLong();

  void ResetStack();
  void Push(Value value);
  Value Pop();
  Value Peek(int distance) const;

  bool CallValue(Value callee, int arg_count);;
  bool Call(ObjFunction* function, int arg_count);

 private:
  InterpretResult Run();
};

void SetCurrent(VM* vm);

#endif

