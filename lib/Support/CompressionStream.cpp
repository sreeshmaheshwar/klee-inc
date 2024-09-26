//===-- CompressionStream.cpp --------------------------------------------===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "klee/Config/config.h"
#include "klee/Config/Version.h"
#ifdef HAVE_ZLIB_H
#include "klee/Support/CompressionStream.h"
#include "klee/Support/ErrorHandling.h"

#include "llvm/Support/FileSystem.h"

#include <fcntl.h>

namespace klee {

compressed_fd_ostream::compressed_fd_ostream(const std::string &Filename,
                                             std::string &ErrorInfo)
    : llvm::raw_ostream(), pos(0) {
  ErrorInfo = "";
  // Open file in binary mode
  std::error_code EC =
      llvm::sys::fs::openFileForWrite(Filename, FD);
  if (EC) {
    ErrorInfo = EC.message();
    FD = -1;
    return;
  }
  // Initialize the compression library
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  strm.next_in = Z_NULL;
  strm.next_out = buffer;
  strm.avail_in = 0;
  strm.avail_out = BUFSIZE;

  const auto ret = deflateInit2(&strm, Z_BEST_COMPRESSION, Z_DEFLATED, 31,
                                8 /* memory usage default, 0 smallest, 9 highest*/,
                                Z_DEFAULT_STRATEGY);
  if (ret != Z_OK)
    ErrorInfo = "Deflate initialisation returned with error: " + std::to_string(ret);
}

void compressed_fd_ostream::writeFullCompressedData() {
  // Check if no space available and write the buffer
  if (strm.avail_out == 0) {
    write_file(reinterpret_cast<const char *>(buffer), BUFSIZE);
    strm.next_out = buffer;
    strm.avail_out = BUFSIZE;
  }
}

void compressed_fd_ostream::flush_compressed_data() {
  // flush data from the raw buffer
  flush();

  // write the remaining data
  int deflate_res = Z_OK;
  while (deflate_res == Z_OK) {
    // Check if no space available and write the buffer
    writeFullCompressedData();
    deflate_res = deflate(&strm, Z_FINISH);
  }
  assert(deflate_res == Z_STREAM_END);
  write_file(reinterpret_cast<const char *>(buffer), BUFSIZE - strm.avail_out);
}

compressed_fd_ostream::~compressed_fd_ostream() {
  if (FD >= 0) {
    // write the remaining data
    flush_compressed_data();
    close(FD);
  }
  deflateEnd(&strm);
}

void compressed_fd_ostream::write_impl(const char *Ptr, size_t Size) {
  strm.next_in =
      const_cast<unsigned char *>(reinterpret_cast<const unsigned char *>(Ptr));
  strm.avail_in = Size;

  // Check if there is still data to compress
  while (strm.avail_in != 0) {
    // compress data
    const auto res __attribute__ ((unused)) = deflate(&strm, Z_NO_FLUSH);
    assert(res == Z_OK);
    writeFullCompressedData();
  }
}

void compressed_fd_ostream::write_file(const char *Ptr, size_t Size) {
  pos += Size;
  assert(FD >= 0 && "File already closed");
  do {
    ssize_t ret = ::write(FD, Ptr, Size);
    if (ret < 0) {
      if (errno == EINTR || errno == EAGAIN)
        continue;
      assert(0 && "Could not write to file");
      break;
    }

    Ptr += ret;
    Size -= ret;
  } while (Size > 0);
}

void compressed_fd_istream::init_zlib() {
  std::memset(&strm, 0, sizeof(strm));
  int ret = inflateInit2(&strm, 31);
  if (ret != Z_OK) {
    klee_error("inflateInit returned with error: %d", ret);
  }
}

void compressed_fd_istream::cleanup_zlib() { inflateEnd(&strm); }

void compressed_fd_istream::fill_decompressed_buffer() {
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

compressed_fd_istream::compressed_fd_istream(const std::string &Filename)
    : decompressed_size(0), decompressed_pos(0), eof_reached(false) {
  FD = open(Filename.c_str(), O_RDONLY);
  if (FD < 0) {
    klee_error("Failed to open file: %s", Filename.c_str());
  }
  init_zlib();
}

compressed_fd_istream::~compressed_fd_istream() {
  cleanup_zlib();
  if (FD >= 0) {
    close(FD);
  }
}

size_t compressed_fd_istream::read_bytes(char *buffer, size_t size) {
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

bool compressed_fd_istream::eof() const {
  return eof_reached && (decompressed_pos >= decompressed_size);
}

}
#endif
