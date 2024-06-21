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

// compressed_fd_istream::compressed_fd_istream(const std::string &Filename, std::string &ErrorInfo)
//     : buffer(BUFSIZE), eofReached(false) {
//   fileStream.open(Filename, std::ios::binary);
//   if (!fileStream.is_open()) {
//     ErrorInfo = "Unable to open file: " + Filename;
//     return;
//   }
//   strm.zalloc = Z_NULL;
//   strm.zfree = Z_NULL;
//   strm.opaque = Z_NULL;
//   strm.avail_in = 0;
//   strm.next_in = Z_NULL;
//   strm.avail_out = BUFSIZE;
//   strm.next_out = buffer.data();
//   const auto ret = inflateInit2(&strm, 31);
//   if (ret != Z_OK) {
//     ErrorInfo = "inflateInit2 returned with error: " + std::to_string(ret);
//     fileStream.close();
//   }
// }

// compressed_fd_istream::~compressed_fd_istream() {
//   inflateEnd(&strm);
//   if (fileStream.is_open()) {
//     fileStream.close();
//   }
// }

// void compressed_fd_istream::fillBuffer() {
//   if (strm.avail_in == 0 && !eofReached) {
//     fileStream.read(reinterpret_cast<char *>(buffer.data()), BUFSIZE);
//     std::streamsize bytesRead = fileStream.gcount();
//     if (bytesRead > 0) {
//       strm.next_in = buffer.data();
//       strm.avail_in = bytesRead;
//     }
//     if (fileStream.eof()) {
//       eofReached = true;
//     }
//   }
// }

// bool compressed_fd_istream::decompressToBuffer(size_t Size) {
//   std::vector<uint8_t> tempBuffer(Size);
//   strm.next_out = tempBuffer.data();
//   strm.avail_out = Size;
//   while (strm.avail_out > 0) {
//     if (strm.avail_in == 0) {
//       fillBuffer();
//     }
//     const auto ret = inflate(&strm, Z_NO_FLUSH);
//     if (ret == Z_STREAM_END) {
//       std::memcpy(buffer.data(), tempBuffer.data(), Size - strm.avail_out);
//       return true;
//     }
//     if (ret != Z_OK) {
//       llvm::errs() << "inflate returned with error: " << ret << "\n";
//       return false;
//     }
//   }
//   std::memcpy(buffer.data(), tempBuffer.data(), Size);
//   return true;
// }

// std::optional<int> compressed_fd_istream::nextInt() {
//   size_t intSize = sizeof(int);
//   if (buffer.size() < intSize) {
//     buffer.resize(intSize);
//   }
//   if (strm.avail_out < intSize && !decompressToBuffer(intSize)) {
//     return std::nullopt;
//   }
//   strm.next_out += intSize;
//   strm.avail_out -= intSize;
//   return *reinterpret_cast<int *>(buffer.data());
// }

compressed_fd_istream::compressed_fd_istream(const std::string &Filename, std::string &ErrorInfo)
    : inputBuffer(BUFSIZE), outputBuffer(BUFSIZE), outputBufferPos(0), outputBufferSize(0), eofReached(false) {
  fileStream.open(Filename, std::ios::binary);
  if (!fileStream.is_open()) {
    ErrorInfo = "Unable to open file: " + Filename;
    return;
  }

  // Initialize the zlib stream
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  strm.avail_in = 0;
  strm.next_in = Z_NULL;
  strm.avail_out = outputBuffer.size();
  strm.next_out = outputBuffer.data();

  const auto ret = inflateInit2(&strm, 31);
  if (ret != Z_OK) {
    ErrorInfo = "inflateInit2 returned with error: " + std::to_string(ret);
    fileStream.close();
  }
}

compressed_fd_istream::~compressed_fd_istream() {
  inflateEnd(&strm);
  if (fileStream.is_open()) {
    fileStream.close();
  }
}

void compressed_fd_istream::fillInputBuffer() {
  if (strm.avail_in == 0 && !eofReached) {
    fileStream.read(reinterpret_cast<char *>(inputBuffer.data()), inputBuffer.size());
    std::streamsize bytesRead = fileStream.gcount();
    if (bytesRead > 0) {
      strm.next_in = inputBuffer.data();
      strm.avail_in = bytesRead;
    }
    if (fileStream.eof()) {
      eofReached = true;
    }
  }
}

bool compressed_fd_istream::decompressToOutputBuffer(size_t size) {
  outputBufferPos = 0;
  strm.next_out = outputBuffer.data();
  strm.avail_out = size;
  while (strm.avail_out > 0) {
    if (strm.avail_in == 0) {
      fillInputBuffer();
    }
    const auto ret = inflate(&strm, Z_NO_FLUSH);
    if (ret == Z_STREAM_END) {
      outputBufferSize = size - strm.avail_out;
      return true;
    }
    if (ret != Z_OK) {
      llvm::errs() << "inflate returned with error: " << ret << "\n";
      return false;
    }
  }
  outputBufferSize = size;
  return true;
}

// std::optional<int> compressed_fd_istream::nextInt() {
//   size_t intSize = sizeof(int);

//   if (outputBufferPos + intSize > outputBufferSize) {
//     if (!decompressToOutputBuffer(BUFSIZE)) {
//       return std::nullopt;
//     }
//   }

//   if (outputBufferPos + intSize > outputBufferSize) {
//     return std::nullopt;
//   }

//   // klee_warning("%c", outputBuffer[outputBufferPos]);

//   int value = *reinterpret_cast<int *>(outputBuffer.data() + outputBufferPos);
//   outputBufferPos += intSize;

//   return value;
// }

}
#endif
