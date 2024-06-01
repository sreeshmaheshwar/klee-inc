//===-- TimingSolver.cpp --------------------------------------------------===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "TimingSolver.h"

#include "ExecutionState.h"

#include "klee/Config/Version.h"
#include "klee/Statistics/Statistics.h"
#include "klee/Statistics/TimerStatIncrementer.h"
#include "klee/Solver/Solver.h"
#include "klee/Solver/SolverStats.h"

#include "CoreStats.h"

using namespace klee;
using namespace llvm;

/***/

bool TimingSolver::evaluate(const ConstraintSet &constraints, const ConstraintSet& unsimplified, ref<Expr> expr,
                            Solver::Validity &result,
                            SolverQueryMetaData &metaData) {
  ++stats::queries;
  // Fast path, to avoid timer and OS overhead.
  if (ConstantExpr *CE = dyn_cast<ConstantExpr>(expr)) {
    result = CE->isTrue() ? Solver::True : Solver::False;
    return true;
  }

  TimerStatIncrementer timer(stats::solverTime);

  if (simplifyExprs)
    expr = ConstraintManager::simplifyExpr(constraints, expr);

  bool success = solver->evaluate(Query(constraints, expr, unsimplified), result);

  metaData.queryCost += timer.delta();

  return success;
}

bool TimingSolver::mustBeTrue(const ConstraintSet &constraints, const ConstraintSet& unsimplified, ref<Expr> expr,
                              bool &result, SolverQueryMetaData &metaData) {
  ++stats::queries;
  // Fast path, to avoid timer and OS overhead.
  if (ConstantExpr *CE = dyn_cast<ConstantExpr>(expr)) {
    result = CE->isTrue() ? true : false;
    return true;
  }

  TimerStatIncrementer timer(stats::solverTime);

  if (simplifyExprs)
    expr = ConstraintManager::simplifyExpr(constraints, expr);

  bool success = solver->mustBeTrue(Query(constraints, expr, unsimplified), result);

  metaData.queryCost += timer.delta();

  return success;
}

bool TimingSolver::mustBeFalse(const ConstraintSet &constraints, const ConstraintSet& unsimplified, ref<Expr> expr,
                               bool &result, SolverQueryMetaData &metaData) {
  return mustBeTrue(constraints, unsimplified, Expr::createIsZero(expr), result, metaData);
}

bool TimingSolver::mayBeTrue(const ConstraintSet &constraints, const ConstraintSet& unsimplified, ref<Expr> expr,
                             bool &result, SolverQueryMetaData &metaData) {
  bool res;
  if (!mustBeFalse(constraints, unsimplified, expr, res, metaData))
    return false;
  result = !res;
  return true;
}

bool TimingSolver::mayBeFalse(const ConstraintSet &constraints, const ConstraintSet& unsimplified, ref<Expr> expr,
                              bool &result, SolverQueryMetaData &metaData) {
  bool res;
  if (!mustBeTrue(constraints, unsimplified, expr, res, metaData))
    return false;
  result = !res;
  return true;
}

bool TimingSolver::getValue(const ConstraintSet &constraints, const ConstraintSet& unsimplified, ref<Expr> expr,
                            ref<ConstantExpr> &result,
                            SolverQueryMetaData &metaData) {
  ++stats::queries;
  // Fast path, to avoid timer and OS overhead.
  if (ConstantExpr *CE = dyn_cast<ConstantExpr>(expr)) {
    result = CE;
    return true;
  }
  
  TimerStatIncrementer timer(stats::solverTime);

  if (simplifyExprs)
    expr = ConstraintManager::simplifyExpr(constraints, expr);

  bool success = solver->getValue(Query(constraints, expr, unsimplified), result);

  metaData.queryCost += timer.delta();

  return success;
}

bool TimingSolver::getInitialValues(
    const ConstraintSet &constraints, const ConstraintSet& unsimplified, const std::vector<const Array *> &objects,
    std::vector<std::vector<unsigned char>> &result,
    SolverQueryMetaData &metaData) {
  ++stats::queries;
  if (objects.empty())
    return true;

  TimerStatIncrementer timer(stats::solverTime);

  bool success = solver->getInitialValues(
      Query(constraints, ConstantExpr::alloc(0, Expr::Bool), unsimplified), objects, result);

  metaData.queryCost += timer.delta();

  return success;
}

std::pair<ref<Expr>, ref<Expr>>
TimingSolver::getRange(const ConstraintSet &constraints, const ConstraintSet& unsimplified, ref<Expr> expr,
                       SolverQueryMetaData &metaData) {
  ++stats::queries;
  TimerStatIncrementer timer(stats::solverTime);
  auto result = solver->getRange(Query(constraints, expr, unsimplified));
  metaData.queryCost += timer.delta();
  return result;
}
