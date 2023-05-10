#ifndef JANUS_MOVEMETRIC
#define JANUS_MOVEMETRIC

namespace Janus {

enum class MoveMetric { QuarterTurn, FaceTurn };

template <typename T> inline T selectQH(MoveMetric moveMetric, T qt, T ht) {
  return moveMetric == MoveMetric::QuarterTurn ? qt : ht;
}

} // namespace Janus
#endif
