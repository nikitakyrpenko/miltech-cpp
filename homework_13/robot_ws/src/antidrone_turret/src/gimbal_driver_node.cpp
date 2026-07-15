#include "antidrone_turret/msg/gimbal_command.hpp"

#include <rclcpp/node.hpp>
#include <rclcpp/rclcpp.hpp>

namespace {
constexpr auto kGimbalTopic = "/gimbal/cmd";

using GimbalType = antidrone_turret::msg::GimbalCommand;
}  // namespace

class GimbalDriverNode final : public rclcpp::Node {
public:
  GimbalDriverNode()
    : rclcpp::Node("gimbal_driver_node")
  {
    subscription_ =
      this->create_subscription<GimbalType>(kGimbalTopic, 10, [this](const GimbalType& msg) { on_gimbal_message_received(msg); });
  }

private:
  auto on_gimbal_message_received(const GimbalType& msg) -> void
  {
    const char* dir = msg.direction > 0 ? "UP" : msg.direction < 0 ? "DOWN" : "CENTER";
    RCLCPP_INFO(get_logger(), "gimbal received dir=%s target_y=%.1f error_y=%.2f", dir, msg.target_y, msg.error_y);
  };

  rclcpp::Subscription<GimbalType>::SharedPtr subscription_;
};

int main(int argc, char** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<GimbalDriverNode>());
  rclcpp::shutdown();
  return 0;
}
