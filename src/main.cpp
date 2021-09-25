// Copyright (C) 2021 Greg Dionne
// Distributed under MIT License
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <unistd.h>

#include "janus/cube.hpp"

const char *moveString[] = {"F",  "R",  "U",  "B",  "L",  "D",
                            "F'", "R'", "U'", "B'", "L'", "D'",
                            "F2", "R2", "U2", "B2", "L2", "D2"};

void usage(const char *progname) {
  fprintf(stderr, "usage:  %s [\"move1 move2 move3 ...\"]\n", progname);
  fprintf(stderr, "valid moves are entered in Singmaster notation:\n");
  fprintf(stderr, " F  R  U  B  L  D  (clockwise moves)\n");
  fprintf(stderr, " F' R' U' B' L' D' (counter-clockwise moves)\n");
  fprintf(stderr, " F2 R2 U2 B2 L2 D2 (half-turn moves)\n");
  fprintf(
      stderr,
      "example:  %s \"L B' L' F2 U F R2 U2 F U' F2 R2 F2 U' L2 U2 B' R'\"\n",
      progname);
}

void printSolution(std::size_t solutionNumber, Janus::Solution solution) {
  if (solution.empty()) {
    printf("No moves required.  Cube is already solved.\n");
  } else {
    if (solutionNumber == 1) {
      printf("minimal %lu-move solution(s) found:\n", solution.size());
    }

    printf("%2lu: ", solutionNumber);

    for (std::size_t i=0; i<solution.size(); ++i) {
      if (i+1<solution.size() && solution[i]%3 == solution[i+1]%3) {
        printf("(");
      }
      printf("%s", moveString[solution[i]]);
      if (i>0 && solution[i-1]%3 == solution[i]%3) {
        printf(")");
      }
      printf(" ");
    }
    printf("\n");
  }
}

bool solveScramble(const char *progname, const char *moves, Janus::Cube &cube) {

  cube.clear();

  fprintf(stderr, "Solving Scramble: ");

  std::size_t pos = 0;
  while (pos < strlen(moves)) {
    if (moves[pos] == ' ' || moves[pos] == '\n') {
      ++pos;
    } else {
      for (int i = 17; i >= -1; --i) {
        if (i == -1) {
          fprintf(stderr, "Unrecognized input: \"%s\"\n", moves + pos);
          usage(progname);
          return false;
        } else if (!strncmp(moves + pos, moveString[i],
                            strlen(moveString[i]))) {
          cube.move(i);
          fprintf(stderr, "%s ", moveString[i]);
          pos += strlen(moveString[i]);
          break;
        }
      }
    }
  }

  fprintf(stderr, "\n");

  if (cube.solve(printSolution).empty()) {
    fprintf(stderr, "No solution found.\n");
    return false;
  }

  return true;
}

// prompt user for input when stdin is a terminal
void prompt() {
  if (isatty(0)) {
    fprintf(stderr,
            "Enter scramble in Singmaster notation (Ctrl+D to exit):\n");
  }
}

int main(int argc, char *argv[]) {

  Janus::Cube cube;

  const char *progname = argv[0];

  if (argc > 2) {
    // too many inputs
    usage(progname);
    return 1;

  } else if (argc == 2) {

    // scramble is argv[1] on command line
    if (!solveScramble(progname, argv[1], cube)) {
      return 1;
    }

  } else {

    // try stdin
    prompt();

    char buf[1024];
    while (fgets(buf, 1024, stdin) != NULL) {
      if (!solveScramble(progname, buf, cube)) {
        return 1;
      }
      prompt();
    }
  }

  return 0;
}
