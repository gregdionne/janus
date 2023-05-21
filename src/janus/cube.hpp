// Copyright (C) 2021-2022 Greg Dionne
// Distributed under MIT License
#ifndef JANUS_CUBE
#define JANUS_CUBE

#include <utility>

#include "fullcube.hpp"
#include "movetablebuilder.hpp"
#include "solver.hpp"

namespace Janus {

class Cube {
public:
  // initialize tables and make a cube in it's solved state
  Cube(const CLIOptions &options,
       std::function<void(const std::string &)> console,
       std::function<bool(uint8_t *, std::size_t)> load,
       std::function<bool(const uint8_t *, std::size_t)> save)
      : moveTable(MoveTableBuilder(options).build()),
        solver(std::make_unique<Solver>(options, moveTable.get(), console, load,
                                        save)),
        cubeIndex(solver->homeCube()), cubeDepth(CubeDepth::home()),
        cubeParity(0) {}

  // reset the cube to its initial state
  void reset() {
    solver->cancel();
    fullCube = FullCube::home();
    cubeIndex = solver->homeCube();
    cubeDepth = CubeDepth::home();
    cubeParity = 0;
  }

  // perform the specified move on the cube
  void move(uint8_t twist) {
    if (twist < nQuarterTwists) {
      fullCube = fullCube.move(twist);
      cubeIndex = moveTable->move(cubeIndex, twist);
      cubeDepth = solver->redepth(cubeDepth, cubeIndex);
      cubeParity ^= 1;
    } else {
      move(twist - nQuarterTwists);
      move(twist - nQuarterTwists);
    }
  }

  // solve the cube (asynchronously by default)
  // invoking callbacks whenever a new depth is searched
  // or a solution is found
  void
  solve(std::function<void(uint8_t)> depthCallback,
        std::function<void(std::size_t, const Solution &)> solutionCallback,
        std::function<void(bool)> finishedCallback, bool allowCancel = true) {
    solver->cancel();
    solver->solve(cubeIndex, cubeDepth, cubeParity, fullCube,
                  std::move(depthCallback), std::move(solutionCallback),
                  std::move(finishedCallback), allowCancel);
  }

private:
  // table for performing moves
  const std::unique_ptr<MoveTable> moveTable;

  // solver
  std::unique_ptr<Solver> solver;

  // state of current cube
  CubeIndex cubeIndex;

  // state of current depth
  CubeDepth cubeDepth;

  // full state of cube
  FullCube fullCube = FullCube::home();

  // state of parity
  uint8_t cubeParity;
};

} // namespace Janus

#endif
