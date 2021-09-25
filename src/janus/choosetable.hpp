// Copyright (C) 2021 Greg Dionne
// Distributed under MIT License
#ifndef JANUS_CHOOSETABLE_HPP
#define JANUS_CHOOSETABLE_HPP

#include "constants.hpp"

#include <cstdint>
#include <vector>

namespace Janus {

// ChooseTable(N,K)
//   ennumerates the ways to construct a bitmask of N bits,
//   where K bits are and N-K bits are cleared.

class ChooseTable {
public:
  ChooseTable(uint8_t n, uint8_t k);

  std::vector<uint16_t> mask2position;
  std::vector<uint16_t> position2mask;
};

} // namespace Janus

#endif
