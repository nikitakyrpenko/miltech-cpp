#pragma once

#include <termios.h>
#include <unistd.h>
#include <cstddef>
#include <cstdint>

class UartPort {
  const int fd_{};
  const termios term_{};

public:
  explicit UartPort(const char* serial);

  bool write_all(const uint8_t* buf, size_t len, int timeout = -1);
  ssize_t read_some(uint8_t* buf, size_t limit, int timeout = -1);

  ~UartPort();
};
