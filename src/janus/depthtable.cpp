// Copyright (C) 2021 Greg Dionne
// Distributed under MIT License
#include "depthtable.hpp"
#include "bitutils.hpp"
#include "constants.hpp"
#include "index.hpp"

#include <numeric>
#include <vector>

#include <future>
#include <mutex>

#include <cstdio>

namespace Janus {

// clear the table (single-threaded)
void DepthTable::clear() {
  // clang-tidy steers us to this implementation
  // however it is probably better to use old-skool
  // C arrays to avail ourselves of optimization
  //
  //   for (auto &i : data) {
  //     i = 0xff;
  //   }

  uint8_t *memloc =
      const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(&data[0]));

  for (std::size_t i = 0; i < nSymCoords / 2; ++i) {
    memloc[i] = 0xff;
  }
}

// recursively build the table from the specified coordinate and currentDepth
// down to the specified depth and mark any newly reached entries
std::size_t DepthTable::rbuild(const MoveTable *moveTable, std::size_t cidx,
                               std::size_t eidx, uint8_t depth,
                               uint8_t currentDepth) {

  std::size_t idx = fullIdx(cidx, eidx);

  if (!currentDepth) {

    if (getDepth(idx) == 0xF) {
      setDepth(idx, depth);
      return 1;
    } else {
      return 0;
    }
  } else {
    std::size_t count = 0;

    // only recurse if we match the correct depth
    if (getDepth(idx) == (depth - currentDepth)) {

      // for every twist...
      for (int iTwist = 0; iTwist < nTwistsPerMove; ++iTwist) {

        // perform the twist on the corner and edge
        uint32_t tcidx = moveTable->cornerTwistTable[iTwist][cidx];
        uint32_t teidx = moveTable->edgeTwistTable[iTwist][eidx];

        // since the edge twist may result in a permutation
        // apply it to the corner
        uint8_t iPerm = teidx & 0x0f;
        uint32_t peidx = teidx >> 4;
        uint32_t pcidx = moveTable->cornerPermuteTable[iPerm][tcidx];

        // recurse another depth with the permuted cube
        count += rbuild(moveTable, pcidx, peidx, depth, currentDepth - 1);

        // ensure expansion of edge positions with 2-, 4-, and 8-fold symmetry
        uint16_t eposition = peidx >> 8;
        for (const auto &p :
             moveTable->equivalentEdgePermutationTable[eposition]) {
          uint32_t epeidx = moveTable->edgePermuteTable[p][peidx];
          uint32_t epcidx = moveTable->cornerPermuteTable[p][pcidx];

          count += rbuild(moveTable, epcidx, epeidx, depth, currentDepth - 1);
        }
      }
    }
    return count;
  }
}

// recursively build the table from the home coordinate down to the
// specified depth (single-threaded)
void DepthTable::altbuild(const MoveTable *moveTable, std::size_t cidx,
                          std::size_t eidx, uint8_t depth) {

  // update the table to the specified depth
  for (uint8_t pass = 1; pass <= depth; ++pass) {
    fprintf(stderr, "starting pass %2i...", pass);
    fflush(stderr);

    std::size_t count = rbuild(moveTable, cidx, eidx, pass, pass);

    fprintf(stderr, "%11lu positions generated\n", count);
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
    for (std::size_t cidx = 0; cidx < nSymCornerCoords; ++cidx) {

      // since we build the table one depth at a time
      // we need not perform a mutex read when finding
      // the previous depth
      if (getDepth(cidx, eidx) == pass - 1) {

        // expand this position and mark any currently unreached
        // positions with the current pass (depth)
        for (int iTwist = 0; iTwist < nTwistsPerMove; ++iTwist) {

          // perform the twist on the corner and edge
          uint32_t tcidx = moveTable->cornerTwistTable[iTwist][cidx];
          uint32_t teidx = moveTable->edgeTwistTable[iTwist][eidx];

          // since the edge twist may result in a permutation
          // apply it to the corner
          uint8_t iPerm = teidx & 0x0f;
          uint32_t peidx = teidx >> 4;
          uint32_t pcidx = moveTable->cornerPermuteTable[iPerm][tcidx];

          // obtain our index into the table
          std::size_t pidx = fullIdx(pcidx, peidx);

          if (getDepth(pidx) == 0xF) {
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
            setDepth(pidx, pass);
          }

          // mark other positions with 2-, 4-, and 8-fold symmetry
          uint16_t eposition = peidx >> 8;
          for (const auto &p :
               moveTable->equivalentEdgePermutationTable[eposition]) {
            uint32_t epeidx = moveTable->edgePermuteTable[p][peidx];
            uint32_t epcidx = moveTable->cornerPermuteTable[p][pcidx];

            std::size_t epidx = fullIdx(epcidx, epeidx);

            if (getDepth(epidx) == 0xF) {
              ++count;
              setDepth(epidx, pass);
            }
          }
        }
      }
    }
  }
  return count;
}

// main entry point for depth table building
// table is built recursively at first in a single
// thread, then in parallel one pass (depth) at a time
void DepthTable::build(const MoveTable *moveTable) {

  fprintf(stderr, "clearing table...\n");
  clear();

  fprintf(stderr, "start table build!\n");

  // mark the "home" Janus position with a depth of zero
  std::size_t cidx = homeCornerIndex;
  std::size_t eidx = homeEdgeIndex;
  std::size_t idx = fullIdx(cidx, eidx);
  setDepth(idx, 0);

  // do seven passes recursively (but single-threaded)
  // this number can be tuned for faster build
  const uint8_t altdepth = 7;
  altbuild(moveTable, cidx, eidx, altdepth);

  const uint8_t fulldepth = 14;

  // build table incrementally for each depth
  for (uint8_t pass = altdepth + 1; pass <= fulldepth; ++pass) {
    fprintf(stderr, "starting pass %2i...", pass);
    fflush(stderr);

    // nThreads may be any factor of nSymEdgeCoords = 2^4 * 3 * 47
    constexpr std::size_t nThreads = 16;
    constexpr std::size_t nSymEdgeCoordsPerThread = nSymEdgeCoords / nThreads;

    // split up the the table into equal-sized chunks and build
    // at the specified depth
    std::future<std::size_t> count[nThreads];
    for (std::size_t thread = 0; thread < nThreads; ++thread) {
      std::size_t start_eidx = thread * nSymEdgeCoordsPerThread;
      std::size_t stop_eidx = start_eidx + nSymEdgeCoordsPerThread;
      count[thread] = std::async(&DepthTable::buildWorker, this, moveTable,
                                 pass, start_eidx, stop_eidx);
    }

    // collate
    std::size_t totalCount = 0;
    for (auto &thread : count) {
      totalCount += thread.get();
    }

    fprintf(stderr, "%11lu positions generated (unpruned)\n", totalCount);
  }
}

// load from disk
//   return true if good, false if not found
//   force-quit if table corrupt
bool DepthTable::load(const char *filename) {

  std::FILE *fp = std::fopen(filename, "rb");

  if (fp == NULL) {

    // failed to open
    fprintf(stderr, "couldn't open %s\n", filename);
    std::perror(filename);
    return false;

  } else {

    // try reading it
    fprintf(stderr, "reading %s...", filename);
    fflush(stderr);
    uint8_t *memloc =
        const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(&data[0]));
    std::size_t result = std::fread(memloc, 1, nSymCoords / 2, fp);
    fprintf(stderr, "%lu positions read\n", result * 2);

    // force-quit if it's not the proper length
    if (result != nSymCoords / 2) {
      fprintf(stderr, "incorrect number of positions read from %s\n", filename);
      fprintf(stderr, "received: %lu\n", result * 2);
      std::exit(1);
    }

    fclose(fp);
    return true;
  }
}

// save table
//   force-quit on failure
void DepthTable::save(const char *filename) const {

  std::FILE *fp = std::fopen(filename, "wb");

  if (fp == NULL) {
    // force-quit if we can't save it
    std::perror("Couldn't open janusdepthtable for writing");
    std::perror(filename);
    exit(1);
  } else {

    // write it
    fprintf(stderr, "writing %s...", filename);

    // no need for multi-threading here
    const uint8_t *memloc = reinterpret_cast<const uint8_t *>(&data[0]);

    std::size_t result = std::fwrite(memloc, 1, nSymCoords / 2, fp);
    fprintf(stderr, "%lu positions written\n", result * 2);

    // force-quit if write failed
    if (result != nSymCoords / 2) {
      fprintf(stderr, "incorrect number of positions written to %s\n",
              filename);
      fprintf(stderr, "written: %lu\n", result * 2);
      std::exit(1);
    }

    fclose(fp);
  }
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

  fprintf(stderr, "generating initial depth checks...\n");

  // run validate() in reverse via modular arithmetic
  for (std::size_t idx = 0; idx < nSymCoords; ++idx) {
    uint32_t depth = getDepth(nSymCoords - 1 - idx);
    checkSum -= checkProduct;
    checkProduct *= reciprocal[depth];
  }

  // display the initial checks to stderr
  fprintf(stderr, "initCheckSum:     %X\n", checkSum);
  fprintf(stderr, "initCheckProduct: %X\n", checkProduct);
}

// validates the table
bool DepthTable::validate() const {
  std::vector<std::size_t> count(16);

  // initialize with what will generate the
  // two-faced Janus magic number
  std::uint32_t checkSum = initCheckSum;
  std::uint32_t checkProduct = initCheckProduct;

  fprintf(stderr, "Validating...\n");

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
  for (uint8_t depth = 0; depth < 16; ++depth) {
    fprintf(stderr, "depth %2i: %11lu (actual)\n", depth, count[depth]);
  }

  auto totalPositions =
      std::accumulate(count.begin(), count.end(), static_cast<std::size_t>(0));

  // did the checks pass?
  bool posCountPassed = totalPositions == nSymCoords;
  bool checkSumPassed = checkSum == janusMagicNumber;
  bool checkProductPassed = checkProduct == janusMagicNumber;

  // report status
  fprintf(stderr, "Total positions: %lu (%s)\n", totalPositions,
          posCountPassed ? "passed" : "failed");
  fprintf(stderr, "checkSum:     %X (%s)\n", checkSum,
          checkSumPassed ? "passed" : "failed");
  fprintf(stderr, "checkProduct: %X (%s)\n", checkProduct,
          checkProductPassed ? "passed" : "failed");

  // return aggregate status
  return posCountPassed && checkSumPassed && checkProductPassed;
}

// read the table from disk if it exists, otherwise build and save it.
void DepthTable::init(const char *filename, const MoveTable *moveTable) {

  if (!load(filename)) {
    build(moveTable);
    if (!validate()) {
      fprintf(stderr, "CHECKSUM FAILED!\n");
      fprintf(stderr, "RESULTS NOT GUARANTEED.\n");
    }
    save(filename);
  }
}

} // namespace Janus
