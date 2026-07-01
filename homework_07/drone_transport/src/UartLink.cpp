#include "UartLink.hpp"

#include <cstring>
#include <iostream>
#include <unistd.h>

static constexpr size_t BUFF_LEN = 256;

UartLink::UartLink(const char* serial)
  : port_(std::make_unique<UartPort>(serial))
{
}

void UartLink::send(const dlink::Control& ctrl)
{
  uint8_t frame[sizeof(dlink::Control) + 6];
  const size_t len = dlink::encode(dlink::PKT_CONTROL, &ctrl, sizeof(ctrl), frame);
  port_->write_all(frame, len);
}

void UartLink::run_loop()
{
  dlink::Parser parser{};

  while (running_) {
      uint8_t buff[BUFF_LEN]{};
      const ssize_t r = port_->read_some(buff, BUFF_LEN, 1);

      uint8_t type, len, payload[260];

      if (r == 0) {
        continue;
      }
      if (r < 0) {
        interrupt();
        break;
      }

      for (int i = 0; i < r; i++) {
        if (parser.feed(buff[i], type, payload, len)) {
          if (!seen_packet_types_[type]) {
            seen_packet_types_[type] = true;
            std::cerr << "UartLink: first frame of packet type 0x" << std::hex << static_cast<int>(type) << std::dec << "\n";
          }
          switch (type) {
            case dlink::PacketType::PKT_TELEMETRY: {
              dlink::Telemetry t{};
              memcpy(&t, payload, sizeof(t));
              std::cerr << "UartLink TELEMETRY t_ms=" << t.t_ms << " x=" << t.x << " y=" << t.y << " z=" << t.z << " vx=" << t.vx
                        << " vy=" << t.vy << " speed=" << t.speed << " dir=" << t.dir << " state=" << static_cast<int>(t.state) << "\n";
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
            default:
              break;
          }
        }
      }
  }
}

