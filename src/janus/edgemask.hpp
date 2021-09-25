// Copyright (C) 2021 Greg Dionne
// Distributed under MIT License
#ifndef JANUS_EDGEMASK_HPP
#define JANUS_EDGEMASK_HPP

#include <cstdint>

namespace Janus {

// An edge mask consists of the positions and flips of the four
// identical lower and upper edges
//
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

struct EdgeMask {
  // A valid bit indicates that the corresponding edge position is
  // occupied by one of the eight edges.
  uint16_t valid;

  // A face bit indicates that the position is occupied by an upper
  // edge piece.
  uint16_t face;

  // A flip bit indicates that the corresponding edge has been "flipped."
  // In a nod to Herbert Kociemba, U, D, F, and B moves do not disturb
  // the orientation of an edge.
  //
  // Only L and R moves perform a flip of each of its edge pieces.
  uint16_t flip;

  // return a new mask after twisting the face
  // twists are numbered:
  //  0 -  5:  F  R  U  B  L  D  (clockwise moves)
  //  6 - 12:  F' R' U' B' L' D' (counter-clockwise moves)
  // 12 - 17: F2 R2 U2 B2 L2 D2 (half-turn moves)
  EdgeMask move(uint8_t twist) const;

  // return a new mask after reflecting/rotating about
  // the up-down axes in the following sequence:
  //   bit 3:  reflect along z axis
  //   bit 2:  rotate a half-turn around x axis
  //   bit 1:  rotate a half-turn around z axis
  //   bit 0:  rotate a quarter-turn around z axis
  EdgeMask permute(uint8_t permutation) const;

private:
  // same as move(), but restricted to quater twists.
  // it calls the quarter twist twice for half-twists
  EdgeMask moveQuarterTwist(uint8_t twist) const;
};

} // namespace Janus

#endif
