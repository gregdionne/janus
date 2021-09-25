// Copyright (C) 2021 Greg Dionne
// Distributed under MIT License
#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <cstddef>
#include <cstdint>

namespace Janus {

constexpr const uint16_t C_12_4 = 12 * 11 * 10 * 9 / (4 * 3 * 2);
constexpr const uint16_t C_8_4 = 8 * 7 * 6 * 5 / (4 * 3 * 2);

// symmetries
//   number of Janus permutations
//   * inversion about x-y plane (along z axis)
//   * rotation of 180 about x axis
//   * rotation of 180 about z axis
//   * rotation of 90 about z axis
constexpr uint8_t nJanusPerms = 16;

// there are 48 cube symmetries (
constexpr uint8_t nCubeSyms = 3 * nJanusPerms;

// twists per move
//   only debugged with 18 (12 would be the quarter-turn metric)
constexpr uint8_t nTwistsPerMove = 18;

// corners
//
//   8C4 ways to position four identical lower and upper corners
constexpr uint8_t nSymCornerPositions = C_8_4;

//   3^7 ways to individually spin all eight corners
//   eighth corner can be computed by the sum of the others modulo 3.
constexpr uint16_t nSymCornerSpins = 2187;

//   full corner coordinate consists of a position and spin
constexpr uint32_t nSymCornerCoords = nSymCornerPositions * nSymCornerSpins;

// edges
//
//   12C4*8C4 ways to position four identical lower and upper edges
//   we first position the "missing" edges (12C4).
//   we then position the lower edges (8C4).
constexpr uint16_t nRegEdgePositions = C_12_4 * C_8_4;

//   number of ways to position the edges independent of rotation or inversion
//   There are:
//         9  2-way symmetries
//        12  4-way symmetries
//       147  8-way symmetries
//      2088 16-way symmetries
constexpr uint16_t nSymEdgePositions = 9 + 12 + 147 + 2088; // 2256

//   2^8 ways to orient (flip) the lower and upper edges
//   we don't track the orientations of the "missing" edges
constexpr uint16_t nEdgeFlips = 256;

//   full symmetric edge coordinate consists of position and orientation
constexpr uint32_t nSymEdgeCoords = nSymEdgePositions * nEdgeFlips;

//   Janus table consists of a full symmetricized coordinate
constexpr std::size_t nSymCoords = static_cast<std::size_t>(nSymCornerCoords) *
                                   static_cast<std::size_t>(nSymEdgeCoords);

// table checks
//
//    magic number for Janus depth table checks.
//    two faces:  one backward one forward.
constexpr uint32_t janusMagicNumber = 0xECAFFACE;

//    validates order of table
constexpr uint32_t initCheckSum = 0x45634A7A;

//    validates values of table in any order
constexpr uint32_t initCheckProduct = 0xD0C5A1BE;

// useful pruning depth beyond which it is better to ignore the depth table
constexpr uint8_t usefulDepth = 12;

// maximum depth to search for any solution
constexpr uint8_t GodsNumber = 20;

} // namespace Janus
#endif
