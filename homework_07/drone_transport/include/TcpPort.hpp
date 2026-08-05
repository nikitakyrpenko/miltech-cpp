#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <cstddef>
#include <cstdint>

class TcpPort {
  int sock_fd_ = -1;
  sockaddr_storage addr_{};
  socklen_t addr_len_{};
  mutable char ip_str_[INET6_ADDRSTRLEN]{};

  void clean();

public:
  TcpPort(const char* domain_name, uint16_t port = 80, int connect_timeout_ms = 0);
  ~TcpPort();

  TcpPort(const TcpPort&) = delete;
  TcpPort& operator=(const TcpPort&) = delete;

  ssize_t write(const uint8_t* buff, size_t len, int timeout);
  ssize_t read(uint8_t* buff, size_t limit, int timeout);

  const char* get_ip_addr() const;
};