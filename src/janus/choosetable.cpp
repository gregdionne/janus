// Copyright (C) 2021 Greg Dionne
// Distributed under MIT License
#include "choosetable.hpp"

namespace Janus {

// return the number of set bits in the specified mask
//   (should be similar to C++20's std::popcount)
template <typename T> T popcount(T mask) {
  T count = 0;
  while (mask) {
    mask &= mask - 1;
    ++count;
  }
  return count;
}

// ChooseTable(N,K)
//   ennumerates the ways to construct a bitmask of N bits,
//   where K bits are and N-K bits are cleared.
ChooseTable::ChooseTable(uint8_t n, uint8_t k) {

  mask2position.resize(1 << n);

  for (uint16_t mask = 0; mask < (1 << n); ++mask) {
    if (popcount(mask) == k) {
      mask2position[mask] = position2mask.size();
      position2mask.push_back(mask);
    }
  }
}

} // namespace Janus
