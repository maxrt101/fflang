#ifndef FF_CORE_VM_H_
#define FF_CORE_VM_H_

#include <string>
#include <vector>
#include <cstdarg>
#include <unordered_map>

#include "core/api.h"
#include "core/chunk.h"
#include "core/value.h"
#include "core/object.h"
#include "core/module.h"
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
 friend class VMContext;

 private:
  Value stack_[kStackMaxSize];
  Value* stack_top_;

  CallFrame frames_[kFramesMax];
  int frame_count_;
  CallFrame* frame = nullptr;

  std::unordered_map<ObjString*, Value> globals_;
  std::vector<FFModule> modules_;

 public:
  std::unordered_map<std::string, ObjString*> strings;

  VMContext this_context;

 public:
  VM();
  ~VM();

  InterpretResult Interpret(std::string& source);
  void InitBuiltins();
  void DefineNative(const char* name, NativeFn function);
  void Import(ObjString* name);

 private:
  void vRuntimeError(const char* fmt, va_list args);
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

void SetCurrent(VM& vm);

#endif

