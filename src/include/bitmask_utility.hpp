#pragma once

#include "common.hpp"

#include <concepts>

template<std::unsigned_integral T>
constexpr T make_bitmask(uInt start, uInt end) noexcept
{
    auto high = start > end ? start : end;
    auto low  = end < start ? end : start;
    auto bit_count = high - low + 1;

    u64 ret = 0u;
    for (auto ii = size_t{0}; ii < bit_count; ++ii)
    {
        ret = ret << 1;
        ret += 1u;
    }
    for (auto ii = size_t{0}; ii < low; ++ii) {
        ret = ret << 1;
    }
    return static_cast<T>(ret);
}

template<std::integral auto Start, std::integral auto End, std::unsigned_integral T>
constexpr T extract_bits(T v) noexcept
{
	SRISCV_CX_STATIC constexpr T mask = make_bitmask<T>(Start, End);
	return (v & mask) >> Start;
}