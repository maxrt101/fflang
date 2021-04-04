#include "disasm.h"
#include <cstdio>
#include "core/value.h"
#include "utils/abi.h"

static inline int SimpleInstruction(const char* name, int offset) {
  printf("%s\n", name);
  return offset+1;
}

static inline int ByteInstruction(const char* name, const Chunk& chunk, int offset) {
  uint8_t slot = chunk.code[offset+1];
  printf("%-16s %4d\n", name, slot);
  return offset+2;
}

static inline int ConstantInstruction(const char* name, const Chunk& chunk, int offset) {
  uint8_t constant = chunk.code[offset+1];
  printf("%-16s %4d '", name, constant);
  chunk.constants[constant].Print();
  printf("'\n");
  return offset+2;
}

static inline int ConstantLongInstruction(const char* name, const Chunk& chunk, int offset) {
  abi::NumericData idx;
  idx.u8[0] = chunk.code[offset+1];
  idx.u8[1] = chunk.code[offset+2];
  idx.u8[2] = chunk.code[offset+3];
  idx.u8[3] = chunk.code[offset+4];;
  printf("%-16s %4d '", name, idx.u32);
  chunk.constants[idx.u32].Print();
  printf("'\n");
  return offset+5;
}

static inline int JumpInstruction(const char* name, int sign,  const Chunk& chunk, int offset) {
  abi::NumericData jump_offset;
  jump_offset.u8[0] = chunk.code[offset+1];
  jump_offset.u8[1] = chunk.code[offset+2];
  printf("%-16s %4d -> %d\n", name, offset, offset + 3 + sign * jump_offset.u16[0]);
  return offset + 3;
}

int debug::DisassembleInstruction(const Chunk& chunk, int offset) {
  printf("%04d: ", offset);
  if (offset > 0 && chunk.GetLine(offset) == chunk.GetLine(offset-1)) {
    printf("   | ");
  } else {
    printf("%4d ", chunk.GetLine(offset));
  }

  uint8_t instruction = chunk.code[offset];
  switch (instruction) {
    case OP_RETURN:             return SimpleInstruction("OP_RETURN", offset);
    case OP_JUMP:               return JumpInstruction("OP_JUMP", 1, chunk, offset);
    case OP_JUMP_IF_FALSE:      return JumpInstruction("OP_JUMP_IF_FALSE", 1, chunk, offset);
    case OP_PRINT:              return SimpleInstruction("OP_PRINT", offset);
    case OP_CONSTANT:           return ConstantInstruction("OP_CONSTANT", chunk, offset);
    case OP_CONSTANT_LONG:      return ConstantLongInstruction("OP_CONSTANT_LONG", chunk, offset);
    case OP_NULL:               return SimpleInstruction("OP_NULL", offset);
    case OP_TRUE:               return SimpleInstruction("OP_TRUE", offset);
    case OP_FALSE:              return SimpleInstruction("OP_FALSE", offset);
    case OP_POP:                return SimpleInstruction("OP_POP", offset);
    case OP_DEFINE_GLOBAL:      return ConstantInstruction("OP_DEFINE_GLOBAL", chunk, offset);
    case OP_DEFINE_GLOBAL_LONG: return ConstantLongInstruction("OP_DEFINE_GLOBAL_LONG", chunk, offset);
    case OP_GET_GLOBAL:         return ConstantInstruction("OP_GET_GLOBAL", chunk, offset);
    case OP_GET_GLOBAL_LONG:    return ConstantLongInstruction("OP_GET_GLOBAL_LONG", chunk, offset);
    case OP_SET_GLOBAL:         return ConstantInstruction("OP_SET_GLOBAL", chunk, offset);
    case OP_SET_GLOBAL_LONG:    return ConstantLongInstruction("OP_SET_GLOBAL_LONG", chunk, offset);
    case OP_GET_LOCAL:          return ByteInstruction("OP_GET_LOCAL", chunk, offset);
    case OP_SET_LOCAL:          return ByteInstruction("OP_SET_LOCAL", chunk, offset);
    case OP_MAKECONST:          return SimpleInstruction("OP_MAKECONST", offset);
    case OP_NOT:                return SimpleInstruction("OP_NOT", offset);
    case OP_NEGATE:             return SimpleInstruction("OP_NEGATE", offset);
    case OP_EQUAL:              return SimpleInstruction("OP_EQUAL", offset);
    case OP_GREATER:            return SimpleInstruction("OP_GREATER", offset);
    case OP_LESS:               return SimpleInstruction("OP_LESS", offset);
    case OP_ADD:                return SimpleInstruction("OP_ADD", offset);
    case OP_SUBTRACT:           return SimpleInstruction("OP_SUBTRACT", offset);
    case OP_MULTIPLY:           return SimpleInstruction("OP_MULTIPLY", offset);
    case OP_DIVIDE:             return SimpleInstruction("OP_DIVIDE", offset);
    default:
      printf("Unknown opcode: %d", instruction);
      return offset+1;
  }
}


void debug::DisassembleChunk(const Chunk& chunk, const std::string& name) {
  printf("=== %s ===\n", name.c_str());

  for (int offset = 0; offset < chunk.code.size(); ) {
    offset = DisassembleInstruction(chunk, offset);
  }
}

