//===-- Z3PoolingSolver.h -------------------------------------------*- C++ -*-====//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "Z3PoolingSolver.h"
#include "Z3Solver.h"
#include "klee/Solver/SolverImpl.h"
#include <algorithm>
#include <memory>
#include <vector>

namespace {

llvm::cl::opt<unsigned>
    PoolSize("pool-size", llvm::cl::init(14),
                     llvm::cl::desc("Number of Z3 solvers in the pool (default=14)"),
                     llvm::cl::cat(klee::SolvingCat));
llvm::cl::opt<bool>
    WarnSolverChoices("pool-warn", llvm::cl::init(false),
                     llvm::cl::desc("Warn pool solver choices (default=false)"),
                     llvm::cl::cat(klee::SolvingCat));

llvm::cl::opt<unsigned>
    PercentLeeway("pool-percent", llvm::cl::init(5),
                     llvm::cl::desc("Percentage of pool leeway to allow (default=5)"),
                     llvm::cl::cat(klee::SolvingCat));

}

namespace klee {

// TODO: Implement remaining SolverImpl methods, e.g. timeout.
class Z3PoolingSolverImpl : public SolverImpl {
  private:
    std::vector<std::unique_ptr<Z3SolverImpl>> pool;
    std::vector<int> lastUsed;
    int previousId;
    int count;

    bool internalRunSolver(const Query &,
                            const std::vector<const Array *> *objects,
                            std::vector<std::vector<unsigned char> > *values,
                            bool &hasSolution);

  public:
    Z3PoolingSolverImpl();
    // Destructor will destroy the pool.

    bool computeTruth(const Query &, bool &isValid) override;
    bool computeValue(const Query &, ref<Expr> &result) override;
    bool computeInitialValues(const Query &,
                                const std::vector<const Array *> &objects,
                                std::vector<std::vector<unsigned char> > &values,
                                bool &hasSolution) override;

    SolverRunStatus getOperationStatusCode() override;
};

Z3PoolingSolverImpl::Z3PoolingSolverImpl() : lastUsed(PoolSize, 0), previousId(0), count(0) {
  if (PoolSize < 1) {
    klee_error("Pool size must be at least 1");
  }
  for (auto i = 0u; i < PoolSize; ++i) {
    pool.push_back(std::make_unique<Z3SolverImpl>());
  }
}

bool Z3PoolingSolverImpl::computeTruth(const Query &query, bool &isValid) {
  bool hasSolution = false; // to remove compiler warning
  bool status =
      internalRunSolver(query, /*objects=*/NULL, /*values=*/NULL, hasSolution);
  isValid = !hasSolution;
  return status;
}

bool Z3PoolingSolverImpl::computeValue(const Query &query, ref<Expr> &result) {
  std::vector<const Array *> objects;
  std::vector<std::vector<unsigned char> > values;
  bool hasSolution;

  // Find the object used in the expression, and compute an assignment
  // for them.
  findSymbolicObjects(query.expr, objects);
  if (!computeInitialValues(query.withFalse(), objects, values, hasSolution))
    return false;
  assert(hasSolution && "state has invalid constraint set");

  // Evaluate the expression with the computed assignment.
  Assignment a(objects, values);
  result = a.evaluate(query.expr);

  return true;
}

bool Z3PoolingSolverImpl::computeInitialValues(
    const Query &query, const std::vector<const Array *> &objects,
    std::vector<std::vector<unsigned char> > &values, bool &hasSolution) {
  return internalRunSolver(query, &objects, &values, hasSolution);
}

bool Z3PoolingSolverImpl::internalRunSolver(
    const Query &_query, const std::vector<const Array *> *objects,
    std::vector<std::vector<unsigned char> > *values, bool &hasSolution) {
  Query query(_query.constraintsToWrite, _query.expr); // Use the constraints to write exclusively (the whole set).

  int maxLCP = 0;
  std::vector<int> lcps(PoolSize);
  size_t largestSize = 0;
  for (auto id = 0u; id < PoolSize; ++id) {
    const auto& assertionStack = pool[id]->assertionStack;
    auto stack_it = assertionStack.begin();
    auto query_it = query.constraints.begin();

    // KLEE Queries are validity queries i.e.
    // ∀ X Constraints(X) → query(X)
    // but Z3 works in terms of satisfiability so instead we ask the
    // negation of the equivalent i.e.
    // ∃ X Constraints(X) ∧ ¬ query(X)
    auto negated_exp = Expr::createIsZero(query.expr);
    bool expr_done = false;

    // We can't modify the query constraints (by pushing the negated
    // expression to it) and don't want to create a copy of them. So
    // we take the approach of manually advancing custom iterators.
    auto all_written = [&]() -> bool {
      return query_it == query.constraints.end() && expr_done;
    };
    auto get_expr_to_write = [&]() {
      if (query_it == query.constraints.end()) {
        return negated_exp;
      }
      return *query_it;
    };
    auto advance_expr_to_write = [&]() {
      if (query_it != query.constraints.end()) {
        ++query_it;
      } else {
        expr_done = true;
      }
    };

    // LCP between the assertion stack and the query constraints.
    while (stack_it != assertionStack.end() && !all_written() && (*stack_it)->hash() == get_expr_to_write()->hash()) {
      ++stack_it;
      advance_expr_to_write();
    }

    lcps[id] = std::distance(assertionStack.begin(), stack_it);
    maxLCP = std::max(maxLCP, lcps[id]);
    largestSize = std::max(largestSize, assertionStack.size());
  }

  double maxFraction = maxLCP / ((double) query.constraints.size() + 1);

  int bestId = -1;
  for (auto id = 0u; id < PoolSize; ++id) {
    if (abs(maxFraction - lcps[id] / ((double) pool[id]->assertionStack.size() + 1)) <= PercentLeeway / 100.0 &&
        pool[id]->assertionStack.size() <= largestSize) {
      bestId = id;
      largestSize = pool[id]->assertionStack.size();
    }
  }
  if (bestId < 0) {
    klee_error("Delegation failed, no solver was found");
  }

  // if (bestId < 0) {
  //   // We need to pick one to evict - we use LRU. TODO: Iterate on this.
  //   bestId = (int) (min_element(lastUsed.begin(), lastUsed.end()) - lastUsed.begin());
  //   if (WarnSolverChoices)
  //     klee_warning("Evicting solver %d", bestId);
  // }

  lastUsed[bestId] = ++count;
  previousId = bestId;
  if (WarnSolverChoices)
    klee_warning("Using solver %d", bestId);

  // return pool[bestId]->popAndAssertRemaining(maxLCP,
  //                                            query,
  //                                            objects,
  //                                            values,
  //                                            hasSolution);

  return pool[bestId]->internalRunSolver(query, objects, values, hasSolution);
}

SolverImpl::SolverRunStatus Z3PoolingSolverImpl::getOperationStatusCode() {
  return pool[previousId]->getOperationStatusCode();
}

Z3PoolingSolver::Z3PoolingSolver() : Solver(std::make_unique<Z3PoolingSolverImpl>()) {}

} // namespace klee