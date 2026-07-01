#include "DroneLink.hpp"
#include "ThreadSafeQueue.hpp"
#include "UartPort.hpp"

#include <unistd.h>
#include <atomic>
#include <cstring>
#include <latch>
#include <memory>
#include <thread>

constexpr size_t BUFF_LEN = 256;

class UartReader {
  std::atomic<bool> running_{false};

  std::unique_ptr<UartPort> port_;

  SynchronizedQueue<dlink::Telemetry>& telemetry_channel_;
  SynchronizedQueue<dlink::TargetPos>& target_channel_;
  SynchronizedQueue<dlink::AmmoCfg>& ammo_channel_;
  SynchronizedQueue<dlink::Result>& result_channel_;
  SynchronizedQueue<dlink::DroneCfg>& config_channel_;

  std::thread worker_;

public:
  explicit UartReader(const char* serial,
                      SynchronizedQueue<dlink::Telemetry>& tele_c,
                      SynchronizedQueue<dlink::TargetPos>& tar_c,
                      SynchronizedQueue<dlink::AmmoCfg>& ammo_c,
                      SynchronizedQueue<dlink::Result>& res_c,
                      SynchronizedQueue<dlink::DroneCfg>& cfg_c)
    : port_(std::make_unique<UartPort>(serial))
    , telemetry_channel_(tele_c)
    , target_channel_(tar_c)
    , ammo_channel_(ammo_c)
    , result_channel_(res_c)
    , config_channel_(cfg_c)
  {
  }

  void run(std::latch& latch)
  {
    running_ = true;

    worker_ = std::thread([&]() {
      latch.arrive_and_wait();

      dlink::Parser parser{};

      while (running_) {
        uint8_t buff[BUFF_LEN]{};

        const ssize_t r = port_->read_some(buff, BUFF_LEN, 1);

        uint8_t type, len, payload[260];

        // check for interrupt flag
        if (r == 0) {
          continue;
        }
        if (r < 0) {
          interrupt();
          break;
        }

        for (int i = 0; i < r; i++) {
          if (parser.feed(buff[i], type, payload, len)) {
            switch (type) {
              case dlink::PacketType::PKT_TELEMETRY: {
                dlink::Telemetry t{};
                memcpy(&t, payload, sizeof(t));
                telemetry_channel_.emplace(t);
                break;
              }
              case dlink::PacketType::PKT_AMMO: {
                dlink::AmmoCfg ammo{};
                memcpy(&ammo, payload, sizeof(ammo));
                ammo_channel_.emplace(ammo);
                break;
              }
              case dlink::PacketType::PKT_TARGET: {
                dlink::TargetPos target{};
                memcpy(&target, payload, sizeof(target));
                target_channel_.emplace(target);
                break;
              }
              case dlink::PacketType::PKT_RESULT: {
                dlink::Result result{};
                memcpy(&result, payload, sizeof(result));
                result_channel_.emplace(result);
                break;
              }
              case dlink::PacketType::PKT_CONFIG: {
                dlink::DroneCfg cfg{};
                memcpy(&cfg, payload, sizeof(cfg));
                config_channel_.emplace(cfg);
                break;
              }
              default: {
                break;
              }
            }
          }
        }
      }
    });
  }

  void interrupt() { running_ = false; }

  ~UartReader()
  {
    interrupt();
    if (worker_.joinable()) {
      worker_.join();
    }
  }
};