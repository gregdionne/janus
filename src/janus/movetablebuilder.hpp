// Copyright (C) 2021 Greg Dionne
// Distributed under MIT License
#ifndef JANUS_MOVETABLEBUILDER
#define JANUS_MOVETABLEBUILDER

#include "choosetable.hpp"
#include "constants.hpp"
#include "cornercoordinate.hpp"
#include "edgecoordinate.hpp"
#include "mask.hpp"

namespace Janus {

class MoveTableBuilder {
public:
  // Builds rec2sec and sec2rec
  MoveTableBuilder();

  // Tables used for building the depth table

  // Builds table that performs whole-cube rotation/inversion
  // of the edges for the given symmetry.
  void buildEquivalentEdgePermutationTable(std::vector<uint8_t> (
      &equivalentEdgePermutationTable)[nSymEdgePositions]);

  // Builds the table that performs whole-cube rotation/inversion
  // for the specified permutation.
  void buildEdgePermuteTable(
      uint32_t (&edgePermuteTable)[nJanusPerms][nSymEdgeCoords]);

  // Five tables are used by MoveTable during solves

  // Builds table that performs whole-cube rotation/inversion
  // of the corners for the given edge permutation bit encoding
  // (Z*, X2, Z2, Z).
  void buildCornerPermuteTable(
      uint32_t (&cornerPermuteTable)[nJanusPerms][nSymCornerCoords]);

  // Builds table that performs the specified twist for the
  // given symmetry.
  void buildCornerTwistTable(
      uint32_t (&cornerTwistTable)[nTwistsPerMove][nSymCornerCoords]);

  // Builds the table that performs the specified twist
  // on the edges.  it also holds the permutation required to
  // move the corners.
  void buildEdgeTwistTable(
      uint32_t (&edgeTwistTable)[nTwistsPerMove][nSymEdgeCoords]);

  // builds the table needed to permute the current symmetry
  void buildSymmetryPermuteTable(
      uint8_t (&symmetryPermuteTable)[nJanusPerms][nCubeSyms]);

  // builds the table needed to convert a twist to the specified
  // symmetry.
  void buildTwistSymmetryTable(
      uint8_t (&twistSymmetryTable)[nCubeSyms][nTwistsPerMove]);

private:
  // conversions between corner bitmasks and coordinates
  CornerCoordinate jcm2jcc(const CornerMask &jcm);
  CornerMask jcc2jcm(const CornerCoordinate &jcc);

  // conversions between edge bitmasks and coordinates
  EdgeCoordinate jem2jec(const EdgeMask &jem, uint8_t &permNeeded);
  EdgeMask jec2jem(const EdgeCoordinate &jec);

  // conversions between edge bitmask and position (without flip)
  EdgeMask pos2jem(uint16_t regPosition);
  uint16_t jem2pos(EdgeMask pem);

  // conversions between "regular" and "symmetric" position coordinates
  uint16_t rec2sec[nRegEdgePositions]; // C(12,4) * C(8,4) three sets of
                                       // identical edge cubies
  uint16_t sec2rec[nSymEdgePositions]; // same as above, but excluding rotations
                                       // and reflections

  // tables to convert between masks and coordinates
  ChooseTable c12_4 = ChooseTable(12, 4);
  ChooseTable c8_4 = ChooseTable(8, 4);
};

} // namespace Janus
#endif
