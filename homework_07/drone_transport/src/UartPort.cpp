#include "UartPort.hpp"

#include <fcntl.h>
#include <poll.h>
#include <cerrno>
#include <stdexcept>

UartPort::UartPort(const char* serial)
  : fd_([serial]() {
    int fd = open(serial, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd == -1) {
      throw std::runtime_error("cannot open serial");
    }
    return fd;
  }())
  , term_([this]() {
    termios tio{};

    if (tcgetattr(fd_, &tio) < 0) {
      throw std::runtime_error("failed to get serial attributes");
    }

    cfmakeraw(&tio);
    cfsetispeed(&tio, B115200);
    cfsetospeed(&tio, B115200);
    tio.c_cflag |= (CLOCAL | CREAD);

    return tio;
  }())
{
  if (tcsetattr(fd_, TCSANOW, &term_) < 0) {
    throw std::runtime_error("failed to set serial attributes");
  }
}

bool UartPort::write_all(const uint8_t* buf, size_t len, int timeout)
{
  size_t written = 0;
  while (written < len) {
    ssize_t n = write(fd_, buf + written, len - written);
    if (n > 0) {
      written += static_cast<size_t>(n);
      continue;
    }
    if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
      pollfd pfd{fd_, POLLOUT, 0};
      poll(&pfd, 1, timeout);
      continue;
    }
    return false;
  }
  return true;
}

ssize_t UartPort::read_some(uint8_t* buf, size_t limit, int timeout)
{
  if (buf == nullptr) {
    return -1;
  }

  while (true) {
    ssize_t n = read(fd_, buf, limit);
    if (n >= 0) {
      return n;
    }
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      pollfd pfd{fd_, POLLIN, 0};
      if (poll(&pfd, 1, timeout) <= 0) {
        return 0;
      }
      continue;
    }
    return -1;
  }
}

UartPort::~UartPort()
{
  close(fd_);
}
