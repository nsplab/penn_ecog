
elam3 - refers to the graphics module, which implements rendering of the 3d virtual environment, robotic arm, and inverse kinematics related to the arm. this module receives requests from the supervisor (written in python code, see supervisor/elam3/supervisor.py) via ZMQ for coordinating the game and specifying hand position (which had be sent to the supervisor from the filter module)

this folder contains:

main.cpp - performs all virtual environment graphics, communicates with supervisor module, controls arm position, calls the inversekinematics.cpp function called SolveArmInvKinematics

inversekinematics.cpp - provides the actual implementation/definition of the inverse kinematics solver function.
inversekinematics.h - declaration of inverse kinematics solver function, included in inversekinematics.cpp and main.cpp

utils.cpp - graphics-related functions and classes
utils.h - function declarations for utils.cpp

fore_arm.3ds - .3ds model for forearm, downloaded from blendswap.com and edited in blender to remove other body components.
upper_arm.3ds - .3ds model for upper arm.
smoke.rgb - contains image used for the smoke effect when the wrist intersects the bar in this game.
Ubuntu-B.ttf - font used for text on the screen in the game

CMakeLists.txt - this contains specific flags for compiling using g++ version 4.7.3 (released 2013); note that Ubuntu is known for having old versions of compiler
build -	subdirectory where the object files are built and the executable is located.
CMakeLists.txt.user - you will not see this file, because it's not in the github repository. generated by cmake, this is machine specific code.
iksurvey.pdf - brief survey explaining various inverse kinematics approaches. our implementation here follows the Jacobian approach using a cross-product as defined in the text box after equation 5.
readme.md -this current file






