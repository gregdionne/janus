// Copyright (C) 2021 Greg Dionne
// Distributed under MIT License
#include "movetable.hpp"
#include "movetablebuilder.hpp"

namespace Janus {

void MoveTable::buildMoveTables() {
  MoveTableBuilder jt;
  jt.buildCornerPermuteTable(cornerPermuteTable);
  jt.buildCornerTwistTable(cornerTwistTable);
  jt.buildEdgeTwistTable(edgeTwistTable);
  jt.buildSymmetryPermuteTable(symmetryPermuteTable);
  jt.buildTwistSymmetryTable(twistSymmetryTable);
  jt.buildEquivalentEdgePermutationTable(equivalentEdgePermutationTable);
  jt.buildEdgePermuteTable(edgePermuteTable);
}

} // namespace Janus
