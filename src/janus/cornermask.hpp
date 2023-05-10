// Copyright (C) 2021 Greg Dionne
// Distributed under MIT License
#ifndef JANUS_CORNERMASK_HPP
#define JANUS_CORNERMASK_HPP

#include <cstdint>

namespace Janus {

// A corner mask consists of the positions and spins
// of the four identical lower and upper edges
//
// The order of corner pieces is taken to satisfy a bit pattern.
// The bit pattern is of the formx XYZ, where:
// a 0 in the corresponding bit is in the (front, right, up) direction
// a 1 in the corresponding bit is in the (back, left, down) direction.
//
//     000 = FRU  100 = BRU
//     001 = FRD  101 = BRD
//     010 = FLU  110 = BLU
//     011 = FLD  111 = BLD
//
// opposing corners sum to 7.
//
//      6---------4      6---------4
//     /         /|     /|        /|
//    /    U    / |    / |       / |
//   /         /  |   /  |      /  |
//  2---------0   |  2---------0   |
//  |         | R |  |   |     |   |
//  |         |   5  |   7-----|---5
//  |    F    |  /   |  /      |  /
//  |         | /    | /       | /
//  |         |/     |/        |/
//  3---------1      3---------1

struct CornerMask {

  // A face bit indicates the the corresponding corner
  // is occupied by an upper corner piece
  uint16_t face = 0; // 0 - 2^8 bit indicates facevalue of corner

  // In a nod to Herbert Kociemba, the orientation "spin"
  // is taken to align with the up-down (z) axis.
  //
  // a 0 means the corner is aligned.
  // a 1 means the corner is spun clockwise from the top or bottom
  // a 2 means the corner is spun counter-clockwise
  uint16_t spin = 0; // 0 - 3^8 encoding of corner states

  // return a new mask after twisting the face
  // twists are numbered:
  //  0 -  5:  F  R  U  B  L  D  (clockwise moves)
  //  6 - 11:  F' R' U' B' L' D' (counter-clockwise moves)
  // 12 - 17: F2 R2 U2 B2 L2 D2 (half-turn moves)
  CornerMask move(uint8_t twist) const;

  // return a new mask after reflecting/rotating about
  // the up-down axes in the following sequence:
  //   bit 3:  reflect along z axis
  //   bit 2:  rotate a half-turn around x axis
  //   bit 1:  rotate a half-turn around z axis
  //   bit 0:  rotate a quarter-turn around z axis
  CornerMask permute(uint8_t permutation, uint8_t reflectBit) const;

private:
  // return a new mask restricted to twists 0 - 11
  CornerMask moveQuarterTwist(uint8_t twist) const;
};

} // namespace Janus
#endif
