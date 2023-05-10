// Copyright (C) 2022 Greg Dionne
// Distributed under MIT License
#ifndef JANUS_CUBEDEPTH_HPP
#define JANUS_CUBEDEPTH_HPP

#include <cstdint>

namespace Janus {

// A depth is used by the solver to keep track of
// how many twists each janus is from the home state

struct CubeDepth {
  uint8_t x; // front-back
  uint8_t y; // right-left
  uint8_t z; // up-down

  // returns the "home" or solved depth for each of the
  // x, y, and z axes.
  static CubeDepth home() { return {0, 0, 0}; }

  // return a depth adjusted by the new modulo-three
  // values from the depth table
  struct CubeDepth redepth(uint8_t X, uint8_t Y, uint8_t Z) const {
    return {static_cast<uint8_t>(x + 1 - (x + 1 - X) % 3),
            static_cast<uint8_t>(y + 1 - (y + 1 - Y) % 3),
            static_cast<uint8_t>(z + 1 - (z + 1 - Z) % 3)};
  }

  // returns true if the cube can't be solved within the specified depth
  bool tooFar(uint8_t depth) const {

    // prune if any Janus has exceeded the current depth
    // if all depths are the same value, then the effective depth is one greater
    // (Michael de Bondt's optimization)

    return ((x > depth) || (y > depth) || (z > depth) ||
            (x == y && y == z && x != 0 && x + 1 > depth));
  }
};

} // namespace Janus
#endif
