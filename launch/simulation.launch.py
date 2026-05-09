import os

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution

from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare

from ament_index_python.packages import get_package_share_directory

"""
ArcanainSimulatorを起動するlaunchファイル
transition_recipe_testのサンプルノードも同時に起動する。
今後実用に向けて拡張予定
"""
def generate_launch_description():
    package_name = 'transition_recipe_test'
    simulator_package = 'arcanain_simulator'

    pkg_share = get_package_share_directory(package_name)
    params_file = os.path.join(pkg_share, 'config', 'sample/multiple_nodes.yaml')

    default_graph_yaml = os.path.join(
        pkg_share, 'config', 'sample/state_graph.yaml'
    )

    graph_yaml_arg = DeclareLaunchArgument(
        'graph_yaml_path',
        default_value=default_graph_yaml,
        description='Path to semantic state graph YAML'
    )

    rviz_config_file = os.path.join(pkg_share, 'rviz', 'demo.rviz')

    # URDF
    file_path = os.path.expanduser(
        '~/ros2_ws/src/arcanain_simulator/urdf/mobile_robot.urdf.xml'
    )
    with open(file_path, 'r') as file:
        robot_description = file.read()

    # RViz
    rviz_node = Node(
        package='rviz2',
        executable='rviz2',
        name='rviz2',
        output='log',
        arguments=['-d', rviz_config_file],
    )

    # arcanain_simulator の可視化ノード
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

    odometry_pub_node = Node(
        package=simulator_package,
        executable='odometry_pub',
        output='screen',
    )

    # transition_recipe_test のノード
    a_node = Node(
        package=package_name,
        executable='a_node',
        name='A_node',
        output='screen',
    )

    b_node = Node(
        package=package_name,
        executable='b_node',
        name='B_node',
        output='screen',
    )

    c_node = Node(
        package=package_name,
        executable='c_node',
        name='C_node',
        output='screen',
    )

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
        robot_description_node,
        joint_state_publisher_node,
        odometry_pub_node,
        a_node,
        b_node,
        c_node,
        multiple_node_manager_node,
    ])
