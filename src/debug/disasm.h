#ifndef FF_DEBUG_DISASM_H_
#define FF_DEBUG_DISASM_H_

#include <string>

#include "core/chunk.h"


namespace debug {
void DisassembleChunk(const Chunk& chunk, const std::string& name);
int DisassembleInstruction(const Chunk& chunk, int offset);
} // namespace debug

#endif

