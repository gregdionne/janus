// Copyright (C) 2021 Greg Dionne
// Distributed under MIT License
#ifndef JANUS_CORNERTWIST_HPP
#define JANUS_CORNERTWIST_HPP

#include <cstdint>

namespace Janus {

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

struct CornerReturn {
  uint8_t position;
  uint8_t spin;
};

// with the given initial corner position perform the specified twist
// return the spin of the operation (0 = no change; 1 = cw; 2 = ccw)
//
// twists are numbered 0-11:  F  R  U  B  L  D  F  F' R' U' B' L' D'
CornerReturn quarterTwistCorner(uint8_t position, uint8_t twist);

// for the given position and spin, perform the specified janus permutation
// Only enares option uses bit 4.
//   bit 4:  reflect along z axis (without colorswap)
//   bit 3:  reflect along z axis (with colorswap)
//   bit 2:  reflect along y axis
//   bit 1:  rotate a half-turn around z axis
//   bit 0:  rotate a quarter-turn around z axis
CornerReturn permuteCorner(uint8_t position, uint8_t spin, uint8_t permutation);

} // namespace Janus
#endif
