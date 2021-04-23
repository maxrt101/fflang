#include "core/vm.h"

#include <iostream>
#include <cstdarg>
#include <cstdio>

#include "compiler/compiler.h"
#include "debug/disasm.h"
#include "utils/abi.h"

/* stdlib */
#include "stdlib/io.h"


VM* current = nullptr;


void SetCurrent(VM* vm) {
  current = vm;
}


VM::VM() {
  ResetStack();  
}


VM::~VM() {}


void VM::InitBuiltins() {
  DefineNative("println", builtin_println);
}


void VM::DefineNative(const char* name, NativeFn function) {
  std::string name_str(name);
  Push(ObjString::FromStr(name_str)->AsValue());
  Push(ObjNative::New(function));
  stack_[1].assignable = false;
  globals_[(ObjString*)(stack_[0].AsObj())] = stack_[1];
  Pop();
  Pop();
}


void VM::RuntimeError(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);

  CallFrame* frame = &frames_[frame_count_ - 1];
  size_t offset = frame->ip - frame->function->chunk.code.data() - 1;
  fprintf(stderr, "[line %d] RuntimeError: ", frame->function->chunk.GetLine(offset));
  vfprintf(stderr, fmt, args);
  fputs("\n", stderr);

  va_end(args);
  ResetStack();
}


void VM::StackTrace() {
  for (int i = frame_count_ - 1; i >= 0; i--) {
    CallFrame* stack_frame = &frames_[i];
    ObjFunction* function = stack_frame->function;
    size_t instruction = stack_frame->ip - function->chunk.code.data() - 1;
    fprintf(stderr, "[line %d] in ", function->chunk.GetLine(instruction));
    if (function->name == nullptr) {
      fprintf(stderr, "script\n");
    } else {
      fprintf(stderr, "%s()", function->name->str.c_str());
    }
  }
}


uint8_t VM::ReadByte() {
  return *frame->ip++;
}


uint16_t VM::ReadShort() {
  abi::NumericData data;
  data.u8[0] = ReadByte();
  data.u8[1] = ReadByte();
  return data.u16[0];
}


Value VM::ReadConstant() {
  return frame->function->chunk.constants[ReadByte()];
}


Value VM::ReadConstantLong() {
  abi::NumericData idx;
  idx.u8[0] = ReadByte();
  idx.u8[1] = ReadByte();
  idx.u8[2] = ReadByte();
  idx.u8[3] = ReadByte();
  return frame->function->chunk.constants[idx.i32];
}


void VM::ResetStack() {
  stack_top_ = stack_;
  frame_count_ = 0;
} 


void VM::Push(Value value) {
  *stack_top_ = value;
  stack_top_++;
}


Value VM::Pop() {
  stack_top_--;
  return *stack_top_;
}


Value VM::Peek(int distance) const {
  return stack_top_[-1 - distance];
}


bool VM::CallValue(Value callee, int arg_count) {
  if (callee.IsType(VAL_OBJ)) {
    switch (callee.AsObj()->type) {
      case OBJ_NATIVE: {
        ObjNative* native = (ObjNative*)(callee.AsObj());
        NativeFn func = native->function;
        Value result = func(arg_count, stack_top_ - arg_count);
        stack_top_ -= arg_count + 1;
        Push(result);
        return true;
      }
      case OBJ_FUNCTION: {
        return Call((ObjFunction*)(callee.AsObj()), arg_count);
      default:
        break;
      }
    }
  }

  RuntimeError("Can only call functions.");
  return false;
}


bool VM::Call(ObjFunction* function, int arg_count) {
  if (arg_count != function->arity) {
    RuntimeError("Expected %d arguments, but got %d.", function->arity, arg_count);
    return false;
  }

  if (frame_count_ == kFramesMax) {
    RuntimeError("Stack overflow.");
    return false;
  }

  CallFrame* function_frame = &frames_[frame_count_++];

  function_frame->function = function;
  function_frame->ip = function->chunk.code.data();
  function_frame->slots = stack_top_ - arg_count - 1;

  return true;
}


InterpretResult VM::Run() {
  for (;;) {
    frame = &frames_[frame_count_ - 1];

#ifdef _DEBUG_EXECUTION_TRACING
    debug::DisassembleInstruction(frame->function->chunk, (int)(frame->ip - frame->function->chunk.code.data()));
#endif

#ifdef _DEBUG_STEP
    int ch = getchar();
    if (ch == 'h') {
      printf("h - help  q - quit  s - stack  g - globals  t - trace");
    } else if (ch == 'q') {
      ResetStack();
      return InterpretResult::kOk;
    } else if (ch == 's') {
#endif
#if defined(_DEBUG_TRACE_STACK) || defined(_DEBUG_STEP)
    for (Value* slot = stack_; slot < stack_top_; slot++) {
      printf("[ ");
      slot->Print();
      printf(" ]");
    }
    printf("\n");
#endif
#ifdef _DEBUG_STEP
    } else if ('g') {
      printf("globals:\n");
      for (auto &p : globals_) {
        std::cout << p.first->str << ": " << p.second.ToString() << "\n";
      }
    } else if (ch == 't') {
      StackTrace();
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
        Push(frame->slots[slot]);
        break;
      }
      case OP_SET_LOCAL: {
        uint8_t slot = ReadByte();
        if (!stack_[slot].assignable) {
          RuntimeError("Cant assign to const variable.");
          return InterpretResult::kRuntimeError;
        }
        frame->slots[slot] = Peek(0);
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
          std::string s = a + b;
          Push(Value(ObjString::FromStr(s)->AsObj()));
        } else if (Peek(0).IsNumber() && Peek(1).IsNumber()) {
          Value b = Pop();
          Value a = Pop();
          Push(Value(a.AsNumber()+b.AsNumber()));
        } else if (Peek(0).IsNumber() && Peek(1).IsString()) { 
          NumberType b = Pop().AsNumber();
          std::string a = Pop().AsString()->str;
          std::string s = a + std::to_string(b);
          Push(Value(ObjString::FromStr(s)->AsObj()));
        } else {
          RuntimeError("Operands must be numbers or strings.");
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
        frame->ip += offset;
        break;
      }
      case OP_JUMP_IF_FALSE: {
        uint16_t offset = ReadShort();
        if (Peek(0).IsFalse()) frame->ip += offset;
        break;
      }
      case OP_LOOP: {
        uint16_t offset = ReadShort();
        frame->ip -= offset;
        break;
      }
      case OP_PRINT: {
        Pop().Print();
        std::cout << std::endl;
        break;
      }
      case OP_CALL: {
        int arg_count = ReadByte();
        if (!CallValue(Peek(arg_count), arg_count)) {
          return InterpretResult::kRuntimeError;
        }
        frame = &frames_[frame_count_ - 1];
        break;
      }
      case OP_RETURN: {
        Value result = Pop();

        frame_count_--;
        if (frame_count_ == 0) {
          Pop();
          return InterpretResult::kOk;
        }

        stack_top_ = frame->slots;
        Push(result);

        frame = &frames_[frame_count_ - 1];
        break;
      }
    }
  }
}

InterpretResult VM::Interpret(std::string& source) {
  Compiler compiler(source);
  ObjFunction* function = compiler.Compile();
  if (!function) return InterpretResult::kCompileError;

  Push(function->AsValue());
  CallValue(function->AsValue(), 0);
  return Run();
}

