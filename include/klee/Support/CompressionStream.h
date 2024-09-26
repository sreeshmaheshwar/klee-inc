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

constexpr size_t COMPRESSED_BUFSIZE = BUFSIZE;
constexpr size_t DECOMPRESSED_BUFSIZE = BUFSIZE;

class compressed_fd_istream {
  int FD;
  uint8_t compressed_buffer[COMPRESSED_BUFSIZE];
  uint8_t decompressed_buffer[DECOMPRESSED_BUFSIZE];
  size_t decompressed_size;
  size_t decompressed_pos;
  z_stream strm;
  bool eof_reached;

  void init_zlib();
  void cleanup_zlib();
  void fill_decompressed_buffer();

public:
  compressed_fd_istream(const std::string &Filename);
  ~compressed_fd_istream();

  size_t read_bytes(char *buffer, size_t size);

  bool eof() const;
};

} // namespace klee

#endif /* KLEE_COMPRESSIONSTREAM_H */
