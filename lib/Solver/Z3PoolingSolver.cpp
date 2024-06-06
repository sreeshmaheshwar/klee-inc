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
#include <vector>
#include <memory>

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

Z3PoolingSolverImpl::Z3PoolingSolverImpl() {
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

} // namespace klee