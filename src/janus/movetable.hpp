// Copyright (C) 2021 Greg Dionne
// Distributed under MIT License
#ifndef JANUS_MOVETABLE_HPP
#define JANUS_MOVETABLE_HPP

#include "array2d.hpp"
#include "choosetable.hpp"
#include "constants.hpp"
#include "cornercoordinate.hpp" // only for nCornerCoords
#include "cubeindex.hpp"
#include "naso.hpp"

namespace Janus {

class MoveTable {
public:
  MoveTable(const Naso &janusNaso, uint8_t numJanusPerms,
            uint8_t numEdgePermBits, uint16_t numSymEdgePositions,
            uint32_t numSymEdgeCoords, uint8_t numCubeSyms, uint32_t homeCorner,
            uint32_t homeEdge)
      : cornerTwistTable(nFaceTwists, nCornerCoords),
        edgeTwistTable(nFaceTwists, numSymEdgeCoords),
        cornerPermuteTable(numJanusPerms, nCornerCoords),
        edgePermuteTable(numJanusPerms, numSymEdgeCoords),
        symmetryPermuteTable(numJanusPerms, numCubeSyms),
        twistSymmetryTable(numCubeSyms, nFaceTwists),
        equivalentEdgePermutationTable(numSymEdgePositions), naso(janusNaso),
        nSymEdgeCoords(numSymEdgeCoords),
        edgePermMask((1 << numEdgePermBits) - 1),
        nEdgePermBits(numEdgePermBits), homeCornerIndex(homeCorner),
        homeEdgeIndex(homeEdge) {}

  CubeIndex move(const CubeIndex &cube, uint8_t twist) const {
    return {move(cube.x, twist), move(cube.y, twist), move(cube.z, twist)};
  }

  // tables perform the twist in a Janus with both
  // permutation=0 and symmetry=0
  //
  //   corner twist table returns a corner index
  Array2D<uint32_t> cornerTwistTable;

  //   edge twist table returns a (permuted) edge index
  //   shifted left by four bits.  the permutation needed
  //   is encoded in the lower three or four bits
  Array2D<uint32_t> edgeTwistTable;

  // tables that perform a permutation on the specified
  // corners, edges, and symmetries
  Array2D<uint32_t> cornerPermuteTable;
  Array2D<uint32_t> edgePermuteTable;
  Array2D<uint8_t> symmetryPermuteTable;

  // table that transforms a twist in the cube frame
  // to a Janus' local symmetry frame
  Array2D<uint8_t> twistSymmetryTable;

  // For edge positions with 2-, 4-, 8- fold symmetry, more
  // than one permutation results in the same edge index.
  // We use this table to make sure corners and edge
  // flips that don't share the edge position symmetry can
  // be reached when incrementally expanding the depth table.
  std::vector<std::vector<uint8_t>> equivalentEdgePermutationTable;

  // used by depthtable
  Naso getNaso() const { return naso; }
  uint32_t getNSymEdgeCoords() const { return nSymEdgeCoords; }

  uint8_t getEdgePermMask() const { return edgePermMask; }
  uint8_t getNEdgePermBits() const { return nEdgePermBits; }

  uint32_t getHomeCornerIndex() const { return homeCornerIndex; }
  uint32_t getHomeEdgeIndex() const { return homeEdgeIndex; }

private:
  // perform a move on a Janus index:
  Index move(const Index &janus, uint8_t twist) const;

  // whether aequivalens or disparilis...
  const Naso naso;

  const uint32_t nSymEdgeCoords; // nSymEdgePositions * nEdgeFlips

  // Naso:                           Aequivalens Disparilis
  const uint8_t edgePermMask;     //    0x0f       0x07
  const uint8_t nEdgePermBits;    //       4          3
  const uint32_t homeCornerIndex; //      20         20
  const uint32_t homeEdgeIndex;   //    2224       3496
};

} // namespace Janus

#endif
