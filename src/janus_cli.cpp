// Copyright (C) 2021-2022 Greg Dionne
// Distributed under MIT License
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <numeric>
#include <string>
#include <unistd.h>

#include "janus/cube.hpp"
#include "janus/options.hpp"
#include "janus/strutils.hpp"

auto moveMetric = Janus::MoveMetric::FaceTurn;
auto naso = Janus::Naso::Disparilis;

const char *filename = nullptr;

const char *moveString[] = {"F",  "R",  "U",  "B",  "L",  "D",
                            "F'", "R'", "U'", "B'", "L'", "D'",
                            "F2", "R2", "U2", "B2", "L2", "D2"};

void argUsage() { fprintf(stderr, "[\"move1 move2 move3 ...\"]\n\n"); }

void argDetails(const char *) {
  fprintf(stderr, "DESCRIPTION\n\n");
  fprintf(stderr, " If moves are not supplied as a quoted string,\n");
  fprintf(stderr, " then they are repeatedly read from the standard input\n");
  fprintf(stderr, " until an end-of-file is encountered\n\n");
  fprintf(stderr, " valid moves are entered in Singmaster notation:\n");
  fprintf(stderr, "  F  R  U  B  L  D  (clockwise moves)\n");
  fprintf(stderr, "  F' R' U' B' L' D' (counter-clockwise moves)\n");
  fprintf(stderr, "  F2 R2 U2 B2 L2 D2 (half-turn moves)\n\n");
}

void helpExample(const char *progname) {
  fprintf(stderr, "EXAMPLE\n\n");
  fprintf(stderr,
          " %s \"L B' L' F2 U F R2 U2 F U' F2 R2 F2 U' L2 U2 B' R'\"\n\n",
          progname);
}

void console(const std::string &message) {
  fprintf(stderr, "%s", message.c_str());
  fflush(stderr);
}

bool loadFile(uint8_t *data, std::size_t nBytes) {

  std::FILE *fp = std::fopen(filename, "rb");

  if (fp == NULL) {
    // failed to open
    std::perror(filename);
    return false;
  }

  fprintf(stderr, "reading %s... ", filename);
  fflush(stderr);

  // try reading it
  std::size_t result = std::fread(data, 1, nBytes, fp);
  fclose(fp);

  fprintf(stderr, "%s bytes read\n", Janus::to_commastring(result, 14).c_str());

  if (result != nBytes) {
    fprintf(stderr, "incorrect number of bytes read from %s\n", filename);
    fprintf(stderr, "expected: %s\n",
            Janus::to_commastring(nBytes, 14).c_str());
    return false;
  }

  return true;
}

bool saveFile(const uint8_t *data, std::size_t nBytes) {
  std::FILE *fp = std::fopen(filename, "wb");

  if (fp == NULL) {
    std::perror(filename);
    return false;
  }

  fprintf(stderr, "writing %s... ", filename);
  fflush(stderr);

  std::size_t result = std::fwrite(data, 1, nBytes, fp);
  fclose(fp);

  fprintf(stderr, "%s bytes written\n",
          Janus::to_commastring(result, 14).c_str());

  // handle if write failed
  if (result != nBytes) {
    fprintf(stderr, "incorrect number of bytes written to %s\n", filename);
    fprintf(stderr, "%s bytes expected ",
            Janus::to_commastring(nBytes, 14).c_str());
    fprintf(stderr, "(22GB required)\n");
    if (std::remove(filename)) {
      fprintf(stderr, "Couldn't remove incomplete file\n");
      perror(filename);
    }
    return false;
  }

  return true;
}

void printDepth(uint8_t depth) {
  fprintf(stderr, "Searching depth %i...\n", depth);
}

void printSolution(std::size_t solutionNumber,
                   const Janus::Solution &solution) {
  if (solution.empty()) {
    printf("No moves required.  Cube is already solved.\n");
  } else {
    if (solutionNumber == 1) {
      auto nMoves = Janus::selectQH(
          moveMetric,
          std::accumulate(
              solution.cbegin(), solution.cend(), static_cast<std::size_t>(0),
              [](std::size_t sum, uint8_t m) {
                return sum + 1 + static_cast<int>(m >= Janus::nQuarterTwists);
              }),
          solution.size());
      auto adjective = Janus::selectQH(moveMetric, "quarter", "face");
      printf("minimal %lu-move (%s turn) solution(s) found:\n", nMoves,
             adjective);
    }

    printf("%2lu: ", solutionNumber);

    for (std::size_t i = 0; i < solution.size(); ++i) {
      if (i + 1 < solution.size() && solution[i] % 3 == solution[i + 1] % 3) {
        printf("(");
      }
      printf("%s", moveString[solution[i]]);
      if (i > 0 && solution[i - 1] % 3 == solution[i] % 3) {
        printf(")");
      }
      printf(" ");
    }
    printf("\n");
  }
}

void searchTerminated(bool success) {
  std::string msg = "search ";
  msg += success ? "complete" : "aborted";
  msg += '\n';
  console(msg);
}

bool solveScramble(const char *progname, const char *moves, Janus::Cube &cube) {

  cube.reset();

  fprintf(stderr, "Solving Scramble: ");

  std::size_t pos = 0;
  while (pos < strlen(moves)) {
    if (moves[pos] == ' ' || moves[pos] == '\n') {
      ++pos;
    } else {
      for (int i = 17; i >= -1; --i) {
        if (i == -1) {
          fprintf(stderr, "Unrecognized input: \"%s\"\n", moves + pos);
          argDetails(progname);
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

  cube.solve(printDepth, printSolution, searchTerminated, false);

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

  const char *progname = argv[0];

  Janus::Options options;
  auto arguments = options.parse(
      argc, argv, argUsage, [&progname]() { argDetails(progname); },
      [&progname]() { helpExample(progname); });

  if (arguments.size() > 1) {
    options.usage(progname, argUsage);
    return 1;
  }

  filename = "depthTable-FTM.janus";

  if (options.qtm.isEnabled()) {
    filename = "depthTable-QTM.janus";
    moveMetric = Janus::MoveMetric::QuarterTurn;
  }

  if (options.enares.isEnabled()) {
    filename = moveMetric == Janus::MoveMetric::QuarterTurn
                   ? "depthTable-QTM-enares.janus"
                   : "depthTable-FTM-enares.janus";
    naso = Janus::Naso::Aequivalens;
  }

  Janus::Cube cube(moveMetric, naso, &console, &loadFile, &saveFile);

  if (arguments.size() == 1) {

    if (!solveScramble(progname, arguments[0], cube)) {
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
