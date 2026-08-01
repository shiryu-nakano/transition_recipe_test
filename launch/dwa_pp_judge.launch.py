import os

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution

from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare

from ament_index_python.packages import get_package_share_directory


def generate_launch_description():
    package_name: str      = 'transition_recipe_test'
    simulator_package: str = 'arcanain_simulator'
    pp_package:        str = 'pure_pursuit_planner'

    pkg_share = get_package_share_directory(package_name)

    params_file = os.path.join(pkg_share, 'config', 'multiple_nodes.yaml')

    default_graph_yaml = os.path.join(pkg_share, 'config', 'dwa_pp.yaml')
    graph_yaml_arg = DeclareLaunchArgument(
        'graph_yaml_path',
        default_value=default_graph_yaml,
        description='Path to semantic state graph YAML',
    )

    default_path_csv = os.path.expanduser(
        '~/ros2_ws/src/path_smoother/path/waypoint_loop.csv'
    )
    path_csv_arg = DeclareLaunchArgument(
        'path_csv',
        default_value=default_path_csv,
        description='Path to the CSV file for pure_pursuit_planner',
    )

    dwa_params = PathJoinSubstitution(
        [FindPackageShare('dwa_planner'), 'config', 'params.yaml']
    )

    urdf_path = os.path.expanduser(
        '~/ros2_ws/src/arcanain_simulator/urdf/mobile_robot.urdf.xml'
    )
    with open(urdf_path, 'r') as f:
        robot_description = f.read()

    rviz_config_file = PathJoinSubstitution(
        [FindPackageShare(pp_package), 'rviz', 'pure_pursuit_planner.rviz']
    )

    return LaunchDescription([
        graph_yaml_arg,
        path_csv_arg,

        # ===== 可視化 =====
        Node(
            package='rviz2',
            executable='rviz2',
            name='rviz2',
            output='log',
            arguments=['-d', rviz_config_file],
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

        # ===== シミュレータ =====
        Node(
            package=simulator_package,
            executable='odometry_pub',
            output='screen',
        ),
        Node(
            package=simulator_package,
            executable='obstacle_pub',
            output='screen',
        ),
        
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
        ),
    ])
