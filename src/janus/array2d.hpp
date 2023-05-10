#ifndef JANUS_ARRAY2D
#define JANUS_ARRAY2D

#include <vector>

namespace Janus {

template <typename T> class Array2D {
public:
  Array2D(std::size_t m, std::size_t n) : v(m * n), stride(n) {}
  Array2D() = delete;
  Array2D(const Array2D &) = delete;
  Array2D(Array2D &&) = delete;
  Array2D &operator=(const Array2D &) = delete;
  Array2D &operator=(Array2D &&) = delete;
  ~Array2D() = default;

  T &operator()(std::size_t m, std::size_t n) { return v[(m * stride) + n]; }

  T const &operator()(std::size_t m, std::size_t n) const {
    return v[(m * stride) + n];
  }

private:
  std::vector<T> v;
  const std::size_t stride;
};

} // namespace Janus
#endif
