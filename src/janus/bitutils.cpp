// Copyright (C) 2021 Greg Dionne
// Distributed under MIT License
#include "bitutils.hpp"

namespace Janus {

// remove bit at position, shifthing remainng bits rightward
unsigned int deleteBit(unsigned int mask, unsigned int bitPos) {
  unsigned int bitmask = 1 << bitPos;
  unsigned int upperbitmask = -bitmask << 1;
  unsigned int lowerbitmask = bitmask - 1;

  return ((mask & upperbitmask) >> 1) | (mask & lowerbitmask);
}

// insert bit at position
unsigned int insertBit(unsigned int mask, unsigned int bitPos,
                       unsigned int bitValue) {
  unsigned int bitmask = 1 << bitPos;
  unsigned int upperbitmask = -bitmask;
  unsigned int lowerbitmask = bitmask - 1;

  return ((mask & upperbitmask) << 1) | (bitValue << bitPos) |
         (mask & lowerbitmask);
}

// exchange two rightmost bits
unsigned int exchangeLowerBits(unsigned int mask) {
  unsigned int flip = ((mask >> 1) ^ (mask)) & 1;
  flip = (flip << 1) | flip;
  return mask ^ flip;
}

// remove masked bits from target, shifting remaining
// bits rightward
uint16_t removeMask(uint16_t mask, uint16_t target) {

  unsigned int count = 0;

  while (mask) {
    uint16_t lowbit = mask & -mask;
    mask &= ~lowbit;
    target = (target & -lowbit) | (target & (lowbit - 1)) << 1;
    ++count;
  }

  return target >> count;
}

// shift bits leftwards to create zeros at set masked bits
uint16_t restoreMask(uint16_t mask, uint16_t target) {

  while (mask) {
    uint16_t lowbit = mask & -mask;
    mask &= ~lowbit;
    target = ((target & -lowbit) << 1) | (target & (lowbit - 1));
  }

  return target;
}

// next largest integer with same number of set bits
uint16_t nextIdenticalHammingWeight(uint16_t mask) {

  uint16_t lowbit = mask & -mask;
  uint16_t upper = mask + lowbit;
  uint16_t lower = ((mask ^ upper) / lowbit) >> 2;

  return upper | lower;
}

// repeatedly divide both arguments by 3 (modulo 2^32)
// whenever first argument is divisible by three
template <typename T> static void reduce(T *u, T *a) {
  T neg_third = (static_cast<T>(-1)) / 3;
  T one_third = -neg_third;

  while (one_third * *u <= neg_third) {
    *u *= one_third;
    *a *= one_third;
  }
}

// compute quotient for odd divisor such that:
//   quotient * divisor == dividend (modulo 2^32)
uint32_t divide(uint32_t dividend, uint32_t divisor) {

  uint32_t a = dividend;
  uint32_t u = divisor;
  uint32_t b = -dividend;
  uint32_t v = -divisor;

  reduce(&u, &a);
  reduce(&v, &b);

  while (u != 1 && v != 1 && u && v) {
    if (u > v) {
      u -= v;
      a -= b;
      reduce(&u, &a);
    } else {
      v -= u;
      b -= a;
      reduce(&v, &b);
    }
  }

  return (v == 1) ? b : a;
}

} // namespace Janus
