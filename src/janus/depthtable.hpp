// Copyright (C) 2021 Greg Dionne
// Distributed under MIT License
#ifndef JANUS_DEPTHTABLE_HPP
#define JANUS_DEPTHTABLE_HPP

#include "constants.hpp"
#include "movetable.hpp"

#include <array>
#include <atomic>

namespace Janus {

// Holds the table of the 88,414,986,240 possible Janus coordinates.
// Each position is packed into half a byte, so ~44GB is required.
//
// The returned nibble tells how many twists are needed to restore
// the Janus to the original state.
//
// NOTE:  Since we also encode reflection about the Z axes, the
//        returned depth is zero for both the solved state, as
//        well as when the two faces are inverted.  Solving all
//        three Janus coordinates may then arise in a 'four-spot'
//        pattern, so the solver will need to perform additional
//        validation to 'throw out' these unwanted solutions.

class DepthTable {
public:
  // by default we use "depthtable.janus" in the same directory
  // as our executable.... this probably could/should change
  explicit DepthTable(const MoveTable *jmt) { init("depthTable.janus", jmt); }

  // returns the depth for the specified corner and edge indices
  uint8_t getDepth(std::size_t cidx, std::size_t eidx) const {
    return getDepth(fullIdx(cidx, eidx));
  }

  // returns the index for the specified corner and edge indices
  static std::size_t fullIdx(std::size_t cidx, std::size_t eidx) {
    return eidx * nSymCornerCoords + cidx;
  }

  // returns the depth for the corresponding index
  uint8_t getDepth(std::size_t idx) const {
    std::size_t loc = idx >> 1;

    // no need for thread safety on read-only table
    // uint8_t dataByte = data[loc].load(std::memory_order_relaxed);
    uint8_t dataByte = reinterpret_cast<const uint8_t *>(&data[0])[loc];

    return idx & 1 ? dataByte >> 4 : dataByte & 0xF;
  }

private:
  // sets the depth at the specified index in a thread-safe manner
  void setDepth(std::size_t idx, uint8_t value) {
    std::size_t loc = idx >> 1;

    uint8_t mask = idx & 1 ? (value << 4) | 0xF : value | 0xF0;

    // atomic implementation of data[loc] &= mask;
    atomic_fetch_and(&data[loc], mask);
  }

  // the following items probably could be moved to a new builder class

  // search all entries within the specified edge index range for
  // values that match the previous pass (depth) and mark any yet
  // unreached entry one twist away with the current pass
  std::size_t buildWorker(const MoveTable *moveTable, uint8_t pass,
                          std::size_t start_eidx, std::size_t stop_eidx);

  // recursively build the table starting from a given coordinate
  std::size_t rbuild(const MoveTable *moveTable, std::size_t cidx,
                     std::size_t eidx, uint8_t depth, uint8_t currentDepth);

  // alternative recursive build entry point
  void altbuild(const MoveTable *moveTable, std::size_t cidx, std::size_t eidx,
                uint8_t depth);

  // main entry point for depth table building
  void build(const MoveTable *moveTable);

  // sets all entries of table to max val (15).
  void clear();

  // validate the table
  bool validate() const;

  // generate a checksum and checkproduct to validate the table
  void certify() const;

  // read the table if possible, otherwise build and save it
  void init(const char *filename, const MoveTable *moveTable);

  // read the table from the file
  bool load(const char *filename);

  // save the table to the file
  void save(const char *filename) const;

  // database needs to be atomic when creating table
  std::array<std::atomic_uint8_t, nSymCoords / 2> data;
};

} // namespace Janus
#endif
