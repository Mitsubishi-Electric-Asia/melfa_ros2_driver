from moveit_configs_utils import MoveItConfigsBuilder
from moveit_configs_utils.launches import generate_move_group_launch


def generate_launch_description():
    moveit_config = MoveItConfigsBuilder("rh6frh5520", package_name="melfa_rh6frh5520_moveit_config").to_moveit_configs()
    return generate_move_group_launch(moveit_config)
