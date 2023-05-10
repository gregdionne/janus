// Copyright (C) 2021 Greg Dionne
// Distributed under MIT License
#ifndef JANUS_OPTIONS_HPP
#define JANUS_OPTIONS_HPP

#include "utils/options.hpp"

namespace Janus {

class Options : public utils::Options {
public:
  Options();
  Option qtm;
  Option enares;
};

} // namespace Janus
#endif
