#include "core/chunk.h"
#include "utils/abi.h"

#include <algorithm>

void Chunk::AppendCode(uint8_t byte, int line) {
  code.push_back(byte);
  if (line == -1) // Means the same line as in the instruction before
    return;
  if (lines.size() == 0) {
    lines.push_back({code.size()-1, line});
  } else {
    if (lines[lines.size()-1].line != line) {
      lines.push_back({code.size()-1, line});
    }
  }
}

int Chunk::AddConstant(Value value) {
  auto itr = std::find(constants.begin(), constants.end(), value);
  if (itr == constants.end()) {
    constants.push_back(value);
    return constants.size() - 1;
  }
  return itr - constants.begin();
}

void Chunk::WriteConstant(Value value, int line) {
  int constant_index = AddConstant(value);
  if (constant_index < 256) {
    AppendCode(OP_CONSTANT, line);
    AppendCode(constant_index, line);
  } else {
    abi::NumericData idx;
    idx.i32 = constant_index;
    AppendCode(idx.u8[1], line);
    AppendCode(idx.u8[2], line);
    AppendCode(idx.u8[3], line);
  }
}

int Chunk::GetLine(int offset) const {
  int prev_start = 0;
  for (int i = 0; i < lines.size(); i++) {
    if (lines[i].start_offset == offset) {
      return lines[i].line;
    } else {
      if (lines[i].start_offset < offset) {
        if (i+1 == lines.size()) {
          return lines[i].line;
        }
      } else if (lines[i].start_offset > offset) {
        return lines[i-1].line;
      }
    }
  }

  return 0;
}

