//===-- CompressionStream.h --------------------------------------------===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef KLEE_COMPRESSIONSTREAM_H
#define KLEE_COMPRESSIONSTREAM_H

#include "llvm/Support/raw_ostream.h"
#include "zlib.h"

#include <cassert>
#include <errno.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include <optional>

namespace klee {
const size_t BUFSIZE = 128 * 1024;

class compressed_fd_ostream : public llvm::raw_ostream {
  int FD;
  uint8_t buffer[BUFSIZE];
  z_stream strm;
  uint64_t pos;

  /// write_impl - See raw_ostream::write_impl.
  virtual void write_impl(const char *Ptr, size_t Size);
  void write_file(const char *Ptr, size_t Size);

  virtual uint64_t current_pos() const { return pos; }

  void flush_compressed_data();
  void writeFullCompressedData();

public:
  /// compressed_fd_ostream - Open the specified file for writing. If an error
  /// occurs, information about the error is put into ErrorInfo, and the stream
  /// should be immediately destroyed; the string will be empty if no error
  /// occurred. This allows optional flags to control how the file will be
  /// opened.
  compressed_fd_ostream(const std::string &Filename, std::string &ErrorInfo);

  ~compressed_fd_ostream();
};

// class compressed_fd_istream {
//   int FD;
//   std::vector<uint8_t> buffer;
//   z_stream strm;
//   std::ifstream fileStream;
//   bool eofReached;

// public:
//   compressed_fd_istream(const std::string &Filename, std::string &ErrorInfo);
//   ~compressed_fd_istream();

//   std::optional<int> nextInt();
//   bool eof() const { return eofReached && strm.avail_in == 0 && strm.avail_out == BUFSIZE; }

// private:
//   void fillBuffer();
//   bool decompressToBuffer(size_t Size);
// };

class compressed_fd_istream {
  std::ifstream fileStream;
  std::vector<uint8_t> inputBuffer;
  std::vector<uint8_t> outputBuffer;
  z_stream strm;
  size_t outputBufferPos;
  size_t outputBufferSize;
  bool eofReached;

  void fillInputBuffer();
  bool decompressToOutputBuffer(size_t size);

public:
  compressed_fd_istream(const std::string &Filename, std::string &ErrorInfo);
  ~compressed_fd_istream();

  template <typename T>
  std::optional<T> next() {
    size_t valueSize = sizeof(T);
    if (outputBufferPos + valueSize > outputBufferSize) {
      if (!decompressToOutputBuffer(BUFSIZE)) {
        return std::nullopt;
      }
    }
    if (outputBufferPos + valueSize > outputBufferSize) {
      return std::nullopt;
    }
    T value;
    std::memcpy(&value, outputBuffer.data() + outputBufferPos, valueSize);
    // Advance buffer ptr.
    outputBufferPos += valueSize;
    return value;
  }
};

}

#endif /* KLEE_COMPRESSIONSTREAM_H */
