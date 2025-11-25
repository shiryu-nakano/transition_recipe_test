#include <rclcpp/rclcpp.hpp>
#include <rclcpp_lifecycle/lifecycle_node.hpp>
#include <lifecycle_msgs/msg/state.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <chrono>
using namespace std::chrono_literals;

namespace transition_recipe_test
{

  class BNode : public rclcpp_lifecycle::LifecycleNode
  {
  public:
    BNode()
        : rclcpp_lifecycle::LifecycleNode("B_node")
    {
      RCLCPP_INFO(this->get_logger(), "B_node constructed");
    }

    using CallbackReturn = rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn;

    CallbackReturn on_configure(const rclcpp_lifecycle::State &) override
    {
      RCLCPP_INFO(this->get_logger(), "[B_node] CONFIGURED");
      cmd_vel_pub_ = this->create_publisher<geometry_msgs::msg::Twist>("cmd_vel", 10);
      return CallbackReturn::SUCCESS;
    }

    CallbackReturn on_activate(const rclcpp_lifecycle::State &) override
    {
      RCLCPP_INFO(this->get_logger(), "[B_node] ACTIVE!");
      cmd_vel_pub_->on_activate();
      timer_ = this->create_wall_timer(
          100ms, std::bind(&BNode::timer_callback, this));
      return CallbackReturn::SUCCESS;
    }

    CallbackReturn on_deactivate(const rclcpp_lifecycle::State &) override
    {
      RCLCPP_INFO(this->get_logger(), "[B_node] DEACTIVATED");
      timer_->cancel();
      cmd_vel_pub_->on_deactivate();
      return CallbackReturn::SUCCESS;
    }

  private:
    //rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_pub_;
    rclcpp_lifecycle::LifecyclePublisher<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_pub_;
    rclcpp::TimerBase::SharedPtr timer_;

    void timer_callback()
    {
      geometry_msgs::msg::Twist msg;
      msg.linear.x = 1.5;  // 0.5 m/s で直進
      msg.angular.z = 0.0; // 回転速度 0 (まっすぐ)
      cmd_vel_pub_->publish(msg);
    }
  };

} // namespace transition_recipe_test

int main(int argc, char **argv)
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<transition_recipe_test::BNode>();
  rclcpp::spin(node->get_node_base_interface());
  rclcpp::shutdown();
  return 0;
}
