// Copyright (C) 2021 Greg Dionne
// Distributed under MIT License
#include "cornermask.hpp"
#include "bitutils.hpp"

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
//
//             delete    xchg  mask  xor   reinsert
//            axis bit                     axis bit
//  F  01 32   01 32    02 31   1   13 20   13 02
//  R  04 51   02 31    01 32   2   23 10   45 10
//  U  02 64   01 32    02 31   1   13 20   26 40
//  B  46 75   02 31    01 32   2   23 10   67 54
//  L  23 76   01 32    02 31   1   13 20   37 62
//  D  15 73   02 31    01 32   2   23 10   57 31
//  F' 01 32   01 32    02 31   2   20 13   20 13
//  R' 04 51   02 31    01 32   1   10 23   10 45
//  U' 02 64   01 32    02 31   2   20 13   40 26
//  B' 46 75   02 31    01 32   1   10 23   54 67
//  L' 23 76   01 32    02 31   2   20 13   62 37
//  D' 15 73   02 31    01 32   1   10 23   31 57

namespace Janus {

struct CornerReturn {
  uint8_t position;
  uint8_t spin;
};

// with the given initial corner position perform the specified twist
// return the spin of the operation (0 = no change; 1 = cw; 2 = ccw)
static CornerReturn twistCorner(uint8_t position, uint8_t twist) {

  // note: twist is filtered so that only quarter turns are permitted
  CornerReturn returnValue{position, 0};

  uint8_t twistAxis = (17 - twist) % 3; // 0 = z, 1 = y, 2 = x
  uint8_t twistPole = (twist % 6) > 2;  // 0 = FRU, 1 = BLD
  uint8_t twistDir = twist / 6;         // 0=cw, 1=ccw

  // if the twist applies to this corner
  if (((position >> twistAxis) & 1) == twistPole) {

    // delete the axis bit
    int del = deleteBit(position, twistAxis);

    // xchg the lower bits
    int xchg = exchangeLowerBits(del);

    // get the bit to flip (either 1 or 2)
    int mask = (twistDir ^ (twistAxis & 1) ^ twistPole) + 1;

    // flip it
    int eor = xchg ^ mask;

    // reinsert the axis bit
    uint8_t dest = insertBit(eor, twistAxis, twistPole);

    returnValue.position = dest;

    // for F, R, L, B moves impart cw and ccw spins
    // to adjacent corners depending on the twist direction
    uint8_t isccw = (position ^ dest ^ twistDir) & 1;
    uint8_t offset = (1 + isccw) % 3;

    // by convention spin orientation is taken from alignment
    // of the corner to either the up or down face:
    // any twist of the U or D face results in a net zero spin.
    uint8_t hasSpin = twistAxis > 0;
    returnValue.spin = hasSpin * offset;
  }

  return returnValue;
}

static int pow3[] = {1, 3, 9, 27, 81, 243, 729, 2187};

// return a new mask restricted to single cw and ccw twists
CornerMask CornerMask::moveQuarterTwist(uint8_t twist) const {
  CornerMask out{0, 0};

  int tmpFace = face;
  int tmpSpin = spin;

  // perform the quater twist for each corner
  for (int corner = 0; corner < 8; ++corner) {
    int thisFace = tmpFace & 1;
    int thisSpin = tmpSpin % 3;

    auto retVal = twistCorner(corner, twist);
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

// with the given initial corner position and spin perform the specified twist
static CornerReturn permuteCorner(uint8_t position, uint8_t spin,
                                  uint8_t permutation) {

  // reflect about x-y plane?
  if (permutation & 0x08) {
    position ^= 0x01;
    spin = (3 - spin) % 3;
  }

  // rotate 180 about x axis?
  if (permutation & 0x04) {
    position ^= 0x03;
  }

  // rotate 180 about z axis?
  if (permutation & 0x02) {
    position ^= 0x06;
  }

  // rotate 90 about z axis?
  if (permutation & 0x01) {
    position = (2 ^ exchangeLowerBits(position >> 1)) << 1 | (position & 1);
  }

  return {position, spin};
}

// return a new mask after performing the specfied permutation
CornerMask CornerMask::permute(uint8_t permutation) const {
  CornerMask out{0, 0};

  int tmpFace = face;
  int tmpSpin = spin;

  for (int corner = 0; corner < 8; ++corner) { // for each corner
    int thisFace = tmpFace & 1;
    int thisSpin = tmpSpin % 3;

    auto retVal = permuteCorner(corner, thisSpin, permutation);
    out.face |= thisFace << retVal.position;
    out.spin += retVal.spin * pow3[retVal.position];

    tmpFace >>= 1;
    tmpSpin /= 3;
  }

  return out;
}

} // namespace Janus
