#include "januscube.hpp"
#include "solver.hpp"

namespace Janus {

JanusCube JanusCube::home(const Solver *solver) {
  CubeIndex cubeIndex = solver->homeCube();
  CubeDepth cubeDepth = CubeDepth::home();
  return {cubeIndex, cubeDepth};
}

JanusCube JanusCube::move(const Solver *solver, const MoveTable *moveTable,
                          uint8_t twist) const {
  CubeIndex cubeIndex = moveTable->move(index, twist);
  CubeDepth cubeDepth = solver->redepth(depth, cubeIndex);
  return {cubeIndex, cubeDepth};
}

} // namespace Janus
