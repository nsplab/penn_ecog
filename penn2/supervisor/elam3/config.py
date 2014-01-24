
# workspace dimensionality
dimension = 1

# when the kinect is used to control the hand position
# true: use kinect, get the position of the hand / don't integrate velocity
# false: don't use kinect, assume the velocity is sent to the supervisor from the filter,
# integrate to get the position of the hand
trackingMode = False

# the length of the workspase in each dimension
workspaceRadius = 4.0

# the width of the blocks
blockWidth = workspaceRadius * 0.325

# whether run the game in the continious mode: False
# or at the rest blocks jump the hand to the origin, assuming we cannot have negative velocity: True
jumpetoStart = True


# threshold on the distance between the hand and cube to increase score
scoreDistanceThreshold = blockWidth

# amount of score to be added per frame/iteration when the hand is close enough to the target block
scoreIncrement = 1.0

# length of the rest block in seconds
blockLengthTime = 10.0

# length of the non-resting block in seconds
restLengthTime = 10.0