#ifndef FF_CORE_MEMORY_H_
#define FF_CORE_MEMORY_H_

#include <unordered_map>
#include <cstdlib>

#include "utils/die.h"


namespace memory {

struct AllocationTableEntry {
  size_t sizeof_element;
  size_t count;
};

// Keeps track of all allocated chunks of memory
extern std::unordered_map<void*, AllocationTableEntry> _allocation_table;

bool IsAllocated(void* pointer);
size_t GetCount(void* pointer);
size_t GetSizeOfElement(void* pointer);

void Cleanup();

template <typename T>
inline T* Reallocate(T* pointer, size_t new_count) {
  if (new_count == 0) {
    _allocation_table.erase((void*)pointer);
    free((void*)pointer);
    return nullptr;
  }

  // Delete old entry
  if (IsAllocated((void*)pointer)) {
    _allocation_table.erase((void*)pointer);
  }
  
  void* result = realloc(pointer, new_count * sizeof(T));
  if (result == NULL) {
    // Print allocation error
    die();
  }
  _allocation_table[result] = {sizeof(T), new_count};
  return (T*)result;
}

template <typename T>
inline T* Allocate(size_t count) {
  return Reallocate<T>(nullptr, count);
}

} // namespace memory


#endif

