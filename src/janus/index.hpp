// Copyright (C) 2021 Greg Dionne
// Distributed under MIT License
#ifndef JANUS_INDEX_HPP
#define JANUS_INDEX_HPP

#include <cstdint>

namespace Janus {

// corner index where corners are in "home" position with zero spin
constexpr uint32_t homeCornerIndex = 20;

// edge index where edges are in "home" state with zero flips
constexpr uint32_t homeEdgeIndex = 2224 << 8;

// index used to examine the depth table
// and inform how it has been rotated/reflected from
// its default orientation

struct Index {

  // see the corresponding .hpp file for
  // more information on a corner coordinate
  uint32_t corners;

  // see the corresponding .hpp file for
  // more information on an edge coordinate
  uint32_t edges;

  // The 48 symmetries are ennumerated via
  // a lehmer code of the six possible
  // axes orientations:
  //     xyz
  //   0 012
  //   1 021
  //   2 102
  //   3 120
  //   4 201
  //   5 210
  // followed by a 3-bit reflection mask
  // of the form XYZ, where a 1 indicates
  // reflection of the corresponding axes,
  // and a 0 indicates no reflection.
  //
  // symm = (lehmer << 3) + reflectionMask.
  uint8_t symmetry;

  // returns true when the Janus coordinate is solved.
  //   true when edges and corners are in home position
  //   and when the "z" symmetry reflection bit is unset.
  //
  //   An unset "z" bit prevents the case where the two
  //   faces of each Janus (corners and edges) are aligned
  //   with the wrong nose (center piece).
  bool isSolved() const {
    return corners == homeCornerIndex && edges == homeEdgeIndex &&
           (symmetry & 1) == 0;
  }

  // "solved" or "home" coordinate
  static Index home(uint8_t symmetry) {
    return {homeCornerIndex, homeEdgeIndex, symmetry};
  }
};

} // namespace Janus
#endif
