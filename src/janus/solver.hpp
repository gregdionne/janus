// Copyright (C) 2021-2022 Greg Dionne
// Distributed under MIT License
#ifndef JANUS_SOLVER_HPP
#define JANUS_SOLVER_HPP

#include "cubedepth.hpp"
#include "depthtable.hpp"
#include "fullcube.hpp"
#include "movetable.hpp"
#include "recurser.hpp"
#include "worklist.hpp"

#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace Janus {

class Solver {
public:
  Solver(const CLIOptions &options, const MoveTable *jmt,
         std::function<void(const std::string &)> console,
         std::function<bool(uint8_t *, std::size_t)> load,
         std::function<bool(const uint8_t *, std::size_t)> save)
      : moveTable(jmt), depthTable(std::make_unique<DepthTable>(
                            options, jmt, console, load, save)),
        recurser(Recurser::makeRecurser(options)),
        GodsNumber(selectGodsNumber(options)),
        usefulDepth(selectUsefulDepth(options)),
        depthIncrement(selectDepthIncrement(options)),
        homeCornerIndex(jmt->getHomeCornerIndex()),
        homeEdgeIndex(jmt->getHomeEdgeIndex()),
        homeCubeIndex({{homeCornerIndex, homeEdgeIndex, 32},
                       {homeCornerIndex, homeEdgeIndex, 24},
                       {homeCornerIndex, homeEdgeIndex, 0}}) {}

  // solve the cube, with a user callback whenever a new
  // solution is generated (useful for printing)
  void
  solve(const CubeIndex &cIndex, const CubeDepth &cDepth, uint8_t cParity,
        const FullCube &startingCube,
        std::function<void(uint8_t)> depthCallback,
        std::function<void(std::size_t, const Solution &)> solutionCallback,
        std::function<void(bool)> terminationCallback, bool asynchronously);

  // returns an adjusted depth from the specified index
  CubeDepth redepth(const CubeDepth &cDepth, const CubeIndex &cIndex) const;

  // cancel any solution in progress
  void cancel();

  // perform a single move...
  bool recurseOne(const CubeIndex &cIndex, const CubeDepth &cDepth,
                  uint8_t depth, Solution &work, uint8_t twist,
                  bool (Solver::*f)(const CubeIndex &cIndex,
                                    const CubeDepth &cDepth, uint8_t depth,
                                    Solution &work));

  // perform a half-twist counting each one twice...
  bool recurseTwo(const CubeIndex &cIndex, const CubeDepth &cDepth,
                  uint8_t depth, Solution &work, uint8_t twist,
                  bool (Solver::*f)(const CubeIndex &cIndex,
                                    const CubeDepth &cDepth, uint8_t depth,
                                    Solution &work));

  CubeIndex homeCube() const { return homeCubeIndex; }

  bool isSolved(const CubeIndex &cIndex) const {
    return cIndex.x.corners == homeCornerIndex && // check each corner
           cIndex.y.corners == homeCornerIndex && //
           cIndex.z.corners == homeCornerIndex && //
           cIndex.x.edges == homeEdgeIndex &&     // check each edge
           cIndex.y.edges == homeEdgeIndex &&     //
           cIndex.z.edges == homeEdgeIndex;       //
  }

private:
  // retrieves the number of moves required to solve the
  // the specified Janus coordinate from the depth table
  uint8_t janusDepth(const Index &janus) const;

  // check the state of the cube index
  // if solved, commit the solution and invoke any user callback
  bool checkWork(const CubeIndex &cIndex, Solution &work);

  // checks the table first to see if cube can be possibly solved
  // in the number of moves allowed by the current depth.  If the
  // position is too far, it exits early.  If the current depth
  // is zero, it checks the work and returns.
  // otherwise it
  // 1.  generates a new move, taking care to avoid twisting the
  //     same face (or the opposing face when the last move was
  //     a B L or D twist).
  // 2.  performs the generated move
  // 3.  adds the move to the working solution and
  // 4.  calls itself with the new move and decremented depth.
  bool tableSolve(const CubeIndex &cIndex, const CubeDepth &cDepth,
                  uint8_t depth, Solution &work);

  // checks to see if it can be solved via the table, and calls
  // tableSolve if so.
  // otherwise it
  // 1.  generates a new move, taking care to avoid twisting the
  //     same face (or the opposing face when the last move was
  //     a B L or D twist).
  // 2.  performs the generated move
  // 3.  adds the move to the working solution and
  // 4.  calls itself with the new move and decremented depth.
  bool trialSolve(const CubeIndex &cIndex, const CubeDepth &cDepth,
                  uint8_t depth, Solution &work);

  // solve the work list
  bool solveWorkList();

  // Make the work list, adding to it when at the specified depth
  bool makeWorkList(const CubeIndex &cIndex, const CubeDepth &cDepth,
                    uint8_t depth, Solution &work);

  void rootMakeWorkList(const CubeIndex &cIndex, const CubeDepth &cDepth,
                        uint8_t depth, Solution &work);

  // Solve the cube within the number of moves specified by depth.
  bool rootTableSolve(const CubeIndex &cIndex, const CubeDepth &cDepth,
                      uint8_t depth, Solution &work);
  bool rootTrialSolve(const CubeIndex &cIndex, const CubeDepth &cDepth,
                      uint8_t depth, Solution &work);
  bool rootThreadSolve(const CubeIndex &cIndex, const CubeDepth &cDepth,
                       uint8_t depth, Solution &work);

  // Fetches number of threads to use when solving
  static unsigned int nRootThreads();

  // top-level solver
  //   rootTableSolve is invoked if depth is within the useful table depth
  //   rootTrialSolve is otherwise invoked if too small for multithreading
  //   rootThreadSolve is invoked if big enough to do threading
  bool solve(const CubeIndex &cIndex, const CubeDepth &cDepth, uint8_t depth);

  // Search the cube incrementing from a depth of zero to God's number
  void search(CubeIndex cIndex, CubeDepth cDepth, uint8_t cParity);

  // tables
  const MoveTable *moveTable;
  const std::unique_ptr<DepthTable> depthTable;
  const std::unique_ptr<Recurser> recurser;

  const uint8_t GodsNumber;
  uint8_t selectGodsNumber(const CLIOptions &options) {
    const uint8_t GodsNumberQTM = 26;
    const uint8_t GodsNumberFTM = 20;
    return options.qtm.isEnabled() ? GodsNumberQTM : GodsNumberFTM;
  }

  const uint8_t usefulDepth;
  uint8_t selectUsefulDepth(const CLIOptions &options) {
    const uint8_t usefulDepthQTME = 13;
    const uint8_t usefulDepthFTME = 12;
    const uint8_t usefulDepthQTM = 14;
    const uint8_t usefulDepthFTM = 13;
    return options.enares.isEnabled()
               ? options.qtm.isEnabled() ? usefulDepthQTME : usefulDepthFTME
           : options.qtm.isEnabled() ? usefulDepthQTM
                                     : usefulDepthFTM;
  }

  const uint8_t depthIncrement;
  uint8_t selectDepthIncrement(const CLIOptions &options) {
    const uint8_t depthIncrementQTM = 2;
    const uint8_t depthIncrementFTM = 1;
    return options.qtm.isEnabled() ? depthIncrementQTM : depthIncrementFTM;
  }

  // number of threads to use if std::thread::hardware_concurrency() returns 0
  constexpr static uint8_t nDefaultThreads = 18;

  // depth at which threads should be launched
  constexpr static uint8_t threadDepth = 16;

  // completed solution list
  std::mutex solutionMutex;
  std::vector<Solution> solutions;

  // cancellation request semaphore
  std::thread supervisor;
  bool canceling = false;

  // callback when new depth is reached
  std::function<void(uint8_t)> newDepthCallback = [](uint8_t) {};

  // callback when new solution is found
  std::function<void(std::size_t, const Solution &)> newSolutionCallback =
      [](std::size_t, const Solution &) {};

  // callback when solver has completed
  std::function<void(std::size_t)> searchTerminationCallback = [](bool) {};

  // the (fully specified) initial cube to solve
  // this is useful under the "enares" condition where
  // the noses of each Janus are missing.  Since the
  // solver can't tell the difference between a solved
  // state and a four-spot pattern without noses, something
  // else must keep track of the full state of the cube
  FullCube startingFullCube = FullCube::home();

  WorkList worklist;

  const uint32_t homeCornerIndex;
  const uint32_t homeEdgeIndex;
  const CubeIndex homeCubeIndex;
};

} // namespace Janus

#endif
