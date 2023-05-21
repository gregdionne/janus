// Copyright (C) 2021 Greg Dionne
// Distributed under MIT License
#include "fullcube.hpp"
#include "cornertwist.hpp"
#include "edgetwist.hpp"

namespace Janus {

void FullCube::clear() {
  for (int i = 0; i < nCorners; ++i) {
    cornerPositions[i] = i;
    cornerSpins[i] = 0;
  }

  for (int i = 0; i < nEdges; ++i) {
    edgePositions[i] = i;
    edgeFlips[i] = 0;
  }
}

FullCube FullCube::moveQuarterTwist(uint8_t twist) const {

  FullCube fullCube;

  for (int i = 0; i < nCorners; ++i) {
    CornerReturn cr = quarterTwistCorner(cornerPositions[i], twist);
    fullCube.cornerPositions[i] = cr.position;
    fullCube.cornerSpins[i] = (cr.spin + cornerSpins[i]) % 3;
  }

  for (int i = 0; i < nEdges; ++i) {
    EdgeReturn er = quarterTwistEdge(edgePositions[i], twist);
    fullCube.edgePositions[i] = er.position;
    fullCube.edgeFlips[i] = er.flip ^ edgeFlips[i];
  }

  return fullCube;
}

FullCube FullCube::move(uint8_t twist) const {
  return twist < nQuarterTwists
             ? moveQuarterTwist(twist)
             : moveQuarterTwist(twist % 6).moveQuarterTwist(twist % 6);
}

bool FullCube::isSolved() const {
  for (int i = 0; i < nEdges; ++i) {
    if (edgePositions[i] != i || edgeFlips[i] != 0) {
      return false;
    }
  }

  for (int i = 0; i < nCorners; ++i) {
    if (cornerPositions[i] != i || cornerSpins[i] != 0) {
      return false;
    }
  }

  return true;
}

} // namespace Janus
