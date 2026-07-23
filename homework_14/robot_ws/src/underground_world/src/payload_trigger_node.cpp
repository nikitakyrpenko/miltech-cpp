#include <rclcpp/rclcpp.hpp>
#include <memory>

#include "underground_world/msg/enemy_down.hpp"
#include "underground_world/srv/payload_trigger.hpp"

namespace {
using underground_world::msg::EnemyDown;
using underground_world::srv::PayloadTrigger;

constexpr auto kEnemyDownTopic = "/payload/enemy_down";
constexpr auto kTriggerService = "/payload/trigger";
}  // namespace

class PayloadTriggerNode final : public rclcpp::Node {
public:
  PayloadTriggerNode()
    : Node("payload_trigger_node")
  {
    enemy_down_pub_ = this->create_publisher<EnemyDown>(kEnemyDownTopic, rclcpp::QoS{10});
    trigger_srv_ = this->create_service<PayloadTrigger>(
      kTriggerService, [this](const std::shared_ptr<PayloadTrigger::Request> request, std::shared_ptr<PayloadTrigger::Response> response) {
        on_trigger(*request, *response);
      });
  }

private:
  void on_trigger(const PayloadTrigger::Request& request, PayloadTrigger::Response& response)
  {
    RCLCPP_INFO(
      get_logger(), "trigger received contact_id=%d at (%d,%d) -> publishing enemy_down", request.contact_id, request.x, request.y);

    EnemyDown msg;
    msg.contact_id = request.contact_id;
    msg.x = request.x;
    msg.y = request.y;
    enemy_down_pub_->publish(msg);

    response.accepted = true;
    response.reason = "payload discharged";
  }

  rclcpp::Publisher<EnemyDown>::SharedPtr enemy_down_pub_;
  rclcpp::Service<PayloadTrigger>::SharedPtr trigger_srv_;
};

int main(int argc, char** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<PayloadTriggerNode>());
  rclcpp::shutdown();
  return 0;
}
