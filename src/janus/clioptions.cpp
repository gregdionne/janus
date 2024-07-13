// Copyright (C) 2021 Greg Dionne
// Distributed under MIT License
#include "clioptions.hpp"

namespace Janus {
// for each option define:
//   default value
//   command line string
//   opposite of commandline string (only needed when default valueis true)
//   summary
//   details
CLIOptions::CLIOptions()
    : qtm{false, "qtm", nullptr, "Use the quater-turn metric.",
          "The face-turn metric (sometimes called the half-turn metric) "
          "considers turning any face either by 90 or 180 degrees counts as "
          "a single move.  The quater-turn metric restricts a move to only 90 "
          "degrees and counts a twist by 180 degrees as two moves."},
      enares{
          false, "enares", nullptr,
          "Use a reduced depth table of 11 GB instead of 22 GB.",
          "By default, Janus constructs a depth table that reports the number "
          "of moves (modulo three) required to completely solve any two "
          "opposing faces.  The table is large and occupies 22 GB of memory.\n "
          "Janus builds a smaller database when using the 'enares' option.  "
          "Instead of storing the number of moves to completely solve two "
          "faces, the centers of each face are ignored when creating the "
          "table.  "
          "This halves the size of the table to 11 GB, but reduces the table's "
          "pruning ability.\n "
          "The option 'enares' originates from the plural form of the Latin "
          "word 'enaris' meaning 'without a nose'.  As the Roman god Janus "
          "is often depicted with a head with two opposing faces, it was "
          "befitting to liken the center cubies to the noses of each face "
          "and to use Latin to specify table computation without the noses."} {
  addOption(&qtm);
  addOption(&enares);
}
} // namespace Janus
