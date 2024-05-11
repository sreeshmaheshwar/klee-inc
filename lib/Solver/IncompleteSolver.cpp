//===-- IncompleteSolver.cpp ----------------------------------------------===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "klee/Solver/IncompleteSolver.h"

#include "klee/Expr/Constraints.h"

#include <utility>

using namespace klee;
using namespace llvm;

/***/

IncompleteSolver::PartialValidity 
IncompleteSolver::negatePartialValidity(PartialValidity pv) {
  switch(pv) {
  default: assert(0 && "invalid partial validity");  
  case MustBeTrue:  return MustBeFalse;
  case MustBeFalse: return MustBeTrue;
  case MayBeTrue:   return MayBeFalse;
  case MayBeFalse:  return MayBeTrue;
  case TrueOrFalse: return TrueOrFalse;
  }
}

IncompleteSolver::PartialValidity 
IncompleteSolver::computeValidity(Query& query) {
 assert(0);
}

/***/

StagedSolverImpl::StagedSolverImpl(std::unique_ptr<IncompleteSolver> primary,
                                   std::unique_ptr<Solver> secondary)
    : primary(std::move(primary)), secondary(std::move(secondary)) {}

bool StagedSolverImpl::computeTruth(Query& query, bool &isValid) {
  IncompleteSolver::PartialValidity trueResult = primary->computeTruth(query); 
  
  if (trueResult != IncompleteSolver::None) {
    isValid = (trueResult == IncompleteSolver::MustBeTrue);
    return true;
  } 

  return secondary->impl->computeTruth(query, isValid);
}

bool StagedSolverImpl::computeValidity(Query& query,
                                       Solver::Validity &result) {
  return 0;
}

bool StagedSolverImpl::computeValue(Query& query,
                                    ref<Expr> &result) {
  if (primary->computeValue(query, result))
    return true;

  return secondary->impl->computeValue(query, result);
}

bool 
StagedSolverImpl::computeInitialValues(Query& query,
                                       const std::vector<const Array*> 
                                         &objects,
                                       std::vector< std::vector<unsigned char> >
                                         &values,
                                       bool &hasSolution) {
  if (primary->computeInitialValues(query, objects, values, hasSolution))
    return true;
  
  return secondary->impl->computeInitialValues(query, objects, values,
                                               hasSolution);
}

SolverImpl::SolverRunStatus StagedSolverImpl::getOperationStatusCode() {
  return secondary->impl->getOperationStatusCode();
}

std::string StagedSolverImpl::getConstraintLog(Query& query) {
  return secondary->impl->getConstraintLog(query);
}

void StagedSolverImpl::setCoreSolverTimeout(time::Span timeout) {
  secondary->impl->setCoreSolverTimeout(timeout);
}

