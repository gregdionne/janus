// Copyright (C) 2021 Greg Dionne
// Distributed under MIT License
#ifndef JANUS_CLIOPTIONS_HPP
#define JANUS_CLIOPTIONS_HPP

#include "utils/binaryoptions.hpp"

namespace Janus {

// command line options
class CLIOptions : public utils::BinaryOptions {
public:
  CLIOptions();
  BinaryOption qtm;
  BinaryOption enares;
};

} // namespace Janus
#endif
