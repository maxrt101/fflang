#ifndef FF_CORE_CHUNK_H_
#define FF_CORE_CHUNK_H_

#include <vector>

#include "common.h"
#include "core/value.h"

enum OpCode : uint8_t {
  OP_CONSTANT,
  OP_CONSTANT_LONG,
  OP_NULL,
  OP_TRUE,
  OP_FALSE,
  OP_POP,
  OP_DEFINE_GLOBAL,
  OP_DEFINE_GLOBAL_LONG,
  OP_GET_GLOBAL,
  OP_GET_GLOBAL_LONG,
  OP_SET_GLOBAL,
  OP_SET_GLOBAL_LONG,
  OP_GET_LOCAL,
  OP_SET_LOCAL,
  OP_MAKECONST,
  OP_NOT,
  OP_NEGATE,
  OP_EQUAL,
  OP_GREATER,
  OP_LESS,
  OP_ADD,
  OP_SUBTRACT,
  OP_MULTIPLY,
  OP_DIVIDE,
  OP_JUMP,
  OP_JUMP_IF_FALSE,
  OP_LOOP,
  OP_PRINT,
  OP_RETURN,
};


class Chunk {
 private:
  struct LineInfo {
    size_t start_offset;
    int line;
  };
 
 public:
  std::vector<uint8_t> code;
  ValueArray constants;
  std::vector<LineInfo> lines;

 public:
  void AppendCode(uint8_t byte, int line = -1);
  int AddConstant(Value value);
  void WriteConstant(Value value, int line = -1);
  int GetLine(int offset) const;
};

#endif

