// Copyright (C) 2021 Greg Dionne
// Distributed under MIT License
#ifndef JANUS_FULLCUBE
#define JANUS_FULLCUBE

#include <cstdint>

namespace Janus {

// When using Janus without noses (enares),
// it is impossible to discern a solved cube
// from one that results in a 'four spot' pattern.
//
// The FullCube can keep track of the state of
// the entire cube.  It does so by keeping track
// of the 8 corners and 12 edges and their
// spins and flips, respectively.

class FullCube {
public:
  // perform the specified twist on the cube
  FullCube move(uint8_t twist) const;

  // return true if solved
  bool isSolved() const;

  // return a (new) unscrambled cube
  static FullCube home() {
    FullCube fullCube;
    fullCube.clear();
    return fullCube;
  }

private:
  // perhaps this should move elsewhere
  static const uint8_t nCorners = 8;
  static const uint8_t nEdges = 12;
  static const uint8_t nQuarterTwists = 12;

  // keep track of each corner position and its respective spin
  uint8_t cornerPositions[nCorners];
  uint8_t cornerSpins[nCorners];

  // keep track of each edge position and its respective flip
  uint8_t edgePositions[nEdges];
  uint8_t edgeFlips[nEdges];

  // moves the cube by a single twist
  FullCube moveQuarterTwist(uint8_t twist) const;

  // puts the cube in a solved state
  void clear();
};

} // namespace Janus

#endif
