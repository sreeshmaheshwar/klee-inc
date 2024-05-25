//===-- Z3Solver.h
//---------------------------------------------------===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef KLEE_Z3SOLVER_H
#define KLEE_Z3SOLVER_H

#include "klee/Solver/Solver.h"

namespace klee {

// namespace z3_options {

// // NOTE: Very useful for debugging Z3 behaviour. These files can be given to
// // the z3 binary to replay all Z3 API calls using its `-log` option.
// extern llvm::cl::opt<std::string> Z3LogInteractionFile;

// extern llvm::cl::opt<std::string> Z3QueryDumpFile;

// extern llvm::cl::opt<bool> Z3ValidateModels;

// extern llvm::cl::opt<unsigned> Z3VerbosityLevel;

// } // namespace z3_options

// using namespace z3_options;

/// Z3Solver - A complete solver based on Z3
class Z3Solver : public Solver {
public:
  /// Z3Solver - Construct a new Z3Solver.
  Z3Solver();

  /// Get the query in SMT-LIBv2 format.
  /// \return A C-style string. The caller is responsible for freeing this.
  std::string getConstraintLog(const Query &) override;

  /// setCoreSolverTimeout - Set constraint solver timeout delay to the given
  /// value; 0
  /// is off.
  virtual void setCoreSolverTimeout(time::Span timeout);
};
}

#endif /* KLEE_Z3SOLVER_H */
