import random
import numpy as np

import config


# moves the ball and box to random positions
def SetInitialPositions(hand_pos):

    workspaceRadius = 12.0

    ball_pos = np.array([0.0, 0.0, 0.0])
    box_pos = np.array([0.0, 0.0, 0.0])

    ball_pos[0] = random.uniform(-0.5, 0.5) * workspaceRadius
    if config.dimension > 1:
        ball_pos[1] = random.uniform(-0.5, 0.5) * workspaceRadius
    if config.dimension > 2:
        ball_pos[2] = random.uniform(-0.5, 0.5) * workspaceRadius

    diff = hand_pos - ball_pos
    dist = np.sqrt(diff.dot(diff))

    # make sure the ball is far enough from the hand
    while dist < 2.0:
        ball_pos[0] = random.uniform(-0.5, 0.5) * workspaceRadius
        if config.dimension > 1:
            ball_pos[1] = random.uniform(-0.5, 0.5) * workspaceRadius
        if config.dimension > 2:
            ball_pos[2] = random.uniform(-0.5, 0.5) * workspaceRadius
        diff = hand_pos - ball_pos
        dist = np.sqrt(diff.dot(diff))

    box_pos[0] = random.uniform(-0.5, 0.5) * workspaceRadius
    if config.dimension > 1:
        box_pos[1] = random.uniform(-0.5, 0.5) * workspaceRadius
    if config.dimension > 2:
        box_pos[2] = random.uniform(-0.5, 0.5) * workspaceRadius

    diff = box_pos - ball_pos
    dist = np.sqrt(diff.dot(diff))

    # make sure the box is far enough from the ball
    while dist < 2.0:
        box_pos[0] = random.uniform(-0.5, 0.5) * workspaceRadius
        if config.dimension > 1:
            box_pos[1] = random.uniform(-0.5, 0.5) * workspaceRadius
        if config.dimension > 2:
            box_pos[2] = random.uniform(-0.5, 0.5) * workspaceRadius
        diff = box_pos - ball_pos
        dist = np.sqrt(diff.dot(diff))

    return ball_pos, box_pos


def SetBallBoxPos(hand_pos):

    ball_pos = np.array([0.0, 0.0, 0.0])
    box_pos = np.array([0.0, 0.0, 0.0])

    # magic numbers keep the ball and box within reachable distance
    ball_pos[0] = random.random() * -5.0
    ball_pos[1] = random.random() * -2.0 - 4.0
    ball_pos[2] = random.random() * -4.0 + 2.0

    dist = np.sqrt(hand_pos.dot(ball_pos))

    while dist < 2.0:
        ball_pos[0] = random.random() * -5.0
        ball_pos[1] = random.random() * -2.0 - 4.0
        ball_pos[2] = random.random() * -4.0 + 2.0
        dist = np.sqrt(hand_pos.dot(ball_pos))

    box_pos[0] = random.random() * -5.0
    box_pos[1] = random.random() * -2.0 - 4.0
    box_pos[2] = random.random() * -4.0 + 2.0

    dist = np.sqrt(ball_pos.dot(box_pos))

    while dist < 3.0:
        box_pos[0] = random.random() * -5.0
        box_pos[1] = random.random() * -2.0 - 4.0
        box_pos[2] = random.random() * -4.0 + 2.0
        dist = np.sqrt(ball_pos.dot(box_pos))

    return ball_pos, box_pos