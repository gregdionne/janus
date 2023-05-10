// Copyright (C) 2021 Greg Dionne
// Distributed under MIT License
#ifndef UTILS_OPTIONS_HPP
#define UTILS_OPTIONS_HPP

#include "option.hpp"

#include <functional>
#include <vector>

namespace utils {

class Options {
public:
  std::vector<const char *> parse(int argc, const char *const argv[],
                                  void (*argUsage)(),
                                  const std::function<void()> &argDetails,
                                  const std::function<void()> &helpExample);
  const std::vector<Option *> &getTable() const { return table; }
  void addOption(Option *option) { table.push_back(option); }
  void usage(const char *progname, void (*argUsage)()) const;

private:
  Option *findOption(const char *arg);
  Option *findOnSwitch(const char *arg);
  Option *findOffSwitch(const char *arg);
  static void helpOptionSummary(const Option *option);
  void helpOptionDetails(const Option *option) const;
  void helpOptions() const;
  void helpTopic(const char *progname, const char *target) const;
  std::vector<Option *> table;
  const int nWrap = 65;
};

} // namespace utils

#endif
