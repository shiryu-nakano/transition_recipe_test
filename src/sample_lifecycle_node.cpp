#include <rclcpp/rclcpp.hpp>
#include <rclcpp_lifecycle/lifecycle_node.hpp>
#include <lifecycle_msgs/msg/state.hpp>

namespace transition_recipe_test
{

class SampleLifecycleNode : public rclcpp_lifecycle::LifecycleNode
{
public:
  SampleLifecycleNode()
  : rclcpp_lifecycle::LifecycleNode("sample_node")
  {
    RCLCPP_INFO(this->get_logger(), "SampleLifecycleNode constructed");
  }

  using CallbackReturn =
    rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn;

  CallbackReturn on_configure(const rclcpp_lifecycle::State &) override
  {
    RCLCPP_INFO(this->get_logger(), "[sample_node] on_configure()");
    return CallbackReturn::SUCCESS;
  }

  CallbackReturn on_activate(const rclcpp_lifecycle::State &) override
  {
    RCLCPP_INFO(this->get_logger(), "[sample_node] on_activate()");
    return CallbackReturn::SUCCESS;
  }

  CallbackReturn on_deactivate(const rclcpp_lifecycle::State &) override
  {
    RCLCPP_INFO(this->get_logger(), "[sample_node] on_deactivate()");
    return CallbackReturn::SUCCESS;
  }

  CallbackReturn on_cleanup(const rclcpp_lifecycle::State &) override
  {
    RCLCPP_INFO(this->get_logger(), "[sample_node] on_cleanup()");
    return CallbackReturn::SUCCESS;
  }

  CallbackReturn on_shutdown(const rclcpp_lifecycle::State &) override
  {
    RCLCPP_INFO(this->get_logger(), "[sample_node] on_shutdown()");
    return CallbackReturn::SUCCESS;
  }
};

} // namespace transition_recipe_test

// ---- main ----
int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);

  auto node = std::make_shared<transition_recipe_test::SampleLifecycleNode>();
  // LifecycleNode のときは base_interface を spin する
  rclcpp::spin(node->get_node_base_interface());

  rclcpp::shutdown();
  return 0;
}
