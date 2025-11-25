from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from ament_index_python.packages import get_package_share_directory
import os

def generate_launch_description():
    pkg_share = get_package_share_directory('transition_recipe_test')
    params_file = os.path.join(pkg_share, 'config', 'multiple_nodes.yaml')

    # ★ グラフ YAML のデフォルトパス
    default_graph_yaml = os.path.join(
        pkg_share, 'config', 'state_graph.yaml'
    )

    # ★ コマンドラインからも変えられるように LaunchArgument を定義
    graph_yaml_arg = DeclareLaunchArgument(
        'graph_yaml_path',
        default_value=default_graph_yaml,
        description='Path to semantic state graph YAML'
    )
    # simulator のパラメータファイル
    config_file = os.path.join(
        get_package_share_directory("transition_recipe_test"),
        'config',
        'sim_params.yaml'
    )
    
    # RViz設定ファイルのパス
    rviz_config_file = os.path.join(
        get_package_share_directory("transition_recipe_test"),
        'rviz',
        'demo.rviz'
    )

    return LaunchDescription([
        graph_yaml_arg,

        Node(
            package="transition_recipe_test",
            executable='simulator_node',
            name='robot_simulator_node',
            parameters=[config_file],
            output='screen'
        ),
        
        Node(
            package='transition_recipe_test',
            executable='a_node',
            name='A_node',
            output='screen'
        ),
        Node(
            package='transition_recipe_test',
            executable='b_node',
            name='B_node',
            output='screen'
        ),
        Node(
            package='transition_recipe_test',
            executable='c_node',
            name='C_node',
            output='screen'
        ),
        Node(
            package='transition_recipe_test',
            executable='multiple_node_manager',
            name='multiple_node_manager',
            parameters=[
                params_file,  # node_ids などはここから読む
                {
                    'graph_yaml_path': LaunchConfiguration('graph_yaml_path')
                }
            ],
            output='screen'
        ),
        
        # RViz
        Node(
            package='rviz2',
            executable='rviz2',
            name='rviz2',
            arguments=['-d', rviz_config_file],
            output='screen'
        ),
    ])
