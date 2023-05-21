#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <numeric>
#include <string>

#include "janus/strutils.hpp"
#include "janus_core.hpp"

Janus::CLIOptions options;

static const char *moveString[] = {"F",  "R",  "U",  "B",  "L",  "D",
                                   "F'", "R'", "U'", "B'", "L'", "D'",
                                   "F2", "R2", "U2", "B2", "L2", "D2"};

static std::string depthTableFilename() {
  std::string filename("depthTable-");
  filename += options.qtm.isEnabled() ? "QTM" : "FTM";
  filename += options.enares.isEnabled() ? "-enares" : "";
  filename += ".janus";
  return filename;
}

bool loadFile(uint8_t *data, std::size_t nBytes) {

  auto filename = depthTableFilename();

  std::FILE *fp = std::fopen(filename.c_str(), "rb");

  if (fp == NULL) {
    // failed to open
    std::perror(filename.c_str());
    return false;
  }

  fprintf(stderr, "reading %s... ", filename.c_str());
  fflush(stderr);

  // try reading it
  std::size_t result = std::fread(data, 1, nBytes, fp);
  fclose(fp);

  fprintf(stderr, "%s bytes read\n", Janus::to_commastring(result, 14).c_str());

  if (result != nBytes) {
    fprintf(stderr, "incorrect number of bytes read from %s\n",
            filename.c_str());
    fprintf(stderr, "expected: %s\n",
            Janus::to_commastring(nBytes, 14).c_str());
    return false;
  }

  return true;
}

bool saveFile(const uint8_t *data, std::size_t nBytes) {

  auto filename = depthTableFilename();

  std::FILE *fp = std::fopen(filename.c_str(), "wb");

  if (fp == NULL) {
    std::perror(filename.c_str());
    return false;
  }

  fprintf(stderr, "writing %s... ", filename.c_str());
  fflush(stderr);

  std::size_t result = std::fwrite(data, 1, nBytes, fp);
  fclose(fp);

  fprintf(stderr, "%s bytes written\n",
          Janus::to_commastring(result, 14).c_str());

  // handle if write failed
  if (result != nBytes) {
    fprintf(stderr, "incorrect number of bytes written to %s\n",
            filename.c_str());
    fprintf(stderr, "%s bytes expected ",
            Janus::to_commastring(nBytes, 14).c_str());
    if (std::remove(filename.c_str())) {
      fprintf(stderr, "Couldn't remove incomplete file\n");
      perror(filename.c_str());
    }
    return false;
  }

  return true;
}

static void printDepth(std::size_t depth) {
  console("searching depth " + std::to_string(depth) + "...\n");
}

static void printSolution(std::size_t n, const Janus::Solution &solution) {
  if (n == 1) {
    auto nMoves =
        options.qtm.isEnabled()
            ? std::accumulate(solution.cbegin(), solution.cend(),
                              static_cast<std::size_t>(0),
                              [](std::size_t sum, uint8_t m) {
                                return sum + 1 +
                                       static_cast<int>(m >=
                                                        Janus::nQuarterTwists);
                              })
            : solution.size();
    auto adjective = options.qtm.isEnabled() ? "quarter" : "face";

    consoleOut("minimal " + std::to_string(nMoves) + "-move (" + adjective +
               " turn) solution(s) found:\n");
  }
  printSolutionNumber(n);
  for (std::size_t i = 0; i < solution.size(); ++i) {
    if (i + 1 < solution.size() && solution[i] % 3 == solution[i + 1] % 3) {
      consoleOut("(");
    }
    consoleOut(moveString[solution[i]]);
    if (i > 0 && solution[i - 1] % 3 == solution[i] % 3) {
      consoleOut(")");
    }
    consoleOut(" ");
  }
  consoleOut("\n");
}

static void searchTerminated(bool success) {
  std::string msg = "search ";
  msg += success ? "complete" : "aborted";
  msg += '\n';
  console(msg);
}

static bool move(const char *moves, Janus::Cube &cube) {
  std::size_t pos = 0;
  while (pos < strlen(moves)) {
    if (moves[pos] == ' ' || moves[pos] == '\n') {
      ++pos;
    } else {
      for (int i = 17; i >= -1; --i) {
        if (i == -1) {
          fprintf(stderr, "Unrecognized input: \"%s\"\n", moves + pos);
          return false;
        } else if (!strncmp(moves + pos, moveString[i],
                            strlen(moveString[i]))) {
          cube.move(i);
          pos += strlen(moveString[i]);
          break;
        }
      }
    }
  }
  return true;
}

bool solveScramble(const char *moves, Janus::Cube &cube, bool async) {

  cube.reset();

  // strip trailing '\n'
  std::string tmpmoves = moves;
  if (tmpmoves.back() == '\n') {
    tmpmoves.pop_back();
  }
  console("solving scramble \"");
  console(tmpmoves.c_str());
  console("\"\n");

  if (!move(moves, cube)) {
    return false;
  }

  cube.solve(&printDepth, &printSolution, &searchTerminated, async);

  return true;
}
