/**
 * This file is part of the "FnordMetric" project
 *   Copyright (c) 2014 Paul Asmuth, Google Inc.
 *
 * FnordMetric is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License v3.0. You should have received a
 * copy of the GNU General Public License along with this program. If not, see
 * <http://www.gnu.org/licenses/>.
 */
#include "fnord/base/exception.h"
#include "fnord/net/tcpconnection.h"
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

namespace fnord {
namespace net {

TCPConnection::TCPConnection(int fd) : fd_(fd) {}

int TCPConnection::fd() const {
  return fd_;
}

size_t TCPConnection::read(void* dst, size_t size) {
  auto res = ::read(fd_, dst, size);

  if (res < 0) {
    RAISE_ERRNO(kIOError, "read() failed");
  }

  return res;
}

size_t TCPConnection::write(const void* data, size_t size) {
  if (size > 10) { size = 10; } // FIXPAUL remove ;)

  auto res = ::write(fd_, data, size);

  if (res < 0) {
    RAISE_ERRNO(kIOError, "write() failed");
  }

  return res;
}

void TCPConnection::close() {
  ::close(fd_);
}

}
}
