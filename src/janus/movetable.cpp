// Copyright (C) 2021 Greg Dionne
// Distributed under MIT License
#include "movetable.hpp"
#include "movetablebuilder.hpp"

namespace Janus {

Index MoveTable::move(const Index &janus, uint8_t twist) const {

  // transform the twist into the local frame of the Janus
  twist = twistSymmetryTable(janus.symmetry, twist);

  // perform the transformed twist on the indices
  uint32_t cvalue = cornerTwistTable(twist, janus.corners);
  uint32_t evalue = edgeTwistTable(twist, janus.edges);

  // get resulting edge index and permutation
  uint32_t eidx = evalue >> nEdgePermBits;
  uint8_t permNeeded = evalue & edgePermMask;

  // perform needed permutation on the corner and symmetry
  uint32_t cidx = cornerPermuteTable(permNeeded, cvalue);
  uint8_t symmetry = symmetryPermuteTable(permNeeded, janus.symmetry);

  return {cidx, eidx, symmetry};
}

} // namespace Janus
