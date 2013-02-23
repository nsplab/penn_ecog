

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


1. Signal Acquisition
---------------------

This modules interfaces the hardware. After grabbing the signal, it publishes the data using ZMQ at "ips:///tmp/signal.pipe". The format of the messages is comma separated values, i.e., the value read from the first channel is followed by a comma, then the value of the second channel and so on.

2. Feature Extraction
---------------------



3. Trainer
---------------------

4. Filter
---------------------

5. Graphics
---------------------
