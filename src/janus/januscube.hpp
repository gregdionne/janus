#ifndef JANUS_JANUSCUBE
#define JANUS_JANUSCUBE

#include "cubedepth.hpp"
#include "cubeindex.hpp"
#include "movetable.hpp"

namespace Janus {

class Solver;

struct JanusCube {

  JanusCube move(const Solver *solver, const MoveTable *moveTable,
                 uint8_t twist) const;
  static JanusCube home(const Solver *solver);

  CubeIndex index;
  CubeDepth depth;
};

} // namespace Janus

#endif
