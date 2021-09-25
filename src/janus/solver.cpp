// Copyright (C) 2021 Greg Dionne
// Distributed under MIT License
#include "solver.hpp"

#include <future>
#include <mutex>

namespace Janus {

static std::mutex solutionMutex;

// optimally solve the specified cube index and return the solutions
std::vector<Solution> Solver::solve(const CubeIndex &cube) {
  uint8_t depth = 0;

  // clear any prior solutions
  solutions.clear();

  // be courteous in the event of failure
  if (!depthTable) {
    fprintf(stderr,
            "Depth table not allocated (do you have at least 45GB free?)\n");
    return solutions;
  }

  // try solving the cube with a depth of zero, and gradually increment
  // the depth until we exceed God's number
  while (!solve(cube, depth) && depth <= GodsNumber) {
    ++depth;
    fprintf(stderr, "Trying depth %i...\n", depth);
  }

  return solutions;
}

// optimally solve the specified cube and return the solutions
// invoke user's callback when any solution is found
std::vector<Solution>
Solver::solve(const CubeIndex &cube,
              std::function<void(int, Solution)> callback) {

  // if the user gave us a callback, overwrite the default
  newSolutionCallback = callback;

  // proceed onwards
  return solve(cube);
}

// return the depth for the specified janus index
uint8_t Solver::janusDepth(const Index &janus) const {

  // promote table indices to 64-bit and fetch depth
  std::size_t cidx = janus.corners;
  std::size_t eidx = janus.edges;

  return depthTable->getDepth(cidx, eidx);
}

// returns true if the depth table indicates that the cube
// can't be solved within the specified depth
bool Solver::tooFar(const CubeIndex &cube, uint8_t depth) const {

  // storage
  uint8_t xDepth;
  uint8_t yDepth;
  uint8_t zDepth;

  // prune if any Janus has exceeded the current depth
  // if all depths are the same value, then the effective depth is one greater
  // (Michael de Bondt's optimization)

  return (
      ((xDepth = janusDepth(cube.x)) > depth) ||
      ((yDepth = janusDepth(cube.y)) > depth) ||
      ((zDepth = janusDepth(cube.z)) > depth) ||
      (xDepth == yDepth && yDepth == zDepth && xDepth && xDepth + 1 > depth));
}

// top-level table solver.
// checks the table first to see if cube can be possibly solved
// in the number of moves allowed by the current depth.  If the
// position is too far, it exits early.  If the current depth
// is zero and the table is solved, it adds an empty solution
// to the solution list and invokes the newSolutionCallback
bool Solver::tableSolve(const CubeIndex &cube, uint8_t depth) {

  // leave if we can't satisfy the depth requirement
  if (tooFar(cube, depth)) {
    return false;
  }

  // still have more to go?
  if (depth > 0) {
    return trialSolve(cube, depth);
  }

  // generate null solution.
  Solution work;

  // prune out any four-spot patterns
  if (!cube.isSolved()) {
    return false;
  }

  // commit the solution and invoke any user callback
  std::lock_guard<std::mutex> lock(solutionMutex);

  solutions.push_back(work);
  newSolutionCallback(solutions.size(), work);

  return true;
}

// checks the table first to see if cube can be possibly solved
// in the number of moves allowed by the current depth.  If the
// position is too far, it exits early.  If the current depth
// is zero and the table is solved, it adds any current work
// to the solution list and invokes the newSolutionCallback
bool Solver::tableSolve(const CubeIndex &cube, uint8_t depth, uint8_t lastTwist,
                        Solution &work) {

  // leave if we can't satisfy the depth requirement
  if (tooFar(cube, depth)) {
    return false;
  }

  // still have more to go?
  if (depth > 0) {
    return trialSolve(cube, depth, lastTwist, work);
  }

  // prune out any four-spot patterns
  if (!cube.isSolved()) {
    // Ever see the movie "Face Off" starring Nicholas Cage and John Travolta?
    // Now imagine them back-to-back with their faces swapped except
    // for their noses.  Run away!!  Avert your eyes!!!  Because
    // that's EXACTLY what happened to two of the three Janus' coordinates
    return false;
  }

  // commit the working solution and invoke any user callback
  std::lock_guard<std::mutex> lock(solutionMutex);

  solutions.push_back(work);
  newSolutionCallback(solutions.size(), work);

  return true;
}

// thread entry point:
// 1.  initializes working solution with the move
// 2.  performs the specified twist and
// 3.  calls solve() with the last move and decremented depth
bool Solver::trialWorker(const CubeIndex &cube, uint8_t depth, uint8_t twist) {

  // create a blank solution
  Solution work;

  // record the first move
  work.push_back(twist);

  // make a new trial cube with our move
  CubeIndex trialCube = moveTable->move(cube, twist);

  // see if we can solve the trial cube in depth-1 moves, taking care to
  // avoid twisting the same face (or opposite face when a B, L, or D twist)
  return solve(trialCube, depth - 1, twist, work);
}

// kick off a trial worker for every starting move
// returns a boolean indicating if we found any solution
bool Solver::trialSolve(const CubeIndex &cube, uint8_t depth) {

  // std::execution::par and its ilk are not yet implemented in clang
  //
  // In the name of simplicity we'll make a thread for each possible
  // twist.  We beg power users with more than 18 cores for clemency.
  std::future<bool> solved[nTwistsPerMove];

  // kick off the threads
  for (uint8_t twist = 0; twist < nTwistsPerMove; ++twist) {
    solved[twist] = std::async(&Solver::trialWorker, this, cube, depth, twist);
  }

  bool foundSolution = false;

  // collate the results
  for (auto &twist : solved) {
    foundSolution |= twist.get();
  }

  return foundSolution;
}

// solve by trial and error...
// 1.  generates a new move, taking care to avoid twisting the
//     same face (or the opposing face when the last move was
//     a B L or D twist).
// 2.  perform the generated move
// 3.  add the move to the working solution and
// 4.  call solve() with the new move and decremented depth.
bool Solver::trialSolve(const CubeIndex &cube, uint8_t depth, uint8_t lastTwist,
                        Solution &work) {

  // push a dummy value onto our temporary move list
  work.push_back(0);

  // Expect failure
  bool foundSolution = false;

  // for each move
  for (uint8_t twist = 0; twist < nTwistsPerMove; ++twist) {

    // if it's not the same face twisted previously and it's not
    // a F, R or U twist immediately after a B, L or D twist, respectively
    if (lastTwist % 6 != twist % 6 && lastTwist % 3 != twist % 6) {

      // record the move
      work.back() = twist;

      // make a trial cube with the move
      CubeIndex trialCube = moveTable->move(cube, twist);

      // see if we can solve the trial cube in depth-1 moves, taking care
      // to avoid twisting the same face (or opposing face when a B L or D
      // twist).
      foundSolution |= solve(trialCube, depth - 1, twist, work);
    }
  }

  // backtrack
  work.pop_back();

  return foundSolution;
}

// top-level solver entry point
bool Solver::solve(const CubeIndex &cube, uint8_t depth) {

  // only check the table if we're likely to get something useful
  return (depth <= usefulDepth) ? tableSolve(cube, depth)
                                : trialSolve(cube, depth);
}

// solver entry point from a trial solution
bool Solver::solve(const CubeIndex &cube, uint8_t depth, uint8_t lastTwist,
                   Solution &work) {

  // only check the table if we're likely to get something useful
  return (depth <= usefulDepth) ? tableSolve(cube, depth, lastTwist, work)
                                : trialSolve(cube, depth, lastTwist, work);
}

} // namespace Janus
