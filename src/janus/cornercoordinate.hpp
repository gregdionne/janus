// Copyright (C) 2021 Greg Dionne
// Distributed under MIT License
#ifndef JANUS_CORNERCOORDINATE_HPP
#define JANUS_CORNERCOORDINATE_HPP

#include "constants.hpp"

#include <cstdint>

namespace Janus {

// A Janus corner coordinate consists
// of the scrambled positions and spins
// of each corner.
//
// There are 8C4 ways to position the
// four identical lower and upper corners
//
// There are 3^7 ways to orient (spin)
// seven of the corners. The eighth corner
// can be determined by computing the sum
// of the other spins modulo 3.

struct CornerCoordinate {
  uint8_t position; // nSymCornerPositions
  uint16_t spin;    // nSymCornerSpins

  // convenience for retrieving the corner table index
  //
  // although all values are less than 32 bit, C++
  // promotes all intermediate values to "int", which
  // we'll make the assumption is at least 32 bits
  uint32_t tableIndex() const { return spin * nSymCornerPositions + position; }
};

} // namespace Janus

#endif
