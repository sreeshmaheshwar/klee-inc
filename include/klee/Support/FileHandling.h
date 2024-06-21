//===-- FileHandling.h ------------------------------------------*- C++ -*-===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef KLEE_FILEHANDLING_H
#define KLEE_FILEHANDLING_H

#include "klee/Support/CompilerWarning.h"
DISABLE_WARNING_PUSH
DISABLE_WARNING_DEPRECATED_DECLARATIONS
#include "llvm/Support/raw_ostream.h"
DISABLE_WARNING_POP

#include <memory>
#include <string>

#include <llvm/Support/MemoryBuffer.h>
#include "klee/Support/CompressionStream.h"

namespace klee {
std::unique_ptr<llvm::MemoryBuffer>
klee_open_input_file(const std::string &path, std::string &error);

std::unique_ptr<llvm::raw_fd_ostream>
klee_open_output_file(const std::string &path, std::string &error);

#ifdef HAVE_ZLIB_H
std::unique_ptr<llvm::raw_ostream>
klee_open_compressed_output_file(const std::string &path, std::string &error);

// We encapsulate both compressed_fd_ostream and compressed_fd_istream below for
// buffered reading and writing with smaller types outputted.

class BufferedTypedOstream {
    std::unique_ptr<llvm::raw_ostream> stream;
  public:

    BufferedTypedOstream(std::unique_ptr<llvm::raw_ostream> _stream) : stream(std::move(_stream)) {}

    template <typename T> void write(const T &value) {
        stream->write(reinterpret_cast<const char *>(&value), sizeof(T));
    }
};

class BufferedTypedIstream {
    std::unique_ptr<compressed_fd_istream> stream;

  public:
    BufferedTypedIstream(std::unique_ptr<compressed_fd_istream> _stream) : stream(std::move(_stream)) {}

    template <typename T>
    std::optional<T> next() {
        return stream->next<T>();
    }
}; 

std::unique_ptr<BufferedTypedOstream> klee_open_buffered_typed_output_file(
    const std::string &path, std::string &error);

std::unique_ptr<BufferedTypedIstream> klee_open_buffered_typed_input_file(
    const std::string &path, std::string &error);

#endif
} // namespace klee

#endif /* KLEE_FILEHANDLING_H */
