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

#include "klee/Support/ErrorHandling.h"
#include "zlib.h"
#include "llvm/Support/raw_ostream.h"
#include <cassert>
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include <zlib.h>

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

constexpr size_t COMPRESSED_BUFSIZE = 16384;
constexpr size_t DECOMPRESSED_BUFSIZE = 16384;

class compressed_fd_istream {
  int FD;
  uint8_t compressed_buffer[COMPRESSED_BUFSIZE];
  uint8_t decompressed_buffer[DECOMPRESSED_BUFSIZE];
  size_t decompressed_size;
  size_t decompressed_pos;
  z_stream strm;
  bool eof_reached;

  void init_zlib() {
    std::memset(&strm, 0, sizeof(strm));
    int ret = inflateInit2(&strm, 31);
    if (ret != Z_OK) {
      klee_error("inflateInit returned with error: %d", ret);
    }
  }

  void cleanup_zlib() { inflateEnd(&strm); }

  void fill_decompressed_buffer() {
    if (eof_reached) {
      decompressed_size = 0;
      return;
    }

    strm.avail_out = DECOMPRESSED_BUFSIZE;
    strm.next_out = decompressed_buffer;

    while (strm.avail_out > 0) {
      if (strm.avail_in == 0) {
        ssize_t bytes_read = read(FD, compressed_buffer, COMPRESSED_BUFSIZE);
        if (bytes_read < 0) {
          klee_error("Error reading from file");
        } else if (bytes_read == 0) {
          eof_reached = true;
          break;
        }
        strm.avail_in = bytes_read;
        strm.next_in = compressed_buffer;
      }

      int ret = inflate(&strm, Z_NO_FLUSH);
      if (ret == Z_STREAM_END) {
        eof_reached = true;
        break;
      }
      if (ret != Z_OK) {
        klee_error("inflate returned with error: %d", ret);
      }
    }

    decompressed_size = DECOMPRESSED_BUFSIZE - strm.avail_out;
    decompressed_pos = 0;
  }

public:
  compressed_fd_istream(const std::string &Filename)
      : decompressed_size(0), decompressed_pos(0), eof_reached(false) {
    FD = open(Filename.c_str(), O_RDONLY);
    if (FD < 0) {
      klee_error("Failed to open file: %s", Filename.c_str());
    }
    init_zlib();
  }

  ~compressed_fd_istream() {
    cleanup_zlib();
    if (FD >= 0) {
      close(FD);
    }
  }

  size_t read_bytes(char *buffer, size_t size) {
    size_t bytes_read = 0;
    while (bytes_read < size) {
      if (decompressed_pos >= decompressed_size) {
        fill_decompressed_buffer();
        if (decompressed_size == 0) {
          break;
        }
      }

      size_t bytes_available = decompressed_size - decompressed_pos;
      size_t bytes_to_copy = std::min(size - bytes_read, bytes_available);
      std::memcpy(buffer + bytes_read, decompressed_buffer + decompressed_pos,
                  bytes_to_copy);
      decompressed_pos += bytes_to_copy;
      bytes_read += bytes_to_copy;
    }
    return bytes_read;
  }

  bool eof() const {
    return eof_reached && (decompressed_pos >= decompressed_size);
  }
};

} // namespace klee

#endif /* KLEE_COMPRESSIONSTREAM_H */
