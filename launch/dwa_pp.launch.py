import os

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution

from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare

from ament_index_python.packages import get_package_share_directory

"""
dwa_planner と pure_pursuit_planner の 2 つを管理対象とする実験用 launch。

シミュレータ：arcanain_simulator（odom + 障害物 + waypoint）
状態グラフ：config/dwa_pp.yaml
管理対象ノード：config/multiple_nodes.yaml で宣言
RViz：pure_pursuit_planner パッケージ提供の rviz（障害物表示済み）を流用

判定ノード（transition_judge_interface）と組み合わせて使う想定。
判定ノードが /transition_request を投げ込めば自動で planner を切り替える。

手動で試すなら：
  ros2 topic pub --once /transition_request transition_recipe_test/msg/TransitionRequest \\
    '{from_state_id: "ALL_UNCONFIGURED", target_state_id: "ALL_CONFIGURED"}'
  ros2 topic pub --once /transition_request transition_recipe_test/msg/TransitionRequest \\
    '{from_state_id: "ALL_CONFIGURED", target_state_id: "pure_pursuit_planner"}'
  ros2 topic pub --once /transition_request transition_recipe_test/msg/TransitionRequest \\
    '{from_state_id: "pure_pursuit_planner", target_state_id: "dwa_planner"}'
"""


def generate_launch_description():
    package_name = 'transition_recipe_test'
    simulator_package = 'arcanain_simulator'
    pp_package = 'pure_pursuit_planner'

    pkg_share = get_package_share_directory(package_name)

    # multiple_node_manager 用の YAML（node_ids 一覧）
    params_file = os.path.join(pkg_share, 'config', 'multiple_nodes.yaml')

    # 状態グラフ YAML
    default_graph_yaml = os.path.join(pkg_share, 'config', 'dwa_pp.yaml')
    graph_yaml_arg = DeclareLaunchArgument(
        'graph_yaml_path',
        default_value=default_graph_yaml,
        description='Path to semantic state graph YAML',
    )

    # dwa_planner のパラメータ
    dwa_params = PathJoinSubstitution(
        [FindPackageShare('dwa_planner'), 'config', 'params.yaml']
    )

    # URDF（arcanain_simulator のロボットモデル）
    urdf_path = os.path.expanduser(
        '~/ros2_ws/src/arcanain_simulator/urdf/mobile_robot.urdf.xml'
    )
    with open(urdf_path, 'r') as f:
        robot_description = f.read()

    # RViz は pure_pursuit_planner の obstacle_simulation 用 config を流用（障害物表示済み）
    rviz_config_file = PathJoinSubstitution(
        [FindPackageShare(pp_package), 'rviz', 'pure_pursuit_planner.rviz']
    )

    # ===== 可視化系 =====
    rviz_node = Node(
        package='rviz2',
        executable='rviz2',
        name='rviz2',
        output='log',
        arguments=['-d', rviz_config_file],
    )

    # obstacle_simulation.py に倣った tf 補助（map → dummy_link）
    dummy_node = Node(
        package='tf2_ros',
        executable='static_transform_publisher',
        output='screen',
        arguments=['0.0', '0.0', '0.0', '0.0', '0.0', '0.0', 'map', 'dummy_link'],
    )

    robot_description_node = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        name='robot_state_publisher',
        output='both',
        parameters=[{'robot_description': robot_description}],
    )

    joint_state_publisher_node = Node(
        package='joint_state_publisher',
        executable='joint_state_publisher',
        name='joint_state_publisher',
        output='both',
        parameters=[{'joint_state_publisher': robot_description}],
    )

    # ===== arcanain_simulator: odom + 障害物 + waypoint =====
    odometry_pub_node = Node(
        package=simulator_package,
        executable='odometry_pub',
        output='screen',
    )

    obstacle_pub_node = Node(
        package=simulator_package,
        executable='obstacle_pub',
        output='screen',
    )

    waypoint_pub_node = Node(
        package=simulator_package,
        executable='waypoint_pub',
        output='screen',
    )

    # ===== path 提供 =====
    # pure_pursuit_planner 用：tgt_path を供給
    path_publisher_node = Node(
        package='path_smoother',
        executable='path_publisher_gps_04',
        output='screen',
    )

    # dwa_planner 用：waypoint を供給
    #path_waypoint_pub_node = Node(
    #    package='path_smoother',
    #    executable='waypoint_publisher',
    #    output='screen',
    #)
    
    #path_waypoint_pub_node = Node(
    #    package=simulator_package,
    #    executable='waypoint_publisher',
    #    output="screen",
    #)

    # ===== 管理対象 lifecycle planner =====
    dwa_planner_node = Node(
        package='dwa_planner',
        executable='dwa_planner',
        output='screen',
        parameters=[dwa_params],
    )

    pure_pursuit_planner_node = Node(
        package='pure_pursuit_planner',
        executable='pure_pursuit_planner',
        # 実装内部のデフォルト名は "pure_pursuit_node" だが、
        # multiple_nodes.yaml / dwa_pp.yaml に合わせるためにリネームする。
        name='pure_pursuit_planner',
        output='screen',
    )

    # ===== 管理ノード =====
    multiple_node_manager_node = Node(
        package=package_name,
        executable='multiple_node_manager',
        name='multiple_node_manager',
        parameters=[
            params_file,
            {'graph_yaml_path': LaunchConfiguration('graph_yaml_path')},
        ],
        output='screen',
    )

    return LaunchDescription([
        graph_yaml_arg,
        rviz_node,
        dummy_node,
        robot_description_node,
        joint_state_publisher_node,
        odometry_pub_node,
        obstacle_pub_node,
        waypoint_pub_node,
        path_publisher_node,
        #path_waypoint_pub_node,
        dwa_planner_node,
        pure_pursuit_planner_node,
        multiple_node_manager_node,
    ])
