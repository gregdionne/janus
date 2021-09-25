// Copyright (C) 2021 Greg Dionne
// Distributed under MIT License
#ifndef JANUS_EDGECOORDINATE_HPP
#define JANUS_EDGECOORDINATE_HPP

#include "constants.hpp"

#include <cstdint>

// An edge coordinate consists of the positions and flips of the
// four identical lower and four identical upper edges as stored
// by the depth table.
//
// To reduce the size of the depth table, the edge positions are
// checked to see if they can be reduced taking symmetry into account
// This reduces the position count from 34,650 to 2,256 positions.
//
// There are 2^8 ways to flip the eight edges

struct EdgeCoordinate {
  uint16_t position; // nSymEdgePositions
  uint16_t flip;     // nSymEdgeFlips

  // convenience for obtaining the table index
  uint32_t tableIndex() const {
    return (static_cast<uint32_t>(position) << 8) + flip;
  }
};

#endif
