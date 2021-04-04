#ifndef FF_UTILS_ABI_H_
#define FF_UTILS_ABI_H_

#include <cstdint>

namespace abi {
union NumericData {
  int8_t    i8[4];
  uint8_t   u8[4];
  int16_t   i16[2];
  uint16_t  u16[2];
  int32_t   i32;
  uint32_t  u32;
  float     f;
};

} // namespace abi

#endif

