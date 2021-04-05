#ifndef FF_CORE_VM_H_
#define FF_CORE_VM_H_

#include <string>
#include <unordered_map>

#include "core/chunk.h"
#include "core/value.h"
#include "core/config.h"


enum class InterpretResult {
  kOk,
  kCompileError,
  kRuntimeError,
};


class VM {
 private:
  Chunk* chunk_;
  const uint8_t* ip_; // Chunk will remain unchanged, so this shouldn't be a problem
  Value stack_[kStackMaxSize];
  Value* stack_top_;
  std::unordered_map<ObjString*, Value> globals_;

 public:
  std::unordered_map<std::string, ObjString*> strings_;

 public:
  VM();
  ~VM();

  InterpretResult Interpret(std::string& source);

 private:
  inline uint8_t ReadByte() { return *ip_++; }
  uint16_t ReadShort();
  
  inline Value ReadConstant() { return chunk_->constants[ReadByte()]; }
  Value ReadConstantLong();

  inline void ResetStack()              { stack_top_ = stack_; }
  inline void Push(Value value)         { *stack_top_ = value; stack_top_++; }
  inline Value Pop()                    { stack_top_--; return *stack_top_; }
  inline Value Peek(int distance) const { return stack_top_[-1 - distance]; }

  void RuntimeError(const char* fmt, ...);

 private:
  InterpretResult Run();
};

void SetCurrent(VM* vm);

#endif

