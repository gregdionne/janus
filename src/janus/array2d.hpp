// Copyright (C) 2021 Greg Dionne
// Distributed under MIT License
#ifndef JANUS_ARRAY2D
#define JANUS_ARRAY2D

#include <vector>

namespace Janus {

// Simple two-dimensional std::vector container
// intended to be constructed and used in-place (only)
//
// construct as Array2D<type> array2d(nrows, ncols)
//
//  use like:   array2d(x,y) = 5;
//              answer = array2d(u,v)
//
template <typename T> class Array2D {
public:
  Array2D(std::size_t m, std::size_t n) : v(m * n), ncols(n) {}

  // no reason to move/copy yet.
  Array2D() = delete;
  Array2D(const Array2D &) = delete;
  Array2D(Array2D &&) = delete;
  Array2D &operator=(const Array2D &) = delete;
  Array2D &operator=(Array2D &&) = delete;

  // nothing fancy under-the-hood
  ~Array2D() = default;

  // use row-major convention
  T const &operator()(std::size_t m, std::size_t n) const {
    return v[(m * ncols) + n];
  }

  // allow use as an l-val
  T &operator()(std::size_t m, std::size_t n) { return v[(m * ncols) + n]; }

private:
  std::vector<T> v;
  const std::size_t ncols;
};

} // namespace Janus
#endif
