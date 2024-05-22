//===-- SMTLIBLoggingSolver.cpp -------------------------------------------===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "QueryLoggingSolver.h"

#include "klee/Expr/ExprSMTLIBPrinter.h"
#include "klee/Support/FileHandling.h"
#include "klee/Solver/SolverCmdLine.h"
#include "klee/Support/ErrorHandling.h"

#include <memory>
#include <sstream>
#include <string>
#include <utility>

using namespace klee;

/// This QueryLoggingSolver will log queries to a file in the SMTLIBv2 format
/// and pass the query down to the underlying solver.
class SMTLIBLoggingSolver : public QueryLoggingSolver
{
        private:
                std::istringstream queryStream;
                ExprSMTLIBPrinter printer;

                virtual void printQuery(const Query& query,
                                        const Query* falseQuery = 0,
                                        const std::vector<const Array*>* objects = 0) 
                {
                        if (0 == falseQuery) 
                        {
                                printer.setQuery(query);
                        }
                        else
                        {
                                printer.setQuery(*falseQuery);
                        }

                        if (0 != objects)
                        {
                                printer.setArrayValuesToGet(*objects);
                        }

                        printer.generateOutput();

                        if (QueryInputFile != "") {
                                std::istringstream contentsStream(contents);
                                std::string receivedLine;
                                while (std::getline(contentsStream, receivedLine)) {
                                        if (receivedLine.empty() || receivedLine.front() == ';') {
                                                continue;
                                        }
                                        std::string expectedLine;
                                        bool found = false;
                                        while (!found && std::getline(queryStream, expectedLine)) {
                                                if (expectedLine.empty() || expectedLine.front() == ';') {
                                                        continue;
                                                }
                                                found = true;
                                        }
                                        if (!found) {
                                                klee_warning("Reached end of query input file");
                                        } else if (receivedLine != expectedLine) {
                                                klee_warning("Expected: %s", expectedLine.c_str());
                                                klee_warning("Received: %s", receivedLine.c_str());
                                                klee_error("Mismatch with query input file");
                                        }
                                }
                        }

                        *fos << contents; // Output query to file.
                        contents.clear();
                }    

public:
  SMTLIBLoggingSolver(std::unique_ptr<Solver> solver, std::string path,
                      time::Span queryTimeToLog, bool logTimedOut)
      : QueryLoggingSolver(std::move(solver), std::move(path), ";",
                           queryTimeToLog, logTimedOut) {
    // Setup the printer
    printer.setOutput(logBuffer);
        if (QueryInputFile != "") {
                std::string error;
                auto buffer = klee_open_input_file(QueryInputFile, error);
                if (!buffer) {
                        klee_error("Error opening query input file: %s", error.c_str());
                }
                std::string content = buffer->getBuffer().str();
                queryStream.str(content);
        }
  }
};

std::unique_ptr<Solver>
klee::createSMTLIBLoggingSolver(std::unique_ptr<Solver> solver,
                                std::string path, time::Span minQueryTimeToLog,
                                bool logTimedOut) {
  return std::make_unique<Solver>(std::make_unique<SMTLIBLoggingSolver>(
      std::move(solver), std::move(path), minQueryTimeToLog, logTimedOut));
}
