// Copyright (C) 2021-2022 Greg Dionne
// Distributed under MIT License
#ifndef JANUS_STRUTILS
#define JANUS_STRUTILS

#include <iomanip>
#include <sstream>
#include <string>

namespace Janus {

// redirect for to_string(uint8_t)
template <typename T> std::string to_ustring(T numeric) {
  return std::to_string(static_cast<unsigned>(numeric));
}

// print an unsigned number with commas
//   TODO:  update with requires std::UnsignedIntegral<T> concept
template <typename T> std::string to_commastring(T number, std::size_t width) {
  std::string out;
  int ndigits = 0;

  if (number == 0) {
    out = "0";
  }

  while (number != 0) {
    std::string digit = std::to_string(number % 10);
    if (ndigits != 0 && ndigits % 3 == 0) {
      digit += ",";
    }
    out = digit + out;
    number /= 10;
    ++ndigits;
  }

  if (out.length() < width) {
    out = std::string(width - out.length(), ' ') + out;
  }

  return out;
}

// print uppercase hex
template <typename T> std::string to_hstring(T numeric) {
  std::stringstream stream;
  stream << "0x" << std::setfill('0') << std::setw(sizeof(T) * 2) << std::hex
         << std::uppercase << numeric;
  return stream.str();
}

} // namespace Janus
#endif
