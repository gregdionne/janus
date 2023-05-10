#include "recurser.hpp"
#include "constants.hpp"
#include "solver.hpp"

namespace Janus {

std::unique_ptr<Recurser> Recurser::makeRecurser(MoveMetric moveMetric) {
  return moveMetric == MoveMetric::QuarterTurn
             ? std::unique_ptr<Recurser>(std::make_unique<RecurserQTM>())
             : std::unique_ptr<Recurser>(std::make_unique<RecurserFTM>());
}

bool RecurserFTM::leaf(const CubeIndex &cIndex, const CubeDepth &cDepth,
                       uint8_t depth, Solution &work, Solver *solver,
                       bool (Solver::*f)(const CubeIndex &cIndex,
                                         const CubeDepth &cDepth, uint8_t depth,
                                         Solution &work)) {

  // fetch last move
  uint8_t lastTwist = work.back();

  // Expect failure
  bool foundSolution = false;

  // push a dummy value onto our temporary move list
  work.push_back(0);

  // for each move
  for (uint8_t twist = 0; twist < nFaceTwists; ++twist) {

    // if it's not the same face twisted previously and it's not
    // a F, R or U twist immediately after a B, L or D twist, respectively
    if (lastTwist % 6 != twist % 6 && lastTwist % 3 != twist % 6) {

      foundSolution |=
          solver->recurseOne(cIndex, cDepth, depth, work, twist, f);
    }
  }

  // backtrack
  work.pop_back();

  return foundSolution;
}

bool RecurserFTM::root(const CubeIndex &cIndex, const CubeDepth &cDepth,
                       uint8_t depth, Solution &work, Solver *solver,
                       bool (Solver::*f)(const CubeIndex &cIndex,
                                         const CubeDepth &cDepth, uint8_t depth,
                                         Solution &work)) {

  // Expect failure
  bool foundSolution = false;

  // push a dummy value onto our temporary move list
  work.push_back(0);

  // for each move
  for (uint8_t twist = 0; twist < nFaceTwists; ++twist) {

    foundSolution |= solver->recurseOne(cIndex, cDepth, depth, work, twist, f);
  }

  // backtrack
  work.pop_back();

  return foundSolution;
}

bool RecurserQTM::leaf(const CubeIndex &cIndex, const CubeDepth &cDepth,
                       uint8_t depth, Solution &work, Solver *solver,
                       bool (Solver::*f)(const CubeIndex &cIndex,
                                         const CubeDepth &cDepth, uint8_t depth,
                                         Solution &work)) {

  // fetch last move
  uint8_t lastTwist = work.back();

  // Expect failure
  bool foundSolution = false;

  // push a dummy value onto our temporary move list
  work.push_back(0);

  // for each quarter twist
  for (uint8_t twist = 0; twist < nQuarterTwists; ++twist) {

    // if it's not the same face twisted previously and it's not
    // a F, R or U twist immediately after a B, L or D twist, respectively
    if (lastTwist % 6 != twist % 6 && lastTwist % 3 != twist % 6) {

      foundSolution |=
          solver->recurseOne(cIndex, cDepth, depth, work, twist, f);
    }
  }

  // for each half twist
  if (depth > 1) {
    for (uint8_t twist = nQuarterTwists; twist < nFaceTwists; ++twist) {

      // if it's not the same face twisted previously and it's not
      // a F, R or U twist immediately after a B, L or D twist, respectively
      if (lastTwist % 6 != twist % 6 && lastTwist % 3 != twist % 6) {

        foundSolution |=
            solver->recurseTwo(cIndex, cDepth, depth, work, twist, f);
      }
    }
  }

  // backtrack
  work.pop_back();

  return foundSolution;
}

bool RecurserQTM::root(const CubeIndex &cIndex, const CubeDepth &cDepth,
                       uint8_t depth, Solution &work, Solver *solver,
                       bool (Solver::*f)(const CubeIndex &cIndex,
                                         const CubeDepth &cDepth, uint8_t depth,
                                         Solution &work)) {

  // Expect failure
  bool foundSolution = false;

  // push a dummy value onto our temporary move list
  work.push_back(0);

  // for each quarter twist
  for (uint8_t twist = 0; twist < nQuarterTwists; ++twist) {

    foundSolution |= solver->recurseOne(cIndex, cDepth, depth, work, twist, f);
  }
  // for each half twist
  if (depth > 1) {
    for (uint8_t twist = nQuarterTwists; twist < nFaceTwists; ++twist) {
      foundSolution |=
          solver->recurseTwo(cIndex, cDepth, depth, work, twist, f);
    }
  }

  // backtrack
  work.pop_back();

  return foundSolution;
}

} // namespace Janus
