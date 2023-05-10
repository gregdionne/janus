// Copyright (C) 2021-2022 Greg Dionne
// Distributed under MIT License
#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <cstddef>
#include <cstdint>

namespace Janus {

constexpr uint16_t C_12_4 = 12 * 11 * 10 * 9 / (4 * 3 * 2);
constexpr uint16_t C_8_4 = 8 * 7 * 6 * 5 / (4 * 3 * 2);

// twists are numbered:
//  0 -  5:  F  R  U  B  L  D  (clockwise moves)
//  6 - 11:  F' R' U' B' L' D' (counter-clockwise moves)
// 12 - 17: F2 R2 U2 B2 L2 D2 (half-turn moves)
constexpr uint8_t nQuarterTwists = 12;
constexpr uint8_t nFaceTwists = 18;

} // namespace Janus
#endif
