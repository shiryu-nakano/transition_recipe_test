// src/test_recipe_node.cpp
#include <rclcpp/rclcpp.hpp>
#include <lifecycle_msgs/srv/change_state.hpp>
#include <lifecycle_msgs/srv/get_state.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>

#include <map>
#include <string>
#include <chrono>

#include "transition_recipe_test/common_types.hpp"
#include "transition_recipe_test/graph.hpp"
#include "transition_recipe_test/graph_generator.hpp"
// #include "transition_recipe_test/recipe_generator.hpp"
#include "transition_recipe_test/switching_strategy.hpp"

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
        // コンストラクタ
        MultipleNodeManager()
            : Node("multiple_node_manager")
        {

            // 1. YAML から node_ids を読み込む
            node_names_ = this->declare_parameter<std::vector<std::string>>(
                "node_ids",
                std::vector<std::string>{} // デフォルトは空
            );

            const std::string graph_yaml_path =
                this->declare_parameter<std::string>("graph_yaml_path", "");

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

            // 3. 状態グラフの構築
            generate_state_graph(graph_yaml_path);

            // Recipe 構築
            recipe_ = create_sample_recipe();

            pose_sub_ = this->create_subscription<geometry_msgs::msg::PoseStamped>(
                "current_pose", 10,
                std::bind(&MultipleNodeManager::pose_callback, this, std::placeholders::_1));

            // x,y
            this->declare_parameter<double>("initial_x", 0.0);
            this->declare_parameter<double>("initial_y", 0.0);
            x_ = this->get_parameter("initial_x").as_double();
            y_ = this->get_parameter("initial_y").as_double();

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

        SemanticState current_semantic_state_;     // 現在のセマンティック状態
        std::size_t pending_semantic_updates_ = 0; // 非同期GetStateの応答待ち数

        Graph state_graph_; // 状態遷移グラフ

        rclcpp::TimerBase::SharedPtr timer_;
        rclcpp::Time start_time_;

        bool recipe_running_ = false;
        rclcpp::Time last_transition_time_;
        std::string last_state_id_;

        std::map<std::string, uint8_t> transition_map_;

        // サブスクライバ
        rclcpp::Subscription<geometry_msgs::msg::PoseStamped>::SharedPtr pose_sub_;

        // 位置情報
        double x_;
        double y_;
        int temp_count_ = 0;

        SwitchingStrategy switching_; // 状態遷移判定ロジック

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
            r.description = "Configure all managed nodes (UNCONFIGURED -> INACTIVE)";

            // node_ids に登録されている全ノードを configure 対象にする
            for (const auto &name : node_names_)
            {
                ActionStep step;
                step.target_node_name = name; // "A_node" / "B_node" / "C_node"
                step.operation = "configure";
                step.timeout_s = 3.0;
                step.retry = 0;
                r.steps.push_back(step);
            }

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
            double elapsed = (now() - start_time_).seconds();

            // ① まだ前回の GetState が返りきっていない場合はスキップ
            if (pending_semantic_updates_ != 0)
            {
                return;
            }

            // ② 前回の snapshot に対して Graph マッチング
            auto state_id_opt = state_graph_.getCurrentSemanticState(current_semantic_state_);
            if (!state_id_opt)
            {
                RCLCPP_WARN(this->get_logger(),
                            "NO MATCH for current SemanticState");
            }
            else
            {
                const std::string &current_state_id = *state_id_opt;

                // 初回（まだ記録無し）
                if (last_state_id_.empty())
                {
                    last_state_id_ = current_state_id;
                    last_transition_time_ = now();
                }

                double since_last = (now() - last_transition_time_).seconds();

                RCLCPP_INFO(this->get_logger(),
                            "System State = %s",
                            current_state_id.c_str());
                RCLCPP_INFO(this->get_logger(),
                            "temp_count_=%d, since_last=%.2f sec, x=%.3f, y=%.3f",
                            temp_count_, since_last, x_, y_);

                // レシピ実行中はトリガー判定をスキップ
                if (!recipe_running_)
                {
                    std::string target_state; // outパラ用

                    auto maybe_recipe = switching_.decide_next_state(
                        current_state_id,
                        temp_count_, // in/out
                        since_last,
                        x_,
                        y_,
                        target_state); // out

                    if (maybe_recipe)
                    {
                        const auto &recipe = *maybe_recipe;

                        RCLCPP_INFO(this->get_logger(),
                                    "[AutoTransition] %s → %s (phase=%d)",
                                    current_state_id.c_str(), target_state.c_str(), temp_count_);

                        execute_transition_recipe(recipe);

                        last_state_id_ = target_state;
                        last_transition_time_ = now();
                    }
                }
            }

            // ③ 次の GetState バッチを投げる
            request_get_all_semantic_state();

            // RCLCPP_INFO(this->get_logger(),
            //             "Hello, elapsed %.2f sec", elapsed);
        }

        // PoseCallBack
        void pose_callback(const geometry_msgs::msg::PoseStamped::SharedPtr msg)
        {

            x_ = msg->pose.position.x;
            y_ = msg->pose.position.y;
            /*RCLCPP_INFO(
                this->get_logger(),
                "Received pose: x=%.3f, y=%.3f",
                msg->pose.position.x,
                msg->pose.position.y);
            */
        }

        // ==== Recipe 実行 ====
        void execute_transition_recipe(const TransitionRecipe &recipe)
        {
            if (recipe_running_)
            {
                RCLCPP_WARN(this->get_logger(),
                            "Cannot start recipe '%s' because another recipe is running.",
                            recipe.description.c_str());
                return;
            }

            if (recipe.steps.empty())
            {
                RCLCPP_WARN(this->get_logger(),
                            "Recipe is empty: '%s'", recipe.description.c_str());
                return;
            }

            // レシピ開始！
            recipe_running_ = true;
            recipe_ = recipe; // 内部変数に代入する
            current_step_index_ = 0;

            RCLCPP_INFO(this->get_logger(),
                        "=== Executing TransitionRecipe: %s ===",
                        recipe_.description.c_str());

            execute_next_step();
        }

        void execute_next_step()
        {
            // ===========================
            // 1. 全ステップ終了
            // ===========================
            if (current_step_index_ >= recipe_.steps.size())
            {
                RCLCPP_INFO(this->get_logger(),
                            "=== TransitionRecipe finished: %s ===",
                            recipe_.description.c_str());

                recipe_running_ = false; // ★ 完了したのでロック解除
                return;
            }

            const auto &step = recipe_.steps[current_step_index_];

            // ===========================
            // 2. Operation → Transition ID
            // ===========================
            auto it_trans = transition_map_.find(step.operation);
            if (it_trans == transition_map_.end())
            {
                RCLCPP_ERROR(this->get_logger(),
                             "Unknown operation '%s' at step %zu. Skipping.",
                             step.operation.c_str(),
                             current_step_index_);
                ++current_step_index_;
                execute_next_step();
                return;
            }
            uint8_t transition_id = it_trans->second;

            // ===========================
            // 3. Node name → ChangeState client
            // ===========================
            auto it_client = change_clients_.find(step.target_node_name);
            if (it_client == change_clients_.end())
            {
                RCLCPP_ERROR(this->get_logger(),
                             "No ChangeState client for node '%s' at step %zu.",
                             step.target_node_name.c_str(),
                             current_step_index_);
                ++current_step_index_;
                execute_next_step();
                return;
            }

            auto client = it_client->second;

            // ===========================
            // 4. ログ
            // ===========================
            RCLCPP_INFO(this->get_logger(),
                        "[Recipe %zu/%zu] %s → %s (timeout=%.1fs)",
                        current_step_index_ + 1,
                        recipe_.steps.size(),
                        step.operation.c_str(),
                        step.target_node_name.c_str(),
                        step.timeout_s);

            // ===========================
            // 5. リクエスト作成
            // ===========================
            auto req = std::make_shared<ChangeState::Request>();
            req->transition.id = transition_id;

            // ===========================
            // 6. 非同期コールバックで次ステップへ進む
            // ===========================
            client->async_send_request(
                req,
                [this, step](ChangeStateFuture future)
                {
                    bool ok = false;
                    try
                    {
                        auto resp = future.get();
                        ok = resp->success;
                    }
                    catch (...)
                    {
                        ok = false;
                    }

                    if (!ok)
                    {
                        RCLCPP_WARN(this->get_logger(),
                                    "Step failed: %s → %s",
                                    step.operation.c_str(),
                                    step.target_node_name.c_str());
                    }
                    else
                    {
                        RCLCPP_INFO(this->get_logger(),
                                    "Step succeeded: %s → %s",
                                    step.operation.c_str(),
                                    step.target_node_name.c_str());
                    }

                    // ===========================
                    // ★ 7. 次のステップへ進む
                    // ===========================
                    ++current_step_index_;
                    execute_next_step();
                });
        }

        // TODO 後で変更
        std::string determine_next_state(const std::string &current)
        {
            if (current == "ALL_UNCONFIGURED")
                return "STATE_ALL_OFF";
            if (current == "STATE_ALL_OFF")
                return "STATE_A_ONLY";
            if (current == "STATE_A_ONLY")
                return "STATE_B_ONLY";
            if (current == "STATE_B_ONLY")
                return "STATE_C_ONLY";
            if (current == "STATE_C_ONLY")
                return "STATE_ALL_OFF";

            // fallback
            return "STATE_ALL_OFF";
        }

        // ==== GetState  ====
        // 全ノードに対して GetState を一回だけ呼び， SemanticState を構築する
        SemanticState request_get_all_semantic_state()
        {
            current_semantic_state_.node_states.clear();    // SemanticStateをクリア
            pending_semantic_updates_ = node_names_.size(); // 応答待ちカウンタをセット

            for (const auto &name : node_names_)
            {
                auto it = getstate_clients_.find(name);
                if (it == getstate_clients_.end()) // Nodeが持っているクライアントに，nameのものが無い場合
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
            return current_semantic_state_;
        }

        // node_id に対して GetState を一回だけ呼び， SemanticState を構築する
        void request_get_semantic_state(
            const std::string &node_name,
            const rclcpp::Client<GetState>::SharedPtr &client)
        {
            if (!client->wait_for_service(500ms)) // 500ms待ってもサービスがなければタイムアウト
            {
                RCLCPP_WARN(this->get_logger(),
                            "GetState service not available for %s", node_name.c_str());
                // このノードは UNKNOWN として扱う
                current_semantic_state_.node_states[node_name] = SemanticState::State::UNKNOWN;
                if (pending_semantic_updates_ > 0)
                {
                    --pending_semantic_updates_;
                    // maybe_log_semantic_state();
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
                        // RCLCPP_INFO(this->get_logger(),
                        //            "[%s] current lifecycle state: id=%u, label=%s",
                        //            node_name.c_str(),
                        //            resp->current_state.id,
                        //            resp->current_state.label.c_str());

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
                        // maybe_log_semantic_state();
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

        // ==== 状態グラフ初期化 ====
        void generate_state_graph(const std::string &yaml_path)
        {
            if (!yaml_path.empty())
            {
                RCLCPP_INFO(this->get_logger(),
                            "Loading state graph from YAML: %s",
                            yaml_path.c_str());
                state_graph_ = init_state_graph_from_yaml(yaml_path);
            }
            else
            {
                RCLCPP_WARN(this->get_logger(),
                            "Parameter 'graph_yaml_path' is empty. Using hardcoded state graph.");
                // state_graph_ = init_state_graph();
                throw std::runtime_error("No graph YAML path provided.");
            }

            RCLCPP_INFO(this->get_logger(),
                        "State graph initialized with %zu states.",
                        state_graph_.size());
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
