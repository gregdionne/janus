// Copyright (C) 2021 Greg Dionne
// Distributed under MIT License
#ifndef JANUS_SOLVER_HPP
#define JANUS_SOLVER_HPP

#include "depthtable.hpp"
#include "movetable.hpp"

#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

namespace Janus {

typedef std::vector<uint8_t> Solution;

class Solver {
public:
  Solver(const MoveTable *jmt, const DepthTable *jdt)
      : moveTable(jmt), depthTable(jdt) {}

  // solve the cube!
  std::vector<Solution> solve(const CubeIndex &cc);

  // solve the cube, with a user callback whenever a new
  // solution is generated (useful for printing)
  std::vector<Solution> solve(const CubeIndex &cc,
                              std::function<void(int, Solution)> callback);

private:
  // retrieves the number of moves required to solve the
  // the specified Janus coordinate from the depth table
  uint8_t janusDepth(const Index &jc) const;

  // returns true if the depth table indicates that the cube
  // can't be solved within the specified depth
  bool tooFar(const CubeIndex &cube, uint8_t depth) const;

  // checks the table first to see if cube can be possibly solved
  // in the number of moves allowed by the current depth.  If the
  // position is too far, it exits early.  If the current depth
  // is zero and the table is solved, it adds any current work
  // to the solution list and invokes the newSolutionCallback
  bool tableSolve(const CubeIndex &cube, uint8_t depth);
  bool tableSolve(const CubeIndex &cube, uint8_t depth, uint8_t lastMove,
                  Solution &work);

  // thread entry point:
  // 1.  initializes working solution with the move
  // 2.  performs the specified twist and
  // 3.  calls solve() with the last move and decremented depth
  bool trialWorker(const CubeIndex &cube, uint8_t depth, uint8_t twist);

  // When called without a last move it kicks off one trial worker
  // for every starting move.  Otherwise it:
  // 1.  generates a new move, taking care to avoid twisting the
  //     same face (or the opposing face when the last move was
  //     a B L or D twist).
  // 2.  performs the generated move
  // 3.  adds the move to the working solution and
  // 4.  calls solve() with the new move and decremented depth.
  bool trialSolve(const CubeIndex &cube, uint8_t depth);
  bool trialSolve(const CubeIndex &cube, uint8_t depth, uint8_t lastMove,
                  Solution &work);

  // Solve the cube within the number of moves specified by depth.
  // it calls tableSolve if the depth is low enough to be within the
  // useful table depth, otherwise it calls trial solve
  bool solve(const CubeIndex &cube, uint8_t depth);
  bool solve(const CubeIndex &cube, uint8_t depth, uint8_t lastMove,
             Solution &work);

  // completed solution list
  std::vector<Solution> solutions;

  // tables
  const MoveTable *moveTable;
  const DepthTable *depthTable;

  // default callback when new solution is found
  std::function<void(int, Solution)> newSolutionCallback = [](int, Solution) {
    return;
  };
};

} // namespace Janus

#endif
