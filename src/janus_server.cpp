// Copyright (C) 2021-2022 Greg Dionne
// Distributed under MIT License
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <memory>
#include <numeric>
#include <string>

#include "janus/cube.hpp"
#include "janus/options.hpp"
#include "janus/strutils.hpp"

void closeSocket();
int createServer(const char *port, std::function<void()> callback);
int readSocket(char *buf, int bufsiz);
int writeSocket(const char *buf, int bufsiz);

auto moveMetric = Janus::MoveMetric::FaceTurn;
auto naso = Janus::Naso::Disparilis;

const char *filename = nullptr;

static const char nMoves = 18;
static const char *moveString[] = {"F",  "R",  "U",  "B",  "L",  "D",
                                   "F'", "R'", "U'", "B'", "L'", "D'",
                                   "F2", "R2", "U2", "B2", "L2", "D2"};

void console(const char *message) {
  printf("%s", message);
  fflush(stdout);
  writeSocket(message, strlen(message));
}

void console(const std::string &message) { console(message.c_str()); }

void consoleStr(const std::string &message) { console(message.c_str()); }

bool loadFile(uint8_t *data, std::size_t nBytes) {

  std::FILE *fp = std::fopen(filename, "rb");

  if (fp == NULL) {
    // failed to open
    std::perror(filename);
    return false;
  }

  printf("reading %s... ", filename);
  fflush(stdout);

  // try reading it
  std::size_t result = std::fread(data, 1, nBytes, fp);
  fclose(fp);

  printf("%s bytes read\n", Janus::to_commastring(result, 14).c_str());

  if (result != nBytes) {
    printf("incorrect number of bytes read from %s\n", filename);
    printf("expected: %s\n", Janus::to_commastring(nBytes, 14).c_str());
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

  printf("writing %s... ", filename);
  fflush(stdout);

  std::size_t result = std::fwrite(data, 1, nBytes, fp);
  fclose(fp);

  printf("%s bytes written\n", Janus::to_commastring(result, 14).c_str());

  // handle if write failed
  if (result != nBytes) {
    printf("incorrect number of bytes written to %s\n", filename);
    printf("%s bytes written\n", Janus::to_commastring(result, 14).c_str());
    printf("%s bytes expected\n", Janus::to_commastring(nBytes, 14).c_str());
    if (std::remove(filename)) {
      fprintf(stderr, "Couldn't remove incomplete file\n");
      perror(filename);
    }
    return false;
  }

  return true;
}

void printNewDepth(std::size_t depth) {
  console("searching depth " + std::to_string(depth) + "...\n");
}

void printSolution(std::size_t n, const Janus::Solution &solution) {
  if (n == 1) {
    auto nMoves = Janus::selectQH(
        moveMetric,
        std::accumulate(
            solution.cbegin(), solution.cend(), static_cast<std::size_t>(0),
            [](std::size_t sum, uint8_t m) {
              return sum + 1 + static_cast<int>(m >= Janus::nQuarterTwists);
            }),
        solution.size());
    auto adjective = Janus::selectQH(moveMetric, "quarter", "face");

    console("minimal " + std::to_string(nMoves) + "-move (" + adjective +
            " turn) solution(s) found:\n");
  }
  console("solution ");
  console(std::to_string(n));
  console(": ");
  for (std::size_t i = 0; i < solution.size(); ++i) {
    if (i + 1 < solution.size() && solution[i] % 3 == solution[i + 1] % 3) {
      console("(");
    }
    console(moveString[solution[i]]);
    if (i > 0 && solution[i - 1] % 3 == solution[i] % 3) {
      console(")");
    }
    console(" ");
  }
  console("\n");
}

void searchTerminated(bool success) {
  std::string msg = "search ";
  msg += success ? "complete" : "aborted";
  msg += '\n';
  console(msg);
}

void unrecognized(char *str) {
  if (strrchr(str, '\n')) {
    *strrchr(str, '\n') = '\0';
  }

  console("unrecognized input: \"");
  console(str);
  console("\"\n");
  console("enter \"help\" for help\n");
}

void move(char *moves, Janus::Cube &cube) {
  std::size_t pos = 0;
  while (pos < strlen(moves)) {
    if (moves[pos] == ' ' || moves[pos] == '\n') {
      ++pos;
    } else {
      for (int i = nMoves - 1; i >= -1; --i) {
        if (i == -1) {
          console("error: move: ");
          unrecognized(moves + pos);
          return;
        } else if (!strncmp(moves + pos, moveString[i],
                            strlen(moveString[i]))) {
          cube.move(i);
          pos += strlen(moveString[i]);
          break;
        }
      }
    }
  }
}

void cmdMetric(const Janus::MoveMetric moveMetric) {
  console(Janus::selectQH(moveMetric, "quater-turn\n", "face-turn\n"));
}

static const char *const cmdList[] = {
    "valid commands are \"help\", \"metric\", \"abort\", \"solve\", or "
    "\"exit\".\n\n",
    "  help\n",
    "    prints this help message\n\n",
    "  metric\n",
    "    prints the current move metric (face-turn or quater-turn).\n",
    "    The quater-turn metric can be invoked via the \"-q\" option "
    "switch\n",
    "    when starting the server from the command line.\n\n",
    "  abort\n",
    "    stops any solution in progress.\n\n",
    "  solve  <moves>\n",
    "    prints all minimal solutions using the current metric\n",
    "    valid moves are entered in Singmaster notation:\n",
    "      F  R  U  B  L  D  (clockwise moves)\n",
    "      F' R' U' B' L' D' (counter-clockwise moves)\n",
    "      F2 R2 U2 B2 L2 D2 (half-turn moves)\n\n",
    "    When reporting solutions, some pairs of moves are placed\n",
    "    within parentheses. The moves within each pair twist\n",
    "    opposing faces of the cube (e.g. front and back) and may\n",
    "    be entered in either order without affacting the solution.\n\n",
    "  exit\n",
    "    closes the program\n\n",
    nullptr};

void cmdHelp() {
  console("Back-end terminal interface program for Janus\n\n");
  for (int i = 0; cmdList[i]; ++i) {
    console(cmdList[i]);
  }
}

void cmdAbort(Janus::Cube &cube) { cube.reset(); }

void cmdSolve(char *moves, Janus::Cube &cube) {
  cube.reset();

  // strip trailing '\n'
  std::string tmpmoves = moves;
  tmpmoves.pop_back();
  console("solving scramble \"");
  console(tmpmoves.c_str());
  console("\"\n");

  move(moves, cube);
  cube.solve(&printNewDepth, &printSolution, &searchTerminated);
}

void prompt() { console("ready\n"); }

void argUsage() { fprintf(stderr, " port\n\n"); }

void argDetails(const char *progname) {
  fprintf(stderr, "DETAILS\n");
  fprintf(stderr, "  port:  port id to host (e.g., 3490)\n");
  fprintf(stderr, "  %s hosts a TCP server on the specified port.\n\n",
          progname);
  fprintf(stderr, "  To use the server, connect to it via TCP.\n\n");
  fprintf(stderr, "  The server replies 'ready' whenever it can accept "
                  "a new command.\n\n");

  for (int i = 0; cmdList[i]; ++i) {
    fprintf(stderr, "  %s", cmdList[i]);
  }
}

void helpExample(const char *progname) {
  fprintf(stderr, "EXAMPLE\n\n");
  fprintf(stderr, "  In one terminal, start the server by entering:\n");
  fprintf(stderr, "    %s 3490\n\n", progname);
  fprintf(stderr, "  Connect to the server via TCP from your own front-end "
                  "program.\n");
  fprintf(stderr, "  For example, using a program like netcat (nc) enter:\n");
  fprintf(stderr, "    nc 127.0.0.1 3490\n\n");
  fprintf(stderr, "  Janus will reply with 'ready'.  Then enter:\n");
  fprintf(stderr, "    solve F R U U F U F L B D U F D B L U D F F U\n\n");
  fprintf(stderr, "  Janus will reply with:\n");
  fprintf(
      stderr,
      "    solving scramble: \"F R U U F U F L B D U F D B L U D F F U\"\n");
  fprintf(stderr, "    ready\n\n");
  fprintf(stderr, "  At this point the server may told to abort or solve a new "
                  "scramble.\n");
  fprintf(stderr, "  Otherwise, it reports solutions as it finds them in the "
                  "following format:\n");
  fprintf(stderr, "    searching depth 0...\n");
  fprintf(stderr, "    searching depth 1...\n");
  fprintf(stderr, "    searching depth 2...\n");
  fprintf(stderr, "    searching depth 3...\n");
  fprintf(stderr, "    searching depth 4...\n");
  fprintf(stderr, "    searching depth 5...\n");
  fprintf(stderr, "    searching depth 6...\n");
  fprintf(stderr, "    searching depth 7...\n");
  fprintf(stderr, "    searching depth 8...\n");
  fprintf(stderr, "    searching depth 9...\n");
  fprintf(stderr, "    searching depth 10...\n");
  fprintf(stderr, "    searching depth 11...\n");
  fprintf(stderr, "    searching depth 12...\n");
  fprintf(stderr, "    searching depth 13...\n");
  fprintf(stderr, "    searching depth 14...\n");
  fprintf(stderr, "    searching depth 15...\n");
  fprintf(stderr, "    searching depth 16...\n");
  fprintf(stderr, "    searching depth 17...\n");
  fprintf(stderr, "    searching depth 18...\n");
  fprintf(stderr, "    minimal 18-move (face turn) solution(s) found:\n");
  fprintf(
      stderr,
      "    solution 1: R B' D L2 F' D B L' U' F' U L' (U D2) (R' L2) (F B2)\n");
  fprintf(stderr, "    solution 2: U' F2 (U' D') L' B' D' F' (U' D') B' L' F' "
                  "U' F' U2 R' F'\n");
  fprintf(stderr,
          "    solution 3: D' L U F' U' B' R' (U D) R2 D2 L2 D' R F2 U R U\n");
  fprintf(stderr,
          "    solution 4: R2 F' D2 R F2 L' B L' D' F2 D L2 F D2 B2 L2 D B'\n");
  fprintf(stderr,
          "    solution 5: (R2 L2) U F' U2 L U2 R' B U R B2 R F R2 F2 U R2\n");
  fprintf(stderr,
          "    solution 6: U2 F2 R2 U F U2 R' U2 F2 L D R U2 F' (R L2) F D\n");
  fprintf(stderr, "    search complete\n\n");
}

int main(int argc, char *argv[]) {

  Janus::Options options;

  const auto progname = argv[0];
  auto arguments = options.parse(
      argc, argv, argUsage, [&progname]() { argDetails(progname); },
      [&progname]() { helpExample(progname); });

  if (arguments.size() != 1) {
    options.usage(argv[0], argUsage);
    return 1;
  }

  filename = "depthTable-FTM.janus";

  if (options.qtm.isEnabled()) {
    moveMetric = Janus::MoveMetric::QuarterTurn;
    filename = "depthTable-QTM.janus";
  }

  if (options.enares.isEnabled()) {
    naso = Janus::Naso::Aequivalens;
    filename = moveMetric == Janus::MoveMetric::QuarterTurn
                   ? "depthTable-QTM-enares.janus"
                   : "depthTable-FTM-enares.janus";
  }

  printf("Initializing...\n");
  Janus::Cube cube(moveMetric, naso, &consoleStr, &loadFile, &saveFile);

  createServer(arguments[0], [&cube]() -> void {
    cube.reset();

    prompt();

    char buf[BUFSIZ];
    for (;;) {
      int n = readSocket(buf, BUFSIZ - 1);
      if (n < 0) {
        break;
      } else {
        buf[n] = '\0';

        if (!strncmp(buf, "help", 4)) {
          cmdHelp();
        } else if (!strncmp(buf, "metric", 6)) {
          cmdMetric(moveMetric);
        } else if (!strncmp(buf, "abort", 5)) {
          cmdAbort(cube);
        } else if (!strncmp(buf, "solve", 5)) {
          cmdSolve(buf + 6, cube);
        } else if (!strncmp(buf, "exit", 4)) {
          break;
        } else if (n > 0) {
          console("error: ");
          unrecognized(buf);
        } else {
          printf("read failed!\n");
          break;
        }
        prompt();
      }
    }
    closeSocket();
    console("session closed\n");
  });

  return 0;
}
