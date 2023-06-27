// Copyright (C) 2021-2022 Greg Dionne
// Distributed under MIT License
#ifndef JANUS_RECURSER_HPP
#define JANUS_RECURSER_HPP

#include "clioptions.hpp"
#include "cubedepth.hpp"
#include "januscube.hpp"
#include "worklist.hpp"

#include <memory>

namespace Janus {

class Solver;

class Recurser {
public:
  virtual ~Recurser() = default;

  // method recurser...
  virtual bool leaf(const JanusCube &janusCube, uint8_t depth, Solution &work,
                    Solver *solver,
                    bool (Solver::*f)(const JanusCube &janusCube, uint8_t depth,
                                      Solution &work)) = 0;

  // root method recurser
  virtual bool root(const JanusCube &janusCube, uint8_t depth, Solution &work,
                    Solver *solver,
                    bool (Solver::*f)(const JanusCube &janusCube, uint8_t depth,
                                      Solution &work)) = 0;
  // utility creation
  static std::unique_ptr<Recurser> makeRecurser(const CLIOptions &options);
};

class RecurserQTM : public Recurser {
public:
  // method recurser...
  bool leaf(const JanusCube &janusCube, uint8_t depth, Solution &work,
            Solver *solver,
            bool (Solver::*f)(const JanusCube &janusCube, uint8_t depth,
                              Solution &work)) final;

  // root method recurser
  bool root(const JanusCube &janusCube, uint8_t depth, Solution &work,
            Solver *solver,
            bool (Solver::*f)(const JanusCube &janusCube, uint8_t depth,
                              Solution &work)) final;
};

class RecurserFTM : public Recurser {
public:
  // method recurser...
  bool leaf(const JanusCube &janusCube, uint8_t depth, Solution &work,
            Solver *solver,
            bool (Solver::*f)(const JanusCube &janusCube, uint8_t depth,
                              Solution &work)) final;

  // root method recurser
  bool root(const JanusCube &janusCube, uint8_t depth, Solution &work,
            Solver *solver,
            bool (Solver::*f)(const JanusCube &janusCube, uint8_t depth,
                              Solution &work)) final;
};

} // namespace Janus

#endif
