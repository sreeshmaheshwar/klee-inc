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
#include "klee/Config/config.h"
#include "klee/Support/ErrorHandling.h"
#include "klee/Support/FileHandling.h"
#include "klee/Support/OptionCategories.h"

#include <csignal>
#include <memory>

#include "Z3Solver.h"
#include "Z3Builder.h"

#include "klee/Expr/Constraints.h"
#include "klee/Expr/Assignment.h"
#include "klee/Expr/ExprUtil.h"
#include "klee/Solver/Solver.h"
#include "klee/Solver/SolverImpl.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"

#include "Z3GlobalSolver.h"

namespace klee {
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

class Z3SolverImpl : public SolverImpl {
private:
  bool validateZ3Model(::Z3_solver &theSolver, ::Z3_model &theModel);

public:
  // FIXME: Make where needed friendly to pooling solver.
  Z3_solver z3Solver;
  std::unique_ptr<Z3Builder> builder;
  time::Span timeout;
  SolverRunStatus runStatusCode;
  std::unique_ptr<Z3GlobalSolver> globalSolver;
  std::unique_ptr<llvm::raw_fd_ostream> dumpedQueriesFile;
  std::unique_ptr<llvm::raw_fd_ostream> coreTimesFile;
  ::Z3_params solverParameters;
  // Parameter symbols
  ::Z3_symbol timeoutParamStrSymbol;

  bool internalRunSolver(const Query &,
                         const std::vector<const Array *> *objects,
                         std::vector<std::vector<unsigned char> > *values,
                         bool &hasSolution);

  Z3SolverImpl();
  ~Z3SolverImpl();

  std::string getConstraintLog(const Query &) override;
  void setCoreSolverTimeout(time::Span _timeout);

  bool computeTruth(const Query &, bool &isValid);
  bool computeValue(const Query &, ref<Expr> &result);
  bool computeInitialValues(const Query &,
                            const std::vector<const Array *> &objects,
                            std::vector<std::vector<unsigned char> > &values,
                            bool &hasSolution);
  SolverRunStatus
  handleSolverResponse(::Z3_solver theSolver, ::Z3_lbool satisfiable,
                       const std::vector<const Array *> *objects,
                       std::vector<std::vector<unsigned char> > *values,
                       bool &hasSolution);
  SolverRunStatus getOperationStatusCode();
};
}

#endif /* KLEE_Z3SOLVER_H */
