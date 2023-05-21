// Copyright (C) 2021 Greg Dionne
// Distributed under MIT License
#ifndef JANUS_EDGETWIST_HPP
#define JANUS_EDGETWIST_HPP

#include <cstdint>

namespace Janus {

// The order of edge pieces is taken from a way to tie up a square package
// using a single piece of string in a cross pattern that overlaps at each
// center, starting at edge 0, looping itself at a right angle at center "F"
// then proceeding to edge 1, etc:
//     0 -> F -> 1 -> D -> 2 -> R -> 3 -> F -> 4 -> U ->  5 -> R ->
//     6 -> B -> 7 -> U -> 8 -> L -> 9 -> B ->10 -> D -> 11 -> L -> 0.
//
// As seen from the outside, the direction the string turns at each
// center starting at L follows the pattern:
//    right->right ->  left->left  -> right->right ->
//     left->left  -> right->right ->  left->left.
//
//     +--- 7 ---+       +--- 7 ---+
//    /         /|      /|        /|
//   8    U    5 |     8 |       5 |
//  /         /  6    /  9      /  6
// +--- 4 ---+   |   +--- 4 ---+   |
// |         | R |   |   |     |   |
// |         |   +   |   +---10|---+
// 0    F    3  /    0  /      3  /
// |         | 2     | 11      | 2
// |         |/      |/        |/
// +--- 1 ---+       +--- 1 ---+
//
// The numerical pattern has the following properties:
// * opposing edges can be found by adding 6 modulo 12.
// * quater rotation about FLU axes found by adding 4 modulo 12.
// * all four edges of any side can be rotated to the edges of
//   another side by adding a multiple of 2 modulo 12 and will
//   appear in either clockwise or counter-clockwise order.

struct EdgeReturn {
  uint8_t position;
  uint8_t flip;
};

// with the given initial edge position perform the specified twist
// return if a flip is required (in a nod to Herbert Kociemba only
// L and R moves impart a "flip" to the piece.
//
// twists are numbered 0-11:  F  R  U  B  L  D  F  F' R' U' B' L' D'
EdgeReturn quarterTwistEdge(uint8_t position, uint8_t twist);

// permute the specified position and flip to a new position and flip
// Only enares option uses bit 4.
//   bit 4:  reflect along z axis (without colorswap)
//   bit 3:  reflect along z axis (with colorswap)
//   bit 2:  reflect along y axis
//   bit 1:  rotate a half-turn around z axis
//   bit 0:  rotate a quarter-turn around z axis
EdgeReturn permuteEdge(uint8_t position, uint8_t flip, uint8_t permutation);
} // namespace Janus

#endif
