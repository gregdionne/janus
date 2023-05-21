// Copyright (C) 2021-2022 Greg Dionne
// Distributed under MIT License
#include "depthtable.hpp"
#include "bitutils.hpp"
#include "constants.hpp"
#include "index.hpp"
#include "strutils.hpp"

#include <future>
#include <mutex>
#include <numeric>
#include <vector>

namespace Janus {

// clear the table (single-threaded)
void DepthTable::clear() {

  for (std::size_t i = 0; i < nSymCoords / 4; ++i) {
    data[i] = 0xff;
  }
}

// recursively build the table from the specified coordinate and currentDepth
// down to the specified depth and mark any newly reached entries
std::size_t DepthTable::rbuild(const MoveTable *moveTable, std::size_t cidx,
                               std::size_t eidx, uint8_t depth,
                               uint8_t currentDepth) {

  std::size_t count = 0;

  std::size_t idx = fullIdx(cidx, eidx);
  uint8_t tableDepth = getDepth(idx);

  if (!currentDepth) {
    if (tableDepth == 0x3) {
      setDepthNonAtomically(idx, depth % 3);
      return 1;
    }

    // ensure expansion of edge positions with 2-, 4-, and 8-fold symmetry
    uint16_t eposition = eidx >> 8;
    for (const auto &p : moveTable->equivalentEdgePermutationTable[eposition]) {
      uint32_t epeidx = moveTable->edgePermuteTable(p, eidx);
      uint32_t epcidx = moveTable->cornerPermuteTable(p, cidx);

      std::size_t tidx = fullIdx(epcidx, epeidx);
      if (getDepth(tidx) == 0x03) {
        setDepthNonAtomically(tidx, depth % 3);
        ++count;
      }
    }

    return count;
  }

  // only recurse if we match the correct depth
  if (tableDepth == (depth - currentDepth) % 3) {

    // for every twist...
    for (int iTwist = 0; iTwist < nTwistsPerMove; ++iTwist) {

      // perform the twist on the corner and edge
      uint32_t tcidx = moveTable->cornerTwistTable(iTwist, cidx);
      uint32_t teidx = moveTable->edgeTwistTable(iTwist, eidx);

      // since the edge twist may result in a permutation
      // apply it to the corner
      uint8_t iPerm = teidx & edgePermMask;
      uint32_t peidx = teidx >> nEdgePermBits;
      uint32_t pcidx = moveTable->cornerPermuteTable(iPerm, tcidx);

      // recurse another depth with the permuted cube
      count += rbuild(moveTable, pcidx, peidx, depth, currentDepth - 1);
    }
  }
  return count;
}

// recursively build the table from the home coordinate down to the
// specified depth (single-threaded)
void DepthTable::altbuild(const MoveTable *moveTable, std::size_t cidx,
                          std::size_t eidx, uint8_t depth) {

  // update the table to the specified depth
  for (uint8_t pass = 1; pass <= depth; ++pass) {
    consoleOut("starting pass " + to_commastring(pass, 2) + "... ");

    std::size_t count = rbuild(moveTable, cidx, eidx, pass, pass);

    consoleOut(to_commastring(count, 14) + " positions generated\n");
  }
}

// search all entries within the specified edge index range for
// values that match the previous pass (depth) and mark any yet
// unreached entry one twist away with the current pass
std::size_t DepthTable::buildWorker(const MoveTable *moveTable, uint8_t pass,
                                    std::size_t start_eidx,
                                    std::size_t stop_eidx) {

  std::size_t count = 0;

  // search for positions that match the previous depth
  // and expand those that do
  for (std::size_t eidx = start_eidx; eidx < stop_eidx; ++eidx) {
    for (std::size_t cidx = 0; cidx < nCornerCoords; ++cidx) {

      // since we build the table one depth at a time
      // we need not perform a mutex read when finding
      // the previous depth
      if (getDepth(cidx, eidx) == (pass - 1) % 3) {

        // expand this position and mark any currently unreached
        // positions with the current pass (depth)
        for (int iTwist = 0; iTwist < nTwistsPerMove; ++iTwist) {

          // perform the twist on the corner and edge
          uint32_t tcidx = moveTable->cornerTwistTable(iTwist, cidx);
          uint32_t teidx = moveTable->edgeTwistTable(iTwist, eidx);

          // since the edge twist may result in a permutation
          // apply it to the corner
          uint8_t iPerm = teidx & edgePermMask;
          uint32_t peidx = teidx >> nEdgePermBits;
          uint32_t pcidx = moveTable->cornerPermuteTable(iPerm, tcidx);

          // obtain our index into the table
          std::size_t pidx = fullIdx(pcidx, peidx);

          if (getDepth(pidx) == 0x3) {
            // Since we do not read atomically, it is possible for one
            // thread to miss data written by another.  This will be
            // reflected in the (unpruned) count below from the actual
            // count reported during validate().
            ++count;

            // All threads attempt to inspect and write the (same)
            // current pass value to the table if not present.
            // We still need to require memory_order_seq_cst on our
            // atomic write operation in case an _adjacent_ nibble is
            // set by another thread.
            setDepthAtomically(pidx, pass % 3);
          }

          // mark other positions with 2-, 4-, and 8-fold symmetry
          uint16_t eposition = peidx >> 8;
          for (const auto &p :
               moveTable->equivalentEdgePermutationTable[eposition]) {
            uint32_t epeidx = moveTable->edgePermuteTable(p, peidx);
            uint32_t epcidx = moveTable->cornerPermuteTable(p, pcidx);

            std::size_t epidx = fullIdx(epcidx, epeidx);

            if (getDepth(epidx) == 0x3) {
              ++count;
              setDepthAtomically(epidx, pass % 3);
            }
          }
        }
      }
    }
  }
  return count;
}

// search all entries within the specified edge index range for
// unreached entries and mark any that can reach a known value
// with a single twist
std::size_t DepthTable::cleanupWorker(const MoveTable *moveTable, uint8_t pass,
                                      std::size_t start_eidx,
                                      std::size_t stop_eidx) {

  std::size_t count = 0;

  // search for positions that match the previous depth
  // and expand those that do
  for (std::size_t eidx = start_eidx; eidx < stop_eidx; ++eidx) {
    for (std::size_t cidx = 0; cidx < nCornerCoords; ++cidx) {

      if (getDepth(cidx, eidx) == 0x3) {

        // expand this position and mark any currently unreached
        // positions with the current pass (depth)
        for (int iTwist = 0; iTwist < nTwistsPerMove; ++iTwist) {

          // perform the twist on the corner and edge
          uint32_t tcidx = moveTable->cornerTwistTable(iTwist, cidx);
          uint32_t teidx = moveTable->edgeTwistTable(iTwist, eidx);

          // since the edge twist may result in a permutation
          // apply it to the corner
          uint8_t iPerm = teidx & edgePermMask;
          uint32_t peidx = teidx >> nEdgePermBits;
          uint32_t pcidx = moveTable->cornerPermuteTable(iPerm, tcidx);

          // obtain our index into the table
          std::size_t pidx = fullIdx(pcidx, peidx);

          if (getDepth(pidx) == (pass - 1) % 3) {

            ++count;

            // since we partition each block into non-overlapping
            // regions, we do not need to set atomically
            setDepthNonAtomically(fullIdx(cidx, eidx), pass % 3);

            // no need to twist anymore
            break;
          }
        }
      }
    }
  }
  return count;
}

void DepthTable::pbuild(std::size_t (DepthTable::*worker)(
                            const MoveTable *moveTable, uint8_t pass,
                            std::size_t start_eidx, std::size_t stop_eidx),
                        uint8_t startdepth, uint8_t stopdepth, bool pruned,
                        const MoveTable *moveTable) {

  for (uint8_t pass = startdepth; pass <= stopdepth; ++pass) {
    consoleOut("starting pass " + to_commastring(pass, 2) + "... ");

    // nThreads may be any factor of nSymEdgeCoords
    constexpr std::size_t nThreads = 16;
    std::size_t nSymEdgeCoordsPerThread =
        moveTable->getNSymEdgeCoords() / nThreads;

    // split up the the table into equal-sized chunks and build
    // at the specified depth
    std::future<std::size_t> count[nThreads];
    for (std::size_t thread = 0; thread < nThreads; ++thread) {
      std::size_t start_eidx = thread * nSymEdgeCoordsPerThread;
      std::size_t stop_eidx = start_eidx + nSymEdgeCoordsPerThread;
      count[thread] =
          std::async(worker, this, moveTable, pass, start_eidx, stop_eidx);
    }

    // collate
    std::size_t totalCount = 0;
    for (auto &thread : count) {
      totalCount += thread.get();
    }

    consoleOut(to_commastring(totalCount, 14) + " positions generated" +
               (pruned ? "\n" : " (unpruned)\n"));
  }
}

// main entry point for depth table building
// table is built recursively at first in a single
// thread, then in parallel one pass (depth) at a time
void DepthTable::build(const MoveTable *moveTable) {

  consoleOut("clearing table...\n");
  clear();

  consoleOut("start table build!\n");

  // mark the "home" Janus position with a depth of zero
  std::size_t cidx = moveTable->getHomeCornerIndex();
  std::size_t eidx = moveTable->getHomeEdgeIndex();
  std::size_t idx = fullIdx(cidx, eidx);
  setDepthNonAtomically(idx, 0);

  // do seven passes recursively (but single-threaded)
  // this number can be tuned for faster build
  const uint8_t altDepth = 7;
  altbuild(moveTable, cidx, eidx, altDepth);

  // do passes in parallel looking for existing
  // moves and seeing if they lead to unreached moves
  pbuild(&DepthTable::buildWorker, altDepth + 1, buildDepth, false, moveTable);

  // do three passes in parallel looking for unreached
  // moves and seeing if they lead to existing moves
  pbuild(&DepthTable::cleanupWorker, buildDepth + 1, finalDepth, true,
         moveTable);
}

// generates checksum and checkproduct used in validate()
// this is not currently used by the program, but is here
// in the event there are mistakes in the table...
void DepthTable::certify() const {

  // make a table with multipicative reciprocal
  // of depth values constrained to be odd
  std::vector<std::size_t> reciprocal(16);
  for (int depth = 0; depth < 16; ++depth) {
    reciprocal[depth] = divide(1, (depth << 1) | 1);
  }

  // initialize checks with the two-faced Janus magic number
  std::uint32_t checkSum = janusMagicNumber;
  std::uint32_t checkProduct = janusMagicNumber;

  consoleOut("generating initial depth checks...\n");

  // run validate() in reverse via modular arithmetic
  for (std::size_t idx = 0; idx < nSymCoords; ++idx) {
    uint32_t depth = getDepth(nSymCoords - 1 - idx);
    checkSum -= checkProduct;
    checkProduct *= reciprocal[depth];
  }

  // display the initial checks to stderr
  consoleOut("initCheckSum:     " + to_hstring(checkSum) + "\n");
  consoleOut("initCheckProduct: " + to_hstring(checkProduct) + "\n");
}

// validates the table
bool DepthTable::validate() const {
  std::vector<std::size_t> count(16);

  // initialize with what will generate the
  // two-faced Janus magic number
  std::uint32_t checkSum = initCheckSum;
  std::uint32_t checkProduct = initCheckProduct;

  consoleOut("Validating...\n");

  // compute the total number of positions
  // the product of all depths (constrained to be odd)
  // and a checksum
  for (std::size_t idx = 0; idx < nSymCoords; ++idx) {
    uint32_t depth = getDepth(idx);
    ++count[depth];
    checkProduct *= (depth << 1) | 1;
    checkSum += checkProduct;
  }

  // report diagnostics
  for (uint8_t depth = 0; depth < 4; ++depth) {
    consoleOut("depth " + to_ustring(depth) + ": " +
               to_commastring(count[depth], 14) + "\n");
  }

  auto totalPositions =
      std::accumulate(count.begin(), count.end(), static_cast<std::size_t>(0));

  // did the checks pass?
  bool posCountPassed = totalPositions == nSymCoords;
  bool checkSumPassed = checkSum == janusMagicNumber;
  bool checkProductPassed = checkProduct == janusMagicNumber;

  // report status
  consoleOut("Total positions: " + to_commastring(totalPositions, 14) +
             (posCountPassed ? " passed\n" : " failed\n"));
  consoleOut("checkSum:            " + to_hstring(checkSum) +
             (checkSumPassed ? " passed\n" : " failed\n"));
  consoleOut("checkProduct:        " + to_hstring(checkProduct) +
             (checkProductPassed ? " passed\n" : " failed\n"));

  // return aggregate status
  return posCountPassed && checkSumPassed && checkProductPassed;
}

// read the table from disk if it exists, otherwise build and save it.
void DepthTable::init(std::function<bool(uint8_t *, std::size_t)> load,
                      std::function<bool(const uint8_t *, std::size_t)> save,
                      const MoveTable *moveTable) {

  // expected number of bytes to read
  std::size_t nBytes = nSymCoords / 4;

  // no multi-threading is done at this point
  // we use raw data pointer when invoking user load/save

  if (!load(data, nBytes)) {
    build(moveTable);
    if (!validate()) {
      consoleOut("CHECKSUM FAILED!\n");
      consoleOut("RESULTS NOT GUARANTEED.\n");
      consoleOut("running certification step just in case...\n");
      certify();
    }
    if (!save(data, nBytes)) {
      consoleOut("COULDN'T WRITE DEPTH TABLE!\n");
      consoleOut("IS IT READ ONLY?  OUT OF SPACE?\n");
    }
  }
}

} // namespace Janus
