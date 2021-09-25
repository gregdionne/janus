// Copyright (C) 2021 Greg Dionne
// Distributed under MIT License
#ifndef JANUS_BITUTILS_HPP
#define JANUS_BITUTILS_HPP

#include <cstdint>

namespace Janus {

// remove bit at position, shifting remaining bits rightward
unsigned int deleteBit(unsigned intmask, unsigned intbitPos);

// insert bit at position
unsigned int insertBit(unsigned intmask, unsigned intbitPos,
                       unsigned intbitValue);

// exchange rightmost two bits
unsigned int exchangeLowerBits(unsigned intmask);

// remove bits specified in mask, shifting remaining
// bits rightward
uint16_t removeMask(uint16_t mask, uint16_t target);

// shift bits leftwards to create zeros at set mask bits
uint16_t restoreMask(uint16_t mask, uint16_t target);

// compute quotient for odd divisor such that:
//   quotient * divisor = dividend (modulo 2^32)
uint32_t divide(uint32_t dividend, uint32_t divisor);

} // namespace Janus

#endif
