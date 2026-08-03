#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <cerrno>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>

#include "TcpPort.hpp"

static constexpr int SYN_ACK_RETRIES = 3;

TcpPort::TcpPort(const char* domain_name, uint16_t port)
{
  // DNS lookup to IPV4 or IPV6
  addrinfo hint{};
  hint.ai_family = AF_UNSPEC;  // IPv4 or IPv6
  hint.ai_socktype = SOCK_STREAM;

  addrinfo* addr_info = nullptr;

  int st = getaddrinfo(domain_name, std::to_string(port).c_str(), &hint, &addr_info);

  if (st != 0) {
    throw std::runtime_error("Cannot obtain ipaddr by DNS");
  }

  std::memcpy(&addr_, addr_info->ai_addr, addr_info->ai_addrlen);
  addr_len_ = addr_info->ai_addrlen;

  freeaddrinfo(addr_info);

  // Init socket file descriptor
  sock_fd_ = socket(addr_.ss_family, SOCK_STREAM, IPPROTO_TCP);

  if (sock_fd_ == -1)
    throw std::runtime_error("Cannot instantiate socket connection");

  setsockopt(sock_fd_, IPPROTO_TCP, TCP_SYNCNT, &SYN_ACK_RETRIES, sizeof(SYN_ACK_RETRIES));

  if (connect(sock_fd_, reinterpret_cast<sockaddr*>(&addr_), addr_len_) < 0) {
    std::cout << "Cannot  establish connection with " << get_ip_addr() << "\n";
    clean();
    throw std::runtime_error("Cannot establish connection");
  }

  std::cout << "Established connection with " << get_ip_addr() << "\n";
}

// blocking write -> blocking thread on send -> if timeout reached return
ssize_t TcpPort::write(const uint8_t* buff, size_t len, int timeout)
{
  if (buff == nullptr) {
    return 0;
  }

  if (timeout > 0) {
    timeval tv{.tv_sec = timeout / 1000, .tv_usec = (timeout % 1000) * 1000};
    setsockopt(sock_fd_, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
  }

  size_t written = 0;

  while (written < len) {
    ssize_t n = send(sock_fd_, buff + written, len - written, MSG_NOSIGNAL);  // no SIGPIPE
    if (n > 0) {
      written += static_cast<size_t>(n);
      continue;
    }
    if (n < 0 && errno == EINTR) {
      continue;
    }
    return written;
  }

  return written;
}

ssize_t TcpPort::read(uint8_t* buff, size_t limit, int timeout)
{
  if (buff == nullptr) {
    return -1;
  }

  if (timeout > 0) {
    timeval tv{.tv_sec = timeout / 1000, .tv_usec = (timeout % 1000) * 1000};
    setsockopt(sock_fd_, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
  }

  for (;;) {
    ssize_t n = recv(sock_fd_, buff, limit, 0);
    if (n < 0 && errno == EINTR) {
      continue;
    }
    return n;
  }
}

const char* TcpPort::get_ip_addr() const
{
  if (addr_.ss_family == AF_INET) {
    const sockaddr_in* v4 = reinterpret_cast<const sockaddr_in*>(&addr_);
    return inet_ntop(AF_INET, &v4->sin_addr, ip_str_, sizeof(ip_str_));
  }

  const sockaddr_in6* v6 = reinterpret_cast<const sockaddr_in6*>(&addr_);
  return inet_ntop(AF_INET6, &v6->sin6_addr, ip_str_, sizeof(ip_str_));
}

void TcpPort::clean()
{
  if (sock_fd_ >= 0) {
    close(sock_fd_);
    sock_fd_ = -1;
  }
}

TcpPort::~TcpPort()
{
  clean();
}