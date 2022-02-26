// Copyright (C) 2021 Greg Dionne
// Distributed under MIT License
#include "choosetable.hpp"
#include "bitutils.hpp"

namespace Janus {

// ChooseTable(N,K)
//   ennumerates the ways to construct a bitmask of N bits,
//   where K bits are set and N-K bits are cleared.
ChooseTable::ChooseTable(uint8_t n, uint8_t k) {

  mask2position.resize(1U << n);

  // get first mask
  uint16_t mask = (1 << k) - 1;

  while (mask < (1 << n)) {
    mask2position[mask] = position2mask.size();
    position2mask.push_back(mask);

    // next largest integer with same number of set bits
    mask = nextIdenticalHammingWeight(mask);
  }
}

} // namespace Janus
