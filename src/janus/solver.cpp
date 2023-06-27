// Copyright (C) 2021-2022 Greg Dionne
// Distributed under MIT License
#include "solver.hpp"

#include <future>
#include <thread>
#include <utility>

namespace Janus {

// optimally solve the specified cube and return the solutions
// invoke user's callback when any solution is found
void Solver::solve(
    uint8_t cParity, const JanusCube &janusCube, const FullCube &startingCube,
    std::function<void(uint8_t)> depthCallback,
    std::function<void(std::size_t, const Solution &)> slnCallback,
    std::function<void(bool)> terminationCallback, bool asynchronously) {

  // copy over the starting cube
  startingFullCube = startingCube;

  // overwrite defaults with provided callbacks
  newDepthCallback = depthCallback;
  newSolutionCallback = slnCallback;
  searchTerminationCallback = terminationCallback;

  // cancel any solution in progress
  cancel();

  if (!asynchronously) {
    // wait for search to complete
    search(janusCube, cParity);
  } else {
    // search without waiting
    supervisor = std::thread(&Solver::search, this, janusCube, cParity);
  }
}

// cancel any solution in progress
void Solver::cancel() {

  // cancel any supervisor thread in progress
  if (supervisor.joinable()) {
    canceling = true;
    supervisor.join();
  }

  // clear any cancellation request
  canceling = false;
}

JanusCube Solver::move(const JanusCube &janusCube, uint8_t twist) const {
  CubeIndex trialCube = moveTable->move(janusCube.index, twist);
  CubeDepth trialDepth = redepth(janusCube.depth, trialCube);
  return {trialCube, trialDepth};
}

// returns an adjusted depth from the specified index
CubeDepth Solver::redepth(const CubeDepth &cDepth,
                          const CubeIndex &cIndex) const {

  return cDepth.redepth(janusDepth(cIndex.x), janusDepth(cIndex.y),
                        janusDepth(cIndex.z));
}

// return the depth for the specified janus index
uint8_t Solver::janusDepth(const Index &janus) const {

  // promote table indices to 64-bit and fetch depth
  std::size_t cidx = janus.corners;
  std::size_t eidx = janus.edges;

  return depthTable->getDepth(cidx, eidx);
}

// check the state of the cube index
// if solved, commit the solution and invoke any user callback
bool Solver::checkWork(const CubeIndex &cIndex, Solution &work) {

  if (isSolved(cIndex)) {
    auto fullCube = startingFullCube;

    for (auto &twist : work) {
      fullCube = fullCube.move(twist);
    }

    if (fullCube.isSolved()) {

      std::lock_guard<std::mutex> lock(solutionMutex);

      solutions.push_back(work);
      newSolutionCallback(solutions.size(), work);

      return true;
    }
  }

  return false;
}

bool Solver::recurseOne(const JanusCube &janusCube, uint8_t depth,
                        Solution &work, uint8_t twist,
                        bool (Solver::*f)(const JanusCube &janusCube,
                                          uint8_t depth, Solution &work)) {
  // record the move
  work.back() = twist;

  // make a trial cube with the move
  JanusCube trialCube = move(janusCube, twist);

  return (this->*f)(trialCube, depth - 1, work);
}

bool Solver::recurseTwo(const JanusCube &janusCube, uint8_t depth,
                        Solution &work, uint8_t twist,
                        bool (Solver::*f)(const JanusCube &janusCube,
                                          uint8_t depth, Solution &work)) {
  // record the move
  work.back() = twist;

  // make a temp cube with the first quarter turn
  JanusCube tempCube = move(janusCube, twist - nQuarterTwists);

  // make a trial cube with the second quarter turn
  JanusCube trialCube = move(tempCube, twist - nQuarterTwists);

  return (this->*f)(trialCube, depth - 2, work);
}

// table solver.
// checks the table first to see if cube can be possibly solved
// in the number of moves allowed by the current depth.  If the
// position is too far, it exits early.  If the current depth
// is zero and the table is solved, it adds an empty solution
// to the solution list and invokes the newSolutionCallback
bool Solver::tableSolve(const JanusCube &janusCube, uint8_t depth,
                        Solution &work) {

  // leave if we can't satisfy the depth requirement
  if (janusCube.depth.tooFar(depth)) {
    return false;
  }

  // no more moves left?
  if (depth == 0) {
    return checkWork(janusCube.index, work);
  }

  return recurser->leaf(janusCube, depth, work, this, &Solver::tableSolve);
}

// solve by trial and error...
// 1.  generates a new move, taking care to avoid twisting the
//     same face (or the opposing face when the last move was
//     a B L or D twist).
// 2.  perform the generated move
// 3.  add the move to the working solution and
// 4.  call solve() with the new move and decremented depth.
bool Solver::trialSolve(const JanusCube &janusCube, uint8_t depth,
                        Solution &work) {

  // invoke table if within useful depth
  if (depth < usefulDepth) {
    return tableSolve(janusCube, depth, work);
  }

  // leave if canceling
  if (canceling) {
    return false;
  }

  return recurser->leaf(janusCube, depth, work, this, &Solver::trialSolve);
}

bool Solver::solveWorkList() {
  bool found = false;
  WorkItem item;
  while (!canceling && worklist.pop(item)) {
    found |= trialSolve(item.janusCube, item.depth, item.work);
  }
  return found && !canceling;
}

bool Solver::makeWorkList(const JanusCube &janusCube, uint8_t depth,
                          Solution &work) {

  if (depth < threadDepth) {
    worklist.push({janusCube, work, depth});
    return false;
  }

  return recurser->leaf(janusCube, depth, work, this, &Solver::makeWorkList);
}

bool Solver::rootTableSolve(const JanusCube &janusCube, uint8_t depth,
                            Solution &work) {

  return recurser->root(janusCube, depth, work, this, &Solver::tableSolve);
}

bool Solver::rootTrialSolve(const JanusCube &janusCube, uint8_t depth,
                            Solution &work) {

  return recurser->root(janusCube, depth, work, this, &Solver::trialSolve);
}

void Solver::rootMakeWorkList(const JanusCube &janusCube, uint8_t depth,
                              Solution &work) {

  recurser->root(janusCube, depth, work, this, &Solver::makeWorkList);
}

bool Solver::rootThreadSolve(const JanusCube &janusCube, uint8_t depth,
                             Solution &work) {

  rootMakeWorkList(janusCube, depth, work);

  std::vector<std::future<bool>> results(nRootThreads());

  for (auto &result : results) {
    result = std::async(&Solver::solveWorkList, this);
  }

  bool foundSolution = false;
  for (auto &result : results) {
    foundSolution |= result.get();
  }

  return foundSolution;
}

unsigned int Solver::nRootThreads() {
  // try OS first
  auto nThreads = std::thread::hardware_concurrency();

  // otherwise just do the default
  if (nThreads == 0) {
    nThreads = nDefaultThreads;
  }

  return nThreads;
}

// top-level solver
bool Solver::solve(const JanusCube &janusCube, uint8_t depth) {
  Solution work;

  return depth == 0             ? checkWork(janusCube.index, work)
         : depth <= usefulDepth ? rootTableSolve(janusCube, depth, work)
         : depth < threadDepth  ? rootTrialSolve(janusCube, depth, work)
                                : rootThreadSolve(janusCube, depth, work);
}

// search via iterative deepening and return the solutions
void Solver::search(const JanusCube &janusCube, uint8_t cParity) {

  // clear any prior solutions
  solutions.clear();
  worklist.clear();

  // if odd parity, need at least one face turn
  uint8_t depth = cParity;
  newDepthCallback(depth);

  // try solving the cube with a depth of zero, and gradually increment
  // the depth until we exceed God's number
  while (!solve(janusCube, depth) && !canceling && depth <= GodsNumber) {
    depth += depthIncrement;
    newDepthCallback(depth);
  }

  // invoke termination callback
  searchTerminationCallback(!canceling);
}

} // namespace Janus
