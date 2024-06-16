//===-- Z3PoolingSolver.h -------------------------------------------*- C++ -*-====//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "klee/Solver/Solver.h"

namespace klee {

/// Z3PoolingSolver - An incomplete solver based on the K-Pooling Strategy.
class Z3PoolingSolver : public Solver {
public:
  /// Z3PoolingSolver - Construct a new Z3PoolingSolver.
  Z3PoolingSolver();
};

} // namespace klee