// Copyright (C) 2021 Greg Dionne
// Distributed under MIT License
#ifndef JANUS_CUBE
#define JANUS_CUBE

#include "solver.hpp"

namespace Janus {

class Cube {
public:
  // initialize tables and make a cube in it's solved state
  Cube() : cubeIndex(CubeIndex::home()) {
    moveTable = std::make_unique<MoveTable>();
    depthTable = std::make_unique<DepthTable>(moveTable.get());
    solver = std::make_unique<Solver>(moveTable.get(), depthTable.get());
  }

  // put the cube in the solved state
  void clear() { cubeIndex = CubeIndex::home(); }

  // perform the specified move on the cube
  void move(uint8_t twist) { cubeIndex = moveTable->move(cubeIndex, twist); }

  // solve the cube invoking the callback whenever a solution is found
  std::vector<Solution> solve(std::function<void(int, Solution)> callback) {
    return solver->solve(cubeIndex, callback);
  }

  // solve the cube without invoking any callbacks
  std::vector<Solution> solve() { return solver->solve(cubeIndex); }

private:
  // table for performing moves
  std::unique_ptr<MoveTable> moveTable;

  // pruning table for solver
  std::unique_ptr<DepthTable> depthTable;

  // solver
  std::unique_ptr<Solver> solver;

  // state of current cube
  CubeIndex cubeIndex;
};

} // namespace Janus

#endif
