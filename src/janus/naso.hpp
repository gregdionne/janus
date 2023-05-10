#ifndef JANUS_NASO
#define JANUS_NASO

namespace Janus {

enum class Naso { Aequivalens, Disparilis };

template <typename T> inline T selectAD(Naso naso, T aequiv, T disp) {
  return naso == Naso::Aequivalens ? aequiv : disp;
}

} // namespace Janus
#endif
