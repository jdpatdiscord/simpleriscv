#pragma once

#include <cstdint>
#include <cstddef>

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

using s8 = std::int8_t;
using s16 = std::int16_t;
using s32 = std::int32_t;
using s64 = std::int64_t;

using uInt = unsigned int;
using sInt = int;


#ifdef __cpp_constexpr
  #if __cpp_constexpr >= 202211L
    #define SRISCV_CX_STATIC static
  #else
    #define SRISCV_CX_STATIC
  #endif
#else
  #define SRISCV_CX_STATIC
#endif