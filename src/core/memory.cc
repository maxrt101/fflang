#include "core/memory.h"

namespace memory {
std::unordered_map<void*, AllocationTableEntry> _allocation_table;
}; // namespace memory

bool memory::IsAllocated(void* pointer) {
  return _allocation_table.find((void*)pointer) != _allocation_table.end();
}

size_t memory::GetCount(void* pointer) {
  if (IsAllocated(pointer)) {
    return _allocation_table.at(pointer).count;
  }
  return 0;
}

size_t memory::GetSizeOfElement(void* pointer) {
  if (IsAllocated(pointer)) {
    return _allocation_table.at(pointer).sizeof_element;
  }
  return 0;
}

void memory::Cleanup() {
  for (auto& entry : _allocation_table) {
    // Print debug
    free(entry.first);
  }
}


