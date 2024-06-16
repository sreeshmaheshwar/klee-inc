#include "Z3PoolingSolver.h"
#include "Z3Solver.h"
#include "klee/Solver/SolverImpl.h"
#include <algorithm>
#include <memory>
#include <vector>
#include "klee/Support/OptionCategories.h"
#include "klee/Support/ErrorHandling.h"

namespace {

llvm::cl::opt<unsigned>
    PoolSize("pool-size", llvm::cl::init(5),
                     llvm::cl::desc("Number of Z3 solvers in the pool (default=5)"),
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
    int previousId;

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

Z3PoolingSolverImpl::Z3PoolingSolverImpl() : previousId(0) {
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
    const Query &query, const std::vector<const Array *> *objects,
    std::vector<std::vector<unsigned char> > *values, bool &hasSolution) {
  auto queryExpr = Expr::createIsZero(query.expr);

  std::vector<int> constraintsInCommon(PoolSize);
  int maxInCommon = 0;
  size_t largestSize = 0;
  for (auto id = 0u; id < PoolSize; ++id) {
    constraintsInCommon[id] = pool[id]->builder->assumptionLiteralCache.count(queryExpr);
    for (const auto& e : query.constraints) {
        constraintsInCommon[id] += pool[id]->builder->assumptionLiteralCache.count(e);
    }
    largestSize = std::max(largestSize, pool[id]->builder->assumptionLiteralCache.size());
    maxInCommon = std::max(maxInCommon, constraintsInCommon[id]);
  }

  double maxFraction = maxInCommon / ((double) query.constraints.size() + 1);
  previousId = -1;
  for (auto id = 0u; id < PoolSize; ++id) {
    if (abs(maxFraction - (constraintsInCommon[id] / ((double) query.constraints.size() + 1))) <= PercentLeeway / ((double) 100) && pool[id]->builder->assumptionLiteralCache.size() <= largestSize) {
        largestSize = pool[id]->builder->assumptionLiteralCache.size();
        previousId = id;
    } 
  }

  if (previousId < 0) {
    klee_error("Delegation failed, no solver was found");
  }

  if (WarnSolverChoices) {
    klee_warning("Using solver %d", previousId);
  }
  return pool[previousId]->internalRunSolver(query, objects, values, hasSolution);
}

SolverImpl::SolverRunStatus Z3PoolingSolverImpl::getOperationStatusCode() {
  return pool[previousId]->getOperationStatusCode();
}

Z3PoolingSolver::Z3PoolingSolver() : Solver(std::make_unique<Z3PoolingSolverImpl>()) {}

} // namespace klee
