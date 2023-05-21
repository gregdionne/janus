#include "janus/cube.hpp"

// provided by core
extern Janus::CLIOptions options;
extern bool loadFile(uint8_t *data, std::size_t nBytes);
extern bool saveFile(const uint8_t *data, std::size_t nBytes);
extern bool solveScramble(const char *moves, Janus::Cube &cube, bool async);

// client must implement
extern void console(const std::string &message);
extern void consoleOut(const std::string &message);
extern void printSolutionNumber(std::size_t n);
