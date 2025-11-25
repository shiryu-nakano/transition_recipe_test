#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <visualization_msgs/msg/marker.hpp>
#include <tf2/LinearMath/Quaternion.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>
#include <tf2_ros/transform_broadcaster.h>

using namespace std::chrono_literals;

namespace transition_recipe_test
{

    class SimulatorNode : public rclcpp::Node
    {
    public:
        SimulatorNode() : Node("simulator_node")
        {
            // パラメータ
            this->declare_parameter<double>("initial_x", 0.0);
            this->declare_parameter<double>("initial_y", 0.0);
            this->declare_parameter<double>("initial_theta", 0.0);
            this->declare_parameter<double>("update_rate", 50.0); // Hz

            x_ = this->get_parameter("initial_x").as_double();
            y_ = this->get_parameter("initial_y").as_double();
            theta_ = this->get_parameter("initial_theta").as_double();

            vx_ = 0.0;
            vtheta_ = 0.0;

            // Subscriber
            cmd_vel_sub_ = this->create_subscription<geometry_msgs::msg::Twist>(
                "cmd_vel", 10,
                std::bind(&SimulatorNode::cmd_vel_callback, this, std::placeholders::_1));

            // Publishers
            odom_pub_ = this->create_publisher<nav_msgs::msg::Odometry>("odom", 10);
            pose_pub_ = this->create_publisher<geometry_msgs::msg::PoseStamped>("current_pose", 10);
            marker_pub_ = this->create_publisher<visualization_msgs::msg::Marker>("robot_marker", 10);

            // TF broadcaster
            tf_broadcaster_ = std::make_shared<tf2_ros::TransformBroadcaster>(this);

            // Timer
            double update_rate = this->get_parameter("update_rate").as_double();
            auto period = std::chrono::duration<double>(1.0 / update_rate);
            timer_ = this->create_wall_timer(
                std::chrono::duration_cast<std::chrono::milliseconds>(period),
                std::bind(&SimulatorNode::timer_callback, this));

            last_time_ = this->now();

            RCLCPP_INFO(this->get_logger(), "SimulatorNode initialized at (%.2f, %.2f, %.2f)",
                        x_, y_, theta_);
        }

    private:
        void cmd_vel_callback(const geometry_msgs::msg::Twist::SharedPtr msg)
        {
            vx_ = msg->linear.x;
            vtheta_ = msg->angular.z;
            RCLCPP_INFO_THROTTLE(this->get_logger(), *this->get_clock(), 1000,
                                 "Received cmd_vel: linear.x=%.2f, angular.z=%.2f", vx_, vtheta_);
        }

        void timer_callback()
        {
            auto current_time = this->now();
            double dt = (current_time - last_time_).seconds();
            last_time_ = current_time;

            // 数値積分で位置を更新
            x_ += vx_ * cos(theta_) * dt;
            y_ += vx_ * sin(theta_) * dt;
            theta_ += vtheta_ * dt;

            // Normalize theta to [-pi, pi]
            while (theta_ > M_PI)
                theta_ -= 2.0 * M_PI;
            while (theta_ < -M_PI)
                theta_ += 2.0 * M_PI;

            RCLCPP_INFO_THROTTLE(this->get_logger(), *this->get_clock(), 2000,
                                 "Robot position: x=%.2f, y=%.2f, theta=%.2f, vx=%.2f, vtheta=%.2f",
                                 x_, y_, theta_, vx_, vtheta_);

            // Quaternion作成
            tf2::Quaternion q;
            q.setRPY(0, 0, theta_);
            geometry_msgs::msg::Quaternion q_msg = tf2::toMsg(q);

            // TFをパブリッシュ
            publish_tf(current_time, q_msg);

            // Odometryをパブリッシュ
            publish_odom(current_time, q_msg);

            // PoseStampedをパブリッシュ
            publish_pose(current_time, q_msg);

            // マーカーをパブリッシュ
            publish_marker(current_time, q_msg);
        }

        void publish_tf(const rclcpp::Time &stamp, const geometry_msgs::msg::Quaternion &q)
        {
            geometry_msgs::msg::TransformStamped tf;
            tf.header.stamp = stamp;
            tf.header.frame_id = "odom";
            tf.child_frame_id = "base_link";
            tf.transform.translation.x = x_;
            tf.transform.translation.y = y_;
            tf.transform.translation.z = 0.0;
            tf.transform.rotation = q;
            tf_broadcaster_->sendTransform(tf);
        }

        void publish_odom(const rclcpp::Time &stamp, const geometry_msgs::msg::Quaternion &q)
        {
            nav_msgs::msg::Odometry odom;
            odom.header.stamp = stamp;
            odom.header.frame_id = "odom";
            odom.child_frame_id = "base_link";

            odom.pose.pose.position.x = x_;
            odom.pose.pose.position.y = y_;
            odom.pose.pose.position.z = 0.0;
            odom.pose.pose.orientation = q;

            odom.twist.twist.linear.x = vx_;
            odom.twist.twist.angular.z = vtheta_;

            odom_pub_->publish(odom);
        }

        void publish_pose(const rclcpp::Time &stamp, const geometry_msgs::msg::Quaternion &q)
        {
            geometry_msgs::msg::PoseStamped pose;
            pose.header.stamp = stamp;
            pose.header.frame_id = "odom";
            pose.pose.position.x = x_;
            pose.pose.position.y = y_;
            pose.pose.position.z = 0.0;
            pose.pose.orientation = q;
            pose_pub_->publish(pose);
        }

        void publish_marker(const rclcpp::Time &stamp, const geometry_msgs::msg::Quaternion &q)
        {
            visualization_msgs::msg::Marker marker;
            marker.header.stamp = stamp;
            marker.header.frame_id = "odom";
            marker.ns = "robot";
            marker.id = 0;
            marker.type = visualization_msgs::msg::Marker::ARROW;
            marker.action = visualization_msgs::msg::Marker::ADD;

            marker.pose.position.x = x_;
            marker.pose.position.y = y_;
            marker.pose.position.z = 0.1;
            marker.pose.orientation = q;

            marker.scale.x = 0.5; // 矢印の長さ
            marker.scale.y = 0.1; // 矢印の幅
            marker.scale.z = 0.1; // 矢印の高さ

            marker.color.r = 0.0;
            marker.color.g = 1.0;
            marker.color.b = 0.0;
            marker.color.a = 1.0;

            marker_pub_->publish(marker);
        }

        // State variables
        double x_, y_, theta_;
        double vx_, vtheta_;
        rclcpp::Time last_time_;

        // ROS interface
        rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_sub_;
        rclcpp::Publisher<nav_msgs::msg::Odometry>::SharedPtr odom_pub_;
        rclcpp::Publisher<geometry_msgs::msg::PoseStamped>::SharedPtr pose_pub_;
        rclcpp::Publisher<visualization_msgs::msg::Marker>::SharedPtr marker_pub_;
        std::shared_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;
        rclcpp::TimerBase::SharedPtr timer_;
    };

}
int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<transition_recipe_test::SimulatorNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
