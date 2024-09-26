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

#include "klee/Support/CompressionStream.h"
#include "klee/Support/ErrorHandling.h"
#include <llvm/Support/MemoryBuffer.h>

#include <optional>
#include <type_traits>

namespace klee {
std::unique_ptr<llvm::MemoryBuffer>
klee_open_input_file(const std::string &path, std::string &error);

std::unique_ptr<llvm::raw_fd_ostream>
klee_open_output_file(const std::string &path, std::string &error);

#ifdef HAVE_ZLIB_H
std::unique_ptr<llvm::raw_ostream>
klee_open_compressed_output_file(const std::string &path, std::string &error);

class BufferedTypedOstream {
  std::unique_ptr<llvm::raw_ostream> stream;

public:
  BufferedTypedOstream(std::unique_ptr<llvm::raw_ostream> _stream)
      : stream(std::move(_stream)) {}

  template <typename T> void write(const T &value) {
    stream->write(reinterpret_cast<const char *>(&value), sizeof(T));
  }
};

class BufferedTypedIstream {
  std::unique_ptr<compressed_fd_istream> stream;
  char internal_buffer[DECOMPRESSED_BUFSIZE];
  size_t buffer_size;
  size_t buffer_pos;

public:
  BufferedTypedIstream(std::unique_ptr<compressed_fd_istream> _stream)
      : stream(std::move(_stream)), buffer_size(0), buffer_pos(0) {}

  template <typename T> std::optional<T> next() {
    static_assert(std::is_trivially_copyable<T>::value,
                  "Type T must be trivially copyable");

    T value;
    size_t bytes_needed = sizeof(T);
    size_t bytes_read = 0;
    char *value_ptr = reinterpret_cast<char *>(&value);

    while (bytes_read < bytes_needed) {
      if (buffer_pos >= buffer_size) {
        if (eof()) {
          if (bytes_read == 0) {
            return std::nullopt;
          } else {
            klee_error("Unexpected EOF while reading object");
          }
        }
        buffer_size = stream->read_bytes(internal_buffer, DECOMPRESSED_BUFSIZE);
        buffer_pos = 0;
        if (buffer_size == 0) {
          if (bytes_read == 0) {
            return std::nullopt;
          } else {
            klee_error("Unexpected EOF while reading object");
          }
        }
      }

      const size_t bytes_available = buffer_size - buffer_pos;
      const size_t bytes_to_copy =
          std::min(bytes_needed - bytes_read, bytes_available);

      std::memcpy(value_ptr + bytes_read, internal_buffer + buffer_pos,
                  bytes_to_copy);

      buffer_pos += bytes_to_copy;
      bytes_read += bytes_to_copy;
    }

    return value;
  }

  bool eof() const { return stream->eof(); }
};

std::unique_ptr<BufferedTypedOstream>
klee_open_buffered_typed_output_file(const std::string &path,
                                     std::string &error);

std::unique_ptr<BufferedTypedIstream>
klee_open_buffered_typed_input_file(const std::string &path,
                                    std::string &error);

#endif
} // namespace klee

#endif /* KLEE_FILEHANDLING_H */
