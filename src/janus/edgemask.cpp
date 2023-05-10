// Copyright (C) 2021 Greg Dionne
// Distributed under MIT License
#include "edgemask.hpp"

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

struct EdgeReturn {
  uint8_t position;
  uint8_t flip;
};

// for the given position and specified twist, return the
// new position and if a flip is requred
static EdgeReturn twistEdge(uint8_t position, uint8_t twist) {

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

// return a new edge mask after performing the specified quarter turn
EdgeMask EdgeMask::moveQuarterTwist(uint8_t twist) const {
  EdgeMask out{0, 0, 0};

  int tmpFace = face;
  int tmpValid = valid;
  int tmpFlip = flip;

  for (int edge = 0; edge < 12; ++edge) { // for each edge
    int thisFace = tmpFace & 1;
    int thisValid = tmpValid & 1;
    int thisFlip = tmpFlip & 1;
    auto retVal = twistEdge(edge, twist);    // get its destination
    out.face |= thisFace << retVal.position; // move the face to its destination
    out.valid |= thisValid << retVal.position;
    out.flip |= (thisValid & (thisFlip ^ retVal.flip)) << retVal.position;

    tmpFace >>= 1;
    tmpValid >>= 1;
    tmpFlip >>= 1;
  }

  return out;
}

// return a new edge mask after performing the specified twist
EdgeMask EdgeMask::move(uint8_t twist) const {
  return twist < 12 ? moveQuarterTwist(twist)
                    : moveQuarterTwist(twist % 6).moveQuarterTwist(twist % 6);
}

// permute the specified position and flip to a new position and flip
static EdgeReturn permuteEdge(uint8_t position, uint8_t flip,
                              uint8_t permutation, uint8_t reflectBit) {

  // reflect across x-y plane?
  if (permutation & reflectBit) {
    uint8_t rem3 = position % 3;
    if (rem3 > 0) {
      uint8_t div3 = position / 3;
      position = (div3 ^ 1) * 3 + rem3;
    }
  }

  // rotate 180 about x axis?
  if (permutation & 0x04) {
    uint8_t rem3 = position % 3;
    uint8_t div3 = position / 3;
    position = (div3 ^ (1 + (rem3 == 2))) * 3 + rem3;
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

// return a new edge mask after performing the specified permutation
EdgeMask EdgeMask::permute(uint8_t permutation, uint8_t reflectBit) const {
  EdgeMask out{0, 0, 0};

  int tmpFace = face;
  int tmpValid = valid;
  int tmpFlip = flip;

  for (int edge = 0; edge < 12; ++edge) { // for each edge
    int thisFace = tmpFace & 1;
    int thisValid = tmpValid & 1;
    int thisFlip = tmpFlip & 1;
    auto retVal = permuteEdge(edge, thisFlip, permutation,
                              reflectBit);   // get its destination
    out.face |= thisFace << retVal.position; // move the face to its destination
    out.valid |= thisValid << retVal.position;
    out.flip |= (thisValid & retVal.flip) << retVal.position;

    tmpFace >>= 1;
    tmpValid >>= 1;
    tmpFlip >>= 1;
  }

  return out;
}

} // namespace Janus
