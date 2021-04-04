#include "core/vm.h"

#include <iostream>
#include <cstdarg>
#include <cstdio>

#include "compiler/compiler.h"
#include "debug/disasm.h"
#include "utils/abi.h"


VM* current = nullptr;


void SetCurrent(VM* vm) {
  current = vm;
}



VM::VM() : chunk_(nullptr), ip_(nullptr) {
  ResetStack();
}


VM::~VM() {}


void VM::RuntimeError(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);

  size_t offset = ip_ - chunk_->code.data() - 1;
  fprintf(stderr, "[line %d] RuntimeError: ", chunk_->GetLine(offset));
  vfprintf(stderr, fmt, args);
  fputs("\n", stderr);

  va_end(args);
  ResetStack();
}


uint16_t VM::ReadShort() {
  abi::NumericData data;
  data.u8[0] = ReadByte();
  data.u8[1] = ReadByte();
  return data.u16[0];
}


Value VM::ReadConstantLong() {
  abi::NumericData idx;
  idx.u8[0] = ReadByte();
  idx.u8[1] = ReadByte();
  idx.u8[2] = ReadByte();
  idx.u8[3] = ReadByte();
  return chunk_->constants[idx.i32];
}


InterpretResult VM::Run() {
  for (;;) {
#ifdef _DEBUG_EXECUTION_TRACING
    debug::DisassembleInstruction(*chunk_, (int)(ip_ - chunk_->code.data()));
#endif
#ifdef _DEBUG_TRACE_STACK
    for (Value* slot = stack_; slot < stack_top_; slot++) {
      printf("[ ");
      slot->Print();
      printf(" ]");
    }
    printf("\n");
#endif

    uint8_t instruction = ReadByte();
    switch (instruction) {
      case OP_CONSTANT: {
        Value constant = ReadConstant();
        Push(constant);
        break;
      }
      case OP_CONSTANT_LONG: {
        Push(ReadConstantLong());
        break;
      }
      case OP_NULL: Push(Value()); break;
      case OP_TRUE: Push(Value(true)); break;
      case OP_FALSE: Push(Value(false)); break;
      case OP_POP: Pop(); break;
      case OP_DEFINE_GLOBAL: {
        ObjString* name = ReadConstant().AsString();
        globals_[name] = Peek(0);
        Pop();
        break;
      }
      case OP_DEFINE_GLOBAL_LONG: {
        ObjString* name = ReadConstantLong().AsString();
        globals_[name] = Peek(0);
        Pop();
        break;
      }
      case OP_GET_GLOBAL: {
        ObjString* name = ReadConstant().AsString();
        auto variable = globals_.find(name);
        if (variable == globals_.end()) {
          RuntimeError("Reference to undefined variable '%s'.", name->str.c_str());
          return InterpretResult::kRuntimeError;
        }
        Push(variable->second);
        break;
      }
      case OP_GET_GLOBAL_LONG: {
        ObjString* name = ReadConstantLong().AsString();
        auto variable = globals_.find(name);
        if (variable == globals_.end()) {
          RuntimeError("Reference to undefined variable '%s'.", name->str.c_str());
          return InterpretResult::kRuntimeError;
        }
        Push(variable->second);
        break;
      }
      case OP_SET_GLOBAL: {
        ObjString* name = ReadConstant().AsString();
        auto variable = globals_.find(name);
        if (variable == globals_.end()) {
          RuntimeError("Reference to undefined variable '%s'.", name->str.c_str());
          return InterpretResult::kRuntimeError;
        }
        if (!variable->second.assignable) {
          RuntimeError("Cant assign to const variable.");
          return InterpretResult::kRuntimeError;
        }
        variable->second = Peek(0);
        break;
        
      }
      case OP_SET_GLOBAL_LONG: {
        ObjString* name = ReadConstantLong().AsString();
        auto variable = globals_.find(name);
        if (variable == globals_.end()) {
          RuntimeError("Reference to undefined variable '%s'.", name->str.c_str());
          return InterpretResult::kRuntimeError;
        }
        if (!variable->second.assignable) {
          RuntimeError("Cant assign to const variable.");
          return InterpretResult::kRuntimeError;
        }
        variable->second = Peek(0);
        break;        
      }
      case OP_GET_LOCAL: {
        uint8_t slot = ReadByte();
        Push(stack_[slot]);
        break;
      }
      case OP_SET_LOCAL: {
        uint8_t slot = ReadByte();
        if (!stack_[slot].assignable) {
          RuntimeError("Cant assign to const variable.");
          return InterpretResult::kRuntimeError;
        }
        stack_[slot] = Peek(0);
        break;
      }
      case OP_MAKECONST: {
        Value val = Pop();
        val.assignable = false;
        Push(val);
        break;
      }
      case OP_NOT:  Push(Value(Pop().IsFalse())); break;
      case OP_NEGATE: {
        if (!Peek(0).IsType(VAL_NUMBER)) {
          RuntimeError("Operand must be a number.");
          return InterpretResult::kRuntimeError;
        }
        Push(Value(-Pop().AsNumber()));
        break;
      }
      case OP_EQUAL: {
        Value b = Pop();
        Value a = Pop();
        Push(Value(a == b));
        break;
      }
      case OP_GREATER: {
        if (!Peek(0).IsType(VAL_NUMBER) || !Peek(1).IsType(VAL_NUMBER)) {
          RuntimeError("Operands must be numbers.");
          return InterpretResult::kRuntimeError;
        }
        Value b = Pop();
        Value a = Pop();
        Push(Value(a.AsNumber()>b.AsNumber()));
        break;
      }
      case OP_LESS: {
        if (!Peek(0).IsType(VAL_NUMBER) || !Peek(1).IsType(VAL_NUMBER)) {
          RuntimeError("Operands must be numbers.");
          return InterpretResult::kRuntimeError;
        }
        Value b = Pop();
        Value a = Pop();
        Push(Value(a.AsNumber()<b.AsNumber()));
        break;
      }
      case OP_ADD: {
        if (Peek(0).IsString() && Peek(1).IsString()) {
          std::string b = Pop().AsString()->str;
          std::string a = Pop().AsString()->str;
          std::cout << "OP_ADD: '" << a << "' '" << b << "'\n";
          std::string s = a + b;
          std::cout << "s: '" << s << "'\n";
          Push(Value(ObjString::FromStr(s)->AsObj()));
        } else if (Peek(0).IsType(VAL_NUMBER) && Peek(1).IsType(VAL_NUMBER)) {
          Value b = Pop();
          Value a = Pop();
          Push(Value(a.AsNumber()+b.AsNumber()));
        } else {
          RuntimeError("Operands must be numbers.");
          return InterpretResult::kRuntimeError;
        }
        break;
      }
      case OP_SUBTRACT: {
        if (!Peek(0).IsType(VAL_NUMBER) || !Peek(1).IsType(VAL_NUMBER)) {
          RuntimeError("Operands must be numbers.");
          return InterpretResult::kRuntimeError;
        }
        Value b = Pop();
        Value a = Pop();
        Push(Value(a.AsNumber()-b.AsNumber()));
        break;
      }
      case OP_MULTIPLY: {
        if (!Peek(0).IsType(VAL_NUMBER) || !Peek(1).IsType(VAL_NUMBER)) {
          RuntimeError("Operands must be numbers.");
          return InterpretResult::kRuntimeError;
        }
        Value b = Pop();
        Value a = Pop();
        Push(Value(a.AsNumber()*b.AsNumber()));
        break;
      }
      case OP_DIVIDE: {
        if (!Peek(0).IsType(VAL_NUMBER) || !Peek(1).IsType(VAL_NUMBER)) {
          RuntimeError("Operands must be numbers.");
          return InterpretResult::kRuntimeError;
        }
        Value b = Pop();
        Value a = Pop();
        Push(Value(a.AsNumber()/b.AsNumber()));
        break;
      }
      case OP_JUMP: {
        uint16_t offset = ReadShort();
        ip_ += offset;
        break;
      }
      case OP_JUMP_IF_FALSE: {
        uint16_t offset = ReadShort();
        if (Peek(0).IsFalse()) ip_ += offset;
        break;
      }
      case OP_PRINT: {
        Pop().Print();
        std::cout << std::endl;
        break;
      }
      case OP_RETURN: {
        return InterpretResult::kOk;
      }
    }
  }
}

InterpretResult VM::Interpret(std::string& source) {
  Chunk chunk;
  Compiler compiler(source);

  if (!compiler.Compile(&chunk)) {
    return InterpretResult::kCompileError;
  }

  this->chunk_ = &chunk;
  this->ip_ = chunk.code.data();
  
  return Run();
}

