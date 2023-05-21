// Copyright (C) 2021 Greg Dionne
// Distributed under MIT License
#include "edgemask.hpp"
#include "edgetwist.hpp"

namespace Janus {

// return a new edge mask after performing the specified quarter turn
EdgeMask EdgeMask::moveQuarterTwist(uint8_t twist) const {
  EdgeMask out{0, 0, 0};

  int tmpFace = face;
  int tmpValid = valid;
  int tmpFlip = flip;

  for (int edge = 0; edge < nEdges; ++edge) { // for each edge
    int thisFace = tmpFace & 1;
    int thisValid = tmpValid & 1;
    int thisFlip = tmpFlip & 1;
    auto retVal = quarterTwistEdge(edge, twist); // get its destination
    out.face |= thisFace << retVal.position; // move the face to its destination
    out.valid |= thisValid << retVal.position;
    out.flip |= (thisValid & (thisFlip ^ retVal.flip)) << retVal.position;

    tmpFace >>= 1;
    tmpValid >>= 1;
    tmpFlip >>= 1;
  }

  return out;
}

// return a new edge mask after performing the specified twist
EdgeMask EdgeMask::move(uint8_t twist) const {
  return twist < nQuarterTwists
             ? moveQuarterTwist(twist)
             : moveQuarterTwist(twist % 6).moveQuarterTwist(twist % 6);
}

// return a new edge mask after performing the specified permutation
EdgeMask EdgeMask::permute(uint8_t permutation) const {
  EdgeMask out{0, 0, 0};

  int tmpFace = face;
  int tmpValid = valid;
  int tmpFlip = flip;
  int exchange = (permutation & 0x08) >> 3;

  for (int edge = 0; edge < nEdges; ++edge) {
    int thisFace = tmpFace & 1;
    int thisValid = tmpValid & 1;
    int thisFlip = tmpFlip & 1;

    // get its destination
    auto retVal = permuteEdge(edge, thisFlip, permutation);

    // move to destination (along with any color permutation)
    out.face |= ((thisFace ^ exchange) & thisValid) << retVal.position;
    out.valid |= thisValid << retVal.position;
    out.flip |= (thisValid & retVal.flip) << retVal.position;

    tmpFace >>= 1;
    tmpValid >>= 1;
    tmpFlip >>= 1;
  }

  return out;
}

} // namespace Janus
