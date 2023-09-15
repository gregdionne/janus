// Copyright (C) 2021-2022 Greg Dionne
// Distributed under MIT License
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <numeric>
#include <string>
#include <unistd.h> // isatty

#include "janus/cube.hpp"
#include "janus/strutils.hpp"
#include "janus_core.hpp"

void console(const std::string &message) {
  fprintf(stderr, "%s", message.c_str());
  fflush(stderr);
}

void consoleOut(const std::string &message) {
  fprintf(stdout, "%s", message.c_str());
  fflush(stdout);
}

void printSolutionNumber(std::size_t n) { fprintf(stdout, "%2lu: ", n); }

void argUsage() { fprintf(stderr, "[\"move1 move2 move3 ...\"]\n\n"); }

void validMoves() {
  fprintf(stderr, " valid moves are entered in Singmaster notation:\n");
  fprintf(stderr, "  F  R  U  B  L  D  (clockwise moves)\n");
  fprintf(stderr, "  F' R' U' B' L' D' (counter-clockwise moves)\n");
  fprintf(stderr, "  F2 R2 U2 B2 L2 D2 (half-turn moves)\n\n");
}

void argDetails() {
  fprintf(stderr, "DESCRIPTION\n\n");
  fprintf(stderr, " Janus reports all optimal solutions for one or more\n");
  fprintf(stderr, " sequences of moves of the Rubik's cube.\n\n");
  fprintf(stderr, " When executed for the first time for a given metric\n");
  fprintf(stderr, " and depth table size, it attempts to save a database\n");
  fprintf(stderr, " in the current working directory.\n\n");
  fprintf(stderr, " If moves are not supplied as a quoted string,\n");
  fprintf(stderr, " then they are repeatedly read from the standard input\n");
  fprintf(stderr, " until an end-of-file is encountered\n\n");
  validMoves();
}

void helpExample(const char *progname) {
  fprintf(stderr, "EXAMPLES\n\n");
  fprintf(stderr,
          " %s \"L B' L' F2 U F R2 U2 F U' F2 R2 F2 U' L2 U2 B' R'\"\n\n",
          progname);
  fprintf(stderr, " %s < tests/benbotto.txt\n\n", progname);
  fprintf(stderr, " %s -help enares\n\n", progname);
  fprintf(stderr, " %s -help qtm\n\n", progname);
}

// prompt user for input when stdin is a terminal
void prompt() {
  if (isatty(0)) {
    fprintf(stderr,
            "Enter scramble in Singmaster notation (Ctrl+D to exit):\n");
  }
}

int main(int argc, char *argv[]) {

  const char *progname = argv[0];

  auto arguments = options.parse(
      argc, argv, argUsage, argDetails,
      [&progname]() { helpExample(progname); });

  if (arguments.size() > 1) {
    options.usage(progname, argUsage);
    return 1;
  }

  Janus::Cube cube(options, &console, &loadFile, &saveFile);

  if (arguments.size() == 1) {

    if (!solveScramble(arguments[0], cube, false)) {
      validMoves();
      return 1;
    }

  } else {

    // try stdin
    prompt();

    char buf[1024];
    while (fgets(buf, 1024, stdin) != NULL) {
      if (!solveScramble(buf, cube, false)) {
        validMoves();
        return 1;
      }
      prompt();
    }
  }

  return 0;
}
