// Copyright (C) 2021 Greg Dionne
// Distributed under MIT License
#ifndef JANUS_MASK_HPP
#define JANUS_MASK_HPP

#include "cornermask.hpp"
#include "edgemask.hpp"

#include <cstdint>

namespace Janus {

struct Mask {

  // contains eight-bit masks for each of the corners
  CornerMask cornerMask;

  // contains twelve-bit masks for each of the edges
  EdgeMask edgeMask;

  // return a new mask after twisting the face
  // twists are numbered:
  //  0 -  5:  F  R  U  B  L  D  (clockwise moves)
  //  6 - 12:  F' R' U' B' L' D' (counter-clockwise moves)
  // 12 - 17: F2 R2 U2 B2 L2 D2 (half-turn moves)
  Mask move(uint8_t twist) const {
    return {cornerMask.move(twist), edgeMask.move(twist)};
  }

  // return a new mask after reflecting/rotating/color swapping
  // Only enares option uses bit 4.
  //   bit 4:  reflect along z axis (without colorswap)
  //   bit 3:  reflect along z axis (with colorswap)
  //   bit 2:  reflect along y axis
  //   bit 1:  rotate a half-turn around z axis
  //   bit 0:  rotate a quarter-turn around z axis
  Mask permute(uint8_t permutation) const {
    return {cornerMask.permute(permutation), edgeMask.permute(permutation)};
  }
};

} // namespace Janus
#endif
