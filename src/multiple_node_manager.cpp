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

    using ChangeState = lifecycle_msgs::srv::ChangeState;
    using GetState = lifecycle_msgs::srv::GetState;
    using ChangeStateFuture = rclcpp::Client<ChangeState>::SharedFuture;
    using GetStateFuture = rclcpp::Client<GetState>::SharedFuture;

    class MultipleNodeManager : public rclcpp::Node
    {
    public:
        MultipleNodeManager()
            : Node("multiple_node_manager")
        {
            // 関数で初期化できるようにすること
            // 初期化時に，ノード名のリストを受け取ってそれぞれのクライアントを作成する
            // a_change_client_ = this->create_client<ChangeState>("/A_node/change_state");
            // a_getstate_client_ = this->create_client<GetState>("/A_node/get_state");
            // b_change_client_ = this->create_client<ChangeState>("/B_node/change_state");
            // b_getstate_client_ = this->create_client<GetState>("/B_node/get_state");
            // c_change_client_ = this->create_client<ChangeState>("/C_node/change_state");
            // c_getstate_client_ = this->create_client<GetState>("/C_node/get_state");
            // change_clients_.push_back(a_change_client_);
            // change_clients_.push_back(b_change_client_);
            // change_clients_.push_back(c_change_client_);
            // getstate_clients_.push_back(a_getstate_client_);
            // getstate_clients_.push_back(b_getstate_client_);
            // getstate_clients_.push_back(c_getstate_client_);

            // 1. YAML から node_ids を読み込む
            node_names_ = this->declare_parameter<std::vector<std::string>>(
                "node_ids",
                std::vector<std::string>{} // デフォルトは空
            );

            if (node_names_.empty())
            {
                RCLCPP_WARN(this->get_logger(),
                            "Parameter 'node_ids' is empty. No lifecycle nodes will be managed.");
            }
            else
            {
                RCLCPP_INFO(this->get_logger(), "According to YAML file, This node is managing %zu nodes:", node_names_.size());
                for (const auto &name : node_names_)
                {
                    RCLCPP_INFO(this->get_logger(), "  - %s", name.c_str());
                }
            }
            // 2. node_names_ からクライアント辞書を構築
            init_clients_from_node_list();
            // Recipe 構築
            recipe_ = create_sample_recipe();

            // operation → Transition ID のマップ
            init_transition_map();

            start_time_ = now();
            timer_ = this->create_wall_timer(
                500ms, std::bind(&MultipleNodeManager::timer_callback, this));

            RCLCPP_INFO(this->get_logger(), "MultipleNodeManager started");
        }

    private:
        // ==== メンバ ====
        TransitionRecipe recipe_;
        bool started_ = false; // 3秒経過後に true にして開始
        std::size_t current_step_index_ = 0;
        // std::vector<std::string> node_names_ = {"A_node", "B_node", "C_node"};
        std::vector<std::string> node_names_;

        std::map<std::string, rclcpp::Client<ChangeState>::SharedPtr> change_clients_;
        std::map<std::string, rclcpp::Client<GetState>::SharedPtr> getstate_clients_;
        // rclcpp::Client<ChangeState>::SharedPtr a_change_client_;
        // rclcpp::Client<GetState>::SharedPtr a_getstate_client_;
        // rclcpp::Client<ChangeState>::SharedPtr b_change_client_;
        // rclcpp::Client<GetState>::SharedPtr b_getstate_client_;
        // rclcpp::Client<ChangeState>::SharedPtr c_change_client_;
        // rclcpp::Client<GetState>::SharedPtr c_getstate_client_;
        // std::vector<rclcpp::Client<ChangeState>::SharedPtr> change_clients_; //= {a_change_client_, b_change_client_, c_change_client_};
        // std::vector<rclcpp::Client<GetState>::SharedPtr> getstate_clients_;  //= {a_getstate_client_, b_getstate_client_, c_getstate_client_};

        // rclcpp::Client<ChangeState>::SharedPtr change_client_;
        // rclcpp::Client<GetState>::SharedPtr getstate_client_;

        SemanticState current_semantic_state_;     // 現在のセマンティック状態
        std::size_t pending_semantic_updates_ = 0; // 非同期GetStateの応答待ち数

        rclcpp::TimerBase::SharedPtr timer_;
        rclcpp::Time start_time_;

        std::map<std::string, uint8_t> transition_map_;

        // ==== 初期化系 ====

        void init_transition_map()
        {
            using lifecycle_msgs::msg::Transition;
            transition_map_["configure"] = Transition::TRANSITION_CONFIGURE;
            transition_map_["activate"] = Transition::TRANSITION_ACTIVATE;
            transition_map_["deactivate"] = Transition::TRANSITION_DEACTIVATE;
            transition_map_["cleanup"] = Transition::TRANSITION_CLEANUP;
            transition_map_["shutdown"] = Transition::TRANSITION_UNCONFIGURED_SHUTDOWN;
        }

        TransitionRecipe create_sample_recipe()
        {
            TransitionRecipe r;
            r.description = "Configure -> Activate sample_node (TransitionRecipe test)";

            ActionStep s1;
            s1.target_node_name = "sample_node";
            s1.operation = "configure";
            s1.timeout_s = 3.0;
            s1.retry = 0;
            r.steps.push_back(s1);

            ActionStep s2;
            s2.target_node_name = "sample_node";
            s2.operation = "activate";
            s2.timeout_s = 3.0;
            s2.retry = 0;
            r.steps.push_back(s2);

            return r;
        }

        void init_clients_from_node_list()
        {
            for (const auto &name : node_names_)
            {
                // /<node_name>/change_state
                auto change_client = this->create_client<ChangeState>("/" + name + "/change_state");
                auto getstate_client = this->create_client<GetState>("/" + name + "/get_state");

                change_clients_[name] = change_client;
                getstate_clients_[name] = getstate_client;
                RCLCPP_INFO(this->get_logger(),
                            "Created ChangeState and GetState clients for node '%s'", name.c_str());
            }
        }
        
        
        // ==== タイマーコールバック ====
        void timer_callback()
        {
            // 既に開始済みなら何もしない（GetState は最後に1回だけ）
            // if (started_)
            //{
            //    return;
            //}

            double elapsed = (now() - start_time_).seconds();

            // サービス準備待ち
            // if (!change_client_->wait_for_service(100ms))
            //{
            //    RCLCPP_WARN_THROTTLE(
            //        get_logger(), *get_clock(), 2000,
            //        "Waiting for /sample_node/change_state...");
            //    return;
            //}
            //request_get_all_state();
            request_get_all_semantic_state();
            RCLCPP_INFO(this->get_logger(),
                        "Hello, elapsed %.2f sec", elapsed);
            maybe_log_semantic_state();
            /*
            if (elapsed < 5.0)
            {
                RCLCPP_INFO_THROTTLE(
                    get_logger(), *get_clock(), 2000,
                    "Elapsed %.2f sec, waiting until 5.0 sec...", elapsed);
                return;
            }

            // ここに来たら一度だけ開始
            // started_ = true;
            if (elapsed >= 5.0)
            {
                RCLCPP_INFO(this->get_logger(),
                            "Elapsed %.2f sec, executing TransitionRecipe: %s",
                            elapsed, recipe_.description.c_str());
                execute_transition_recipe();
            }
            current_step_index_ = 0;
            execute_next_step();
            */
        }

        // ==== Recipe 実行 ====

        void execute_next_step()
        {
            if (current_step_index_ >= recipe_.steps.size())
            {
                // 全ステップ終了 → 最後に一回だけ状態取得
                // request_get_state();
                return;
            }

            const auto &step = recipe_.steps[current_step_index_];

            auto it = transition_map_.find(step.operation);
            if (it == transition_map_.end())
            {
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
            /*
            change_client_->async_send_request(
                req,
                [this, step](ChangeStateFuture future)
                {
                    try
                    {
                        auto resp = future.get();
                        if (!resp->success)
                        {
                            RCLCPP_WARN(this->get_logger(),
                                        "Transition '%s' for %s failed",
                                        step.operation.c_str(), step.target_node_name.c_str());
                        }
                        else
                        {
                            RCLCPP_INFO(this->get_logger(),
                                        "Transition '%s' for %s succeeded",
                                        step.operation.c_str(), step.target_node_name.c_str());
                        }
                    }
                    catch (const std::exception &e)
                    {
                        RCLCPP_ERROR(this->get_logger(),
                                     "Exception in ChangeState callback: %s", e.what());
                    }

                    // 次のステップへ
                    ++current_step_index_;
                    execute_next_step();
                });
                */
        }

        // ==== GetState  ====
        // 全ノードに対して GetState を一回だけ呼ぶ
        void request_get_all_state()
        {
            for (const auto &name : node_names_)
            {
                auto it = getstate_clients_.find(name);
                if (it == getstate_clients_.end())
                {
                    RCLCPP_WARN(this->get_logger(),
                                "No GetState client found for node '%s'", name.c_str());
                    continue;
                }
                request_get_state(name, it->second);
            }
        }
        // 全ノードに対して GetState を一回だけ呼び， SemanticState を構築する
        void request_get_all_semantic_state()
        {
            // 新しいスナップショットを開始
            current_semantic_state_.node_states.clear();
            pending_semantic_updates_ = node_names_.size();

            for (const auto &name : node_names_)
            {
                auto it = getstate_clients_.find(name);
                if (it == getstate_clients_.end())
                {
                    RCLCPP_WARN(this->get_logger(),
                                "No GetState client found for node '%s'", name.c_str());
                    // このノード分は応答なしとして扱う
                    current_semantic_state_.node_states[name] = SemanticState::State::UNKNOWN;
                    if (pending_semantic_updates_ > 0)
                    {
                        --pending_semantic_updates_;
                    }
                    continue;
                }
                request_get_semantic_state(name, it->second);
            }
        }

        // node_id に対して GetState を一回だけ呼ぶ
        void request_get_state(
            const std::string &node_name,
            const rclcpp::Client<GetState>::SharedPtr &client)
        {
            if (!client->wait_for_service(500ms))
            {
                RCLCPP_WARN(this->get_logger(),
                            "GetState service not available for %s", node_name.c_str());
                return;
            }

            auto req = std::make_shared<GetState::Request>();
            client->async_send_request(
                req,
                [this, node_name](GetStateFuture future)
                {
                    try
                    {
                        auto resp = future.get();
                        RCLCPP_INFO(this->get_logger(),
                                    "[%s] current lifecycle state: id=%u, label=%s",
                                    node_name.c_str(),
                                    resp->current_state.id,
                                    resp->current_state.label.c_str());
                    }
                    catch (const std::exception &e)
                    {
                        RCLCPP_ERROR(this->get_logger(),
                                     "Exception in GetState callback for %s: %s",
                                     node_name.c_str(), e.what());
                    }
                });
        }

        // node_id に対して GetState を一回だけ呼び， SemanticState を構築する
        void request_get_semantic_state(
            const std::string &node_name,
            const rclcpp::Client<GetState>::SharedPtr &client)
        {
            if (!client->wait_for_service(500ms))
            {
                RCLCPP_WARN(this->get_logger(),
                            "GetState service not available for %s", node_name.c_str());
                // このノードは UNKNOWN として扱う
                current_semantic_state_.node_states[node_name] = SemanticState::State::UNKNOWN;
                if (pending_semantic_updates_ > 0)
                {
                    --pending_semantic_updates_;
                    maybe_log_semantic_state();
                }
                return;
            }

            auto req = std::make_shared<GetState::Request>();
            client->async_send_request(
                req,
                [this, node_name](GetStateFuture future)
                {
                    try
                    {
                        auto resp = future.get();

                        // ① 各ノードの状態を従来通り LOG 出力
                        RCLCPP_INFO(this->get_logger(),
                                    "[%s] current lifecycle state: id=%u, label=%s",
                                    node_name.c_str(),
                                    resp->current_state.id,
                                    resp->current_state.label.c_str());

                        // ② SemanticState に反映
                        current_semantic_state_.node_states[node_name] =
                            map_lifecycle_to_semantic(resp->current_state.id);
                    }
                    catch (const std::exception &e)
                    {
                        RCLCPP_ERROR(this->get_logger(),
                                     "Exception in GetState callback for %s: %s",
                                     node_name.c_str(), e.what());
                        current_semantic_state_.node_states[node_name] = SemanticState::State::UNKNOWN;
                    }

                    // ③ 応答カウンタを減らして、全部そろったらまとめログ
                    if (pending_semantic_updates_ > 0)
                    {
                        --pending_semantic_updates_;
                        maybe_log_semantic_state();
                    }
                });
        }

        // lifecycle_msgs::msg::State ID を SemanticState::State に変換
        SemanticState::State map_lifecycle_to_semantic(uint8_t lifecycle_id) const
        {
            using lifecycle_msgs::msg::State;

            switch (lifecycle_id)
            {
            case State::PRIMARY_STATE_UNCONFIGURED:
                return SemanticState::State::UNCONFIGURED;
            case State::PRIMARY_STATE_INACTIVE:
                return SemanticState::State::INACTIVE;
            case State::PRIMARY_STATE_ACTIVE:
                return SemanticState::State::ACTIVE;
            case State::PRIMARY_STATE_FINALIZED:
                return SemanticState::State::FINALIZED;
            default:
                return SemanticState::State::UNKNOWN;
            }
        }

        // 全てのノードの SemanticState がそろったらログ出力
        void maybe_log_semantic_state()
        {
            if (pending_semantic_updates_ != 0)
            {
                return; // まだそろっていない
            }

            // ここに来た時点で current_semantic_state_ に
            // node_name -> SemanticState::State が一通り入っている

            RCLCPP_INFO(this->get_logger(), "[SemanticState] snapshot:");

            for (const auto &kv : current_semantic_state_.node_states)
            {
                const auto &name = kv.first;
                auto semantic_state = kv.second;

                const char *state_str = nullptr;
                switch (semantic_state)
                {
                case SemanticState::State::UNCONFIGURED:
                    state_str = "UNCONFIGURED";
                    break;
                case SemanticState::State::INACTIVE:
                    state_str = "INACTIVE";
                    break;
                case SemanticState::State::ACTIVE:
                    state_str = "ACTIVE";
                    break;
                case SemanticState::State::FINALIZED:
                    state_str = "FINALIZED";
                    break;
                default:
                    state_str = "UNKNOWN";
                    break;
                }

                RCLCPP_INFO(this->get_logger(),
                            "  [%s] semantic_state=%s",
                            name.c_str(), state_str);
            }
        }
    };

} // namespace transition_recipe_test

// ---- main ----
int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);

    // auto node = std::make_shared<transition_recipe_test::RecipeTestNode>();
    auto node = std::make_shared<transition_recipe_test::MultipleNodeManager>();
    rclcpp::spin(node);

    rclcpp::shutdown();
    return 0;
}
