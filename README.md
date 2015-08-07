# Bridge between the Khepera III robot and Robot OS (ROS)

Khepera III robots are nice small robots from K-Team. They contain many sensors (10 IR proximity sensors, 5 ultrasonic range finders, encoders at their motors, and extension boards exist for advanced features) and, with the addition of a Korebot card, can run a complete embedded Linux operating system, including networking over Wifi.

This project contains two pieces of code that allow to command the Khepera III robot using ROS topics. The C code in "server/" has to be cross-compiled for the Korebot board (instructions in the Korebot manual, basically you have to install stuff on your computer, modify "server/env.sh", source it and type "make" in "server/"). The server is then uploaded to the robot and executed (provided that the robot is connected to a network, instructions depend on your networking environment but an ad-hoc Wifi network created by the robot works great).

The C++ code in the main directory of this repository is meant to be compiled on the host computer. It produces an executable that is invoked by "./roskhepera rosnode_name ip_of_the_robot" (this requires that the IP address of the robot is known and that it is connected on the same network as the host computer). The executable connects to a ROS server and exposes the sensors and motors of the robot as ROS topics (under the given ROS node name). You can use rqt (a GUI that allows to browse ROS topics) or the "rostopic" utility to set the torque of the motors and get readings of all the sensors on the robot.

Note: the sensor readings are updated only when as value is published on one of the torque topics. If you want to regularly get sensor readings, publish something on leftTorque or rightTorque at the desired frequency.

Have fun!
