//===-- Z3GlobalSolver.h
//---------------------------------------------------===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef KLEE_Z3GLOBAL_SOLVER_H
#define KLEE_Z3GLOBAL_SOLVER_H

#include "klee/Solver/Solver.h"

namespace klee {

/// Z3GlobalSolver - A complete solver based on Z3's Solver2.
class Z3GlobalSolver : public Solver {
private:
  bool internalRunSolver(const Query &,
                         const std::vector<const Array *> *objects,
                         std::vector<std::vector<unsigned char> > *values,
                         bool &hasSolution);

public:
  friend class Z3SolverImpl; // Is delegated to after CSA.

  /// Z3GlobalSolver - Construct a new Z3GlobalSolver.
  Z3GlobalSolver();

  /// Get the query in SMT-LIBv2 format.
  /// \return A C-style string. The caller is responsible for freeing this.
  std::string getConstraintLog(const Query &) override;

  /// setCoreSolverTimeout - Set constraint solver timeout delay to the given
  /// value; 0
  /// is off.
  virtual void setCoreSolverTimeout(time::Span timeout);
};
}

#endif /* KLEE_Z3GLOBAL_SOLVER_H */
