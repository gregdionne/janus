// Copyright (C) 2021 Greg Dionne
// Distributed under MIT License
#include "movetablebuilder.hpp"
#include "bitutils.hpp"

namespace Janus {

// builds the (temporary) rec2sec and sec2rec tables required
// to build the move table
//
// rec:  regular edge position coordinate
// sec:  symmetricized edge position coordinate
//       after removing permutations
//
// rec2sec[regEdgePosition]
//  returns the symmetricized position in the lower 12 bits
//  and the permutation required in the upper 4 bits.
//
// sec2rec[symEdgePosition]
//  returns the regular position for the specified symmetricized
//  edge position.  It is typically a stepping stone to create
//  the position mask for an edge.
MoveTableBuilder::MoveTableBuilder() {
  uint16_t symEdgePosition = 0;

  // for each regular position
  for (uint16_t regPosition = 0; regPosition < nRegEdgePositions;
       ++regPosition) {

    // use the lowest numerical position as the "best"
    uint8_t bestPerm = 0;
    uint16_t bestRegPosition = regPosition;

    EdgeMask jem = pos2jem(regPosition);

    for (uint8_t perm = 1; perm < nJanusPerms; ++perm) {

      EdgeMask pem = jem.permute(perm);
      uint16_t permutedRegPosition = jem2pos(pem);

      if (permutedRegPosition < bestRegPosition) {
        bestPerm = perm;
        bestRegPosition = permutedRegPosition;
      }
    }

    // if this is a new position
    if (bestPerm == 0) {
      // add it to both tables
      sec2rec[symEdgePosition] = regPosition;
      rec2sec[regPosition] = symEdgePosition;
      ++symEdgePosition;
    } else {
      // add it to the rec2sec table and indicate the permutation
      // needed to get to it.
      rec2sec[regPosition] = rec2sec[bestRegPosition] | (bestPerm << 12);
    }
  }
}

// builds the table that performs whole-cube rotation/inversion
// of the edges for the given symmetry
void MoveTableBuilder::buildEquivalentEdgePermutationTable(
    std::vector<uint8_t> (&equivalentEdgePermutationTable)[nSymEdgePositions]) {
  for (uint16_t symPosition = 0; symPosition < nSymEdgePositions;
       ++symPosition) {

    uint16_t regPosition = sec2rec[symPosition];
    EdgeMask jem = pos2jem(regPosition);

    for (uint8_t perm = 1; perm < nJanusPerms; ++perm) {
      EdgeMask pem = jem.permute(perm);
      uint16_t permutedRegPosition = jem2pos(pem);

      if (permutedRegPosition == regPosition) {
        equivalentEdgePermutationTable[symPosition].push_back(perm);
      }
    }
  }
}

// create an edge mask with the specified regular position coordinate
// the mask will have no flips
EdgeMask MoveTableBuilder::pos2jem(uint16_t regPosition) {

  uint16_t mask0 = c12_4.position2mask[regPosition / C_8_4];
  uint16_t mask1 = c8_4.position2mask[regPosition % C_8_4];

  uint16_t valid = ~mask0 & ((1 << 12) - 1);
  uint16_t face = restoreMask(mask0, mask1);

  return {valid, face, 0};
}

// obtain the regular position coordinate of the specified (permuted)
// edge mask.  flips in the mask are ignored.
uint16_t MoveTableBuilder::jem2pos(EdgeMask pem) {

  uint16_t pmask0 = ((1 << 12) - 1) & ~pem.valid;
  uint16_t pmask1 = removeMask(pmask0, pem.face);

  uint16_t permutedRegPosition =
      c12_4.mask2position[pmask0] * C_8_4 + c8_4.mask2position[pmask1];

  return permutedRegPosition;
}

static const uint16_t pow3[8] = {1, 3, 9, 27, 81, 243, 729, 2187};

// corner coordinates only tracks the spin of seven corners
// corner masks need the eighth corner
static uint16_t restoreSpinParity(uint16_t spin) {

  uint16_t outSpin = 0;
  uint16_t sumSpin = 0;

  for (int i = 0; i < 7; ++i) {
    uint16_t inSpin = spin % 3;
    sumSpin += inSpin;
    outSpin += inSpin * pow3[i];
    spin /= 3;
  }

  uint16_t lastspin = (3 - (sumSpin % 3)) % 3;
  outSpin += lastspin * pow3[7];

  return outSpin;
}

// discard the eighth corner spin
// used when going from a corner mask to a corner coordinate
static uint16_t removeSpinParity(uint16_t spin) { return spin % pow3[7]; }

// convert from a Janus corner coordinate to a Janus corner mask
CornerMask MoveTableBuilder::jcc2jcm(const CornerCoordinate &jcc) {
  uint16_t face = c8_4.position2mask[jcc.position];
  uint16_t spin = restoreSpinParity(jcc.spin);
  return {face, spin};
}

// convert from a Janus corner mask to a Janus corner coordinate
CornerCoordinate MoveTableBuilder::jcm2jcc(const CornerMask &jcm) {
  uint8_t position = c8_4.mask2position[jcm.face];
  uint16_t spin = removeSpinParity(jcm.spin);
  return {position, spin};
}

// convert from a Janus edge coordinate to a Janus edge mask
EdgeMask MoveTableBuilder::jec2jem(const EdgeCoordinate &jec) {

  uint16_t regPosition = sec2rec[jec.position];
  uint16_t mask0 = c12_4.position2mask[regPosition / C_8_4];
  uint16_t mask1 = c8_4.position2mask[regPosition % C_8_4];

  uint16_t valid = ~mask0 & ((1 << 12) - 1);
  uint16_t face = restoreMask(mask0, mask1);
  uint16_t flip = restoreMask(mask0, jec.flip);

  return {valid, face, flip};
}

// convert from a Janus edge mask to a Janus edge coordinate
EdgeCoordinate MoveTableBuilder::jem2jec(const EdgeMask &jem,
                                         uint8_t &permNeeded) {
  uint16_t mask0 = ((1 << 12) - 1) & ~jem.valid;
  uint16_t mask1 = removeMask(mask0, jem.face);
  uint16_t position =
      c12_4.mask2position[mask0] * C_8_4 + c8_4.mask2position[mask1];
  uint16_t entry = rec2sec[position];
  permNeeded = entry >> 12;

  EdgeMask pjem = jem.permute(permNeeded);
  uint16_t pMask0 = ((1 << 12) - 1) & ~pjem.valid;
  uint16_t pFlip = removeMask(pMask0, pjem.flip);
  uint16_t pMask1 = removeMask(pMask0, pjem.face);
  uint16_t pPosition =
      c12_4.mask2position[pMask0] * C_8_4 + c8_4.mask2position[pMask1];
  uint16_t pEntry = rec2sec[pPosition];
  uint16_t pSymPosition = pEntry & ((1 << 12) - 1);

  return {pSymPosition, pFlip};
}

// build table that performs specified permutation on the corners
void MoveTableBuilder::buildCornerPermuteTable(
    uint32_t (&cornerPermuteTable)[nJanusPerms][nSymCornerCoords]) {
  for (uint8_t position = 0; position < nSymCornerPositions; ++position) {
    for (uint16_t spin = 0; spin < nSymCornerSpins; ++spin) {

      CornerCoordinate jcc{position, spin};
      CornerMask jcm = jcc2jcm(jcc);
      uint32_t cidx = jcc.tableIndex();

      for (uint8_t perm = 0; perm < nJanusPerms; ++perm) {
        CornerCoordinate pjcc = jcm2jcc(jcm.permute(perm));
        cornerPermuteTable[perm][cidx] = pjcc.tableIndex();
      }
    }
  }
}

// build table that performs specified twist on the corners
void MoveTableBuilder::buildCornerTwistTable(
    uint32_t (&cornerTwistTable)[nTwistsPerMove][nSymCornerCoords]) {
  for (uint8_t position = 0; position < nSymCornerPositions; ++position) {
    for (uint16_t spin = 0; spin < nSymCornerSpins; ++spin) {

      CornerCoordinate jcc{position, spin};
      CornerMask jcm = jcc2jcm(jcc);
      uint32_t cidx = jcc.tableIndex();

      for (uint8_t twist = 0; twist < nTwistsPerMove; ++twist) {
        CornerCoordinate pjcc = jcm2jcc(jcm.move(twist));
        cornerTwistTable[twist][cidx] = pjcc.tableIndex();
      }
    }
  }
}

// build table that performs specified permutation on the edges
void MoveTableBuilder::buildEdgePermuteTable(
    uint32_t (&edgePermuteTable)[nJanusPerms][nSymEdgeCoords]) {
  for (uint16_t position = 0; position < nSymEdgePositions; ++position) {
    for (uint16_t flip = 0; flip < nEdgeFlips; ++flip) {

      EdgeCoordinate jec{position, flip};
      EdgeMask jem = jec2jem(jec);
      uint32_t eidx = jec.tableIndex();

      for (uint8_t perm = 0; perm < nJanusPerms; ++perm) {
        uint8_t permNeeded;
        EdgeCoordinate pjec = jem2jec(jem.permute(perm), permNeeded);
        edgePermuteTable[perm][eidx] = pjec.tableIndex();
      }
    }
  }
}

// build table that performs specified twist on the edges
// the permutation needed to rotate the cube to the new sym edge coordinate
// is also returned.
void MoveTableBuilder::buildEdgeTwistTable(
    uint32_t (&edgeTwistTable)[nTwistsPerMove][nSymEdgeCoords]) {
  for (uint16_t position = 0; position < nSymEdgePositions; ++position) {
    for (uint16_t flip = 0; flip < nEdgeFlips; ++flip) {

      EdgeCoordinate jec{position, flip};
      EdgeMask jem = jec2jem(jec);
      uint32_t eidx = jec.tableIndex();

      for (uint8_t twist = 0; twist < nTwistsPerMove; ++twist) {
        uint8_t permNeeded;
        EdgeCoordinate mjec = jem2jec(jem.move(twist), permNeeded);

        edgeTwistTable[twist][eidx] = (mjec.tableIndex() << 4) + permNeeded;
      }
    }
  }
}

// convenience functions to convert
//   axis/pole notation to/from symmetry

// axis/pole numbering:
// xyz
// 012
// 021
// 102
// 120
// 201
// 210

struct AxesPole {
  uint8_t axis[3];
  uint8_t pole[3];
};

static AxesPole symmetry2axesPole(uint8_t symmetry) {
  AxesPole ap;
  ap.axis[0] = symmetry >> 4;
  ap.axis[1] = (symmetry >> 3) & 1;
  ap.axis[1] += ap.axis[1] >= ap.axis[0];
  ap.axis[2] = (ap.axis[0] | ap.axis[1]) ^ 0x3;

  uint8_t pole = symmetry & 0x07;
  ap.pole[0] = pole >> 2;
  ap.pole[1] = (pole >> 1) & 1;
  ap.pole[2] = pole & 1;

  return ap;
}

static uint8_t axesPole2symmetry(const AxesPole &ap) {
  uint8_t symmetry = 2 * ap.axis[0] + (ap.axis[2] < ap.axis[1]);
  symmetry = (symmetry << 1) | ap.pole[0];
  symmetry = (symmetry << 1) | ap.pole[1];
  symmetry = (symmetry << 1) | ap.pole[2];

  return symmetry;
}

// builds table used to permute a symmetry
void MoveTableBuilder::buildSymmetryPermuteTable(
    uint8_t (&symmetryPermuteTable)[nJanusPerms][nCubeSyms]) {

  // for each of the 48 possible cube symmetries
  for (uint8_t symmetry = 0; symmetry < nCubeSyms; ++symmetry) {
    const auto ap = symmetry2axesPole(symmetry);

    // apply each of the 16 possible janus permutations
    for (uint8_t jperm = 0; jperm < nJanusPerms; ++jperm) {

      AxesPole jap = ap;

      // reflect about 0-1 plane
      if (jperm & 0x08) {
        jap.pole[2] ^= 1;
      }

      // rotate 180 degrees about 0 axis
      if (jperm & 0x04) {
        jap.pole[1] ^= 1;
        jap.pole[2] ^= 1;
      }

      // rotate 180 degrees about 2 axis
      if (jperm & 0x02) {
        jap.pole[0] ^= 1;
        jap.pole[1] ^= 1;
      }

      // rotate 90 degrees about 2 axis
      if (jperm & 0x01) {
        std::swap(jap.axis[0], jap.axis[1]);
        std::swap(jap.pole[0], jap.pole[1]);
        jap.pole[0] ^= 1;
      }

      uint8_t newSymmetry = axesPole2symmetry(jap);

      symmetryPermuteTable[jperm][symmetry] = newSymmetry;
    }
  }
}

// builds table that converts a twist to the specified symmetry
void MoveTableBuilder::buildTwistSymmetryTable(
    uint8_t (&twistSymmetryTable)[nCubeSyms][nTwistsPerMove]) {

  for (uint8_t symmetry = 0; symmetry < nCubeSyms; ++symmetry) {
    const auto ap = symmetry2axesPole(symmetry);

    for (uint8_t twist = 0; twist < nTwistsPerMove; ++twist) {
      uint8_t twistAxis = twist % 3;       // 0 = x, 1 = y, 2 = z
      uint8_t twistPole = (twist % 6) > 2; // 0 = FRU, 1 = BLD
      uint8_t twistDir = twist / 6;        // 0=cw, 1=ccw, 2=2x

      twistAxis = (twistAxis + 1 + ap.axis[2]) % 3;
      uint8_t newTwistAxis = (2 - ap.axis[2] + ap.axis[twistAxis]) % 3;

      uint8_t newTwistPole = twistPole ^ ap.pole[newTwistAxis];
      uint8_t newTwistDir =
          twistDir == 2 ? twistDir
                        : twistDir ^ (ap.axis[0] % 3 == (ap.axis[1] + 1) % 3) ^
                              ap.pole[0] ^ ap.pole[1] ^ ap.pole[2];

      auto newTwist = newTwistAxis + newTwistPole * 3 + newTwistDir * 6;

      twistSymmetryTable[symmetry][twist] = newTwist;
    }
  }
}

} // namespace Janus
