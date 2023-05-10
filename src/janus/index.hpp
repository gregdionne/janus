// Copyright (C) 2021 Greg Dionne
// Distributed under MIT License
#ifndef JANUS_INDEX_HPP
#define JANUS_INDEX_HPP

#include <cstdint>

namespace Janus {

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
};

} // namespace Janus
#endif
