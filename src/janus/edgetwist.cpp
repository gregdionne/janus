// Copyright (C) 2021 Greg Dionne
// Distributed under MIT License
#include "edgetwist.hpp"

//      +--- 7 ---+       +--- 7 ---+
//     /         /|      /|        /|
//    8    U    5 |     8 |       5 |
//   /         /  6    /  9      /  6
//  +--- 4 ---+   |   +--- 4 ---+   |
//  |         | R |   |   |     |   |
//  |         |   +   |   +--- A|---+
//  0    F    3  /    0  /      3  /
//  |         | 2     | B       | 2
//  |         |/      |/        |/
//  +--- 1 ---+       +--- 1 ---+
//
//            deface  -(>1)    twist  +(>1)  reface
//  F  01 34   01 34  01 23  - 30 12  13 40   13 40
//  R  23 56   01 34  01 23  + 12 30  13 40   35 62
//  U  45 78   01 34  01 23  - 30 12  13 40   57 84
//  B  67 9A   01 34  01 23  + 12 30  13 40   79 A6
//  L  89 B0   01 34  01 23  - 30 12  13 40   9B 08
//  D  AB 12   01 34  01 23  + 12 30  13 40   B1 2A
//  F' 01 34   01 34  01 23  + 12 30  40 13   40 13
//  R' 23 56   01 34  01 23  - 30 12  40 13   62 35
//  U' 45 78   01 34  01 23  + 12 30  40 13   84 57
//  B' 67 9A   01 34  01 23  - 30 12  40 13   A6 79
//  L' 89 B0   01 34  01 23  + 12 30  40 13   08 9B
//  D' AB 12   01 34  01 23  - 30 12  40 13   2A B1

namespace Janus {

// for the given position and specified twist, return the
// new position and if a flip is requred
EdgeReturn quarterTwistEdge(uint8_t position, uint8_t twist) {

  EdgeReturn returnValue{position, 0};

  uint8_t twistAxis = (17 - twist) % 3; // 0 = z, 1 = y, 2 = x
  uint8_t twistFace = twist % 6; // 0 = F, 1 = R, 2 = U, 3 = B, 4 = L, 5 = D
  uint8_t twistDir = twist / 6;  // 0=cw, 1=ccw, 2=2x

  uint8_t startPosition = twistFace << 1;

  // deface
  uint8_t relativePosition = (12 + position - startPosition) % 12;

  if (relativePosition < 5 && relativePosition != 2) {

    // -(>1)
    relativePosition -= relativePosition > 1;

    // twist
    relativePosition += 3 + (((twistDir ^ twist) & 1) << 1);
    relativePosition &= 3;

    // +(>1)
    relativePosition += relativePosition > 1;

    // reface
    returnValue.position = (relativePosition + startPosition) % 12;

    // flip when L or R
    returnValue.flip = twistAxis & 1;
  }

  return returnValue;
}

// permute the specified position and flip to a new position and flip
// Only enares option uses bit 4.
//   bit 4:  reflect along z axis (without colorswap)
//   bit 3:  reflect along z axis (with colorswap)
//   bit 2:  reflect along y axis
//   bit 1:  rotate a half-turn around z axis
//   bit 0:  rotate a quarter-turn around z axis
EdgeReturn permuteEdge(uint8_t position, uint8_t flip, uint8_t permutation) {

  // reflect across x-y plane without color exchange
  if (permutation & 0x10) {
    uint8_t rem3 = position % 3;
    if (rem3 > 0) {
      uint8_t div3 = position / 3;
      position = (div3 ^ 1) * 3 + rem3;
    }
  }

  // reflect across x-y plane with color exchange
  if (permutation & 0x08) {
    uint8_t rem3 = position % 3;
    if (rem3 > 0) {
      uint8_t div3 = position / 3;
      position = (div3 ^ 1) * 3 + rem3;
    }
  }

  // reflect across x-z plane (along y-axis)?
  if (permutation & 0x04) {
    uint8_t rem3 = position % 3;
    uint8_t div3 = position / 3;
    position = (((5 - rem3) & 0x3) ^ div3) * 3 + rem3;
  }

  // rotate 180 about z axis?
  if (permutation & 0x02) {
    uint8_t rem3 = position % 3;
    uint8_t div3 = position / 3;
    position = (div3 ^ (2 + (rem3 != 0))) * 3 + rem3;
  }

  // rotate 90 about z axis?
  if (permutation & 0x01) {
    uint8_t rem3 = position % 3;
    uint8_t div3 = position / 3;
    position = rem3 == 0   ? ((div3 + 1) & 0x3) * 3 + rem3
               : rem3 == 1 ? (div3)*3 + (rem3 ^ 3)
                           : (div3 ^ 3) * 3 + (rem3 ^ 3);
    flip ^= rem3 == 0;
  }

  return {position, flip};
}

} // namespace Janus
