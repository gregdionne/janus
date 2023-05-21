// Copyright (C) 2021 Greg Dionne
// Distributed under MIT License
#include "cornermask.hpp"
#include "bitutils.hpp"
#include "cornertwist.hpp"

namespace Janus {

static const int pow3[] = {1, 3, 9, 27, 81, 243, 729, 2187};

// return a new mask restricted to single cw and ccw twists
CornerMask CornerMask::moveQuarterTwist(uint8_t twist) const {
  CornerMask out{0, 0};

  int tmpFace = face;
  int tmpSpin = spin;

  // perform the quater twist for each corner
  for (int corner = 0; corner < 8; ++corner) {
    int thisFace = tmpFace & 1;
    int thisSpin = tmpSpin % 3;

    auto retVal = quarterTwistCorner(corner, twist);
    out.face |= thisFace << retVal.position;
    out.spin += ((thisSpin + retVal.spin) % 3) * pow3[retVal.position];

    tmpFace >>= 1;
    tmpSpin /= 3;
  }

  return out;
}

// return a new mask after twisting the specified face
CornerMask CornerMask::move(uint8_t twist) const {
  // if a half turn use two clockwise quater twists in sequence
  return twist < 12 ? moveQuarterTwist(twist)
                    : moveQuarterTwist(twist % 6).moveQuarterTwist(twist % 6);
}

// return a new mask after performing the specfied permutation
CornerMask CornerMask::permute(uint8_t permutation) const {
  CornerMask out{0, 0};

  int tmpFace = face;
  int tmpSpin = spin;
  int exchange = (permutation & 0x08) >> 3;

  for (int corner = 0; corner < 8; ++corner) { // for each corner
    int thisFace = tmpFace & 1;
    int thisSpin = tmpSpin % 3;

    auto retVal = permuteCorner(corner, thisSpin, permutation);
    out.face |= (thisFace ^ exchange) << retVal.position;
    out.spin += retVal.spin * pow3[retVal.position];

    tmpFace >>= 1;
    tmpSpin /= 3;
  }

  return out;
}

} // namespace Janus
