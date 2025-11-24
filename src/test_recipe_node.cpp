// src/test_recipe_node.cpp
#include <rclcpp/rclcpp.hpp>
#include <lifecycle_msgs/srv/change_state.hpp>
#include <lifecycle_msgs/srv/get_state.hpp>

#include <map>
#include <string>
#include <chrono>

#include "transition_recipe_test/common_types.hpp"

using namespace std::chrono_literals;

namespace transition_recipe_test
{

using ChangeState      = lifecycle_msgs::srv::ChangeState;
using GetState         = lifecycle_msgs::srv::GetState;
using ChangeStateFuture = rclcpp::Client<ChangeState>::SharedFuture;
using GetStateFuture    = rclcpp::Client<GetState>::SharedFuture;

class RecipeTestNode : public rclcpp::Node
{
public:
  RecipeTestNode()
  : Node("recipe_test_node")
  {
    // /sample_node のライフサイクルサービスクライアント
    change_client_ = this->create_client<ChangeState>("/sample_node/change_state");
    getstate_client_ = this->create_client<GetState>("/sample_node/get_state");

    // Recipe 構築
    recipe_ = create_sample_recipe();

    // operation → Transition ID のマップ
    init_transition_map();

    start_time_ = now();
    timer_ = this->create_wall_timer(
      500ms, std::bind(&RecipeTestNode::timer_callback, this));

    RCLCPP_INFO(this->get_logger(), "RecipeTestNode started");
  }

private:
  // ==== メンバ ====
  TransitionRecipe recipe_;
  bool started_ = false;           // 3秒経過後に true にして開始
  std::size_t current_step_index_ = 0;

  rclcpp::Client<ChangeState>::SharedPtr change_client_;
  rclcpp::Client<GetState>::SharedPtr    getstate_client_;
  rclcpp::TimerBase::SharedPtr           timer_;
  rclcpp::Time                           start_time_;

  std::map<std::string, uint8_t> transition_map_;

  // ==== 初期化系 ====

  void init_transition_map()
  {
    using lifecycle_msgs::msg::Transition;
    transition_map_["configure"]  = Transition::TRANSITION_CONFIGURE;
    transition_map_["activate"]   = Transition::TRANSITION_ACTIVATE;
    transition_map_["deactivate"] = Transition::TRANSITION_DEACTIVATE;
    transition_map_["cleanup"]    = Transition::TRANSITION_CLEANUP;
    transition_map_["shutdown"]   = Transition::TRANSITION_UNCONFIGURED_SHUTDOWN;
  }

  TransitionRecipe create_sample_recipe()
  {
    TransitionRecipe r;
    r.description = "Configure -> Activate sample_node (TransitionRecipe test)";

    ActionStep s1;
    s1.target_node_name = "sample_node";
    s1.operation        = "configure";
    s1.timeout_s        = 3.0;
    s1.retry            = 0;
    r.steps.push_back(s1);

    ActionStep s2;
    s2.target_node_name = "sample_node";
    s2.operation        = "activate";
    s2.timeout_s        = 3.0;
    s2.retry            = 0;
    r.steps.push_back(s2);

    return r;
  }

  // ==== タイマーコールバック ====

  void timer_callback()
  {
    // 既に開始済みなら何もしない（GetState は最後に1回だけ）
    if (started_) {
      return;
    }

    double elapsed = (now() - start_time_).seconds();

    // サービス準備待ち
    if (!change_client_->wait_for_service(100ms)) {
      RCLCPP_WARN_THROTTLE(
        get_logger(), *get_clock(), 2000,
        "Waiting for /sample_node/change_state...");
      return;
    }

    if (elapsed < 3.0) {
      RCLCPP_INFO_THROTTLE(
        get_logger(), *get_clock(), 2000,
        "Elapsed %.2f sec, waiting until 3.0 sec...", elapsed);
      return;
    }

    // ここに来たら一度だけ開始
    started_ = true;
    RCLCPP_INFO(this->get_logger(),
                "Elapsed %.2f sec, executing TransitionRecipe: %s",
                elapsed, recipe_.description.c_str());

    current_step_index_ = 0;
    execute_next_step();
  }

  // ==== Recipe 実行 ====

  void execute_next_step()
  {
    if (current_step_index_ >= recipe_.steps.size()) {
      // 全ステップ終了 → 最後に一回だけ状態取得
      request_get_state();
      return;
    }

    const auto & step = recipe_.steps[current_step_index_];

    auto it = transition_map_.find(step.operation);
    if (it == transition_map_.end()) {
      RCLCPP_WARN(this->get_logger(),
                  "Unknown operation '%s'; skipping step.",
                  step.operation.c_str());
      ++current_step_index_;
      execute_next_step();
      return;
    }

    uint8_t transition_id = it->second;

    RCLCPP_INFO(this->get_logger(),
                "Step %zu / %zu: %s -> %s (timeout=%.1fs)",
                current_step_index_ + 1,
                recipe_.steps.size(),
                step.operation.c_str(),
                step.target_node_name.c_str(),
                step.timeout_s);

    auto req = std::make_shared<ChangeState::Request>();
    req->transition.id = transition_id;

    // 非同期でサービス呼び出しし、コールバック内で次のステップを呼ぶ
    change_client_->async_send_request(
      req,
      [this, step](ChangeStateFuture future)
      {
        try {
          auto resp = future.get();
          if (!resp->success) {
            RCLCPP_WARN(this->get_logger(),
                        "Transition '%s' for %s failed",
                        step.operation.c_str(), step.target_node_name.c_str());
          } else {
            RCLCPP_INFO(this->get_logger(),
                        "Transition '%s' for %s succeeded",
                        step.operation.c_str(), step.target_node_name.c_str());
          }
        } catch (const std::exception & e) {
          RCLCPP_ERROR(this->get_logger(),
                       "Exception in ChangeState callback: %s", e.what());
        }

        // 次のステップへ
        ++current_step_index_;
        execute_next_step();
      }
    );
  }

  // ==== GetState 一回だけ ====

  void request_get_state()
  {
    if (!getstate_client_->wait_for_service(500ms)) {
      RCLCPP_WARN(this->get_logger(),
                  "GetState service not available for /sample_node/get_state");
      return;
    }

    auto req = std::make_shared<GetState::Request>();
    getstate_client_->async_send_request(
      req,
      [this](GetStateFuture future)
      {
        try {
          auto resp = future.get();
          RCLCPP_INFO(this->get_logger(),
                      "[sample_node] current lifecycle state: id=%u, label=%s",
                      resp->current_state.id,
                      resp->current_state.label.c_str());
        } catch (const std::exception & e) {
          RCLCPP_ERROR(this->get_logger(),
                       "Exception in GetState callback: %s", e.what());
        }
      }
    );
  }
};

} // namespace transition_recipe_test

// ---- main ----
int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);

  auto node = std::make_shared<transition_recipe_test::RecipeTestNode>();
  rclcpp::spin(node);

  rclcpp::shutdown();
  return 0;
}
