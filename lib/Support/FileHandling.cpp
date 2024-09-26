//===-- FileHandling.cpp --------------------------------------------------===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "klee/Support/FileHandling.h"

#include "klee/Config/Version.h"
#include "klee/Config/config.h"
#include "klee/Support/ErrorHandling.h"

#include "llvm/Support/FileSystem.h"

#ifdef HAVE_ZLIB_H
#include "klee/Support/CompressionStream.h"
#endif

namespace klee {

std::unique_ptr<llvm::MemoryBuffer>
klee_open_input_file(const std::string &path, std::string &error) {
  error.clear();
  llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> bufferOrError =
      llvm::MemoryBuffer::getFile(path, true);
  
  if (!bufferOrError) {
    error = bufferOrError.getError().message();
    return nullptr;
  }
  
  return std::move(bufferOrError.get());
}

std::unique_ptr<llvm::raw_fd_ostream>
klee_open_output_file(const std::string &path, std::string &error) {
  error.clear();
  std::error_code ec;

  auto f = std::make_unique<llvm::raw_fd_ostream>(path.c_str(), ec,
                                                  llvm::sys::fs::OF_None);
  if (ec)
    error = ec.message();
  if (!error.empty()) {
    f.reset(nullptr);
  }
  return f;
}

#ifdef HAVE_ZLIB_H
std::unique_ptr<llvm::raw_ostream>
klee_open_compressed_output_file(const std::string &path, std::string &error) {
  error.clear();
  auto f = std::make_unique<compressed_fd_ostream>(path, error);
  if (!error.empty()) {
    f.reset(nullptr);
  }
  return f;
}

std::unique_ptr<BufferedTypedOstream>
klee_open_buffered_typed_output_file(const std::string &path, std::string &error) {
  return std::make_unique<BufferedTypedOstream>(klee_open_compressed_output_file(path, error));
}

std::unique_ptr<BufferedTypedIstream>
klee_open_buffered_typed_input_file(const std::string &path,
                                    std::string &error) {
  return std::make_unique<BufferedTypedIstream>(
      std::make_unique<compressed_fd_istream>(path));
}

#endif
}
