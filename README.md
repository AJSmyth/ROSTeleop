
# ROSTeleop
## Motivation
This project helped me to familiarize myself with ROS 2 Foxy and Micro-ROS. Although the STM32 Nucleo board should be able to run an RTOS compatible with Micro-ROS, the baremetal Micro-ROS library seemed much simpler to build and use.

## Demo
[Here](https://youtu.be/QsulMWzZdlU) is a demonstration with the STM32 Nucleo. The system in mind for this application is a turret with pitch and yaw servos, as well as two dc motors for accelerating projectiles. 

## Code Summary
On the Micro-ROS side there is a single Arduino program which subscribes to a topic and refreshes the position of the servos on a 10 Hz timer callback. While the servos could simply be moved whenever new data is received, this leads to jumpier looking servo movement.

The ROS 2 side has two packages. One package, teleop_interfaces, is for the custom interface, which includes a pitch and yaw value between 0 and 180 degrees, as well as a boolean for motor state. The second package has two executables. The first C++ file, teleop_servo.cpp, was the first attempt at controlling a single servo using the standard messages. The second, teleop_servo_2.cpp, is the most recent version. This code implements the custom interface from the teleop_interfaces package, and only allows the collection and publishing of movement data every 200ms. This value is somewhat arbitrary, but it visually reduced the jittery look of the servo motor. This also helped to prevent the buildup of movement messages, which makes the movement feel more responsive. 

Note: the Arduino Micro-ROS code dealing with the initialization of the ROS subscriber node (excepting the inclusion of a custom message) is taken from [this example](https://github.com/micro-ROS/micro_ros_arduino/tree/humble/examples/micro-ros_subscriber) from the [Arduino Micro-ROS project](https://github.com/micro-ROS/micro_ros_arduino).

## Build Instructions
