// Copyright (C) 2021 Greg Dionne
// Distributed under MIT License
#ifndef JANUS_CUBEINDEX_HPP
#define JANUS_CUBEINDEX_HPP

#include "index.hpp"

namespace Janus {

// A cube index is used by the solver to determine
// when each of the three Janus indices are solved.

struct CubeIndex {
  Index x; // front-back
  Index y; // right-left
  Index z; // up-down

  // returns true if all Janus indices are solved
  bool isSolved() const { return x.isSolved() && y.isSolved() && z.isSolved(); }

  // returns the "home" or solved state for each of the
  // x, y, and z axes.
  static CubeIndex home() {
    return {Index::home(32), Index::home(24), Index::home(0)};
  }
};

} // namespace Janus
#endif
