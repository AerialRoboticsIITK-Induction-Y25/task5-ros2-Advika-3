from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        # Drone Node: Alpha
        Node(
            package='drone_fleet',
            executable='drone_node',
            name='drone_node_Alpha',
            parameters=[
                {'drone_name': 'Alpha'},
                {'initial_battery': 100.0},
                {'mission_name': 'Alpha_Patrol'}
            ],
            output='screen'
        ),
        # Drone Node: Beta
        Node(
            package='drone_fleet',
            executable='drone_node',
            name='drone_node_Beta',
            parameters=[
                {'drone_name': 'Beta'},
                {'initial_battery': 60.0},
                {'mission_name': 'Beta_Mapping'}
            ],
            output='screen'
        ),
        # Drone Node: Gamma
        Node(
            package='drone_fleet',
            executable='drone_node',
            name='drone_node_Gamma',
            parameters=[
                {'drone_name': 'Gamma'},
                {'initial_battery': 35.0},
                {'mission_name': 'Gamma_Recon'}
            ],
            output='screen'
        ),
        # Fleet Manager Node
        Node(
            package='drone_fleet',
            executable='fleet_manager',
            name='fleet_manager_node',
            output='screen'
        ),
        # Health Monitor Node
        Node(
            package='drone_fleet',
            executable='health_monitor',
            name='health_monitor_node',
            output='screen'
        )
    ])
