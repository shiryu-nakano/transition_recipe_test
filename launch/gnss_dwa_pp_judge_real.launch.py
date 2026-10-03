import os

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.conditions import IfCondition
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution

from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare

from ament_index_python.packages import get_package_share_directory

"""
実機（GNSS）で dwa_planner / pure_pursuit_planner を lifecycle で切り替える launch。

構成は pure_pursuit_planner/launch/odrive_gps_switch_pure_pursuit.py と
dwa_planner/launch/gnss_dwa_planner.py の共通部分（GNSS 自己位置推定）をベースに、
planner を両方 lifecycle で起動し、multiple_node_manager と判定ノードで切り替える。

- LiDAR：lidar_based_obstacle_detection の launch のみ（sllidar_s2 と base_link->laser の TF を含む）
         perception_obstacle は同じ topic を出すので起動しない
- 経路：path_smoother::waypoint_publisher が tgt_path（PP 用）と waypoint（DWA 用）を同一 CSV から publish
- 判定ノード：実機では global_obstacle_markers が無いので /filtered_scan で障害物判定する

判定ノード無しで手動切り替えを試す場合：
  ros2 launch transition_recipe_test gnss_dwa_pp_judge_real.launch.py use_judge:=false
  ros2 topic pub --once /transition_request transition_recipe_test/msg/TransitionRequest \\
    '{from_state_id: "ALL_UNCONFIGURED", target_state_id: "ALL_CONFIGURED"}'
  ros2 topic pub --once /transition_request transition_recipe_test/msg/TransitionRequest \\
    '{from_state_id: "ALL_CONFIGURED", target_state_id: "pure_pursuit_planner"}'
"""


def generate_launch_description():
    package_name: str      = 'transition_recipe_test'
    simulator_package: str = 'arcanain_simulator'
    odrive_package:    str = 'odrive_ros2_control'
    rtk_judge_package: str = 'rtk_judge'

    pkg_share = get_package_share_directory(package_name)

    params_file = os.path.join(pkg_share, 'config', 'multiple_nodes.yaml')

    default_graph_yaml = os.path.join(pkg_share, 'config', 'dwa_pp.yaml')
    graph_yaml_arg = DeclareLaunchArgument(
        'graph_yaml_path',
        default_value=default_graph_yaml,
        description='Path to semantic state graph YAML',
    )

    # waypoint_publisher の既定値と同じ CSV
    default_path_csv = os.path.expanduser(
        '~/ros2_ws/src/path_smoother/path/tsukuba_1018_2025.csv'
    )
    path_csv_arg = DeclareLaunchArgument(
        'path_csv',
        default_value=default_path_csv,
        description='Path to the CSV file for waypoint_publisher (tgt_path / waypoint)',
    )

    # 判定ノードの閾値。src 側の yaml を直接読むので、書き換えて launch し直すだけで反映される（ビルド不要）
    default_judge_params = os.path.expanduser(
        '~/ros2_ws/src/transition_judge_interface/config/params.yaml'
    )
    judge_params_arg = DeclareLaunchArgument(
        'judge_params',
        default_value=default_judge_params,
        description='Path to transition_judge_interface params YAML (thresholds)',
    )

    use_judge_arg = DeclareLaunchArgument(
        'use_judge',
        default_value='true',
        description='Launch transition_judge_interface (false: send /transition_request manually)',
    )

    use_rviz_arg = DeclareLaunchArgument(
        'use_rviz',
        default_value='true',
        description='Launch rviz2 (set false when no display, e.g. over SSH)',
    )

    dwa_params = PathJoinSubstitution(
        [FindPackageShare('dwa_planner'), 'config', 'params.yaml']
    )

    urdf_path = os.path.expanduser(
        '~/ros2_ws/src/arcanain_simulator/urdf/mobile_robot.urdf.xml'
    )
    with open(urdf_path, 'r') as f:
        robot_description = f.read()

    rviz_config_file = os.path.join(pkg_share, 'rviz', 'dwa_pp_real.rviz')

    lidar_obstacle_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            PathJoinSubstitution([
                FindPackageShare('lidar_based_obstacle_detection'),
                'launch',
                'lidar_based_obstacle_detection_launch.py',
            ])
        )
    )

    return LaunchDescription([
        graph_yaml_arg,
        path_csv_arg,
        judge_params_arg,
        use_judge_arg,
        use_rviz_arg,

        # ===== 可視化 =====
        Node(
            package='rviz2',
            executable='rviz2',
            name='rviz2',
            output='log',
            arguments=['-d', rviz_config_file],
            condition=IfCondition(LaunchConfiguration('use_rviz')),
        ),
        Node(
            package='tf2_ros',
            executable='static_transform_publisher',
            output='screen',
            arguments=['0.0', '0.0', '0.0', '0.0', '0.0', '0.0', 'map', 'dummy_link'],
        ),
        Node(
            package='robot_state_publisher',
            executable='robot_state_publisher',
            name='robot_state_publisher',
            output='both',
            parameters=[{'robot_description': robot_description}],
        ),
        Node(
            package='joint_state_publisher',
            executable='joint_state_publisher',
            name='joint_state_publisher',
            output='both',
            parameters=[{'joint_state_publisher': robot_description}],
        ),

        # ===== GNSS 自己位置推定 =====
        Node(
            package='gnss_preprocessing',
            executable='gnss_preprocessing',
            output='screen',
        ),
        Node(
            package=odrive_package,
            executable='control_odrive_and_odom_pub_gps',
            output='screen',
        ),
        Node(
            package=rtk_judge_package,
            executable='judge_rtk_status_origin',
            output='screen',
        ),
        Node(
            package=simulator_package,
            executable='odrive_gps_switch_pub',
            output='screen',
        ),

        # ===== LiDAR + 障害物検出（/scan, /filtered_scan） =====
        lidar_obstacle_launch,

        # ===== Path Publisher =====
        # tgt_path（PP用）と waypoint（DWA用）を同一CSVから両方publishする
        Node(
            package='path_smoother',
            executable='waypoint_publisher',
            output='screen',
            parameters=[{'path_file': LaunchConfiguration('path_csv')}],
        ),

        # ===== lifecycle Path Planner =====
        Node(
            package='dwa_planner',
            executable='dwa_planner',
            output='screen',
            parameters=[dwa_params],
        ),
        Node(
            package='pure_pursuit_planner',
            executable='pure_pursuit_planner',
            # 実装内部のデフォルト名は "pure_pursuit_node" なので dwa_pp.yaml に合わせてリネーム
            name='pure_pursuit_planner',
            output='screen',
        ),

        # ===== 管理ノード =====
        Node(
            package=package_name,
            executable='multiple_node_manager',
            name='multiple_node_manager',
            parameters=[
                params_file,
                {'graph_yaml_path': LaunchConfiguration('graph_yaml_path')},
            ],
            output='screen',
        ),

        # ===== 判定ノード =====
        Node(
            package='transition_judge_interface',
            executable='transition_judge_interface',
            name='transition_judge_interface',
            output='screen',
            parameters=[LaunchConfiguration('judge_params')],
            # 実機では DWA と同じフィルタ後の scan で障害物判定する
            remappings=[('/scan', '/filtered_scan')],
            condition=IfCondition(LaunchConfiguration('use_judge')),
        ),
    ])
