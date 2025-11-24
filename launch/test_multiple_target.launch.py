from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
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
            name='recipe_test_node',
            output='screen'
        ),
    ])

