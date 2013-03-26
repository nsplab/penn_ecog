The Filter Module
=================

Filter interacts with two other modules, Supervisor and FeatureExtractor. This readme explains these connections. 

The benefits of this design:
----------------------------
* the modules run asynchronously
* while Filter still knows the time features were extracted
* and supervisor does not start a new trial without the confirmation of Filter


Supervisor-Filter Interaction
=============================

In the following scenario the Supervisor module is started first, in case Filter is started first, Filter simple waits for a message from Supervisor and then from Step 2 the process starts.

![Supervisor-Filter Interaction](../docs/filter_supervisor.png)

