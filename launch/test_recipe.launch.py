# launch/test_recipe.launch.py
from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        Node(
            package='transition_recipe_test',
            executable='sample_lifecycle_node',
            name='sample_node',
            output='screen'
        ),
        Node(
            package='transition_recipe_test',
            executable='test_recipe_node',
            name='recipe_test_node',
            output='screen'
        ),
    ])
