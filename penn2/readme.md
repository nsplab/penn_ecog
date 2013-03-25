

Modules
=======

The following diagram shows the main modules and the connection between them.

                               +-----------+
                               |           |
                               |Supervisor +---+
                               |           |   |
     +--------------------+    +-----^-----+   |
     |                    |          |         |
     | Signal Acquisition |          |         |
     |                    |    +-----v----+    |
     +--------+-----------+    |          |    |
              |                |  Filter  |    |
    +---------v----------+     |          |    |
    |                    |     +-----^----+    |
    | Feature Extraction |           |         |
    |                    +-----------+         |
    +--------------------+      +----------+   |
                                |          |   |
      +--------+ +--------+     | Graphics <---+
      |        | |        |     |          |
      | Logger | | Config |     +----------+
      |        | |        |
      +--------+ +--------+

Note
----
In the 

1. Signal Acquisition
---------------------

This modules interfaces the hardware. After grabbing the signal, it publishes the data using ZMQ at "ipc:///tmp/signal.pipe". The format of the messages is comma separated values (CSV), i.e., the value read from the first channel is followed by a comma, then the value of the second channel and so on.

2. Feature Extraction
---------------------

The Feature Extraction module receives the output of the Signal Acquisition module from "ipc:///tmp/signal.pipe", parses and processes the data, and publishes the extracted features in the CSV format at "ipc:///tmp/features.pipe" using ZMQ.

3. Filter
---------------------

After reading the features (in the CSV format) from "ipc:///tmp/features.pipe", this module publishes the predicted arm position at "ipc:///tmp/hand_position.pipe" (in the CSV format). The Supervisor module controls this module by publishing messages at "ipc:///tmp/status.pipe". 

4. Supervisor
---------------------


5. Graphics
---------------------
