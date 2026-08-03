#pragma once

#include <chrono>

struct RetryPolicy {
  int max_attempts = 3;
  std::chrono::milliseconds backoff{500};
  int send_timeout_ms = 5000;
  int recv_timeout_ms = 15000;
};
