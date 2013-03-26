

Modules
=======

The following diagram shows the main modules and the connections between them.

![system Modules](./docs/modules.png)


1. Signal Acquisition
---------------------

This modules interfaces the hardware. After grabbing the signal, it publishes the data using ZMQ at "ipc:///tmp/signal.pipe". The format of the messages is comma separated values (CSV), i.e., the value read from the first channel is followed by a comma, then the value of the second channel and so on.

2. Feature Extraction
---------------------

The Feature Extraction module receives the output of the Signal Acquisition module from "ipc:///tmp/signal.pipe", parses and processes the data, and publishes the extracted features in the CSV format at "ipc:///tmp/features.pipe" using ZMQ. The first column is the timestamp in milliseconds, the seconds column is the number of features, and remaining columns are the features. This structure speeds up the processes that read the data from this pipe.


3. Supervisor
---------------------

This modules decides when a new trial starts and whether it is a training or test trial. Each trial has a unique ID. Supervisor publishes the trial ID, goal position, hand position, whether it is a training or test trial, and whether the trial has started at "ipc:///tmp/status.pipe". Supervisor at "ipc:///tmp/graphics.pipe" publishes the positions of the hand, the ball, and the target box, and the score/level.

3. Filter
---------------------

After reading the features (in the CSV format) from "ipc:///tmp/features.pipe" and the trial ID from Supervisor, this module publishes the predicted arm movement in the three dimensions at "ipc:///tmp/hand_position.pipe" (in the CSV format). Supervisor knows whether Filter is lagging behind by checking the trial ID published by Filter. 


5. Graphics
---------------------

The Graphics module receives the state of the game from "ipc:///tmp/graphics.pipe" published by Supervisor and updates the virtual environment.


