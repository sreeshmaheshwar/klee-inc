//===-- ValidatingSolver.cpp ----------------------------------------------===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "klee/Expr/Constraints.h"
#include "klee/Solver/Solver.h"
#include "klee/Solver/SolverImpl.h"

#include <memory>
#include <utility>
#include <vector>

namespace klee {

class ValidatingSolver : public SolverImpl {
private:
  std::unique_ptr<Solver> solver;
  std::unique_ptr<Solver, void(*)(Solver*)> oracle;

public:
  ValidatingSolver(std::unique_ptr<Solver> solver, Solver *oracle,
                   bool ownsOracle)
      : solver(std::move(solver)),
        oracle(
            oracle, ownsOracle ? [](Solver *solver) { delete solver; }
                               : [](Solver *) {}) {}

  bool computeValidity(Query &, Solver::Validity &result);
  bool computeTruth(Query &, bool &isValid);
  bool computeValue(Query &, ref<Expr> &result);
  bool computeInitialValues(Query &,
                            const std::vector<const Array *> &objects,
                            std::vector<std::vector<unsigned char>> &values,
                            bool &hasSolution);
  SolverRunStatus getOperationStatusCode();
  std::string getConstraintLog(Query &) override;
  void setCoreSolverTimeout(time::Span timeout);
};

bool ValidatingSolver::computeTruth(Query &query, bool &isValid) {
  bool answer;

  if (!solver->impl->computeTruth(query, isValid))
    return false;
  if (!oracle->impl->computeTruth(query, answer))
    return false;

  if (isValid != answer)
    assert(0 && "invalid solver result (computeTruth)");

  return true;
}

bool ValidatingSolver::computeValidity(Query &query,
                                       Solver::Validity &result) {
  Solver::Validity answer;

  if (!solver->impl->computeValidity(query, result))
    return false;
  if (!oracle->impl->computeValidity(query, answer))
    return false;

  if (result != answer)
    assert(0 && "invalid solver result (computeValidity)");

  return true;
}

bool ValidatingSolver::computeValue(Query &query, ref<Expr> &result) {
  return true;
}

bool ValidatingSolver::computeInitialValues(
    Query &query, const std::vector<const Array *> &objects,
    std::vector<std::vector<unsigned char> > &values, bool &hasSolution) {
  assert(0);
}

SolverImpl::SolverRunStatus ValidatingSolver::getOperationStatusCode() {
  return solver->impl->getOperationStatusCode();
}

std::string ValidatingSolver::getConstraintLog(Query &query) {
  return solver->impl->getConstraintLog(query);
}

void ValidatingSolver::setCoreSolverTimeout(time::Span timeout) {
  solver->impl->setCoreSolverTimeout(timeout);
}

std::unique_ptr<Solver> createValidatingSolver(std::unique_ptr<Solver> s,
                                               Solver *oracle,
                                               bool ownsOracle) {
  return std::make_unique<Solver>(
      std::make_unique<ValidatingSolver>(std::move(s), oracle, ownsOracle));
}
}
