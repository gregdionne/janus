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
// four identical lower and upper corners.
constexpr uint8_t nCornerPositions = C_8_4;
//
// There are 3^7 ways to orient (spin)
// seven of the corners. The eighth corner
// can be determined by computing the sum
// of the other spins modulo 3.
constexpr uint16_t nCornerSpins = 2187;

// full corner coordinate consists of a position and spin
constexpr uint32_t nCornerCoords = nCornerPositions * nCornerSpins;

struct CornerCoordinate {
  uint8_t position; // nCornerPositions
  uint16_t spin;    // nCornerSpins

  // convenience for retrieving the corner table index
  //
  // although all values are less than 32 bit, C++
  // promotes all intermediate values to "int", which
  // we'll make the assumption is at least 32 bits
  uint32_t tableIndex() const { return spin * nCornerPositions + position; }
};

} // namespace Janus

#endif
