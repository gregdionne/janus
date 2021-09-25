// Copyright (C) 2021 Greg Dionne
// Distributed under MIT License
#ifndef JANUS_MOVETABLE_HPP
#define JANUS_MOVETABLE_HPP

#include "choosetable.hpp"
#include "constants.hpp"
#include "cubeindex.hpp"

namespace Janus {

class MoveTable {
public:
  MoveTable() { buildMoveTables(); }

  CubeIndex move(const CubeIndex &cube, uint8_t twist) const {
    return {move(cube.x, twist), move(cube.y, twist), move(cube.z, twist)};
  }

  // tables perform the twist in a Janus with both
  // permutation=0 and symmetry=0
  //
  //   corner twist table returns a corner index
  uint32_t cornerTwistTable[nTwistsPerMove][nSymCornerCoords];

  //   edge twist table returns a (permuted) edge index
  //   shifted left by four bits.  the permutation needed
  //   is encoded in the lower four bits
  uint32_t edgeTwistTable[nTwistsPerMove][nSymEdgeCoords];

  // tables that perform a permutation on the specified
  // corners, edges, and symmetries
  uint32_t cornerPermuteTable[nJanusPerms][nSymCornerCoords];
  uint32_t edgePermuteTable[nJanusPerms][nSymEdgeCoords];
  uint8_t symmetryPermuteTable[nJanusPerms][nCubeSyms];

  // table that transforms a twist in the cube frame
  // to a Janus' local symmetry frame
  uint8_t twistSymmetryTable[nCubeSyms][nTwistsPerMove];

  // For edge positions with 2-, 4-, 8- fold symmetry, more
  // than one permutation results in the same edge index.
  // We use this table to make sure corners and edge
  // flips that don't share the edge position symmetry can
  // be reached when incrementally expanding the depth table.
  std::vector<uint8_t> equivalentEdgePermutationTable[nSymEdgePositions];

private:
  void buildMoveTables();

  // perform a move on a Janus index:
  Index move(const Index &janus, uint8_t twist) const {

    // transform the twist into the local frame of the Janus
    twist = twistSymmetryTable[janus.symmetry][twist];

    // perform the transformed twist on the indices
    uint32_t cvalue = cornerTwistTable[twist][janus.corners];
    uint32_t evalue = edgeTwistTable[twist][janus.edges];

    // get resulting edge index and permutation
    uint32_t eidx = evalue >> 4;
    uint8_t permNeeded = evalue & 0x0f;

    // perform needed permutation on the corner and symmetry
    uint32_t cidx = cornerPermuteTable[permNeeded][cvalue];
    uint8_t symmetry = symmetryPermuteTable[permNeeded][janus.symmetry];

    return {cidx, eidx, symmetry};
  }
};

} // namespace Janus

#endif
