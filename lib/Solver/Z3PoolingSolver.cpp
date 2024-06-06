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
    PoolSize("pool-size", llvm::cl::init(5),
                     llvm::cl::desc("Number of Z3 solvers in the pool (default=5)"),
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
    pool.emplace_back();
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
    const Query &query, const std::vector<const Array *> *objects,
    std::vector<std::vector<unsigned char> > *values, bool &hasSolution) {
  count += 1;
  auto constraints = query.constraints.constraints;
  constraints.push_back(Expr::createIsZero(query.expr));
  std::sort(constraints.begin(), constraints.end());
  constraints.erase(std::unique(constraints.begin(), constraints.end()),
                    constraints.end());
  auto in_set = [&](const ref<Expr> &e, int& location) {
    location = int(std::lower_bound(constraints.begin(), constraints.end(), e) - constraints.begin());
    return location < (int) constraints.size() && constraints[location] == e;
  };
  int use = -1;
  int maxLCP = 0;
  int n = (int) constraints.size();
  std::vector<bool> bestInPrefix(n, false);
  for (auto id = 0u; id < PoolSize; ++id) {
    std::vector<bool> inPrefix(n, false);
    int j = 0, location;
    while (j < (int) pool[id]->assertionStack.size() && in_set(pool[id]->assertionStack[j], location)) {
      inPrefix[location] = true;
      j++;
    }
    if (j > maxLCP) {
      maxLCP = j;
      use = id;
      bestInPrefix = inPrefix;
    }
  }
  if (use < 0) {
    // We need to pick one to evict so let's pick the least recently used.
    use = (int) (min_element(lastUsed.begin(), lastUsed.end()) - lastUsed.begin());
  }
  lastUsed[use] = count;
  previousId = use;
  return pool[use]->popAndAssertThoseInPrefix(maxLCP, bestInPrefix, constraints, objects, values, hasSolution);
}

SolverImpl::SolverRunStatus Z3PoolingSolverImpl::getOperationStatusCode() {
  return pool[previousId]->getOperationStatusCode();
}

Z3PoolingSolver::Z3PoolingSolver() : Solver(std::make_unique<Z3PoolingSolverImpl>()) {}

} // namespace klee