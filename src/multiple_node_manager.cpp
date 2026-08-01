// src/test_recipe_node.cpp
#include <rclcpp/rclcpp.hpp>
#include <lifecycle_msgs/srv/change_state.hpp>
#include <lifecycle_msgs/srv/get_state.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <std_msgs/msg/string.hpp>
#include <std_msgs/msg/float64.hpp>
#include <visualization_msgs/msg/marker.hpp>

#include <map>
#include <string>
#include <chrono>

#include "transition_recipe_test/common_types.hpp"
#include "transition_recipe_test/graph/graph.hpp"
#include "transition_recipe_test/graph/graph_generator.hpp"
// #include "transition_recipe_test/recipe_generator.hpp"
#include "transition_recipe_test/switching_strategy.hpp"
#include "transition_recipe_test/msg/transition_request.hpp"

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

            // 1.1 YAML から node_ids を読み込む
            // 管理対象のnodeの名前
            node_names_ = this->declare_parameter<std::vector<std::string>>(
                "node_ids",
                std::vector<std::string>{} // デフォルトは空
            );

            // 1.2 状態を管理するgraphをyamlから読み取る
            // 管理対象のnode群の状態組み合わせによって決まる全体の状態ごとのstate id
            const std::string graph_yaml_path = this->declare_parameter<std::string>(
                "graph_yaml_path", 
                ""
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

            // 2. node_names_ からクライアント辞書を構築 → lifecycle nodeの状態取得と状態遷移に用いる辞書
            init_clients_from_node_list();

            // 3. 状態グラフの構築
            generate_state_graph(graph_yaml_path);


            /*
            // x,y の初期値（現状ハードコーディングしている）
            this->declare_parameter<double>("initial_x", 0.0);
            this->declare_parameter<double>("initial_y", 0.0);
            x_ = this->get_parameter("initial_x").as_double();
            y_ = this->get_parameter("initial_y").as_double();
            */

            // ---- 新: 現在状態と経過時間を publish するための pub ----
            state_id_pub_ = this->create_publisher<std_msgs::msg::String>(
                "/current_state_id", 10);
            timespan_pub_ = this->create_publisher<std_msgs::msg::Float64>(
                "/state_timespan_sec", 10);
            state_text_marker_pub_ = this->create_publisher<visualization_msgs::msg::Marker>(
                "/state_text_marker", 10);

            // 判定ノードからの遷移指示を受け取る subscriber
            transition_request_sub_ =
                this->create_subscription<transition_recipe_test::msg::TransitionRequest>(
                    "/transition_request", 10,
                    std::bind(&MultipleNodeManager::on_transition_request, this,
                              std::placeholders::_1));

            // operation → Transition ID のマップ（辞書？）を作成
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
        std::vector<std::string> node_names_; //= {"A_node", "B_node", "C_node"};

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

        // ---- 旧: pose/odom サブスクライバと位置情報（判定外部化に伴い保留） ----
        //rclcpp::Subscription<geometry_msgs::msg::PoseStamped>::SharedPtr pose_sub_;
	    //rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr odom_sub_;
        //double x_;
        //double y_;
        int temp_count_ = 0;

        // ---- 新: 現在状態と経過時間の出力 / 判定ノードからの遷移指示の入力 ----
        rclcpp::Publisher<std_msgs::msg::String>::SharedPtr state_id_pub_;
        rclcpp::Publisher<std_msgs::msg::Float64>::SharedPtr timespan_pub_;
        rclcpp::Publisher<visualization_msgs::msg::Marker>::SharedPtr state_text_marker_pub_;
        rclcpp::Subscription<transition_recipe_test::msg::TransitionRequest>::SharedPtr
            transition_request_sub_;

        SwitchingStrategy switcher_; // 状態遷移判定ロジック
        // このswitcherは、判定ではなくて、recipeを(from, to)　の入力に対してrecipeを返すだけになる。今のrecipe_generatorをそのままくっつければOK
        //判定は他のnodeが行い、判定で状態遷移が必要になったときにのみ、topicが来る。それをサブスクライブしたときにcallbackで上記のswitcherが呼ばれるようにしたい。
        // TODO　このstrategyは今後外部のパッケージとして実装されるので不要になるが、すぐに消さない

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

        // ==== タイマーコールバック（main処理） ====
        void timer_callback()
        {
            //　timerCallbackで行う処理
            /*
            このノードの出力：
                SemanticState current_semantic_state から得られるstr state_id
                timespan （現在の状態になってからの経過秒数）
            */



            // double elapsed = (now() - start_time_).seconds();

            // ① まだ前回の GetState が返りきっていない場合はスキップ
            if (pending_semantic_updates_ != 0) return;

            // レシピ実行中は過渡状態をpublishしない（judgeの誤発火を防ぐ）
            if (recipe_running_) return;


            // ② client経由で取得した最新のsemanticStateをKeyにしてgraphからstate_id を取得
            auto state_id_opt = state_graph_.getCurrentSemanticState(current_semantic_state_);
            std::string current_state_id;
            if (!state_id_opt)
            {
                RCLCPP_WARN(this->get_logger(),
                            "NO MATCH for current SemanticState");
                current_state_id = "UNKNOWN";
            }
            else
            {
                current_state_id = *state_id_opt;
            }

            // ③ state_id が変わったら経過時間をリセット
            if (last_state_id_.empty() || current_state_id != last_state_id_)
            {
                last_state_id_ = current_state_id;
                last_transition_time_ = now();
            }
            double since_last = (now() - last_transition_time_).seconds();

            // ④ state_id と経過時間を publish（判定ノード向けの出力）
            {
                std_msgs::msg::String state_msg;
                state_msg.data = current_state_id;
                state_id_pub_->publish(state_msg);

                std_msgs::msg::Float64 timespan_msg;
                timespan_msg.data = since_last;
                timespan_pub_->publish(timespan_msg);

                // 現在状態をRVizにテキストマーカーで表示
                visualization_msgs::msg::Marker text_marker;
                text_marker.header.frame_id = "odom";
                text_marker.header.stamp = now();
                text_marker.ns = "system_state";
                text_marker.id = 0;
                text_marker.type = visualization_msgs::msg::Marker::TEXT_VIEW_FACING;
                text_marker.action = visualization_msgs::msg::Marker::ADD;
                text_marker.pose.position.x = 0.0;
                text_marker.pose.position.y = 0.0;
                text_marker.pose.position.z = 1.5;
                text_marker.pose.orientation.w = 1.0;
                text_marker.scale.z = 0.5;
                text_marker.text = current_state_id;
                if (current_state_id == "pure_pursuit_planner") {
                    text_marker.color = []{
                        std_msgs::msg::ColorRGBA c; c.r=0.0; c.g=1.0; c.b=0.0; c.a=1.0; return c;}();
                } else if (current_state_id == "dwa_planner") {
                    text_marker.color = []{
                        std_msgs::msg::ColorRGBA c; c.r=1.0; c.g=0.3; c.b=0.0; c.a=1.0; return c;}();
                } else {
                    text_marker.color = []{
                        std_msgs::msg::ColorRGBA c; c.r=1.0; c.g=1.0; c.b=1.0; c.a=1.0; return c;}();
                }
                state_text_marker_pub_->publish(text_marker);
            }

            RCLCPP_INFO(this->get_logger(),
                        "System State = %s, since_last=%.2f sec",
                        current_state_id.c_str(), since_last);


            // ⑤ 次の GetState バッチを投げる
            request_get_all_semantic_state();

            // RCLCPP_INFO(this->get_logger(),
            //             "Hello, elapsed %.2f sec", elapsed);
        }

        // PoseCallBack（判定外部化に伴い保留）
        // void pose_callback(const geometry_msgs::msg::PoseStamped::SharedPtr msg)
        // {
        //     x_ = msg->pose.position.x;
        //     y_ = msg->pose.position.y;
        //     // RCLCPP_INFO(this->get_logger(),
        //     //             "Received pose: x=%.3f, y=%.3f",
        //     //             msg->pose.position.x, msg->pose.position.y);
        // }

        // OdomCallBack（判定外部化に伴い保留）
        // void odom_callback(const nav_msgs::msg::Odometry::SharedPtr msg)
        // {
        //     x_ = msg->pose.pose.position.x;
        //     y_ = msg->pose.pose.position.y;
        // }

        // ==== 判定ノードからの遷移指示の受信 ====
        void on_transition_request(
            const transition_recipe_test::msg::TransitionRequest::SharedPtr msg)
        {
            const auto &from = msg->from_state_id;
            const auto &to = msg->target_state_id;

            RCLCPP_INFO(this->get_logger(),
                        "[TransitionRequest] %s -> %s",
                        from.c_str(), to.c_str());

            auto maybe_recipe = switcher_.call_transition_recipe(from, to);
            if (!maybe_recipe)
            {
                RCLCPP_WARN(this->get_logger(),
                            "No recipe defined for %s -> %s",
                            from.c_str(), to.c_str());
                return;
            }

            execute_transition_recipe(*maybe_recipe);

            // 経過時間カウンタの基準を遷移先で更新
            last_state_id_ = to;
            last_transition_time_ = now();
        }

        // ==== Recipe 実行 ====
        void execute_transition_recipe(const TransitionRecipe &recipe)
        {
            if (recipe_running_)
            {
                // 他の状態遷移が走っているとき
                RCLCPP_WARN(this->get_logger(),
                            "Cannot start recipe '%s' because another recipe is running.",
                            recipe.description.c_str());
                return;
            }

            if (recipe.steps.empty())
            {
                // recipeがからのとき
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
