// Copyright (C) 2021 Greg Dionne
// Distributed under MIT License
#ifndef JANUS_MOVETABLEBUILDER
#define JANUS_MOVETABLEBUILDER

#include "array2d.hpp"
#include "choosetable.hpp"
#include "constants.hpp"
#include "cornercoordinate.hpp"
#include "edgecoordinate.hpp"
#include "mask.hpp"
#include "movetable.hpp"
#include "naso.hpp"

#include <memory>
#include <vector>

namespace Janus {

class MoveTableBuilder {
public:
  // Builds rec2sec and sec2rec
  MoveTableBuilder(Naso naso);

  // constructs and returns the move table
  std::unique_ptr<MoveTable> build();

private:
  // builds the (temporary) rec2sec and sec2rec tables required
  // to build the move table
  //
  // rec:  regular edge position coordinate
  // sec:  symmetricized edge position coordinate
  //       after removing permutations
  //
  // rec2sec[regEdgePosition]
  //  returns the symmetricized position in the lower 12/13 bits
  //  and the permutation required in the upper 4/3 bits.
  //
  // sec2rec[symEdgePosition]
  //  returns the regular position for the specified symmetricized
  //  edge position.  It is typically a stepping stone to create
  //  the position mask for an edge.
  void buildEdgePositionTables();

  // initializes the homeEdgeIndex and homeCornerIndex
  // to their "home" positions
  void initHomeIndices();

  // Builds table that performs whole-cube rotation/inversion
  // of the edges for the given symmetry.
  void buildEquivalentEdgePermutationTable(
      std::vector<std::vector<uint8_t>> &equivalentEdgePermutationTable);

  // Builds the table that performs whole-cube rotation/inversion
  // for the specified permutation.
  void buildEdgePermuteTable(Array2D<uint32_t> &edgePermuteTable);

  // Five tables are used by MoveTable during solves

  // Builds table that performs whole-cube rotation/inversion
  // of the corners for the given edge permutation bit encoding
  // (Z*, X2, Z2, Z).
  void buildCornerPermuteTable(Array2D<uint32_t> &cornerPermuteTable);

  // Builds table that performs the specified twist for the
  // given symmetry.
  void buildCornerTwistTable(Array2D<uint32_t> &cornerTwistTableX);

  // Builds the table that performs the specified twist
  // on the edges.  it also holds the permutation required to
  // move the corners.
  void buildEdgeTwistTable(Array2D<uint32_t> &edgeTwistTable);

  // builds the table needed to permute the current symmetry
  void buildSymmetryPermuteTable(Array2D<uint8_t> &symmetryPermuteTable);

  // builds the table needed to convert a twist to the specified
  // symmetry.
  void buildTwistSymmetryTable(Array2D<uint8_t> &twistSymmetryTable);

  // conversions between corner bitmasks and coordinates
  CornerCoordinate jcm2jcc(const CornerMask &jcm);
  CornerMask jcc2jcm(const CornerCoordinate &jcc);

  // conversions between edge bitmasks and coordinates
  EdgeCoordinate jem2jec(const EdgeMask &jem, uint8_t &permNeeded);
  EdgeMask jec2jem(const EdgeCoordinate &jec);

  // conversions between edge bitmask and position (without flip)
  EdgeMask pos2jem(uint16_t regPosition);
  uint16_t jem2pos(EdgeMask pem);

  Naso naso;

  constexpr static uint16_t nRegEdgePositions = C_12_4 * C_8_4;
  // conversions between "regular" and "symmetric" position coordinates
  //
  // rec2sec[regEdgePosition]
  //   holds the symmetricized position in the lower 12/13 bits
  //   and the permutation required in the upper 4/3 bits.
  uint16_t rec2sec[nRegEdgePositions]; // C(12,4) * C(8,4) three sets of
                                       // identical edge cubies
  //  sec2rec[symEdgePosition]
  //    returns the regular position for the specified symmetricized
  //    edge position.  It is typically a stepping stone to create
  //    the position mask for an edge.
  std::vector<uint16_t> sec2rec; // either 2256/4425 symmetric positions

  // tables to convert between masks and coordinates
  ChooseTable c12_4 = ChooseTable(12, 4);
  ChooseTable c8_4 = ChooseTable(8, 4);

  // number of permutation bits for the given "naso" of Janus
  //
  // The aequivalens variant allows for reflection about the z axis
  // independently of rotation of 180 degrees about the x axis.
  // The disparilis variant couples them together.
  constexpr static uint8_t nEdgePermBitsA = 4;
  constexpr static uint8_t nEdgePermBitsD = 3;
  const uint8_t nEdgePermBits;
  const uint8_t nJanusPerms;
  const uint8_t reflectBit;

  // when the aequivalens variant is used, only twelve bits
  // are needed to encode the 2,256 distinct symmetric edge
  // positions.  Otherwise, thirteen bits are needed to encode
  // when using the disparilis variant
  const uint8_t nSymEdgePosBits;
  constexpr static uint8_t nSymEdgePosBitsA = 12;
  constexpr static uint8_t nSymEdgePosBitsD = 13;

  // number of symmetries
  constexpr static uint8_t nCubeSyms = 48;

  // 2^8 ways to orient (flip) the lower and upper edges
  // we don't track the orientations of the "missing" edges
  constexpr static uint16_t nEdgeFlips = 256;

  uint16_t nSymEdgePositions = 0; // sec2rec.size()
  uint32_t nSymEdgeCoords = 0;    // nSymEdgePositions * nEdgeFlips

  // corner index where corners are in "home" position with zero spin
  uint32_t homeCornerIndex = 0; // set from constructor (value will be 20)

  // edge index where edges are in "root" position with no flips
  uint32_t homeEdgeIndex = 0; // set in constructor (value will be 2224 or 3496)
};

} // namespace Janus
#endif
