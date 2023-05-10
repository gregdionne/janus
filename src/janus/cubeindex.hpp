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
};

} // namespace Janus
#endif
