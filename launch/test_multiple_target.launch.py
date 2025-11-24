from launch import LaunchDescription
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory
import os

def generate_launch_description():
    pkg_share = get_package_share_directory('transition_recipe_test')
    params_file = os.path.join(pkg_share, 'config', 'multiple_nodes.yaml')

    return LaunchDescription([
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
            parameters=[params_file],  
            output='screen'
        ),
    ])