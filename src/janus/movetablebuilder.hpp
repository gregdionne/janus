// Copyright (C) 2021 Greg Dionne
// Distributed under MIT License
#ifndef JANUS_MOVETABLEBUILDER
#define JANUS_MOVETABLEBUILDER

#include "array2d.hpp"
#include "choosetable.hpp"
#include "clioptions.hpp"
#include "constants.hpp"
#include "cornercoordinate.hpp"
#include "edgecoordinate.hpp"
#include "mask.hpp"
#include "movetable.hpp"

#include <memory>
#include <vector>

namespace Janus {

class MoveTableBuilder {
public:
  // Builds rec2sec and sec2rec
  MoveTableBuilder(const CLIOptions &options);

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

  // Command-line options
  const CLIOptions &options;

  // conversions between corner bitmasks and coordinates
  CornerCoordinate jcm2jcc(const CornerMask &jcm);
  CornerMask jcc2jcm(const CornerCoordinate &jcc);

  // conversions between edge bitmasks and coordinates
  EdgeCoordinate jem2jec(const EdgeMask &jem, uint8_t &permNeeded);
  EdgeMask jec2jem(const EdgeCoordinate &jec);

  // conversions between edge bitmask and position (without flip)
  EdgeMask pos2jem(uint16_t regPosition);
  uint16_t jem2pos(EdgeMask pem);

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
  std::vector<uint16_t> sec2rec; // either 1171/2270 symmetric positions

  // tables to convert between masks and coordinates
  ChooseTable c12_4 = ChooseTable(12, 4);
  ChooseTable c8_4 = ChooseTable(8, 4);

  // number of permutation bits for the given "naso" of Janus
  // Only enares option uses bit 4.
  //   bit 4:  reflect along z axis (without colorswap)
  //   bit 3:  reflect along z axis (with colorswap)
  //   bit 2:  reflect along y axis
  //   bit 1:  rotate a half-turn around z axis
  //   bit 0:  rotate a quarter-turn around z axis
  uint8_t selectNEdgePermBits() {
    const uint8_t nEdgePermBitsA = 5;
    const uint8_t nEdgePermBitsD = 4;
    return options.enares.isEnabled() ? nEdgePermBitsA : nEdgePermBitsD;
  }

  const uint8_t nEdgePermBits;
  const uint8_t nJanusPerms;

  // when the enares option is used, only eleven bits
  // are needed to encode the 2,256 distinct symmetric edge
  // positions.  Otherwise, twelve bits are needed.
  const uint8_t nSymEdgePosBits;
  uint8_t selectNSymEdgePosBits() {
    const uint8_t nSymEdgePosBitsA = 11;
    const uint8_t nSymEdgePosBitsD = 12;
    return options.enares.isEnabled() ? nSymEdgePosBitsA : nSymEdgePosBitsD;
  }

  // number of symmetries
  constexpr static uint8_t nCubeSyms = 48;

  // 2^8 ways to orient (flip) the lower and upper edges
  // we don't track the orientations of the "missing" edges
  constexpr static uint16_t nEdges = 12;
  constexpr static uint16_t nEdgeFlips = 256;

  uint16_t nSymEdgePositions = 0; // sec2rec.size()
  uint32_t nSymEdgeCoords = 0;    // nSymEdgePositions * nEdgeFlips

  // corner index where corners are in "home" position with zero spin
  uint32_t homeCornerIndex = 0; // set from constructor? (value will be 20)

  // edge index where edges are in "root" position with no flips
  uint32_t homeEdgeIndex =
      0; // set in constructor? (value will be 2224 or 3496)
};

} // namespace Janus
#endif
