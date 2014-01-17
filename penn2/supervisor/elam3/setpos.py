import random
import numpy as np

import config


# moves the ball and box to random positions
def SetInitialPositions(hand_pos):

    box_pos = np.array([0.0, 0.0, 0.0])

    box_pos[0] = random.uniform(-0.5, 0.5) * config.workspaceRadius
    if config.dimension > 1:
        box_pos[1] = random.uniform(-0.5, 0.5) * config.workspaceRadius
    if config.dimension > 2:
        box_pos[2] = random.uniform(-0.5, 0.5) * config.workspaceRadius

    return box_pos


def SetBoxPos(hand_pos):

    box_pos = np.array([0.0, 0.0, 0.0])

    box_pos[0] = random.uniform(-0.5, 0.5) * config.workspaceRadius
    if config.dimension > 1:
        box_pos[1] = random.uniform(-0.5, 0.5) * config.workspaceRadius
    if config.dimension > 2:
        box_pos[2] = random.uniform(-0.5, 0.5) * config.workspaceRadius

    return box_pos