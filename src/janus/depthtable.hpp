// Copyright (C) 2021-2022 Greg Dionne
// Distributed under MIT License
#ifndef JANUS_DEPTHTABLE_HPP
#define JANUS_DEPTHTABLE_HPP

#include "constants.hpp"
#include "movemetric.hpp"
#include "movetable.hpp"

#include <array>
#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <utility>

namespace Janus {

//
// The returned bits tells how many twists modulo three are needed
// to restore the Janus to the original state.
//
// NOTE:  Since we also encode reflection about the Z axes, the
//        returned depth is zero for both the solved state, as
//        well as when the two faces are inverted.  Solving all
//        three Janus coordinates may then arise in a 'four-spot'
//        pattern, so the solver will need to perform additional
//        validation to 'throw out' these unwanted solutions.

class DepthTable {
public:
  DepthTable(MoveMetric moveMetric, const MoveTable *jmt,
             std::function<void(const std::string &)> console,
             std::function<bool(uint8_t *, std::size_t)> load,
             std::function<bool(const uint8_t *, std::size_t)> save)
      : nSymCoords(static_cast<std::size_t>(nCornerCoords) *
                   static_cast<std::size_t>(jmt->getNSymEdgeCoords())),
        consoleOut(std::move(console)),
        nTwistsPerMove(selectQH(moveMetric, nQuarterTwists, nFaceTwists)),
        buildDepth(selectQH(moveMetric, buildDepthQ, buildDepthH)),
        finalDepth(selectQH(moveMetric, finalDepthQ, finalDepthH)),
        initCheckSum(
            selectAD(jmt->getNaso(),
                     selectQH(moveMetric, initCheckSumQL, initCheckSumHL),
                     selectQH(moveMetric, initCheckSumQF, initCheckSumHF))),
        initCheckProduct(selectAD(
            jmt->getNaso(),
            selectQH(moveMetric, initCheckProductQL, initCheckProductHL),
            selectQH(moveMetric, initCheckProductQF, initCheckProductHF))),
        edgePermMask(jmt->getEdgePermMask()),
        nEdgePermBits(jmt->getNEdgePermBits()) {

    atomicData = std::make_unique<std::atomic_uint8_t[]>(nSymCoords / 4);
    adata = atomicData.get();
    data = const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(&adata[0]));

    init(std::move(load), std::move(save), jmt);
  }

  // returns the depth for the specified corner and edge indices
  uint8_t getDepth(std::size_t cidx, std::size_t eidx) const {
    return getDepth(fullIdx(cidx, eidx));
  }

  // returns the index for the specified corner and edge indices
  static std::size_t fullIdx(std::size_t cidx, std::size_t eidx) {
    return eidx * nCornerCoords + cidx;
  }

  // returns the depth for the corresponding index
  //   0 = depth is 0 mod 3
  //   1 = depth is 1 mod 3
  //   2 = depth is 2 mod 3
  //   3 = uninitialized
  uint8_t getDepth(std::size_t idx) const {
    std::size_t loc = idx >> 2;

    // no need for thread safety on write-once read-many table
    // uint8_t dataByte = atomicData[loc].load(std::memory_order_relaxed);

    return (data[loc] >> ((idx & 3) << 1)) & 0x3;
  }

private:
  // sets the depth at a specified index (not thread-safe)
  void setDepthNonAtomically(std::size_t idx, uint8_t value) {
    std::size_t loc = idx >> 2;

    uint8_t mask = ~((~value & 0x03) << ((idx & 3) << 1));

    data[loc] &= mask;
  }

  // sets the depth at the specified index in a thread-safe manner
  void setDepthAtomically(std::size_t idx, uint8_t value) {
    std::size_t loc = idx >> 2;

    uint8_t mask = ~((~value & 0x03) << ((idx & 3) << 1));

    // atomic implementation of data[loc] &= mask;
    atomic_fetch_and(&adata[loc], mask);
  }

  // the following items probably could be moved to a new builder class

  // search all entries within the specified edge index range for
  // values that match the previous pass (depth) and mark any yet
  // unreached entry one twist away with the current pass
  std::size_t buildWorker(const MoveTable *moveTable, uint8_t pass,
                          std::size_t start_eidx, std::size_t stop_eidx);

  // search all entries within the specified edge index range for
  // unreached entries and mark any that can reach a known entry
  // with one twist
  std::size_t cleanupWorker(const MoveTable *moveTable, uint8_t pass,
                            std::size_t start_eidx, std::size_t stop_eidx);

  // build table in parallel using the specified worker
  void pbuild(std::size_t (DepthTable::*worker)(const MoveTable *moveTable,
                                                uint8_t pass,
                                                std::size_t start_eidx,
                                                std::size_t stop_eidx),
              uint8_t startdepth, uint8_t stopdepth, bool pruned,
              const MoveTable *moveTable);

  // recursively build the table starting from a given coordinate
  std::size_t rbuild(const MoveTable *moveTable, std::size_t cidx,
                     std::size_t eidx, uint8_t depth, uint8_t currentDepth);

  // alternative recursive build entry point
  void altbuild(const MoveTable *moveTable, std::size_t cidx, std::size_t eidx,
                uint8_t depth);

  // main entry point for depth table building
  void build(const MoveTable *moveTable);

  // sets all entries of table to max val (3).
  void clear();

  // validate the table
  bool validate() const;

  // generate a checksum and checkproduct to validate the table
  void certify() const;

  // read the table if possible, otherwise build and save it
  void init(std::function<bool(uint8_t *, std::size_t)> load,
            std::function<bool(const uint8_t *, std::size_t)> save,
            const MoveTable *moveTable);

  // read the table from the file
  bool load(const char *filename);

  // save the table to the file
  bool save(const char *filename) const;

  // database needs to be atomic when creating table
  //  std::array<std::atomic_uint8_t, nSymCoords / 4> adata;
  std::unique_ptr<std::atomic_uint8_t[]> atomicData;
  std::atomic_uint8_t *adata;
  uint8_t *data;

  // number of total symmetricized coordinates
  // There are 88,414,986,240 possible Janus coordinates
  // when noses are interchangeable and 173,420,352,000
  // when noses are unique.
  //
  // Four positions are packed into each byte, so ~22GB or ~44GB
  // are required (depending on whether noses are interchangeable
  // or unique).
  //
  const std::size_t nSymCoords; // static_cast<std::size_t>(nCornerCoords) *
                                // static_cast<std::size_t>(nSymEdgeCoords);

  // console out ptr
  std::function<void(const std::string &)> consoleOut;

  // set to:
  //   12 when using quarter-turn metric
  //   18 when using face-turn metric
  const uint8_t nTwistsPerMove;

  const uint8_t buildDepth;
  constexpr static uint8_t buildDepthQ = 13;
  constexpr static uint8_t buildDepthH = 11;

  const uint8_t finalDepth;
  constexpr static uint8_t finalDepthQ = 16;
  constexpr static uint8_t finalDepthH = 14;

  // table checks
  //
  //    magic number for Janus depth table checks.
  //    two faces:  one backward one forward.
  constexpr static uint32_t janusMagicNumber = 0xECAFFACE;

  //    validates order of table
  const uint32_t initCheckSum;
  constexpr static uint32_t initCheckSumQF = 0x20A3021A;
  constexpr static uint32_t initCheckSumHF = 0xF380E7D2;
  constexpr static uint32_t initCheckSumQL = 0xC1FF0922;
  constexpr static uint32_t initCheckSumHL = 0x50CBEBFE;

  //    validates values of table in any order
  const uint32_t initCheckProduct;
  constexpr static uint32_t initCheckProductQF = 0xBC2142C6;
  constexpr static uint32_t initCheckProductHF = 0x3C9F3CBA;
  constexpr static uint32_t initCheckProductQL = 0xBCACF066;
  constexpr static uint32_t initCheckProductHL = 0xD1C4CF72;

  const uint8_t edgePermMask;
  const uint8_t nEdgePermBits;
};

} // namespace Janus
#endif
