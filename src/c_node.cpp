#include <rclcpp/rclcpp.hpp>
#include <rclcpp_lifecycle/lifecycle_node.hpp>
#include <lifecycle_msgs/msg/state.hpp>

namespace transition_recipe_test
{

class CNode : public rclcpp_lifecycle::LifecycleNode
{
public:
  CNode()
  : rclcpp_lifecycle::LifecycleNode("C_node")
  {
    RCLCPP_INFO(this->get_logger(), "C_node constructed");
  }

  using CallbackReturn = rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn;

  CallbackReturn on_configure(const rclcpp_lifecycle::State &) override
  {
    RCLCPP_INFO(this->get_logger(), "[C_node] CONFIGURED");
    return CallbackReturn::SUCCESS;
  }

  CallbackReturn on_activate(const rclcpp_lifecycle::State &) override
  {
    RCLCPP_INFO(this->get_logger(), "[C_node] ACTIVE!");
    return CallbackReturn::SUCCESS;
  }

  CallbackReturn on_deactivate(const rclcpp_lifecycle::State &) override
  {
    RCLCPP_INFO(this->get_logger(), "[C_node] DEACTIVATED");
    return CallbackReturn::SUCCESS;
  }
};

}  // namespace transition_recipe_test

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<transition_recipe_test::CNode>();
  rclcpp::spin(node->get_node_base_interface());
  rclcpp::shutdown();
  return 0;
}

